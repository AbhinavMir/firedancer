/* THIS FILE IS GENERATED BY gen_metrics.py. DO NOT HAND EDIT. */

#include "../fd_metrics_base.h"

/* Start of LINK OUT metrics */

#define FD_METRICS_COUNTER_LINK_SLOW_COUNT_OFF  (0UL)
#define FD_METRICS_COUNTER_LINK_SLOW_COUNT_NAME "link_slow_count"
#define FD_METRICS_COUNTER_LINK_SLOW_COUNT_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_LINK_SLOW_COUNT_DESC "The number of times the consumer was detected as rate limiting consumer by the producer."

/* Start of LINK IN metrics */

#define FD_METRICS_COUNTER_LINK_PUBLISHED_COUNT_OFF  (0UL)
#define FD_METRICS_COUNTER_LINK_PUBLISHED_COUNT_NAME "link_published_count"
#define FD_METRICS_COUNTER_LINK_PUBLISHED_COUNT_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_LINK_PUBLISHED_COUNT_DESC "The number of times the link has consumed and published a fragment."

#define FD_METRICS_COUNTER_LINK_PUBLISHED_SIZE_BYTES_OFF  (1UL)
#define FD_METRICS_COUNTER_LINK_PUBLISHED_SIZE_BYTES_NAME "link_published_size_bytes"
#define FD_METRICS_COUNTER_LINK_PUBLISHED_SIZE_BYTES_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_LINK_PUBLISHED_SIZE_BYTES_DESC "The total number of bytes read by the link consumer."

#define FD_METRICS_COUNTER_LINK_FILTERED_COUNT_OFF  (2UL)
#define FD_METRICS_COUNTER_LINK_FILTERED_COUNT_NAME "link_filtered_count"
#define FD_METRICS_COUNTER_LINK_FILTERED_COUNT_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_LINK_FILTERED_COUNT_DESC "The number of consumed fragments that were filtered."

#define FD_METRICS_COUNTER_LINK_FILTERED_SIZE_BYTES_OFF  (3UL)
#define FD_METRICS_COUNTER_LINK_FILTERED_SIZE_BYTES_NAME "link_filtered_size_bytes"
#define FD_METRICS_COUNTER_LINK_FILTERED_SIZE_BYTES_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_LINK_FILTERED_SIZE_BYTES_DESC "The total number of bytes read  by the link consumer that were filtered."

#define FD_METRICS_COUNTER_LINK_OVERRUN_POLLING_COUNT_OFF  (4UL)
#define FD_METRICS_COUNTER_LINK_OVERRUN_POLLING_COUNT_NAME "link_overrun_polling_count"
#define FD_METRICS_COUNTER_LINK_OVERRUN_POLLING_COUNT_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_LINK_OVERRUN_POLLING_COUNT_DESC "The number of times the link has been overrun while polling."

#define FD_METRICS_COUNTER_LINK_OVERRUN_READING_COUNT_OFF  (5UL)
#define FD_METRICS_COUNTER_LINK_OVERRUN_READING_COUNT_NAME "link_overrun_reading_count"
#define FD_METRICS_COUNTER_LINK_OVERRUN_READING_COUNT_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_LINK_OVERRUN_READING_COUNT_DESC "The number of input overruns detected while reading metadata by the consumer."

/* Start of TILE metrics */

#define FD_METRICS_GAUGE_TILE_PID_OFF  (0UL)
#define FD_METRICS_GAUGE_TILE_PID_NAME "tile_pid"
#define FD_METRICS_GAUGE_TILE_PID_TYPE (FD_METRICS_TYPE_GAUGE)
#define FD_METRICS_GAUGE_TILE_PID_DESC "The process ID of the tile."

#define FD_METRICS_GAUGE_STEM_IN_BACKPRESSURE_OFF  (1UL)
#define FD_METRICS_GAUGE_STEM_IN_BACKPRESSURE_NAME "stem_in_backpressure"
#define FD_METRICS_GAUGE_STEM_IN_BACKPRESSURE_TYPE (FD_METRICS_TYPE_GAUGE)
#define FD_METRICS_GAUGE_STEM_IN_BACKPRESSURE_DESC "Whether the tile is currently backpressured or not, either 1 or 0."

#define FD_METRICS_COUNTER_STEM_BACKPRESSURE_COUNT_OFF  (2UL)
#define FD_METRICS_COUNTER_STEM_BACKPRESSURE_COUNT_NAME "stem_backpressure_count"
#define FD_METRICS_COUNTER_STEM_BACKPRESSURE_COUNT_TYPE (FD_METRICS_TYPE_COUNTER)
#define FD_METRICS_COUNTER_STEM_BACKPRESSURE_COUNT_DESC "Number of times the times the tile has had to wait for one of more consumers to catch up to resume publishing."

