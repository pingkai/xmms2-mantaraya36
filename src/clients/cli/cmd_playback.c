/*  XMMS2 - X Music Multiplexer System
 *  Copyright (C) 2003	Peter Alm, Tobias Rundström, Anders Gustafsson
 * 
 *  PLUGINS ARE NOT CONSIDERED TO BE DERIVED WORK !!!
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
 */

#include "cmd_playback.h"
#include "common.h"


static void
do_reljump (xmmsc_connection_t *conn, gint where)
{
	xmmsc_result_t *res;

	res = xmmsc_playlist_set_next_rel (conn, where);
	xmmsc_result_wait (res);

	if (xmmsc_result_iserror (res)) {
		print_error ("Couldn't advance in playlist: %s",
		             xmmsc_result_get_error (res));
	}
	xmmsc_result_unref (res);

	res = xmmsc_playback_tickle (conn);
	xmmsc_result_wait (res);

	if (xmmsc_result_iserror (res)) {
		print_error ("%s", xmmsc_result_get_error (res));
	}
	xmmsc_result_unref (res);
}


void
cmd_play (xmmsc_connection_t *conn, gint argc, gchar **argv)
{
	xmmsc_result_t *res;
	
	res = xmmsc_playback_start (conn);
	xmmsc_result_wait (res);
	
	if (xmmsc_result_iserror (res)) {
		print_error ("Couldn't start playback: %s",
		             xmmsc_result_get_error (res));
	}
	xmmsc_result_unref (res);
}


void
cmd_stop (xmmsc_connection_t *conn, gint argc, gchar **argv)
{
	xmmsc_result_t *res;
	
	res = xmmsc_playback_stop (conn);
	xmmsc_result_wait (res);
	
	if (xmmsc_result_iserror (res)) {
		print_error ("Couldn't stop playback: %s",
		             xmmsc_result_get_error (res));
	}
	xmmsc_result_unref (res);
}


void
cmd_pause (xmmsc_connection_t *conn, gint argc, gchar **argv)
{
	xmmsc_result_t *res;

	res = xmmsc_playback_pause (conn);
	xmmsc_result_wait (res);

	if (xmmsc_result_iserror (res)) {
		print_error ("Couldn't pause playback: %s",
		             xmmsc_result_get_error (res));
	}
	xmmsc_result_unref (res);
}


void
cmd_next (xmmsc_connection_t *conn, gint argc, gchar **argv)
{
	do_reljump (conn, 1);
}


void
cmd_prev (xmmsc_connection_t *conn, gint argc, gchar **argv)
{
	do_reljump (conn, -1);
}


void
cmd_seek (xmmsc_connection_t *conn, gint argc, gchar **argv)
{
	xmmsc_result_t *res;
	guint id, ms, pt = 0;
	gint dur = 0;

	if (argc < 3) {
		print_error ("You need to specify a number of seconds. Usage:\n"
		             "xmms2 seek n  - seek to absolute position in song\n"
		             "xmms2 seek +n - seek n seconds forward in song\n"
                     "xmms2 seek -n - seek n seconds backwards");
	}

	res = xmmsc_playback_current_id (conn);
	xmmsc_result_wait (res);

	if (xmmsc_result_iserror (res)) {
		print_error ("%s", xmmsc_result_get_error (res));
	}

	if (!xmmsc_result_get_uint (res, &id)) {
		print_error ("Broken resultset");
	}
	xmmsc_result_unref (res);
	
	res = xmmsc_medialib_get_info (conn, id);
	xmmsc_result_wait (res);

	if (xmmsc_result_iserror (res)) {
		print_error ("%s", xmmsc_result_get_error (res));
	}

	if (!xmmsc_result_get_dict_entry_int32 (res, "duration", &dur)) {
		print_error ("Broken resultset");
	}
	xmmsc_result_unref (res);

	if (argv[2][0] == '+' || argv[2][0] == '-') {
		res = xmmsc_playback_playtime (conn);
		xmmsc_result_wait (res);

		if (xmmsc_result_iserror (res)) {
			print_error ("%s", xmmsc_result_get_error (res));
		}

		if (!xmmsc_result_get_uint (res, &pt))
			print_error ("Broken resultset");

		xmmsc_result_unref (res);
	}

	ms = pt + 1000 * strtol (argv[2], NULL, 10);

	if (dur && ms > dur) {
		print_info ("Skipping to next song");
		do_reljump (conn, 1);
	} else {
		res = xmmsc_playback_seek_ms (conn, ms);
		xmmsc_result_wait (res);

		if (xmmsc_result_iserror (res)) {
			print_error ("Couldn't seek to %d ms: %s", ms,
						 xmmsc_result_get_error (res));
		}
		xmmsc_result_unref (res);
	}
}


void
cmd_move (xmmsc_connection_t *conn, gint argc, gchar **argv)
{
	xmmsc_result_t *res;
	guint cur_pos, new_pos;

	if (argc < 4) {
		print_error ("You'll need to specifiy current and new position");
	}

	cur_pos = strtol (argv[2], NULL, 10);
	new_pos = strtol (argv[3], NULL, 10);

	res = xmmsc_playlist_move (conn, cur_pos, new_pos);
	xmmsc_result_wait (res);

	if (xmmsc_result_iserror (res)) {
		print_error ("Unable to move playlist entry: %s",
		             xmmsc_result_get_error (res));
	}
	xmmsc_result_unref (res);

	print_info ("Moved %u to %u", cur_pos, new_pos);
}


void
cmd_jump (xmmsc_connection_t *conn, gint argc, gchar **argv)
{
	xmmsc_result_t *res;

	if (argc < 3) {
		print_error ("You'll need to specify a ID to jump to.");
	}

	res = xmmsc_playlist_set_next (conn, strtol (argv[2], NULL, 10));
	xmmsc_result_wait (res);

	if (xmmsc_result_iserror (res)) {
		print_error ("Couldn't jump to that song: %s",
		             xmmsc_result_get_error (res));
	}
	xmmsc_result_unref (res);

	res = xmmsc_playback_tickle (conn);
	xmmsc_result_wait (res);

	if (xmmsc_result_iserror (res)) {
		print_error ("Couldn't go to next song: %s",
					 xmmsc_result_get_error (res));
	}
	xmmsc_result_unref (res);
}