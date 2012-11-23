/*
 *  Copyright (C) 2006, Sergio Slobodrian
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

#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <cmyth_local.h>
#include <mvp_widget.h>

#define ALLOC_FRAC 10
#define EVENT_WILL_RECORD 1
#define EVENT_AUTO_TUNE 2

#define MERGE_CELLS 1

#if 0
int ssdebug = 0;
#define PRINTF(x...) if(ssdebug) printf(x) 
#define TRC(fmt, args...) printf(fmt, ## args)
#else
#define PRINTF(x...)
#define TRC(fmt, args...) 
#endif

struct cell_hilite_s {
	time_t start_time;
	int chan_num;
	mvpw_array_cell_theme * theme;
};
typedef struct cell_hilite_s *cell_hilite_t;

struct cell_hilite_list_s {
	cell_hilite_t list;
	int count;
	int avail;
};
typedef struct cell_hilite_list_s *cell_hilite_list_t;

static mvpw_array_cell_theme mvpw_record_theme = {
	.cell_fg = MVPW_YELLOW,
	.cell_bg = MVPW_MIDNIGHTBLUE,
	.hilite_fg = MVPW_MIDNIGHTBLUE,
	.hilite_bg = MVPW_YELLOW,
};

static cell_hilite_list_t hilites = NULL;

int
myth_tvguide_add_hilite(time_t start_time, int chan_num,
													mvpw_array_cell_theme *theme)
{
	int rtrn = 0;
	int sz,av;

	PRINTF("** SSDEBUG: adding hilite on start time: %ld, chan %d\n",
				 start_time, chan_num);
	if(hilites == NULL) {
		hilites = ref_alloc(sizeof(*hilites));
		if(hilites) {
			hilites->list = ref_alloc(sizeof(*(hilites->list))*ALLOC_FRAC);
			if(hilites->list) {
				rtrn = 1;
				av = hilites->avail = ALLOC_FRAC;
				sz = hilites->count = 0;
			}
			else {
				ref_release(hilites);
				hilites = NULL;
			}
		}
	}
	else {
		sz = hilites->count;
		av = hilites->avail;
		if(sz == av) {
			av += ALLOC_FRAC;
			hilites->list =
								ref_realloc(hilites->list, sizeof(*(hilites->list))*av);
			if(hilites->list) {
				rtrn = 1;
				hilites->avail = av;
			}
			else {
				ref_release(hilites);
				hilites = NULL;
			}
		}
	}

	if(rtrn == 1) {
		hilites->list[sz].start_time = start_time;
		hilites->list[sz].chan_num = chan_num;
		hilites->list[sz].theme = theme;
		sz++;
		hilites->count = sz;
	}

	return rtrn;
}

int
myth_tvguide_remove_hilite(time_t start_time, int chan_num)
{
	int rtrn = 0;
	int i;

	PRINTF("** SSDEBUG: removing hilite on start time: %ld, chan %d\n",
				 start_time, chan_num);
	if(hilites) {
		for(i=0; i<hilites->count; i++) {
			if(hilites->list[i].start_time == start_time
					&& hilites->list[i].chan_num == chan_num)
				break;
		}
		if(i == hilites->count - 1) {
			hilites->count--;
			rtrn = 1;
		}
		else if(i < hilites->count) {
			i++;
			while(i<hilites->count) {
				memmove(&(hilites->list[i-1]), &(hilites->list[i]),
								sizeof(struct cell_hilite_s));
				i++;
			}
			hilites->count--;
			rtrn = 1;
		}
		if(hilites->count == 0) {
			ref_release(hilites->list);
			ref_release(hilites);
			hilites = NULL;
		}
	}

	return rtrn;
}

void
myth_tvguide_clear_hilites(void)
{
	if(hilites) {
		if(hilites->list)
			ref_release(hilites->list);
		ref_release(hilites);
	}
	hilites = NULL;
}

static cell_hilite_t
myth_tvguide_should_hilite(time_t start_time, int chan_num)
{
	int i;
	cell_hilite_t rtrn = NULL;

	PRINTF("** SSDEBUG: checking hilite on start time: %ld, chan %d\n",
				 start_time, chan_num);
	if(hilites) {
		for(i=0;i<hilites->count;i++) {
			if(hilites->list[i].start_time == start_time
					&& hilites->list[i].chan_num == chan_num) {
				rtrn = &(hilites->list[i]);
				break;
			}
		}
	}

	return rtrn;
}

int
mvp_tvguide_sql_check(cmyth_database_t db)
{
	MYSQL *mysql;

  mysql=mysql_init(NULL);
	if(mysql == NULL ||
		 !(mysql_real_connect(mysql,db->db_host,db->db_user,
													db->db_pass,db->db_name,0,NULL,0))) {
		cmyth_dbg(CMYTH_DBG_ERROR, "%s: mysql_connect() Failed: %s\n",
                           __FUNCTION__, mysql_error(mysql));
		fprintf(stderr, "mysql_connect() Failed: %s\n",mysql_error(mysql));

    if(mysql) mysql_close(mysql);
		return 0;
	}
  mysql_close(mysql);
	return 1;
}

/*
 *
 */