#define FD_METRICS_HISTOGRAM_STEM_LOOP_HOUSEKEEPING_DURATION_SECONDS_OFF  (3UL)
#define FD_METRICS_HISTOGRAM_STEM_LOOP_HOUSEKEEPING_DURATION_SECONDS_NAME "stem_loop_housekeeping_duration_seconds"
#define FD_METRICS_HISTOGRAM_STEM_LOOP_HOUSEKEEPING_DURATION_SECONDS_TYPE (FD_METRICS_TYPE_HISTOGRAM)
#define FD_METRICS_HISTOGRAM_STEM_LOOP_HOUSEKEEPING_DURATION_SECONDS_DESC "Duration of one iteration of the run loop which did housekeeping. The various loop durations are mutually exclusive and exhaustive, so the sum of time across all of them is roughly the total running time of the tile. Loop durations are per iteration of the run loop and non-blocking, so for example each 'caught up' sample does not represent the time we waited for new input data, but rather how long each iteration of the spin loop waiting for the data took."
#define FD_METRICS_HISTOGRAM_STEM_LOOP_HOUSEKEEPING_DURATION_SECONDS_MIN  (5e-08)
#define FD_METRICS_HISTOGRAM_STEM_LOOP_HOUSEKEEPING_DURATION_SECONDS_MAX  (5e-05)
#define FD_METRICS_HISTOGRAM_STEM_LOOP_HOUSEKEEPING_DURATION_SECONDS_CVT  (FD_METRICS_CONVERTER_SECONDS)

#define FD_METRICS_HISTOGRAM_STEM_LOOP_BACKPRESSURE_DURATION_SECONDS_OFF  (20UL)
#define FD_METRICS_HISTOGRAM_STEM_LOOP_BACKPRESSURE_DURATION_SECONDS_NAME "stem_loop_backpressure_duration_seconds"
#define FD_METRICS_HISTOGRAM_STEM_LOOP_BACKPRESSURE_DURATION_SECONDS_TYPE (FD_METRICS_TYPE_HISTOGRAM)
#define FD_METRICS_HISTOGRAM_STEM_LOOP_BACKPRESSURE_DURATION_SECONDS_DESC "Duration of one iteration of the run loop which terminated because we were backpressured by a consumer."
#define FD_METRICS_HISTOGRAM_STEM_LOOP_BACKPRESSURE_DURATION_SECONDS_MIN  (5e-08)
#define FD_METRICS_HISTOGRAM_STEM_LOOP_BACKPRESSURE_DURATION_SECONDS_MAX  (5e-05)
#define FD_METRICS_HISTOGRAM_STEM_LOOP_BACKPRESSURE_DURATION_SECONDS_CVT  (FD_METRICS_CONVERTER_SECONDS)

#define FD_METRICS_HISTOGRAM_STEM_LOOP_CAUGHT_UP_DURATION_SECONDS_OFF  (37UL)
#define FD_METRICS_HISTOGRAM_STEM_LOOP_CAUGHT_UP_DURATION_SECONDS_NAME "stem_loop_caught_up_duration_seconds"
#define FD_METRICS_HISTOGRAM_STEM_LOOP_CAUGHT_UP_DURATION_SECONDS_TYPE (FD_METRICS_TYPE_HISTOGRAM)
#define FD_METRICS_HISTOGRAM_STEM_LOOP_CAUGHT_UP_DURATION_SECONDS_DESC "Duration of one iteration of the run loop which terminated because there was no new data to process."
#define FD_METRICS_HISTOGRAM_STEM_LOOP_CAUGHT_UP_DURATION_SECONDS_MIN  (5e-08)
#define FD_METRICS_HISTOGRAM_STEM_LOOP_CAUGHT_UP_DURATION_SECONDS_MAX  (5e-05)
#define FD_METRICS_HISTOGRAM_STEM_LOOP_CAUGHT_UP_DURATION_SECONDS_CVT  (FD_METRICS_CONVERTER_SECONDS)

