#ifndef __CMYTH_LOCAL_H_REMOVED
#define __CMYTH_LOCAL_H_REMOVED

#include <cmyth.h>
#include <cmyth_local.h>
#include <cmyth_removed.h>

/* Sergio: Added to support the tvguide functionality */
struct cmyth_tvguide_progs {
       cmyth_program_t * progs;
       int count;
       int alloc;
};

/* Below: Removed from cmyth on 2013-01-06 */
/* Sergio: Added to clean up channel list handling */
struct cmyth_channel {
        long chanid;
        int channum;
        char chanstr[10];
        long cardids;/* A bit array of recorders/tuners supporting the channel */
        char *callsign;
        char *name;
};

struct cmyth_chanlist {
        cmyth_channel_t chanlist_list;
        int chanlist_sort_desc;
        int chanlist_count;
        int chanlist_alloc;
};

#endif