void
mvp_tvguide_move(int direction, mvp_widget_t * proglist, mvp_widget_t * descr)
{
	cmyth_program_t *prog;

	mvpw_move_array_selection(proglist, direction);
	prog = (cmyth_program_t *)
 	mvpw_get_array_cur_cell_data(proglist);
 	mvpw_set_text_str(descr, prog->description);
}

/*
 *
 */
void
mvp_tvguide_show(mvp_widget_t *proglist, mvp_widget_t *descr,
								 mvp_widget_t *clock)
{
	cmyth_program_t *prog;

	mvpw_show(descr);
	mvpw_show(clock);
	mvpw_reset_array_selection(proglist);
	mvpw_show(proglist);
	prog = (cmyth_program_t *)
 	mvpw_get_array_cur_cell_data(proglist);
 	mvpw_set_text_str(descr, prog->description);
}

/*
 *
 */
void
mvp_tvguide_hide(void *proglist, void *descr, void * clock)
{
	mvpw_hide((mvp_widget_t *)clock);
	mvpw_hide((mvp_widget_t *)descr);
	mvpw_hide((mvp_widget_t *)proglist);
}

/*
 * Based on the integer passed in, return the index into the
 * provided chanlist array to the channel that is the one we
 * provided or greater.
 */
int
myth_get_chan_index_from_int(cmyth_chanlist_t chanlist, int nchan)
{
	int rtrn;
/*
	if(chanlist->chanlist_sort_desc) {
		for(rtrn = 0; rtrn < cmyth_chanlist_get_count(chanlist); rtrn++) {
		    cmyth_channel_t channel = cmyth_chanlist_get_item(chanlist,rtrn);
		    if (channel) {
			if (!cmyth_channel_visible(channel)) {
			    ref_release(channel);
			    continue;
			}
			if (cmyth_channel_channum(channel) <= nchan)
				break;
			}
		}
	}
	else { 
*/
		for(rtrn = 0; rtrn < cmyth_chanlist_get_count(chanlist); rtrn++) {
		    cmyth_channel_t channel = cmyth_chanlist_get_item(chanlist,rtrn);
		    if (channel) {
			if (!cmyth_channel_visible(channel)) {
			    ref_release(channel);
			    continue;
			}
			if (cmyth_channel_channum(channel) >= nchan)
				break;
			}
		}
/*	}	*/
	rtrn = rtrn==cmyth_chanlist_get_count(chanlist)?rtrn-1:rtrn;

	return rtrn;
}

/*
 * Based on the string passed in, return the index into the
 * provided chanlist array to the channel that is the one we
 * provided or greater.
 */
int
myth_get_chan_index_from_str(cmyth_chanlist_t chanlist, char * chan)
{
	int nchan = atoi(chan);

	return myth_get_chan_index_from_int(chanlist, nchan);
}

/*
 * Based on the proginfo passed in, return the index into the
 * provided chanlist array or return -1 if a match for the channel
 * number and callsign.
 */
int
myth_get_chan_index(cmyth_chanlist_t chanlist, cmyth_proginfo_t prog)
{
	int rtrn;
	cmyth_channel_t channel;

	for(rtrn = 0; rtrn < cmyth_chanlist_get_count(chanlist); rtrn++) {
		channel = cmyth_chanlist_get_item(chanlist, rtrn);
		if(channel->chanid == prog->proginfo_chanId
			 && strcmp(channel->callsign,
			 					 prog->proginfo_chansign) == 0)
			break;
	}
	return rtrn;
}

/*
 * Returns 1 if the current prog and chan index are the same
 * 0 otherwise.
 */
