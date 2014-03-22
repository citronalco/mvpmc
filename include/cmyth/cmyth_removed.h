#ifndef __CMYTH_REMOVED
#define __CMYTH_REMOVED

extern char * cmyth_get_dbschemaver_mysql(cmyth_database_t db);

/******************************************************************************/
/* Below: removed from cmyth on 2013-01-06 */
/******************************************************************************/
/* Sergio: Added to support the tvguide functionality */
struct cmyth_channel_mvpmc_deprecated;
typedef struct cmyth_channel_mvpmc_deprecated *cmyth_channel_mvpmc_deprecated_t;

struct cmyth_chanlist_mvpmc_deprecated;
typedef struct cmyth_chanlist_mvpmc_deprecated *cmyth_chanlist_mvpmc_deprecated_t;

extern int mythtv_new_livetv(void);

extern cmyth_chanlist_mvpmc_deprecated_t myth_tvguide_load_channels(cmyth_database_t db, int sort_desc);

#endif
