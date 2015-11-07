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

/******************************************************************************/
/* Below: removed from cmyth on 2014-05-08 */
/******************************************************************************/

struct cmyth_tvguide_progs;
typedef struct cmyth_tvguide_progs *cmyth_tvguide_progs_t;

#define CMYTH_NUM_SORTS 2
#define PROGRAM_ADJUST 3600

extern int cmyth_get_delete_list(cmyth_conn_t, char *, cmyth_proglist_t);

/**
 * Retrieve the tuner type of a recorder.
 * \deprecated This function seems quite useless.
 */
extern int cmyth_tuner_type_check_deprecated(cmyth_database_t db, cmyth_recorder_t rec, int check_tuner_enabled);

#endif