int
myth_is_chan_index(cmyth_chanlist_t chanlist, cmyth_proginfo_t prog,
									 int index)
{
	int rtrn = 0;
	cmyth_proginfo_t lprog = ref_hold(prog);
	cmyth_channel_t channel;
	
	channel = cmyth_chanlist_get_item(chanlist, index);
	
	if(channel->chanid == lprog->proginfo_chanId)
		rtrn = 1;

	ref_release(prog);

	return rtrn;
}

/*
 *
 */
static int
get_chan_num(long chanid, cmyth_chanlist_t chanlist)
{
	int i;
	cmyth_channel_t channel;

	for(i=0; i < cmyth_chanlist_get_count(chanlist); i++) {
		channel = cmyth_chanlist_get_item(chanlist, i);
		if(channel->chanid == chanid)
			return channel->channum;
	}

	return 0;
}

/*
 * Based on the integer passed in, return the index into the
 * provided chanlist array to the channel that is the one we
 * provided or greater.
 */
static char *
myth_get_chan_str_from_int(cmyth_chanlist_t chanlist, int nchan)
{
	int idx = myth_get_chan_index_from_int(chanlist, nchan);
	cmyth_channel_t channel = cmyth_chanlist_get_item(chanlist,idx);
	return channel->chanstr;
}

/*
 *
 */
char *
get_tvguide_selected_channel_str(mvp_widget_t *proglist,
																 cmyth_chanlist_t chanlist)
{
	cmyth_program_t *prog;

	prog = (cmyth_program_t *)
 	mvpw_get_array_cur_cell_data(proglist);

	PRINTF("** SSDEBUG: Current prog showing as: %s\n", prog->title);

	return myth_get_chan_str_from_int(chanlist, prog->channum);
}


/*
 *
 */
