#include "fd_bpf_loader_v4_program.h"
#include "../fd_runtime.h"
#include "../fd_executor.h"
#include "../fd_acc_mgr.h"
#include "../fd_system_ids.h"
#include "../sysvar/fd_sysvar_clock.h"
#include "../sysvar/fd_sysvar_rent.h"
#include "../../types/fd_types.h"
#include "../../../util/bits/fd_sat.h"

#define DEPLOYMENT_COOLDOWN_IN_SLOTS (750)

/* TODO Links are no longer valid, needs update */

/* Methods for processing specific BPF Loader v4 instructions */

static int _process_write             ( fd_exec_instr_ctx_t, fd_bpf_loader_v4_program_instruction_write_t const * );
static int _process_truncate          ( fd_exec_instr_ctx_t, uint );
static int _process_deploy            ( fd_exec_instr_ctx_t );
static int _process_retract           ( fd_exec_instr_ctx_t );
static int _process_transfer_authority( fd_exec_instr_ctx_t );

/* check_program_account runs a sequence of checks on an account owned
   by the BPF Loader v4 program.  Used by most instruction handlers.

   List of checks:
   - Account owner is BPF Loader v4
   - Account data is not zero-length
   - State header at start of account data within bounds
   - Program is writable
   - First instruction account (presumed authority) has signed operation
   - Program is not finalized (i.e. authority in state header is not NULL)
   - First instruction account matches the authority in the state header

   Returns an executor instruction error or 0 on success.
   On success, ensures that a pointer to the account's first byte can
   be safely casted to a pointer to fd_bpf_loader_v4_state_t.

   Linearly matches Solana Labs:
   https://github.com/solana-labs/solana/blob/d90e1582869d8ef8d386a1c156eda987404c43be/programs/loader-v4/src/lib.rs#L203 */

static int
check_program_account( fd_exec_instr_ctx_t       ctx,
                       fd_account_meta_t const * program_meta ) {

  /* Unpack arguments */

  fd_borrowed_account_t * const * instr_accs = ctx.instr->borrowed_accounts;

  ulong         instr_acc_cnt = ctx.instr->acct_cnt;
  uchar const * program_data  = (uchar const *)program_meta + program_meta->hlen;

  /* Assume instruction account index 1 to be authority */

  FD_TEST( instr_acc_cnt >= 2 );
  fd_pubkey_t const * authority = instr_accs[1]->pubkey;

  /* https://github.com/solana-labs/solana/blob/d90e1582869d8ef8d386a1c156eda987404c43be/programs/loader-v4/src/lib.rs#L209 */
  if( FD_UNLIKELY( 0!=memcmp( program_meta->info.owner, fd_solana_bpf_loader_v4_program_id.key, sizeof(fd_pubkey_t) ) ) ) {
    // TODO Log: "Program not owner by loader"
    return FD_EXECUTOR_INSTR_ERR_INVALID_ACC_OWNER;
  }

  /* https://github.com/solana-labs/solana/blob/d90e1582869d8ef8d386a1c156eda987404c43be/programs/loader-v4/src/lib.rs#L213 */
  if( FD_UNLIKELY( program_meta->dlen == 0UL ) ) {
    // TODO Log: "Program is uninitialized"
    return FD_EXECUTOR_INSTR_ERR_INVALID_ACC_DATA;
  }

  /* https://github.com/solana-labs/solana/blob/d90e1582869d8ef8d386a1c156eda987404c43be/programs/loader-v4/src/lib.rs#L217 */
  fd_bpf_loader_v4_state_t const * state =
    fd_bpf_loader_v4_get_state_const( program_meta, program_data );
  if( FD_UNLIKELY( state==NULL ) )
    return FD_EXECUTOR_INSTR_ERR_ACC_DATA_TOO_SMALL;

  /* https://github.com/solana-labs/solana/blob/d90e1582869d8ef8d386a1c156eda987404c43be/programs/loader-v4/src/lib.rs#L218 */
  if( FD_UNLIKELY( !fd_instr_acc_is_writable_idx( ctx.instr, 0 ) ) ) {
    // TODO Log: "Program account is not writeable"
    return FD_EXECUTOR_INSTR_ERR_INVALID_ARG;
  }

  /* https://github.com/solana-labs/solana/blob/d90e1582869d8ef8d386a1c156eda987404c43be/programs/loader-v4/src/lib.rs#L222 */
  if( FD_UNLIKELY( !fd_instr_acc_is_signer_idx( ctx.instr, 1 ) ) ) {
    // TODO Log: "Authority did not sign"
    return FD_EXECUTOR_INSTR_ERR_MISSING_REQUIRED_SIGNATURE;
  }

  if( FD_UNLIKELY( 0!=memcmp( state->authority_addr, authority->key, sizeof(fd_pubkey_t) ) ) ) {
    // TODO Log: "Incorrect authority provided"
    return FD_EXECUTOR_INSTR_ERR_INCORRECT_AUTHORITY;
  }

  if( FD_UNLIKELY( state->status == FD_BPF_LOADER_V4_STATUS_FINALIZED ) ) {
    // TODO Log: "Program is finalized"
    return FD_EXECUTOR_INSTR_ERR_ACC_IMMUTABLE;
  }

  return 0;
}

