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

#include <xmmsclient/xmmsclient.h>

#include <glib.h>
#include <stdlib.h>
#include <string.h>

#include "cli_infos.h"
#include "commands.h"
#include "command_trie.h"
#include "readline.h"


void
command_argument_free (void *x)
{
	/* FIXME: free value? */
	g_free (x);
}

void
command_dispatch (cli_infos_t *infos, gint argc, gchar **argv)
{
	gchar *after;
	command_action_t *action;
	action = command_trie_find (infos->commands, *argv);
	if (action) {

		/* FIXME: look at the error! */
		GOptionContext *context;
		GError *error = NULL;
		gint i;

		command_context_t *ctx;
		ctx = g_new0 (command_context_t, 1);

		/* Register a hashtable to receive flag values and pass them on */
		ctx->argc = argc;
		ctx->argv = argv;
		ctx->flags = g_hash_table_new_full (g_str_hash, g_str_equal,
		                                    g_free, command_argument_free);

		for (i = 0; action->argdefs[i].long_name; ++i) {
			command_argument_t *arg = g_new (command_argument_t, 1);

			/* FIXME: customizable default values? */
			switch (action->argdefs[i].arg) {
			case G_OPTION_ARG_INT:
				arg->type = COMMAND_ARGUMENT_TYPE_INT;
				arg->value.vint = 0;
				action->argdefs[i].arg_data = &arg->value.vint;
				break;

			case G_OPTION_ARG_STRING:
				arg->type = COMMAND_ARGUMENT_TYPE_STRING;
				arg->value.vstring = NULL;
				action->argdefs[i].arg_data = &arg->value.vstring;
				break;

			default:
				printf ("Trying to register a flag '%s' of invalid type!",
				        action->argdefs[i].long_name);
				break;
			}

			/* FIXME: check for duplicates */
			g_hash_table_insert (ctx->flags,
			                     g_strdup (action->argdefs[i].long_name), arg);
		}

		context = g_option_context_new (NULL);
		g_option_context_set_help_enabled (context, FALSE);  /* runs exit(0)! */
		g_option_context_add_main_entries (context, action->argdefs, NULL);
		g_option_context_parse (context, &ctx->argc, &ctx->argv, &error);
		g_option_context_free (context);

		/* Run action if connection status ok */
		if (!action->req_connection || cli_infos_connect (infos)) {
			cli_infos_loop_suspend (infos);
			action->callback (infos, ctx);
		}
	} else {
		printf ("Unknown command: '%s'\n", *argv);
		printf ("Type 'help' for usage.\n");
	}

}

void
loop_select (cli_infos_t *infos)
{
	fd_set rfds, wfds;
	gint modfds;
	gint xmms2fd;
	gint maxfds = 0;

	FD_ZERO(&rfds);
	FD_ZERO(&wfds);

	/* Listen to xmms2 if connected */
	if (infos->conn) {
		xmms2fd = xmmsc_io_fd_get (infos->conn);
		if (xmms2fd == -1) {
			/* FIXME: Handle error */
		}

		FD_SET(xmms2fd, &rfds);
		if (xmmsc_io_want_out (infos->conn)) {
			FD_SET(xmms2fd, &wfds);
		}

		if (maxfds < xmms2fd) {
			maxfds = xmms2fd;
		}
	}

	/* Listen to readline in shell mode */
	if (infos->mode == CLI_EXECUTION_MODE_SHELL) {
		FD_SET(STDINFD, &rfds);
		if (maxfds < STDINFD) {
			maxfds = STDINFD;
		}
	}

	modfds = select (maxfds + 1, &rfds, &wfds, NULL, NULL);

	if(modfds < 0) {
		/* FIXME: Handle error */
	}
	else if(modfds != 0) {
		/* Get/send data to xmms2 */
		if (infos->conn) {
			if(FD_ISSET(xmms2fd, &rfds)) {
				xmmsc_io_in_handle (infos->conn);
			}
			if(FD_ISSET(xmms2fd, &wfds)) {
				xmmsc_io_out_handle (infos->conn);
			}
		}

		/* User input found, read it */
		if (infos->mode == CLI_EXECUTION_MODE_SHELL
		    && FD_ISSET(STDINFD, &rfds)) {
			rl_callback_read_char ();
		}
	}
}

void
loop_once (cli_infos_t *infos, gint argc, gchar **argv)
{
	/* FIXME: need to dispatch after the loop is started */
	command_dispatch (infos, argc, argv);

	do {
		loop_select (infos);
	} while (infos->status == CLI_ACTION_STATUS_BUSY);
}

void
loop_run (cli_infos_t *infos)
{
	while (infos->status != CLI_ACTION_STATUS_FINISH) {
		loop_select (infos);
	}
}



gint
main (gint argc, gchar **argv)
{
	gint i;
	GError *error = NULL;
	GOptionContext *context;
	cli_infos_t *cli_infos;

	cli_infos = cli_infos_init (argc - 1, argv + 1);

	/* Execute command, if connection status is ok */
	if (cli_infos) {
		if (cli_infos->mode == CLI_EXECUTION_MODE_INLINE) {
			loop_once (cli_infos, argc - 1, argv + 1);
		} else {
			loop_run (cli_infos);
		}
	}

	cli_infos_free (cli_infos);

	return 0;
}