static cmyth_tvguide_progs_t
get_tvguide_page(MYSQL *mysql, cmyth_chanlist_t chanlist,
								 cmyth_tvguide_progs_t proglist, int index,
								 time_t start_time, time_t end_time) 
{
	MYSQL_RES *res=NULL;
	MYSQL_ROW row;
	char query[350];
	char channels[50];
	int i, rows = 0, idxs[4], idx=0; 
	static cmyth_program_t * cache = NULL;
	int cache_ct;
	long ch=0;

	if(!cache)
		cache = ref_alloc(sizeof(*cache)*4);


	PRINTF("** SSDEBUG: index is: %d\n", index);
	idxs[0] = index < 0 ? chanlist->chanlist_count+index:index;
	index--;
	PRINTF("** SSDEBUG: index is: %d\n", index);
	idxs[1] = index < 0 ? chanlist->chanlist_count+index:index;
	index--;
	PRINTF("** SSDEBUG: index is: %d\n", index);
	idxs[2] = index < 0 ? chanlist->chanlist_count+index:index;
	index--;
	PRINTF("** SSDEBUG: index is: %d\n", index);
	idxs[3] = index < 0 ? chanlist->chanlist_count+index:index;

	PRINTF("** SSDEBUG: indexes are: %d, %d, %d, %d\n", idxs[0], idxs[1],
					idxs[2], idxs[3]);
	PRINTF("** SSDEBUG: callsigns are: %s, %s, %s, %s\n",
		(cmyth_chanlist_get_item(chanlist,idxs[0]))->callsign,
		(cmyth_chanlist_get_item(chanlist,idxs[1]))->callsign,
		(cmyth_chanlist_get_item(chanlist,idxs[2]))->callsign,
		(cmyth_chanlist_get_item(chanlist,idxs[3]))->callsign
//		chanlist->chanlist_list[idxs[0]].callsign,
//		chanlist->chanlist_list[idxs[1]].callsign,
//		chanlist->chanlist_list[idxs[2]].callsign,
//		chanlist->chanlist_list[idxs[3]].callsign
	);

	sprintf(channels, "(%ld, %ld, %ld, %ld)",
		(cmyth_chanlist_get_item(chanlist,idxs[0]))->chanid,
		(cmyth_chanlist_get_item(chanlist,idxs[1]))->chanid,
		(cmyth_chanlist_get_item(chanlist,idxs[2]))->chanid,
		(cmyth_chanlist_get_item(chanlist,idxs[3]))->chanid
//		chanlist->chanlist_list[idxs[0]].chanid,
//		chanlist->chanlist_list[idxs[1]].chanid,
//		chanlist->chanlist_list[idxs[2]].chanid,
//		chanlist->chanlist_list[idxs[3]].chanid
	);

	PRINTF("** SSDEBUG: starttime:%d, endtime:%d\n", starttime, endtime);

	sprintf(query, 
"SELECT chanid,UNIX_TIMESTAMP(starttime),UNIX_TIMESTAMP(endtime), title,description,subtitle,programid,seriesid,category FROM program WHERE starttime<FROM_UNIXTIME(%ld) AND endtime>FROM_UNIXTIME(%ld) AND chanid in %s ORDER BY chanid DESC, starttime ASC",end_time,start_time, channels);

	cmyth_dbg(CMYTH_DBG_ERROR, "%s: query= %s\n", __FUNCTION__, query);
	if(mysql_query(mysql,query)) {
		cmyth_dbg(CMYTH_DBG_ERROR, "%s: mysql_query() Failed: %s\n", 
                           __FUNCTION__, mysql_error(mysql));
		mysql_close(mysql);
		return NULL;
	}
	res = mysql_store_result(mysql);


	
	PRINTF("** SSDEBUG: got %llu rows from query\n", res->row_count);
	/*
	 * Need to do some special handling on the query results. Sometimes
	 * no data exists for certain channels as a result of the query
	 * and we need to fill it in with unknown. In other cases, there
	 * are a series of 10 min shows in a 30 min period so there may
	 * be more than one so only the first one should be used. Also
	 * in the case we're wrapping around the rows are returned from
	 * the query in the wrong order so we need to suck in the entire
	 * query result and search for our specific line in it.
	 */

	idx = 0;
	while((row = mysql_fetch_row(res))) {
		ch = atol(row[0]);
		cache[idx].channum = get_chan_num(ch, chanlist);

		if(idx > 0 && ch == cache[idx-1].chanid) {
			PRINTF("** SSDEBUG: Cache discarding entry with same chanid in same slot\n");
			continue;
		}
		PRINTF("** SSDEBUG: cache: row: %d, %ld, %d, %s, %s, %s\n", idx, ch,
						cache[idx].channum, row[3], row[5], row[8]);
		cache[idx].chanid=ch;
		cache[idx].event_flags=0;
		cache[idx].starttime = atol(row[1]);
		cache[idx].endtime = atol(row[2]);
		strncpy ( cache[idx].title, row[3], 130);
		strncpy ( cache[idx].description, row[4], 256);
		strncpy ( cache[idx].subtitle, row[5], 130);
		strncpy ( cache[idx].programid, row[6], 20);
		strncpy ( cache[idx].seriesid, row[7], 12);
		strncpy ( cache[idx].category, row[8], 64);
		idx++;
	}
	cache_ct = idx;
  mysql_free_result(res);


	sprintf(query,
		"SELECT chanid, programid \
		 FROM record \
		 WHERE startdate = FROM_UNIXTIME(%ld,'%%Y-%%m-%%d') \
		 AND enddate = FROM_UNIXTIME(%ld, '%%Y-%%m-%%d') \
		 AND starttime < FROM_UNIXTIME(%ld, '%%h:%%i:%%s') \
		 AND endtime > FROM_UNIXTIME(%ld, '%%h:%%i:%%s') \
		 AND chanid IN %s",
		 start_time, end_time, end_time, start_time, channels);
	cmyth_dbg(CMYTH_DBG_ERROR, "%s: query= %s\n", __FUNCTION__, query);
	if(mysql_query(mysql,query)) {
		cmyth_dbg(CMYTH_DBG_ERROR, "%s: mysql_query() Failed: %s\n", 
                           __FUNCTION__, mysql_error(mysql));
		mysql_close(mysql);
		return NULL;
	}

	/* Now flag all programs schduled to record */
	res = mysql_store_result(mysql);
	PRINTF("** SSDEBUG: got %llu rows from query\n", res->row_count);
	while((row = mysql_fetch_row(res))) {
		ch = atol(row[0]);
		PRINTF("** SSDEBUG: chanid returned is %ld\n", ch);
		for(i = 0; i<cache_ct; i++) {
			PRINTF("** SSDEBUG: cache chanid is %ld\n", cache[i].chanid);
			if(cache[i].chanid == ch && strcmp(row[1], cache[i].programid) == 0) {
				PRINTF("** SSDEBUG: chanid match on %ld\n", ch);
				cache[i].event_flags |= EVENT_WILL_RECORD;
			}
		}
	}
  mysql_free_result(res);

	/* Now flag all programs with auto tune set */
	for(i=0; i<cache_ct; i++) {
		if(myth_tvguide_should_hilite(cache[i].starttime, cache[i].channum))
			cache[i].event_flags |= EVENT_AUTO_TUNE;
	}

	rows = proglist->count;
	for(idx=0;idx<4;idx++) {


		for(i = 0; i<cache_ct; i++) {
//			if(cache[i].chanid == chanlist->chanlist_list[idxs[idx]].chanid) {
			if(cache[i].chanid == (cmyth_chanlist_get_item(chanlist,idxs[idx]))->chanid) {
				break;
			}
		}
		
		/*
		if(cache[i].chanid != chanlist->chanlist_list[idxs[idx]].chanid) {
		*/
		if(i == cache_ct) {
			PRINTF("** SSDEBUG: no program info on channel id: %d between %s, %s\n",
							idxs[idx], starttime, endtime);
			proglist->progs[rows].channum = get_chan_num(idxs[idx], chanlist);
//			proglist->progs[rows].chanid=chanlist->chanlist_list[idxs[idx]].chanid;
			proglist->progs[rows].chanid=(cmyth_chanlist_get_item(chanlist,idxs[idx]))->chanid;
			proglist->progs[rows].event_flags=cache[i].event_flags;
			proglist->progs[rows].starttime = start_time;
			proglist->progs[rows].endtime = end_time;
			strncpy ( proglist->progs[rows].title, "Unknown", 130);
			strncpy ( proglist->progs[rows].description, 
				"There are no entries in the database for this channel at this time",
			 	256);
			strncpy ( proglist->progs[rows].subtitle, "Unknown", 130);
			strncpy ( proglist->progs[rows].programid, "Unknown", 20);
			strncpy ( proglist->progs[rows].seriesid, "Unknown", 12);
			strncpy ( proglist->progs[rows].category, "Unknown", 64);
		}
		else { /* All aligns, move the information */
			proglist->progs[rows].channum = get_chan_num(cache[i].chanid, chanlist);
			PRINTF("** SSDEBUG: row: %d, %ld, %d, %s, %s, %s\n", rows,
							cache[i].chanid, proglist->progs[rows].channum,
							cache[i].title, cache[i].subtitle, cache[i].category);
			proglist->progs[rows].chanid=cache[i].chanid;
			proglist->progs[rows].event_flags=cache[i].event_flags;
			proglist->progs[rows].starttime = cache[i].starttime;
			proglist->progs[rows].endtime = cache[i].endtime;
			strncpy ( proglist->progs[rows].title, cache[i].title, 130);
			strncpy ( proglist->progs[rows].description, cache[i].description, 256);
			strncpy ( proglist->progs[rows].subtitle, cache[i].subtitle, 130);
			strncpy ( proglist->progs[rows].programid, cache[i].programid, 20);
			strncpy ( proglist->progs[rows].seriesid, cache[i].seriesid, 12);
			strncpy ( proglist->progs[rows].category, cache[i].category, 64);
			cmyth_dbg(CMYTH_DBG_ERROR, "prog[%d].chanid =  %d\n",rows,
								proglist->progs[rows].chanid);
			cmyth_dbg(CMYTH_DBG_ERROR, "prog[%d].title =  %s\n",rows,
								proglist->progs[rows].title);
		}
		rows++;
	}
	proglist->count = rows;

	return proglist;
}