/* _process_meta_instruction handles a direct invocation of the
   BPF Loader v4 program (i.e. instruction's program ID matches) */

static int
_process_meta_instruction( fd_exec_instr_ctx_t ctx ) {

  /* TODO: Consume DEFAULT_COMPUTE_UNITS upfront if feature_set::native_programs_consume_cu is active
     https://github.com/solana-labs/solana/blob/d90e1582869d8ef8d386a1c156eda987404c43be/programs/loader-v4/src/lib.rs#L577 */

  /* Scratch frame -- Deallocated when instruction processor exits */

  FD_SCRATCH_SCOPE_BEGIN {

    /* Deserialize instruction */

    uchar const * instr_data = ctx.instr->data;
    fd_bincode_decode_ctx_t instr_decode = {
      .data    = instr_data,
      .dataend = instr_data + ctx.instr->data_sz,
      .valloc  = fd_scratch_virtual()
    };

    fd_bpf_loader_v4_program_instruction_t instr[1];
    int err = fd_bpf_loader_v4_program_instruction_decode( instr, &instr_decode );
    if( FD_UNLIKELY( err!=FD_BINCODE_SUCCESS ) ) return FD_EXECUTOR_INSTR_ERR_INVALID_INSTR_DATA;

    /* Handle instruction */

    switch( instr->discriminant ) {
    case fd_bpf_loader_v4_program_instruction_enum_write:
      return _process_write( ctx, &instr->inner.write );
    case fd_bpf_loader_v4_program_instruction_enum_truncate:
      return _process_truncate( ctx, instr->inner.truncate );
    case fd_bpf_loader_v4_program_instruction_enum_deploy:
      return _process_deploy( ctx );
    case fd_bpf_loader_v4_program_instruction_enum_retract:
      return _process_retract( ctx );
    case fd_bpf_loader_v4_program_instruction_enum_transfer_authority:
      return _process_transfer_authority( ctx );
    default:
      __builtin_unreachable();
      FD_LOG_CRIT(( "entered unreachable code" ));
    }
  } FD_SCRATCH_SCOPE_END;
}

int
fd_bpf_loader_v4_program_execute( fd_exec_instr_ctx_t ctx ) {

  //if( !FD_FEATURE_ACTIVE( ctx.slot_ctx, enable_program_runtime_v2_and_loader_v4 ) ) {
  //  return FD_EXECUTOR_INSTR_ERR_UNSUPPORTED_PROGRAM_ID;
  //}

  /* Query program ID */
  fd_pubkey_t const * program_id = &ctx.instr->program_id_pubkey;

  if( 0==memcmp( program_id, fd_solana_bpf_loader_v4_program_id.key, sizeof(fd_pubkey_t) ) ) {
    return _process_meta_instruction( ctx );
  } else {
    FD_LOG_WARNING(( "BPF loader v4 program execution not yet supported" ));
    return FD_EXECUTOR_INSTR_ERR_UNSUPPORTED_PROGRAM_ID;
  }
}

