#ifndef __CMYTH_REMOVED
#define __CMYTH_REMOVED

/* commented: not used by mvpmc */


/**
 * Create a file connection to a backend.
 * \param path path to file
 * \param control control handle
 * \param buflen buffer size for the connection to use
 * \param tcp_rcvbuf if non-zero, the TCP receive buffer size for the socket
 * \return file handle
 */
/*
extern cmyth_file_t cmyth_conn_connect_path(char* path, cmyth_conn_t control,
                                           unsigned buflen, int tcp_rcvbuf);

*/
/* 
extern int cmyth_livetv_keep_recording(cmyth_recorder_t rec, cmyth_database_t db, int keep);
*/

/*
extern int cmyth_livetv_read(cmyth_recorder_t rec,
                            char *buf,
                            unsigned long len);

*/
/* 
extern int cmyth_ringbuf_read(cmyth_recorder_t rec,
                             char *buf,
                             unsigned long len);
*/
/*
extern cmyth_proginfo_t cmyth_proginfo_get_from_basename(cmyth_conn_t control,
                                                        const char* basename);

*/
/*
extern int cmyth_file_read(cmyth_file_t file,
                          char *buf,
                          unsigned long len);
*/
extern char * cmyth_get_dbschemaver_mysql(cmyth_database_t db);

/******************************************************************************/
/* Below: removed from cmyth on 2013-01.06 */
/******************************************************************************/
/*
extern cmyth_file_t cmyth_livetv_get_cur_file(cmyth_recorder_t rec);
*/
extern int mythtv_new_livetv(void);

extern cmyth_chanlist_t myth_tvguide_load_channels(cmyth_database_t db, int sort_desc);

/*
 * -----------------------------------------------------------------
 * Timestamp Operations
 * -----------------------------------------------------------------
 */
/* 
extern cmyth_timestamp_t cmyth_datetime_from_string(char *str);
*/
/*
 * -----------------------------------------------------------------
 * Key Frame Operations
 * -----------------------------------------------------------------
 */
/*
extern cmyth_keyframe_t cmyth_keyframe_tcmyth_keyframe_get(
        unsigned long keynum,
        unsigned long long pos);
*/
#endif
