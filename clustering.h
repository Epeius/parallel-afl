/*
 * A wrapper to call sklearn-kit to perfrom clustering.
 */
#ifndef AFL_SKLEARN_CLUSTERING_H_
#define AFL_SKLEARN_CLUSTERING_H_

#include "types.h"

struct queue_entry {
  u8* fname;                          /* File name for the test case      */
  u32 len;                            /* Input length                     */
  
  u32 f_cksum;
  u8  cal_failed,                     /* Calibration failed?              */
      trim_done,                      /* Trimmed?                         */
      was_fuzzed,                     /* Had any fuzzing done yet?        */
      passed_det,                     /* Deterministic stages passed?     */
      has_new_cov,                    /* Triggers new coverage?           */
      var_behavior,                   /* Variable behavior?               */
      favored,                        /* Currently favored?               */
      fs_redundant;                   /* Marked as redundant in the fs?   */

  u32 bitmap_size,                    /* Number of bits set in bitmap     */
      exec_cksum;                     /* Checksum of the execution trace  */

  u64 exec_us,                        /* Execution time (us)              */
      handicap,                       /* Number of queue cycles behind    */
      depth;                          /* Path depth                       */

  u8* trace_mini;                     /* Trace bytes, if kept             */
  u32 tc_ref;                         /* Trace bytes ref count            */

  u8  bitmap_dumped;
  u32 id;
  struct queue_entry *next,           /* Next element, if any             */
                     *next_100;       /* 100 elements ahead               */

};

#ifdef _cplusplus
extern "C" {
#endif

void clustering_queue(void);

#ifdef _cplusplus
}
#endif

#endif  /* AFL_SKLEARN_CLUSTERING_H_ */