static int
_process_write( fd_exec_instr_ctx_t                                  ctx,
                fd_bpf_loader_v4_program_instruction_write_t const * write ) {

  /* Context */

  fd_borrowed_account_t * const * instr_accs = ctx.instr->borrowed_accounts;

  ulong           instr_acc_cnt = ctx.instr->acct_cnt;
  fd_acc_mgr_t *  acc_mgr       = ctx.acc_mgr;
  fd_funk_txn_t * funk_txn      = ctx.funk_txn;

  /* Unpack accounts

     https://github.com/solana-labs/solana/blob/d90e1582869d8ef8d386a1c156eda987404c43be/programs/loader-v4/src/lib.rs#L245-L251 */

  if( FD_UNLIKELY( instr_acc_cnt < 2 ) )
    return FD_EXECUTOR_INSTR_ERR_NOT_ENOUGH_ACC_KEYS;

  fd_pubkey_t const * program_id = instr_accs[0]->pubkey;

  /* Read program account */
  FD_BORROWED_ACCOUNT_DECL(program_rec);
  int err = fd_acc_mgr_view( acc_mgr, funk_txn, program_id, program_rec);
  if( FD_UNLIKELY( err!=FD_ACC_MGR_SUCCESS ) ) return err;

  /* Check program account
     https://github.com/solana-labs/solana/blob/c0fbfc6422fa5b739049c01bfda48a0da1bf6a46/programs/loader-v4/src/lib.rs#L238 */
  err = check_program_account( ctx, program_rec->const_meta );
  if( FD_UNLIKELY( err!=0 ) ) return err;
  fd_bpf_loader_v4_state_t const * state = (fd_bpf_loader_v4_state_t const *)
    fd_type_pun_const( program_rec->const_data );

  /* https://github.com/solana-labs/solana/blob/c0fbfc6422fa5b739049c01bfda48a0da1bf6a46/programs/loader-v4/src/lib.rs#L244 */
  if( FD_UNLIKELY( state->status != FD_BPF_LOADER_V4_STATUS_RETRACTED ) ) {
    // TODO Log: "Program is not retracted"
    return FD_EXECUTOR_INSTR_ERR_INVALID_ARG;
  }

  /* Unpack instruction arguments */
  uchar const * bytes      = write->bytes;
  ulong const   bytes_len  = write->bytes_len;
  ulong const   offset     = write->offset;
  ulong const   end_offset = offset + write->bytes_len;

  /* Upgrade program to writable handle */
  err = fd_acc_mgr_modify( acc_mgr, funk_txn, program_id, 0, 0UL, program_rec);
  if( FD_UNLIKELY( err!=FD_ACC_MGR_SUCCESS ) ) return err;

  /* https://github.com/solana-labs/solana/blob/c0fbfc6422fa5b739049c01bfda48a0da1bf6a46/programs/loader-v4/src/lib.rs#L251 */
  ulong program_data_sz = program_rec->const_meta->dlen - sizeof(fd_bpf_loader_v4_state_t);
  if( FD_UNLIKELY( ( offset     > program_data_sz )
                 | ( end_offset > program_data_sz ) ) ) {
    /* TODO Log: "Write out of bounds" */
    return FD_EXECUTOR_INSTR_ERR_ACC_DATA_TOO_SMALL;
  }

  ulong const write_off     = sizeof(fd_bpf_loader_v4_state_t) + offset;
  ulong const write_off_end = write_off + bytes_len;
  FD_TEST( write_off_end <= program_rec->const_meta->dlen );

  uchar * write_ptr = program_rec->data + write_off;
  fd_memcpy( write_ptr, bytes, bytes_len );

  return 0;
}