#define FD_METRICS_HISTOGRAM_STEM_LOOP_OVERRUN_POLLING_DURATION_SECONDS_OFF  (54UL)
#define FD_METRICS_HISTOGRAM_STEM_LOOP_OVERRUN_POLLING_DURATION_SECONDS_NAME "stem_loop_overrun_polling_duration_seconds"
#define FD_METRICS_HISTOGRAM_STEM_LOOP_OVERRUN_POLLING_DURATION_SECONDS_TYPE (FD_METRICS_TYPE_HISTOGRAM)
#define FD_METRICS_HISTOGRAM_STEM_LOOP_OVERRUN_POLLING_DURATION_SECONDS_DESC "Duration of one iteration of the run loop which terminated because we were overrun while polling."
#define FD_METRICS_HISTOGRAM_STEM_LOOP_OVERRUN_POLLING_DURATION_SECONDS_MIN  (5e-08)
#define FD_METRICS_HISTOGRAM_STEM_LOOP_OVERRUN_POLLING_DURATION_SECONDS_MAX  (5e-05)
#define FD_METRICS_HISTOGRAM_STEM_LOOP_OVERRUN_POLLING_DURATION_SECONDS_CVT  (FD_METRICS_CONVERTER_SECONDS)

#define FD_METRICS_HISTOGRAM_STEM_LOOP_OVERRUN_READING_DURATION_SECONDS_OFF  (71UL)
#define FD_METRICS_HISTOGRAM_STEM_LOOP_OVERRUN_READING_DURATION_SECONDS_NAME "stem_loop_overrun_reading_duration_seconds"
#define FD_METRICS_HISTOGRAM_STEM_LOOP_OVERRUN_READING_DURATION_SECONDS_TYPE (FD_METRICS_TYPE_HISTOGRAM)
#define FD_METRICS_HISTOGRAM_STEM_LOOP_OVERRUN_READING_DURATION_SECONDS_DESC "Duration of one iteration of the run loop which terminated because we were overrun while reading."
#define FD_METRICS_HISTOGRAM_STEM_LOOP_OVERRUN_READING_DURATION_SECONDS_MIN  (5e-08)
#define FD_METRICS_HISTOGRAM_STEM_LOOP_OVERRUN_READING_DURATION_SECONDS_MAX  (5e-05)
#define FD_METRICS_HISTOGRAM_STEM_LOOP_OVERRUN_READING_DURATION_SECONDS_CVT  (FD_METRICS_CONVERTER_SECONDS)

#define FD_METRICS_HISTOGRAM_STEM_LOOP_FILTER_BEFORE_FRAGMENT_DURATION_SECONDS_OFF  (88UL)
#define FD_METRICS_HISTOGRAM_STEM_LOOP_FILTER_BEFORE_FRAGMENT_DURATION_SECONDS_NAME "stem_loop_filter_before_fragment_duration_seconds"
#define FD_METRICS_HISTOGRAM_STEM_LOOP_FILTER_BEFORE_FRAGMENT_DURATION_SECONDS_TYPE (FD_METRICS_TYPE_HISTOGRAM)
#define FD_METRICS_HISTOGRAM_STEM_LOOP_FILTER_BEFORE_FRAGMENT_DURATION_SECONDS_DESC "Duration of one iteration of the run loop which terminated because we filtered the fragment before reading it."
#define FD_METRICS_HISTOGRAM_STEM_LOOP_FILTER_BEFORE_FRAGMENT_DURATION_SECONDS_MIN  (5e-08)
#define FD_METRICS_HISTOGRAM_STEM_LOOP_FILTER_BEFORE_FRAGMENT_DURATION_SECONDS_MAX  (5e-05)
#define FD_METRICS_HISTOGRAM_STEM_LOOP_FILTER_BEFORE_FRAGMENT_DURATION_SECONDS_CVT  (FD_METRICS_CONVERTER_SECONDS)

#define FD_METRICS_HISTOGRAM_STEM_LOOP_FILTER_AFTER_FRAGMENT_DURATION_SECONDS_OFF  (105UL)
#define FD_METRICS_HISTOGRAM_STEM_LOOP_FILTER_AFTER_FRAGMENT_DURATION_SECONDS_NAME "stem_loop_filter_after_fragment_duration_seconds"
#define FD_METRICS_HISTOGRAM_STEM_LOOP_FILTER_AFTER_FRAGMENT_DURATION_SECONDS_TYPE (FD_METRICS_TYPE_HISTOGRAM)
#define FD_METRICS_HISTOGRAM_STEM_LOOP_FILTER_AFTER_FRAGMENT_DURATION_SECONDS_DESC "Duration of one iteration of the run loop which terminated because we filtered the fragment after reading it."
#define FD_METRICS_HISTOGRAM_STEM_LOOP_FILTER_AFTER_FRAGMENT_DURATION_SECONDS_MIN  (5e-08)
#define FD_METRICS_HISTOGRAM_STEM_LOOP_FILTER_AFTER_FRAGMENT_DURATION_SECONDS_MAX  (5e-05)
#define FD_METRICS_HISTOGRAM_STEM_LOOP_FILTER_AFTER_FRAGMENT_DURATION_SECONDS_CVT  (FD_METRICS_CONVERTER_SECONDS)

