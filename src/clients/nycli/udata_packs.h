/*  XMMS2 - X Music Multiplexer System
 *  Copyright (C) 2003-2007 XMMS2 Team
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

#ifndef __UDATA_PACKS_H__
#define __UDATA_PACKS_H__

#include <glib.h>

#include "cli_infos.h"

typedef struct pack_infos_playlist_St pack_infos_playlist_t;
pack_infos_playlist_t *pack_infos_playlist (cli_infos_t *infos, gchar *playlist);
void unpack_infos_playlist (pack_infos_playlist_t *pack, cli_infos_t **infos, gchar **playlist);
void free_infos_playlist (pack_infos_playlist_t *pack);

typedef struct pack_infos_playlist_pos_St pack_infos_playlist_pos_t;
pack_infos_playlist_pos_t *pack_infos_playlist_pos (cli_infos_t *infos, gchar *playlist, gint pos);
void unpack_infos_playlist_pos (pack_infos_playlist_pos_t *pack, cli_infos_t **infos, gchar **playlist, gint *pos);
void free_infos_playlist_pos (pack_infos_playlist_pos_t *pack);

typedef struct pack_infos_playlist_config_St pack_infos_playlist_config_t;
pack_infos_playlist_config_t *pack_infos_playlist_config (cli_infos_t *infos, gchar *playlist, gint history, gint upcoming, xmmsc_coll_type_t type, gchar *input);
void unpack_infos_playlist_config (pack_infos_playlist_config_t *pack, cli_infos_t **infos, gchar **playlist, gint *history, gint *upcoming, xmmsc_coll_type_t *type, gchar **input);
void free_infos_playlist_config (pack_infos_playlist_config_t *pack);

typedef struct pack_infos_ctx_St pack_infos_ctx_t;
pack_infos_ctx_t *pack_infos_ctx (cli_infos_t *infos, command_context_t *ctx);
void unpack_infos_ctx(pack_infos_ctx_t *pack, cli_infos_t **infos,command_context_t **ctx);
void free_infos_ctx(pack_infos_ctx_t *pack);



#endif /* __UDATA_PACKS__ */