static int
_process_truncate( fd_exec_instr_ctx_t ctx,
                   uint                new_sz ) {

  fd_borrowed_account_t * const * instr_accs = ctx.instr->borrowed_accounts;

  if( FD_UNLIKELY( ctx.instr->acct_cnt < 2 ) )
    return FD_EXECUTOR_INSTR_ERR_NOT_ENOUGH_ACC_KEYS;

  fd_pubkey_t const * authority_addr = instr_accs[1]->pubkey;

  fd_borrowed_account_t * program = NULL;
  int err = fd_instr_borrowed_account_view_idx( &ctx, 0, &program );
  if( FD_UNLIKELY( err ) ) return FD_EXECUTOR_INSTR_ERR_GENERIC_ERR;  /* TODO */

  /* https://github.com/solana-labs/solana/blob/fb80288f885a62bcd923f4c9579fd0edeafaff9b/programs/loader-v4/src/lib.rs#L274-L275 */
  int is_init = (new_sz>0U) & (program->const_meta->dlen < sizeof(fd_bpf_loader_v4_state_t));
  if( is_init ) {
    /* https://github.com/solana-labs/solana/blob/fb80288f885a62bcd923f4c9579fd0edeafaff9b/programs/loader-v4/src/lib.rs#L277 */
    if( FD_UNLIKELY( 0!=memcmp( program->const_meta->info.owner, fd_solana_bpf_loader_v4_program_id.key, sizeof(fd_pubkey_t) ) ) ) {
      /* TODO Log: "Program not owned by loader" */
      return FD_EXECUTOR_INSTR_ERR_INVALID_ACC_OWNER;
    }
    /* https://github.com/solana-labs/solana/blob/fb80288f885a62bcd923f4c9579fd0edeafaff9b/programs/loader-v4/src/lib.rs#L281 */
    if( FD_UNLIKELY( !fd_instr_acc_is_writable_idx( ctx.instr, 0 ) ) ) {
      /* TODO Log: "Program is not writeable" */
      return FD_EXECUTOR_INSTR_ERR_INVALID_ARG;
    }
    /* https://github.com/solana-labs/solana/blob/fb80288f885a62bcd923f4c9579fd0edeafaff9b/programs/loader-v4/src/lib.rs#L285 */
    if( FD_UNLIKELY( !fd_instr_acc_is_signer_idx( ctx.instr, 0 ) ) ) {
      /* TODO Log: "Program did not sign" */
      return FD_EXECUTOR_INSTR_ERR_MISSING_REQUIRED_SIGNATURE;
    }
    /* https://github.com/solana-labs/solana/blob/fb80288f885a62bcd923f4c9579fd0edeafaff9b/programs/loader-v4/src/lib.rs#L289 */
    if( FD_UNLIKELY( !fd_instr_acc_is_signer_idx( ctx.instr, 1 ) ) ) {
      /* TODO Log: "Authority did not sign" */
      return FD_EXECUTOR_INSTR_ERR_MISSING_REQUIRED_SIGNATURE;
    }
  } else {
    /* Check program account
      https://github.com/solana-labs/solana/blob/fb80288f885a62bcd923f4c9579fd0edeafaff9b/programs/loader-v4/src/lib.rs#L294 */
    err = check_program_account( ctx, program->const_meta );
    if( FD_UNLIKELY( err!=0 ) ) return err;

    fd_bpf_loader_v4_state_t const * state =
      (fd_bpf_loader_v4_state_t const *)fd_type_pun_const( program->const_data );

    /* https://github.com/solana-labs/solana/blob/fb80288f885a62bcd923f4c9579fd0edeafaff9b/programs/loader-v4/src/lib.rs#L300 */
    if( FD_UNLIKELY( state->status != FD_BPF_LOADER_V4_STATUS_RETRACTED ) ) {
      /* TODO Log: "Program is not retracted" */
      return FD_EXECUTOR_INSTR_ERR_INVALID_ARG;
    }
  }

  /* https://github.com/solana-labs/solana/blob/fb80288f885a62bcd923f4c9579fd0edeafaff9b/programs/loader-v4/src/lib.rs#L305-L310 */
  ulong required_lamports;
  if( new_sz==0U ) {
    required_lamports = 0UL;
  } else {
    ulong raw_new_sz  = fd_ulong_sat_add( sizeof(fd_bpf_loader_v4_state_t), new_sz );
    required_lamports = fd_rent_exempt_minimum_balance( ctx.slot_ctx, raw_new_sz );
  }

  if( program->const_meta->info.lamports < required_lamports ) {
    /* https://github.com/solana-labs/solana/blob/fb80288f885a62bcd923f4c9579fd0edeafaff9b/programs/loader-v4/src/lib.rs#L313-L318 */
    /* TODO Log: "Insufficient lamports, {} are required */
    return FD_EXECUTOR_INSTR_ERR_INSUFFICIENT_FUNDS;
  } else if( program->const_meta->info.lamports > required_lamports ) {
    /* https://github.com/solana-labs/solana/blob/fb80288f885a62bcd923f4c9579fd0edeafaff9b/programs/loader-v4/src/lib.rs#L321-L326 */
    if( FD_UNLIKELY( ctx.instr->acct_cnt < 3 ) )
      return FD_EXECUTOR_INSTR_ERR_NOT_ENOUGH_ACC_KEYS;
    if( FD_UNLIKELY( !fd_instr_acc_is_writable_idx( ctx.instr, 2 ) ) )
      return FD_EXECUTOR_INSTR_ERR_INVALID_ARG;
    fd_borrowed_account_t * recipient = NULL;
    err = fd_instr_borrowed_account_modify_idx( &ctx, 2, 0UL, &recipient );
    if( FD_UNLIKELY( err ) ) {
      /* TODO Log: "Recipient is not writeable" */
      return FD_EXECUTOR_INSTR_ERR_INVALID_ARG;
    }
    /* https://github.com/solana-labs/solana/blob/fb80288f885a62bcd923f4c9579fd0edeafaff9b/programs/loader-v4/src/lib.rs#L327-L329 */
    err = fd_instr_borrowed_account_modify_idx( &ctx, 0, 0UL, &program );
    if( FD_UNLIKELY( err ) ) {
      /* TODO what error code to return here? */
      return FD_EXECUTOR_INSTR_ERR_INVALID_ARG;
    }
    ulong lamports_to_receive = program->meta->info.lamports - required_lamports;
    program  ->meta->info.lamports -= lamports_to_receive;
    recipient->meta->info.lamports += lamports_to_receive;
  }

  ulong raw_new_sz;
  if( new_sz==0U ) {
    /* https://github.com/solana-labs/solana/blob/fb80288f885a62bcd923f4c9579fd0edeafaff9b/programs/loader-v4/src/lib.rs#L334 */
    raw_new_sz = 0UL;
  } else {
    /* https://github.com/solana-labs/solana/blob/fb80288f885a62bcd923f4c9579fd0edeafaff9b/programs/loader-v4/src/lib.rs#L336 */
    raw_new_sz = sizeof(fd_bpf_loader_v4_state_t) + new_sz;
  }

  /* Gather writable handle to program if not done yet */
  if( !program->rec ) {
    err = fd_instr_borrowed_account_modify_idx( &ctx, 0, raw_new_sz, &program );
    if( FD_UNLIKELY( err ) ) {
      /* TODO what error code to return here? */
      return FD_EXECUTOR_INSTR_ERR_INVALID_ARG;
    }
  }

  if( raw_new_sz > program->meta->dlen )
    fd_memset( program->data + program->meta->dlen, 0, raw_new_sz - program->meta->dlen );
  program->meta->dlen = raw_new_sz;

  if( new_sz!=0U ) {
    /* https://github.com/solana-labs/solana/blob/fb80288f885a62bcd923f4c9579fd0edeafaff9b/programs/loader-v4/src/lib.rs#L339-L344 */
    if( is_init ) {
      fd_bpf_loader_v4_state_t * state =
        (fd_bpf_loader_v4_state_t *)fd_type_pun( program->data );
      state->slot   = 0UL;
      state->status = FD_BPF_LOADER_V4_STATUS_RETRACTED;
      fd_memcpy( state->authority_addr, authority_addr, sizeof(fd_pubkey_t) );
    }
  }

  return 0;
}

