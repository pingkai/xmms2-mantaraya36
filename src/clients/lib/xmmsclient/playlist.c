/*  XMMS2 - X Music Multiplexer System
 *  Copyright (C) 2003-2006 XMMS2 Team
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "xmmsclient/xmmsclient.h"
#include "xmmsclientpriv/xmmsclient.h"
#include "xmmsclientpriv/xmmsclient_ipc.h"
#include "xmmsc/xmmsc_idnumbers.h"
#include "xmmsc/xmmsc_stdbool.h"
#include "xmmsc/xmmsc_stringport.h"

/**
 * @defgroup PlaylistControl PlaylistControl
 * @ingroup XMMSClient
 * @brief This controls the playlist.
 *
 * @{
 */

/**
 * Retrive the current position in the playlist
 */
xmmsc_result_t *
xmmsc_playlist_current_pos (xmmsc_connection_t *c)
{
	x_check_conn (c, NULL);

	return xmmsc_send_msg_no_arg (c, XMMS_IPC_OBJECT_PLAYLIST, XMMS_IPC_CMD_CURRENT_POS);
}

/**
 * List the existing playlists.
 */
xmmsc_result_t *
xmmsc_playlist_list (xmmsc_connection_t *c)
{
	return xmmsc_coll_list (c, XMMS_COLLECTION_NS_PLAYLISTS);
}

/**
 * Shuffles the current playlist.
 */
xmmsc_result_t *
xmmsc_playlist_shuffle (xmmsc_connection_t *c, const char *playlist)
{
	xmms_ipc_msg_t *msg;

	x_check_conn (c, NULL);

	/* default to the active playlist */
	if (playlist == NULL) {
		playlist = "_active";
	}

	msg = xmms_ipc_msg_new (XMMS_IPC_OBJECT_PLAYLIST, XMMS_IPC_CMD_SHUFFLE);
	xmms_ipc_msg_put_string (msg, playlist);

	return xmmsc_send_msg (c, msg);
}

/**
 * Sorts the playlist according to the property
 */
xmmsc_result_t *
xmmsc_playlist_sort (xmmsc_connection_t *c, const char *playlist, const char **properties)
{
	xmms_ipc_msg_t *msg;

	x_check_conn (c, NULL);
	x_api_error_if (!properties, "with a NULL property", NULL);

	/* default to the active playlist */
	if (playlist == NULL) {
		playlist = "_active";
	}

	msg = xmms_ipc_msg_new (XMMS_IPC_OBJECT_PLAYLIST, XMMS_IPC_CMD_SORT);
	xmms_ipc_msg_put_string (msg, playlist);
	xmms_ipc_msg_put_string (msg, *properties); /* FIXME: stringlist ? */

	return xmmsc_send_msg (c, msg);
}

/**
 * Clears the current playlist.
 */
xmmsc_result_t *
xmmsc_playlist_clear (xmmsc_connection_t *c, const char *playlist)
{
	xmms_ipc_msg_t *msg;

	x_check_conn (c, NULL);

	/* default to the active playlist */
	if (playlist == NULL) {
		playlist = "_active";
	}

	msg = xmms_ipc_msg_new (XMMS_IPC_OBJECT_PLAYLIST, XMMS_IPC_CMD_CLEAR);
	xmms_ipc_msg_put_string (msg, playlist);

	return xmmsc_send_msg (c, msg);
}

/**
 * Remove the given playlist.
 */
xmmsc_result_t *
xmmsc_playlist_remove (xmmsc_connection_t *c, const char *playlist)
{
	return xmmsc_coll_remove (c, playlist, XMMS_COLLECTION_NS_PLAYLISTS);
}


/**
 * List current playlist.
 */
xmmsc_result_t *
xmmsc_playlist_list_entries (xmmsc_connection_t *c, const char *playlist)
{
	xmms_ipc_msg_t *msg;

	x_check_conn (c, NULL);

	/* default to the active playlist */
	if (playlist == NULL) {
		playlist = "_active";
	}

	msg = xmms_ipc_msg_new (XMMS_IPC_OBJECT_PLAYLIST, XMMS_IPC_CMD_LIST);
	xmms_ipc_msg_put_string (msg, playlist);

	return xmmsc_send_msg (c, msg);
}

/**
 * Insert a medialib id at given position in playlist. 
 *
 * @param c The connection structure.
 * @param pos A position in the playlist
 * @param id A medialib id.
 *
 */
