#include "fd_sysvar_rent.h"
#include "../fd_acc_mgr.h"
#include "../fd_system_ids.h"
#include <assert.h>

/* https://github.com/solana-labs/solana/blob/8f2c8b8388a495d2728909e30460aa40dcc5d733/sdk/program/src/rent.rs#L36 */
#define ACCOUNT_STORAGE_OVERHEAD (128)

fd_rent_t *
fd_sysvar_rent_read( fd_rent_t *          result,
                     fd_exec_slot_ctx_t * slot_ctx ) {

  FD_BORROWED_ACCOUNT_DECL(rent_rec);

  int err = fd_acc_mgr_view( slot_ctx->acc_mgr, slot_ctx->funk_txn, &fd_sysvar_rent_id, rent_rec );
  if( FD_UNLIKELY( err != FD_ACC_MGR_SUCCESS ) ) {
    FD_LOG_WARNING(( "failed to read rent sysvar: %d", err ));
    return NULL;
  }

  fd_bincode_decode_ctx_t decode = {
    .data    = rent_rec->const_data,
    .dataend = rent_rec->const_data + rent_rec->const_meta->dlen,
    .valloc  = slot_ctx->valloc
  };
  err = fd_rent_decode( result, &decode );
  if( FD_UNLIKELY( err ) ) {
    FD_LOG_WARNING(( "fd_rent_decode failed" ));
    return NULL;
  }

  return result;
}

ulong
fd_rent_exempt_minimum_balance2( fd_rent_t const * rent,
                                 ulong             data_len ) {
  /* https://github.com/solana-labs/solana/blob/792fafe0c25ac06868e3ac80a2b13f1a5b4a1ef8/sdk/program/src/rent.rs#L72 */
  return (ulong)( (double)((data_len + ACCOUNT_STORAGE_OVERHEAD) * rent->lamports_per_uint8_year) * (double)rent->exemption_threshold );
}

ulong
fd_rent_exempt_minimum_balance( fd_exec_slot_ctx_t * slot_ctx,
                                ulong                data_len ) {
  /* TODO wire up with sysvar cache */
  fd_rent_t rent;
  fd_rent_new( &rent );
  fd_rent_t * result = fd_sysvar_rent_read( &rent, slot_ctx );
  assert( result );
  return fd_rent_exempt_minimum_balance2( &rent, data_len );
}