static int
_process_deploy( fd_exec_instr_ctx_t ctx ) {

  /* Accounts */

  fd_borrowed_account_t * const * instr_accs = ctx.instr->borrowed_accounts;

  if( FD_UNLIKELY( ctx.instr->acct_cnt < 2 ) )
    return FD_EXECUTOR_INSTR_ERR_NOT_ENOUGH_ACC_KEYS;

  fd_pubkey_t const * program_acc       = instr_accs[0]->pubkey;
  fd_pubkey_t const * authority_address = instr_accs[1]->pubkey;
  fd_pubkey_t const * source_program    = NULL;

  if( ctx.instr->acct_cnt >= 3 )
    source_program = instr_accs[2]->pubkey;

  // Load program account
  FD_BORROWED_ACCOUNT_DECL(program_acc_rec);
  int err = fd_acc_mgr_modify(ctx.acc_mgr, ctx.funk_txn, program_acc, 1, 0, program_acc_rec);
  if( FD_UNLIKELY( err ) ) return err;
  err = check_program_account(ctx, program_acc_rec->meta);
  if( FD_UNLIKELY( err ) ) return err;

  fd_bpf_loader_v4_state_t const * state = (fd_bpf_loader_v4_state_t const *)
      fd_type_pun_const( program_acc_rec->const_data );

  fd_sol_sysvar_clock_t clock;
  fd_sysvar_clock_read(ctx.slot_ctx, &clock);
  ulong current_slot = clock.slot;

  if( fd_ulong_sat_add(state->slot, DEPLOYMENT_COOLDOWN_IN_SLOTS) > current_slot ) {
    /* TODO Log: "Program was deployed recently, cooldown still in effect" */
    return FD_EXECUTOR_INSTR_ERR_INVALID_ARG;
  }

  if( state->status != FD_BPF_LOADER_V4_STATUS_RETRACTED ) {
    /* TODO Log: "Destination program is not retracted" */
    return FD_EXECUTOR_INSTR_ERR_INVALID_ARG;
  }

  fd_account_meta_t * buffer_metadata = NULL;
  uchar * buffer_data = NULL;

  FD_BORROWED_ACCOUNT_DECL( _source_program_rec );
  fd_borrowed_account_t *source_program_rec = NULL;
  if( source_program ) {
    source_program_rec = _source_program_rec;
    err = fd_acc_mgr_modify(ctx.acc_mgr, ctx.funk_txn, source_program, 1, 0, source_program_rec);
    if( FD_UNLIKELY( err!=FD_EXECUTOR_INSTR_SUCCESS ) ) return err;

    err = check_program_account(ctx, source_program_rec->const_meta);
    if( FD_UNLIKELY( err!=FD_EXECUTOR_INSTR_SUCCESS ) ) return err;

    fd_bpf_loader_v4_state_t const * source_state = (fd_bpf_loader_v4_state_t const *)
      fd_type_pun_const( source_program_rec->const_data);

    if( FD_UNLIKELY( source_state->status != FD_BPF_LOADER_V4_STATUS_RETRACTED ) ) {
      /* TODO Log: "Source program is not retracted" */
      return FD_EXECUTOR_INSTR_ERR_INVALID_ARG;
    }
    buffer_metadata = source_program_rec->meta;
    buffer_data = source_program_rec->data;
  } else {
    buffer_metadata = program_acc_rec->meta;
    buffer_data = program_acc_rec->data;
  }

  FD_LOG_WARNING(("TODO: load program"));

  if( source_program ) {
    ulong required_lamports = fd_rent_exempt_minimum_balance(ctx.slot_ctx, program_acc_rec->meta->dlen);
    ulong transfer_lamports = fd_ulong_sat_sub(program_acc_rec->meta->info.lamports, required_lamports);

    program_acc_rec->meta->dlen = source_program_rec->meta->dlen;
    fd_memcpy(program_acc_rec->data, source_program_rec->data, source_program_rec->meta->dlen);
    source_program_rec->meta->dlen = 0;
    source_program_rec->meta->info.lamports -= transfer_lamports;
    program_acc_rec->meta->info.lamports += transfer_lamports;
  }

  fd_bpf_loader_v4_state_t * mut_state = (fd_bpf_loader_v4_state_t *) program_acc_rec->data;
  mut_state->slot   = current_slot;
  mut_state->status = FD_BPF_LOADER_V4_STATUS_DEPLOYED;

  (void) buffer_data;
  (void) buffer_metadata;
  (void) authority_address;
  return FD_EXECUTOR_INSTR_SUCCESS;
}