xmmsc_result_t *
xmmsc_playlist_insert_id (xmmsc_connection_t *c, const char *playlist, int pos, unsigned int id)
{
	xmms_ipc_msg_t *msg;

	x_check_conn (c, NULL);

	/* default to the active playlist */
	if (playlist == NULL) {
		playlist = "_active";
	}

	msg = xmms_ipc_msg_new (XMMS_IPC_OBJECT_PLAYLIST, XMMS_IPC_CMD_INSERT_ID);
	xmms_ipc_msg_put_string (msg, playlist);
	xmms_ipc_msg_put_uint32 (msg, pos);
	xmms_ipc_msg_put_uint32 (msg, id);

	return xmmsc_send_msg (c, msg);
}

/**
 * Insert entry at given position in playlist.
 *
 * @param c The connection structure.
 * @param pos A position in the playlist
 * @param url The URL to insert
 *
 */
xmmsc_result_t *
xmmsc_playlist_insert_url (xmmsc_connection_t *c, const char *playlist, int pos, const char *url)
{
	return xmmsc_playlist_insert_args (c, playlist, pos, url, 0, NULL);
}

/**
 * Insert entry at given position in playlist wit args.
 *
 * @param c The connection structure.
 * @param pos A position in the playlist
 * @param url The URL to insert
 * @param numargs The number of arguments
 * @param args array of numargs strings used as arguments
 */
xmmsc_result_t *
xmmsc_playlist_insert_args (xmmsc_connection_t *c, const char *playlist, int pos, const char *url, int numargs, const char **args)
{
	xmms_ipc_msg_t *msg;
	char *enc_url;

	x_check_conn (c, NULL);
	x_api_error_if (!url, "with a NULL url", NULL);

	enc_url = xmmsc_medialib_encode_url (url, numargs, args);
	if (!enc_url)
		return NULL;

	/* default to the active playlist */
	if (playlist == NULL) {
		playlist = "_active";
	}
	
	msg = xmms_ipc_msg_new (XMMS_IPC_OBJECT_PLAYLIST, XMMS_IPC_CMD_INSERT_URL);
	xmms_ipc_msg_put_string (msg, playlist);
	xmms_ipc_msg_put_uint32 (msg, pos);
	xmms_ipc_msg_put_string (msg, url);

	return xmmsc_send_msg (c, msg);
}

/**
 * Queries the medialib for media and inserts the matching ones to
 * the current playlist at the given position.
 *
 * @param c The connection structure.
 * @param playlist The playlist in which to insert the media.
 * @param pos A position in the playlist
 * @param coll The collection to find media in the medialib.
 */
xmmsc_result_t *
xmmsc_playlist_insert_collection (xmmsc_connection_t *c, const char *playlist,
                                  int pos, xmmsc_coll_t *coll)
{
	xmms_ipc_msg_t *msg;

	x_check_conn (c, NULL);

	/* default to the active playlist */
	if (playlist == NULL) {
		playlist = "_active";
	}

	msg = xmms_ipc_msg_new (XMMS_IPC_OBJECT_PLAYLIST, XMMS_IPC_CMD_INSERT_COLL);
	xmms_ipc_msg_put_string (msg, playlist);
	xmms_ipc_msg_put_uint32 (msg, pos);
	xmms_ipc_msg_put_collection (msg, coll);

	return xmmsc_send_msg (c, msg);

}



/**
 * Add a medialib id to the playlist. 
 *
 * @param c The connection structure.
 * @param id A medialib id.
 *
 */
xmmsc_result_t *
xmmsc_playlist_add_id (xmmsc_connection_t *c, const char *playlist, unsigned int id)
{
	xmms_ipc_msg_t *msg;

	x_check_conn (c, NULL);

	/* default to the active playlist */
	if (playlist == NULL) {
		playlist = "_active";
	}

	msg = xmms_ipc_msg_new (XMMS_IPC_OBJECT_PLAYLIST, XMMS_IPC_CMD_ADD_ID);
	xmms_ipc_msg_put_string (msg, playlist);
	xmms_ipc_msg_put_uint32 (msg, id);

	return xmmsc_send_msg (c, msg);
}

/**
 * Add the url to the playlist. The url should be absolute to the
 * server-side. Note that you will have to include the protocol for
 * the url to. ie: file://mp3/my_mp3s/first.mp3.
 *
 * @param c The connection structure.
 * @param url path.
 *
 */
