/**
 * Create a file connection to a backend.
 * \param path path to file
 * \param control control handle
 * \param buflen buffer size for the connection to use
 * \param tcp_rcvbuf if non-zero, the TCP receive buffer size for the socket
 * \return file handle
 */
extern cmyth_file_t cmyth_conn_connect_path(char* path, cmyth_conn_t control,
                                           unsigned buflen, int tcp_rcvbuf);


extern int cmyth_livetv_keep_recording(cmyth_recorder_t rec, cmyth_database_t db, int keep);

extern int cmyth_livetv_read(cmyth_recorder_t rec,
                            char *buf,
                            unsigned long len);

extern int cmyth_ringbuf_read(cmyth_recorder_t rec,
                             char *buf,
                             unsigned long len);

extern cmyth_proginfo_t cmyth_proginfo_get_from_basename(cmyth_conn_t control,
                                                        const char* basename);

extern int cmyth_file_read(cmyth_file_t file,
                          char *buf,
                          unsigned long len);

extern char * cmyth_get_dbschemaver_mysql(cmyth_database_t db);