static int
_process_retract( fd_exec_instr_ctx_t ctx ) {

  /* Context */

  fd_borrowed_account_t * const * instr_accs = ctx.instr->borrowed_accounts;

  fd_acc_mgr_t *  acc_mgr  = ctx.acc_mgr;
  fd_funk_txn_t * funk_txn = ctx.funk_txn;

  /* Unpack accounts

     https://github.com/solana-labs/solana/blob/d90e1582869d8ef8d386a1c156eda987404c43be/programs/loader-v4/src/lib.rs#L494 */

  if( FD_UNLIKELY( ctx.instr->acct_cnt < 2 ) )
    return FD_EXECUTOR_INSTR_ERR_NOT_ENOUGH_ACC_KEYS;

  fd_pubkey_t const * program_id = instr_accs[0]->pubkey;

  /* Read program account */
  FD_BORROWED_ACCOUNT_DECL(program_rec);

  int err = fd_acc_mgr_view( acc_mgr, funk_txn, program_id, program_rec);
  if( FD_UNLIKELY( err!=FD_ACC_MGR_SUCCESS ) ) return err;

  /* Check program account
     https://github.com/solana-labs/solana/blob/d90e1582869d8ef8d386a1c156eda987404c43be/programs/loader-v4/src/lib.rs#L498 */
  err = check_program_account( ctx, program_rec->const_meta );
  if( FD_UNLIKELY( err!=0 ) ) return err;
  fd_bpf_loader_v4_state_t const * state = (fd_bpf_loader_v4_state_t const *)
    fd_type_pun_const( program_rec->const_data );

  /* Solana Labs reads from the clock sysvar here
     https://github.com/solana-labs/solana/blob/d90e1582869d8ef8d386a1c156eda987404c43be/programs/loader-v4/src/lib.rs#L504 */
  ulong current_slot = ctx.slot_ctx->slot_bank.slot;
  if( state->slot + DEPLOYMENT_COOLDOWN_IN_SLOTS > current_slot ) {
    // TODO Log: "Program was deployed recently, cooldown stil in effect"
    return FD_EXECUTOR_INSTR_ERR_INVALID_ARG;
  }

  /* https://github.com/solana-labs/solana/blob/d90e1582869d8ef8d386a1c156eda987404c43be/programs/loader-v4/src/lib.rs#L512 */
  if( FD_UNLIKELY( state->status == FD_BPF_LOADER_V4_STATUS_RETRACTED ) ) {
    // TODO Log: "Program is not deployed"
    return FD_EXECUTOR_INSTR_ERR_INVALID_ARG;
  }

  /* Upgrade to writable handle
     https://github.com/solana-labs/solana/blob/d90e1582869d8ef8d386a1c156eda987404c43be/programs/loader-v4/src/lib.rs#L516 */
  if( FD_UNLIKELY( !fd_instr_acc_is_writable_idx( ctx.instr, 0 ) ) )
    return FD_EXECUTOR_INSTR_ERR_INVALID_ARG;
  err = fd_acc_mgr_modify( acc_mgr, funk_txn, program_id, /* do_create */ 0, 0UL, program_rec);
  if( FD_UNLIKELY( err!=FD_ACC_MGR_SUCCESS ) ) return err;
  fd_bpf_loader_v4_state_t * state_rw = (fd_bpf_loader_v4_state_t *)fd_type_pun( program_rec->data );

  /* https://github.com/solana-labs/solana/blob/d90e1582869d8ef8d386a1c156eda987404c43be/programs/loader-v4/src/lib.rs#L517 */
  state_rw->status = FD_BPF_LOADER_V4_STATUS_RETRACTED;

  return 0;
}