xmmsc_result_t *
xmmsc_playlist_add_url (xmmsc_connection_t *c, const char *playlist, const char *url)
{
	return xmmsc_playlist_add_args (c, playlist, url, 0, NULL);
}

/**
 * Add the url to the playlist with arguments.
 *
 * @param c The connection structure.
 * @param url path.
 * @param numargs The number of arguments
 * @param args array of numargs strings used as arguments
 */
xmmsc_result_t *
xmmsc_playlist_add_args (xmmsc_connection_t *c, const char *playlist, const char *url, int nargs, const char **args)
{
	xmmsc_result_t *res;
	xmms_ipc_msg_t *msg;
	char *enc_url;

	x_check_conn (c, NULL);
	x_api_error_if (!url, "with a NULL url", NULL);

	enc_url = xmmsc_medialib_encode_url (url, nargs, args);
	if (!enc_url)
		return NULL;

	/* default to the active playlist */
	if (playlist == NULL) {
		playlist = "_active";
	}
	
	msg = xmms_ipc_msg_new (XMMS_IPC_OBJECT_PLAYLIST, XMMS_IPC_CMD_ADD_URL);
	xmms_ipc_msg_put_string (msg, playlist);
	xmms_ipc_msg_put_string (msg, enc_url);
	res = xmmsc_send_msg (c, msg);

	free (enc_url);

	return res;
}

/**
 * Queries the medialib for media and adds the matching ones to
 * the current playlist.
 *
 * @param c The connection structure.
 * @param playlist The playlist in which to add the media.
 * @param coll The collection to find media in the medialib.
 */
xmmsc_result_t *
xmmsc_playlist_add_collection (xmmsc_connection_t *c, const char *playlist,
                               xmmsc_coll_t *coll)
{
	xmms_ipc_msg_t *msg;

	x_check_conn (c, NULL);

	/* default to the active playlist */
	if (playlist == NULL) {
		playlist = "_active";
	}

	msg = xmms_ipc_msg_new (XMMS_IPC_OBJECT_PLAYLIST, XMMS_IPC_CMD_ADD_COLL);
	xmms_ipc_msg_put_string (msg, playlist);
	xmms_ipc_msg_put_collection (msg, coll);

	return xmmsc_send_msg (c, msg);

}

/**
 * Move a playlist entry to a new position (absolute move)
 */
xmmsc_result_t *
xmmsc_playlist_move_entry (xmmsc_connection_t *c, const char *playlist,
                           unsigned int cur_pos, unsigned int new_pos)
{
	xmms_ipc_msg_t *msg;

	x_check_conn (c, NULL);

	/* default to the active playlist */
	if (playlist == NULL) {
		playlist = "_active";
	}

	msg = xmms_ipc_msg_new (XMMS_IPC_OBJECT_PLAYLIST, XMMS_IPC_CMD_MOVE_ENTRY);
	xmms_ipc_msg_put_string (msg, playlist);
	xmms_ipc_msg_put_uint32 (msg, cur_pos);
	xmms_ipc_msg_put_uint32 (msg, new_pos);

	return xmmsc_send_msg (c, msg);

}

/**
 * Remove an entry from the playlist.
 *
 * @param pos The position that should be removed from the playlist.
 *
 * @sa xmmsc_playlist_list
 */
xmmsc_result_t *
xmmsc_playlist_remove_entry (xmmsc_connection_t *c, const char *playlist,
                             unsigned int pos)
{
	xmms_ipc_msg_t *msg;

	x_check_conn (c, NULL);

	/* default to the active playlist */
	if (playlist == NULL) {
		playlist = "_active";
	}

	msg = xmms_ipc_msg_new (XMMS_IPC_OBJECT_PLAYLIST, XMMS_IPC_CMD_REMOVE_ENTRY);
	xmms_ipc_msg_put_string (msg, playlist);
	xmms_ipc_msg_put_uint32 (msg, pos);

	return xmmsc_send_msg (c, msg);

}

/**
 * Request the playlist changed broadcast from the server. Everytime someone
 * manipulate the playlist this will be emitted.
 */
xmmsc_result_t *
xmmsc_broadcast_playlist_changed (xmmsc_connection_t *c)
{
	x_check_conn (c, NULL);

	return xmmsc_send_broadcast_msg (c, XMMS_IPC_SIGNAL_PLAYLIST_CHANGED);
}