int
myth_guide_set_channels(void * widget, cmyth_chanlist_t chanlist,
												int index, int yofs,
												long free_recorders)
{
	int i,j, rtrn;
	char buf[64];
	mvp_widget_t * prog_widget = (mvp_widget_t *) widget;

	PRINTF("** SSDEBUG: request to load row labels: %d\n", index);

	index += yofs;

	index = index >= chanlist->chanlist_count
					?index-chanlist->chanlist_count:index;
	index = index + chanlist->chanlist_count < 0
					? index+chanlist->chanlist_count:index;
	
	rtrn = index;

	PRINTF("** SSDEBUG: index is %d\n", index);

	/*
	 * Set the four visible channels in the widget
	 */
	for(i = index; i>index-4; i--) {
		if(i <0) {
			j = chanlist->chanlist_count + i;
			sprintf(buf, "%d\n%s", (cmyth_chanlist_get_item(chanlist,j))->channum,
						(cmyth_chanlist_get_item(chanlist,j))->callsign);
			if((free_recorders &  (cmyth_chanlist_get_item(chanlist,j))->cardids) == 0)
				mvpw_set_array_row_bg(prog_widget, index-i, MVPW_DARK_RED);
			else
				mvpw_set_array_row_bg(prog_widget, index-i, MVPW_DARKGREY);
			mvpw_set_array_row(prog_widget, index-i, buf, NULL);
			PRINTF("** SSDEBUG: loading guide: %d:%s\n",
						(cmyth_chanlist_get_item(chanlist,j))->channum,
						(cmyth_chanlist_get_item(chanlist,j))->callsign);
		}
		else {
			sprintf(buf, "%d\n%s", (cmyth_chanlist_get_item(chanlist,i))->channum,
						(cmyth_chanlist_get_item(chanlist,i))->callsign);
			if((free_recorders & (cmyth_chanlist_get_item(chanlist,i))->cardids) == 0)
				mvpw_set_array_row_bg(prog_widget, index-i, MVPW_DARK_RED);
			else
				mvpw_set_array_row_bg(prog_widget, index-i, MVPW_DARKGREY);
			mvpw_set_array_row(prog_widget, index-i, buf, NULL);
			PRINTF("** SSDEBUG: loading guide: %d:%s\n",
						(cmyth_chanlist_get_item(chanlist,i))->channum,
						(cmyth_chanlist_get_item(chanlist,i))->callsign);
		}
	}

	return rtrn;
}