static int
_process_transfer_authority( fd_exec_instr_ctx_t ctx ) {

  /* Context */

  fd_borrowed_account_t * const * instr_accs = ctx.instr->borrowed_accounts;

  fd_acc_mgr_t *  acc_mgr  = ctx.acc_mgr;
  fd_funk_txn_t * funk_txn = ctx.funk_txn;

  /* Unpack accounts

     https://github.com/solana-labs/solana/blob/d90e1582869d8ef8d386a1c156eda987404c43be/programs/loader-v4/src/lib.rs#L527 */

  if( FD_UNLIKELY( ctx.instr->acct_cnt < 2 ) )
    return FD_EXECUTOR_INSTR_ERR_NOT_ENOUGH_ACC_KEYS;

  fd_pubkey_t const * program_id    = instr_accs[0]->pubkey;
  fd_pubkey_t const * new_authority = NULL;

  if( ctx.instr->acct_cnt >= 3 )
    new_authority = instr_accs[2]->pubkey;

  /* Read program account */
  FD_BORROWED_ACCOUNT_DECL(program_rec);
  int err = fd_acc_mgr_view( acc_mgr, funk_txn, program_id, program_rec );
  if( FD_UNLIKELY( err!=FD_ACC_MGR_SUCCESS ) ) return err;

  /* Check program account
     https://github.com/solana-labs/solana/blob/d90e1582869d8ef8d386a1c156eda987404c43be/programs/loader-v4/src/lib.rs#L536 */
  err = check_program_account( ctx, program_rec->const_meta );
  if( FD_UNLIKELY( err!=0 ) ) return err;

  /* For some reason, third instruction account is checked later */
  if( new_authority ) {
    if( FD_UNLIKELY( !fd_instr_acc_is_signer_idx( ctx.instr, 2 ) ) ) {
      // TODO Log: "New authority did not sign"
      return FD_EXECUTOR_INSTR_ERR_MISSING_REQUIRED_SIGNATURE;
    }
  }

  /* Upgrade to writable handle
     https://github.com/solana-labs/solana/blob/d90e1582869d8ef8d386a1c156eda987404c43be/programs/loader-v4/src/lib.rs#L542 */
  if( FD_UNLIKELY( !fd_instr_acc_is_writable_idx( ctx.instr, 0 ) ) )
    return FD_EXECUTOR_INSTR_ERR_INVALID_ARG;
  err = fd_acc_mgr_modify( acc_mgr, funk_txn, program_id, /* do_create */ 0, 0UL, program_rec );
  if( FD_UNLIKELY( err!=FD_ACC_MGR_SUCCESS ) ) return err;
  fd_bpf_loader_v4_state_t * state_rw = (fd_bpf_loader_v4_state_t *)fd_type_pun( program_rec->data );

  /* https://github.com/solana-labs/solana/blob/d90e1582869d8ef8d386a1c156eda987404c43be/programs/loader-v4/src/lib.rs#L547 */
  if( new_authority ) {
    fd_memcpy( state_rw->authority_addr, new_authority->key, sizeof(fd_pubkey_t) );
  } else if( state_rw->status == FD_BPF_LOADER_V4_STATUS_DEPLOYED ) {
    state_rw->status = FD_BPF_LOADER_V4_STATUS_FINALIZED;
  } else {
    // TODO Log: "Program must be deployed to be finalized"
    return FD_EXECUTOR_INSTR_ERR_INVALID_ARG;
  }
  return 0;
}