/**
 * Request the playlist current pos broadcast. When the position
 * in the playlist is changed this will be called.
 */
xmmsc_result_t *
xmmsc_broadcast_playlist_current_pos (xmmsc_connection_t *c)
{
	x_check_conn (c, NULL);

	return xmmsc_send_broadcast_msg (c, XMMS_IPC_SIGNAL_PLAYLIST_CURRENT_POS);
}

/**
 * Set next entry in the playlist. Alter the position pointer.
 */
xmmsc_result_t *
xmmsc_playlist_set_next (xmmsc_connection_t *c, unsigned int pos)
{
	xmms_ipc_msg_t *msg;

	x_check_conn (c, NULL);

	msg = xmms_ipc_msg_new (XMMS_IPC_OBJECT_PLAYLIST, XMMS_IPC_CMD_SET_POS);
	xmms_ipc_msg_put_uint32 (msg, pos);

	return xmmsc_send_msg (c, msg);
}

/**
 * Same as #xmmsc_playlist_set_next but relative to the current postion.
 * -1 will back one and 1 will move to the next.
 */
xmmsc_result_t *
xmmsc_playlist_set_next_rel (xmmsc_connection_t *c, signed int pos)
{
	xmms_ipc_msg_t *msg;

	x_check_conn (c, NULL);

	msg = xmms_ipc_msg_new (XMMS_IPC_OBJECT_PLAYLIST, XMMS_IPC_CMD_SET_POS_REL);
	xmms_ipc_msg_put_uint32 (msg, pos);

	return xmmsc_send_msg (c, msg);
}

/**
 * Load a playlist as the current active playlist
 */
xmmsc_result_t *
xmmsc_playlist_load (xmmsc_connection_t *c, const char *name)
{
	xmms_ipc_msg_t *msg;

	x_check_conn (c, NULL);

	msg = xmms_ipc_msg_new (XMMS_IPC_OBJECT_PLAYLIST, XMMS_IPC_CMD_LOAD);
	xmms_ipc_msg_put_string (msg, name);

	return xmmsc_send_msg (c, msg);
}

/**
 * Request the playlist_loaded broadcast. This will be called
 * if a playlist is loaded server-side. The argument will be a string
 * with the playlist name.
 */
xmmsc_result_t *
xmmsc_broadcast_playlist_loaded (xmmsc_connection_t *c)
{
	x_check_conn (c, NULL);

	return xmmsc_send_broadcast_msg (c, XMMS_IPC_SIGNAL_PLAYLIST_LOADED);
}

/**
 * Export a serverside playlist to a format that could be read
 * from another mediaplayer.
 * @param conn The #xmmsc_connection_t
 * @param playlist Name of a serverside playlist
 * @param mime Mimetype of the export format.
 */
xmmsc_result_t *
xmmsc_playlist_export (xmmsc_connection_t *conn, const char *playlist, const char *mime)
{
	xmms_ipc_msg_t *msg;

	x_check_conn (conn, NULL);

	/* default to the active playlist */
	if (playlist == NULL) {
		playlist = "_active";
	}

	msg = xmms_ipc_msg_new (XMMS_IPC_OBJECT_PLAYLIST, XMMS_IPC_CMD_EXPORT);
	xmms_ipc_msg_put_string (msg, playlist);
	xmms_ipc_msg_put_string (msg, mime);

	return xmmsc_send_msg (conn, msg);
}

/**
 * Import a playlist from a playlist file.
 * @param conn The #xmmsc_connection_t
 * @param playlist The name of the new playlist.
 * @param url URL to the playlist file.
 */
xmmsc_result_t *
xmmsc_playlist_import (xmmsc_connection_t *conn, const char *playlist, const char *url)
{
	xmms_ipc_msg_t *msg;

	x_check_conn (conn, NULL);

	/* default to the active playlist */
	if (playlist == NULL) {
		playlist = "_active";
	}

	msg = xmms_ipc_msg_new (XMMS_IPC_OBJECT_PLAYLIST, XMMS_IPC_CMD_IMPORT);
	xmms_ipc_msg_put_string (msg, playlist);
	xmms_ipc_msg_put_string (msg, url);

	return xmmsc_send_msg (conn, msg);
}

/** @} */