/*
 * For testing, this function just loads the view that we need to
 * look at.
 */
cmyth_tvguide_progs_t
myth_load_guide(void * widget, cmyth_database_t db,
											 cmyth_chanlist_t chanlist,
											 cmyth_tvguide_progs_t proglist,
											 int index, int * xofs, int * yofs,
											 long free_recorders)
{
	MYSQL *mysql;
	int i, j, k, m, prev;
	time_t curtime, nexttime;
	struct  tm now, later;
	cmyth_tvguide_progs_t rtrn = proglist;
	cmyth_program_t * prog;

	PRINTF("** SSDEBUG: request to load guide: %d\n", index);

	/* Handle wraparound properly */
	(*yofs) = (*yofs)%chanlist->chanlist_count;

	index = myth_guide_set_channels(widget, chanlist, index, *yofs,
																	free_recorders);

	/* Allocate a new proglist if required TODO, this needs to be
	 * changed to use the standard methodology.
	 */
	if(!proglist) {
		proglist = (cmyth_tvguide_progs_t) ref_alloc(sizeof(*proglist));
		proglist->progs =
			ref_alloc(sizeof(*(proglist->progs)) * 3 * 4);
		proglist->count = 0;
		proglist->alloc = 0;
	}
	if(proglist->progs) {
		proglist->count = 0;
	}

  mysql=mysql_init(NULL);
	if(!(mysql_real_connect(mysql,db->db_host,db->db_user,
													db->db_pass,db->db_name,0,NULL,0))) {
		cmyth_dbg(CMYTH_DBG_ERROR, "%s: mysql_connect() Failed: %s\n",
                           __FUNCTION__, mysql_error(mysql));
		fprintf(stderr, "mysql_connect() Failed: %s\n",mysql_error(mysql));
        	mysql_close(mysql);
		return NULL;
	}

	curtime = time(NULL);
	curtime += 60*30*(*xofs);
#ifdef MERGE_CELLS_dont
	mvpw_reset_array_cells(widget);
#endif
	for(j=0;j<3;j++) {

		/* Start of 30 minute window */
		localtime_r(&curtime, &now);
		now.tm_min = now.tm_min >= 30?30:0;
		now.tm_sec = 0;
		curtime = mktime(&now);

		/* End of 30 minute window */
		nexttime = curtime + 60*30;
		localtime_r(&nexttime, &later);
		later.tm_min = later.tm_min >= 30?30:0;
		later.tm_sec = 0;
		nexttime = mktime(&later);

		prev = proglist->count;
		PRINTF("** SSEDBUG: Calling set_guide_mysql2\n");
		rtrn = get_tvguide_page(mysql, chanlist, proglist, index, curtime,
														nexttime);
		PRINTF("** SSEDBUG: done set_guide_mysql2 rtrn = %p\n", rtrn);
		if(rtrn == NULL)
			return proglist;

		k=0;
		for(i=prev; i<rtrn->count; i++) {
			if(i>prev && rtrn->progs[i].chanid == rtrn->progs[i-1].chanid) {
				k++;
				continue;
			}
			PRINTF("** SSDEBUG: Loaded prog(%d): %d, %ld, %d, %s, %s, %s, %s, %s, %s, %s, %s\n",
			i,
			rtrn->progs[i].channum,
			rtrn->progs[i].chanid,
			rtrn->progs[i].event_flags,
			rtrn->progs[i].starttime,
			rtrn->progs[i].endtime,
			rtrn->progs[i].title,
			rtrn->progs[i].description,
			rtrn->progs[i].subtitle,
			rtrn->progs[i].programid,
			rtrn->progs[i].seriesid,
			rtrn->progs[i].category);
			/* Determine if the left neighbour is the same show and if so just
			 * extend the previous cell instead of filling the current one in
			 */
			/* Fill in the info in the guide */
#ifdef MERGE_CELLS
			for(m=0; m<j; m++) {
					prog = (struct cmyth_program *)
											mvpw_get_array_cell_data(widget, m, i-prev-k);
					if(prog->starttime == rtrn->progs[i].starttime
					 	&& prog->endtime == rtrn->progs[i].endtime ) {
						/*
						PRINTF("** SSDEBUG: Need collapse %d cells for %s\n",
							j-m+1, prog->title);
						*/
						break;
					}
			}
			mvpw_set_array_cell_span(widget, m, i-prev-k, j-m+1);
#endif
			if(rtrn->progs[i].event_flags & EVENT_WILL_RECORD) {
				mvpw_set_array_cell_theme(widget, j, i-prev-k, &mvpw_record_theme);
				PRINTF("** SSDEBUG: setting cell color for cell %d, %d\n",
							 j, i-prev-k);
			}
			else if(rtrn->progs[i].event_flags & EVENT_AUTO_TUNE) {
				PRINTF("** SSDEBUG: setting cell color for cell %d, %d\n",
							 j, i-prev-k);
				mvpw_set_array_cell_theme(widget, j, i-prev-k,
					myth_tvguide_should_hilite(rtrn->progs[i].starttime,
																		rtrn->progs[i].channum)->theme);
					
			}
			else {
				mvpw_set_array_cell_theme(widget, j, i-prev-k, NULL);
			}

			mvpw_set_array_cell_data(widget, j, i-prev-k, &rtrn->progs[i]);
			mvpw_set_array_cell(widget, j, i-prev-k, rtrn->progs[i].title, NULL);
		}
		curtime = nexttime;
		/*
		PRINTF("** SSDEBUG: Looping next column\n");
		*/
	}
	mvpw_array_clear_dirty(widget);

  mysql_close(mysql);
	
	return rtrn;
}

