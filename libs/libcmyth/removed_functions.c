/*
 *  Copyright (C) 2004-2014, Eric Lund
 *  http://www.mvpmc.org/
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <inttypes.h>
#include <string.h>
#include <sys/types.h>
#include <cmyth_local.h>

/* Functions in this file got removed from Gettler's cmyth library */
/* file.c */

/* removed 2014-05-08 */
/*
 * cmyth_file_set_closed_callback(cmyth_file_t file, void (*callback)(cmyth_file_t))
 *
 * Scope: PUBLIC
 *
 * Description
 *
 * Sets a callback which will be called when a file connection has been
 * signalled as done. Passing in NULL means no callback.
 */


/* proginfo.c */
int
cmyth_get_delete_list_deprecated(cmyth_conn_t conn, char * msg, cmyth_proglist_t prog)
{
 int err=0;
 int count;
 int prog_count=0;

 if (!conn) {
 cmyth_dbg(CMYTH_DBG_ERROR, "%s: no connection\n", __FUNCTION__);
 return -1;
 }
 pthread_mutex_lock(&conn->conn_mutex);
 if ((err = cmyth_send_message(conn, msg)) < 0) {
 fprintf (stderr, "ERROR %d \n",err);
 cmyth_dbg(CMYTH_DBG_ERROR,
 "%s: cmyth_send_message() failed (%d)\n",__FUNCTION__,err);
 return err;
 }
 count = cmyth_rcv_length(conn);
 cmyth_rcv_proglist(conn, &err, prog, count);
 prog_count=cmyth_proglist_get_count(prog);
 pthread_mutex_unlock(&conn->conn_mutex);
 return prog_count;
}

/* mythtv_mysql.c */
int
cmyth_tuner_type_check_deprecated(cmyth_database_t db, cmyth_recorder_t rec, int check_tuner_type) {
 MYSQL_RES *res=NULL;
 MYSQL_ROW row;
 const char * query_str = "SELECT cardtype from capturecard WHERE cardid=?";
 cmyth_mysql_query_t * query;

 if ( check_tuner_type == 0 ) {
 cmyth_dbg(CMYTH_DBG_ERROR,"MythTV Tuner check not enabled in Mythtv Options\n");
 return (1);
 }


 query = cmyth_mysql_query_create(db,query_str);
 if (cmyth_mysql_query_param_uint(query,rec->rec_id) < 0) {
 cmyth_dbg(CMYTH_DBG_ERROR,"%s, binding of query failed\n",__FUNCTION__);
 ref_release(query);
 return -1;
 }
 res = cmyth_mysql_query_result(query);

 if(res == NULL) {
 cmyth_dbg(CMYTH_DBG_ERROR,"%s, finalisation/execution\n",__FUNCTION__);
 return -1;
 }
 row = mysql_fetch_row(res);
 ref_release(query);
 mysql_free_result(res);
 if (strcmp(row[0],"MPEG") == 0) {
 return (1); //return the first available MPEG tuner
 }
 else if (strcmp(row[0],"HDHOMERUN") == 0) {
 return (1); //return the first available MPEG2TS tuner
 }
 else if (strcmp(row[0],"DVB") == 0) {
 return (1); //return the first available DVB tuner
 }
 else {
 return (0);
 }
}