#define FD_METRICS_HISTOGRAM_STEM_LOOP_FINISH_DURATION_SECONDS_OFF  (122UL)
#define FD_METRICS_HISTOGRAM_STEM_LOOP_FINISH_DURATION_SECONDS_NAME "stem_loop_finish_duration_seconds"
#define FD_METRICS_HISTOGRAM_STEM_LOOP_FINISH_DURATION_SECONDS_TYPE (FD_METRICS_TYPE_HISTOGRAM)
#define FD_METRICS_HISTOGRAM_STEM_LOOP_FINISH_DURATION_SECONDS_DESC "Duration of one iteration of the run loop which received, did not filter, and processed the fragment."
#define FD_METRICS_HISTOGRAM_STEM_LOOP_FINISH_DURATION_SECONDS_MIN  (5e-08)
#define FD_METRICS_HISTOGRAM_STEM_LOOP_FINISH_DURATION_SECONDS_MAX  (5e-05)
#define FD_METRICS_HISTOGRAM_STEM_LOOP_FINISH_DURATION_SECONDS_CVT  (FD_METRICS_CONVERTER_SECONDS)

#define FD_METRICS_HISTOGRAM_STEM_FRAGMENT_FILTERED_SIZE_BYTES_OFF  (139UL)
#define FD_METRICS_HISTOGRAM_STEM_FRAGMENT_FILTERED_SIZE_BYTES_NAME "stem_fragment_filtered_size_bytes"
#define FD_METRICS_HISTOGRAM_STEM_FRAGMENT_FILTERED_SIZE_BYTES_TYPE (FD_METRICS_TYPE_HISTOGRAM)
#define FD_METRICS_HISTOGRAM_STEM_FRAGMENT_FILTERED_SIZE_BYTES_DESC "Size of each fragment that was filtered and not processed by the tile."
#define FD_METRICS_HISTOGRAM_STEM_FRAGMENT_FILTERED_SIZE_BYTES_MIN  (0UL)
#define FD_METRICS_HISTOGRAM_STEM_FRAGMENT_FILTERED_SIZE_BYTES_MAX  (2094UL)
#define FD_METRICS_HISTOGRAM_STEM_FRAGMENT_FILTERED_SIZE_BYTES_CVT  (FD_METRICS_CONVERTER_NONE)

#define FD_METRICS_HISTOGRAM_STEM_FRAGMENT_HANDLED_SIZE_BYTES_OFF  (156UL)
#define FD_METRICS_HISTOGRAM_STEM_FRAGMENT_HANDLED_SIZE_BYTES_NAME "stem_fragment_handled_size_bytes"
#define FD_METRICS_HISTOGRAM_STEM_FRAGMENT_HANDLED_SIZE_BYTES_TYPE (FD_METRICS_TYPE_HISTOGRAM)
#define FD_METRICS_HISTOGRAM_STEM_FRAGMENT_HANDLED_SIZE_BYTES_DESC "Size of each fragment that was processed (not filtered) by the tile."
#define FD_METRICS_HISTOGRAM_STEM_FRAGMENT_HANDLED_SIZE_BYTES_MIN  (0UL)
#define FD_METRICS_HISTOGRAM_STEM_FRAGMENT_HANDLED_SIZE_BYTES_MAX  (2094UL)
#define FD_METRICS_HISTOGRAM_STEM_FRAGMENT_HANDLED_SIZE_BYTES_CVT  (FD_METRICS_CONVERTER_NONE)


#define FD_METRICS_ALL_TOTAL (13UL)
extern const fd_metrics_meta_t FD_METRICS_ALL[FD_METRICS_ALL_TOTAL];

#define FD_METRICS_ALL_LINK_IN_TOTAL (6UL)
extern const fd_metrics_meta_t FD_METRICS_ALL_LINK_IN[FD_METRICS_ALL_LINK_IN_TOTAL];

#define FD_METRICS_ALL_LINK_OUT_TOTAL (1UL)
extern const fd_metrics_meta_t FD_METRICS_ALL_LINK_OUT[FD_METRICS_ALL_LINK_OUT_TOTAL];

#define FD_METRICS_TOTAL_SZ (8UL*305UL)