/*
 * Determines if the program highlighted occurs in the future
 */
int
myth_guide_is_future(void * widget, int xofs)
{
	int rtrn;
	cmyth_program_t *cur;
	time_t now;

	now = time(NULL);

	cur = mvpw_get_array_cur_cell_data(widget);

	PRINTF("** SSDEBUG: start = %ld\n", cur->starttime);

	if(now <cur->endtime && now > cur->starttime)
		rtrn = 0;
	else
		rtrn = 1;

	return rtrn;
}

/*
 *
 */
static int guide_times_last_minutes = -1;
int
myth_set_guide_times(void * widget, int xofs, int time_format_12)
{
	mvp_widget_t * prog_widget = (mvp_widget_t *) widget;
	struct tm *ltime;
	char timestr[25];
	time_t curtime, nexthr;
	static int last_ofs = 0;
	int minutes, rtrn=1;
	char hour_format[10];
	char halfhour_format[10];

	if (time_format_12)
	{
		strcpy(hour_format, "%I:00 %P");
		strcpy(halfhour_format, "%I:30 %P");
	}
	else
	{
		strcpy(hour_format, "%H:00");
		strcpy(halfhour_format, "%H:30");
	}

	curtime = time(NULL);
	curtime += 60*30*xofs;
	ltime = localtime(&curtime);
	/*
	strftime(timestr, 25, "%M", ltime);
	minutes = atoi(timestr);
	*/
	minutes = ltime->tm_min;
	if(guide_times_last_minutes == -1
	|| (guide_times_last_minutes < 30 && minutes >= 30)
	|| minutes < guide_times_last_minutes
	|| last_ofs != xofs) {
		guide_times_last_minutes = minutes;
		last_ofs = xofs;
		strftime(timestr, 25, "%b/%d", ltime);
		mvpw_set_array_col(prog_widget, 0, timestr, NULL);
		if(minutes < 30) {
			strftime(timestr, 25, hour_format, ltime);
			mvpw_set_array_col(prog_widget, 1, timestr, NULL);
			strftime(timestr, 25, halfhour_format, ltime);
			mvpw_set_array_col(prog_widget, 2, timestr, NULL);
			nexthr = curtime + 60*60;
			ltime = localtime(&nexthr);
			strftime(timestr, 25, hour_format, ltime);
			mvpw_set_array_col(prog_widget, 3, timestr, NULL);
		}
		else {
			strftime(timestr, 25, halfhour_format, ltime);
			mvpw_set_array_col(prog_widget, 1, timestr, NULL);
			nexthr = curtime + 60*60;
			ltime = localtime(&nexthr);
			strftime(timestr, 25, hour_format, ltime);
			mvpw_set_array_col(prog_widget, 2, timestr, NULL);
			strftime(timestr, 25, halfhour_format, ltime);
			mvpw_set_array_col(prog_widget, 3, timestr, NULL);
		}
	}
	else
		rtrn = 0;
	
	mvpw_array_clear_dirty(widget);

	return rtrn;
}

void
mythtv_guide_reset_guide_times(void)
{
	guide_times_last_minutes = -1;
}

cmyth_chanlist_t
myth_release_chanlist(cmyth_chanlist_t cl)
{
	int i;
	if (cl) {
		for (i = 0; i < cmyth_chanlist_get_count(cl); i++) {
        		ref_release(cmyth_chanlist_get_item(cl, i));
		}
		ref_release(cl);
	}
	return NULL;
}

cmyth_tvguide_progs_t
myth_release_proglist(cmyth_tvguide_progs_t proglist)
{
	if(proglist) {
		ref_release(proglist->progs);
		ref_release(proglist);
	}
	return NULL;
}

#define MAX_TUNER 16
long
myth_tvguide_get_free_cardids(cmyth_conn_t control)
{
	long rtrn = 0;
	int i;
	cmyth_conn_t ctrl = ref_hold(control);
	cmyth_recorder_t rec;
	static int last_tuner = MAX_TUNER;

	for (i=1; i<last_tuner; i++) {
		/*
		fprintf(stderr, "Looking for recorder %d\n", i);
		*/
		if ((rec = cmyth_conn_get_recorder_from_num(ctrl,i)) == NULL) {
			last_tuner = i;
			break;
		}
		if(cmyth_recorder_is_recording(rec) != 1) {
			rtrn |= 1<<(i-1);
			/*
			PRINTF("** SSDEBUG recorder %d is free\n", i);
			*/
		}
		ref_release(rec);
	}
	ref_release(ctrl);


	return rtrn;
}

long
myth_tvguide_get_active_card(cmyth_recorder_t rec)
{
	long rtrn = 0;

	if(rec)
		rtrn |= 1<<(rec->rec_id-1);
	
	/*
	PRINTF("** SSDEBUG recorder bitmap %ld is our active device\n", rtrn);
	*/

	return rtrn;
}

/*
 *
 */

static int chanid_compare_desc(const void *a, const void *b) {
	const cmyth_channel_t x = *(cmyth_channel_t *)a;
	const cmyth_channel_t y = *(cmyth_channel_t *)b;
	if (x->chanid < y->chanid)
	    return 1;
	else if (x->chanid > y->chanid)
	    return -1;
	else
	    return 0;
}

static int chanid_compare_asc(const void *a, const void *b) {
	const cmyth_channel_t x = *(cmyth_channel_t *)a;
	const cmyth_channel_t y = *(cmyth_channel_t *)b;
	if (x->chanid > y->chanid)
	    return 1;
	else if (x->chanid < y->chanid)
	    return -1;
	else
	    return 0;
}

cmyth_chanlist_t
myth_tvguide_load_channels(cmyth_database_t db, int sort_desc)
{
	cmyth_chanlist_t rtrn=cmyth_mysql_get_chanlist(db);
	if (sort_desc) {
	    qsort((cmyth_channel_t)cmyth_chanlist_get_item(rtrn,0),
	           cmyth_chanlist_get_count(rtrn),
	           sizeof(cmyth_channel_t),
	           chanid_compare_desc);
	}
	else {
	    qsort((cmyth_channel_t)cmyth_chanlist_get_item(rtrn,0),
	           cmyth_chanlist_get_count(rtrn),
	           sizeof(cmyth_channel_t),
	           chanid_compare_asc);
	}
		
	return rtrn;
}

