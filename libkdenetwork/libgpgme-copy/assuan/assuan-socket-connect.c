/* assuan-socket-connect.c - Assuan socket based client
 *	Copyright (C) 2002, 2003 Free Software Foundation, Inc.
 *
 * This file is part of Assuan.
 *
 * Assuan is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Assuan is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA 
 */

#include <config.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "assuan-defs.h"

#define LOG(format, args...) \
	fprintf (assuan_get_assuan_log_stream (), \
	         assuan_get_assuan_log_prefix (), \
	         "%s" format , ## args)

static int
do_finish (ASSUAN_CONTEXT ctx)
{
  if (ctx->inbound.fd != -1)
    {
      close (ctx->inbound.fd);
    }
  ctx->inbound.fd = -1;
  ctx->outbound.fd = -1;
  return 0;
}

static void
do_deinit (ASSUAN_CONTEXT ctx)
{
  do_finish (ctx);
}
/* Make a connection to the Unix domain socket NAME and return a new
   Assuan context in CTX.  SERVER_PID is currently not used but may
   become handy in the future.  */
AssuanError
assuan_socket_connect (ASSUAN_CONTEXT *r_ctx,
                       const char *name, pid_t server_pid)
{
  static struct assuan_io io = { _assuan_simple_read,
				 _assuan_simple_write };

  AssuanError err;
  ASSUAN_CONTEXT ctx;
  int fd;
  struct sockaddr_un srvr_addr;
  size_t len;

  if (!r_ctx || !name)
    return ASSUAN_Invalid_Value;
  *r_ctx = NULL;

  /* we require that the name starts with a slash, so that we can
     alter reuse this function for other socket types */
  if (*name != '/')
    return ASSUAN_Invalid_Value;
  if (strlen (name)+1 >= sizeof srvr_addr.sun_path)
    return ASSUAN_Invalid_Value;

  err = _assuan_new_context (&ctx); 
  if (err)
      return err;
  ctx->pid = server_pid; /* save it in case we need it later */
  ctx->deinit_handler = do_deinit;
  ctx->finish_handler = do_finish;

  fd = socket (PF_LOCAL, SOCK_STREAM, 0);
  if (fd == -1)
    {
      LOG ("can't create socket: %s\n", strerror (errno));
      _assuan_release_context (ctx);
      return ASSUAN_General_Error;
    }

  memset (&srvr_addr, 0, sizeof srvr_addr);
  srvr_addr.sun_family = AF_LOCAL;
  len = strlen (srvr_addr.sun_path) + 1;
  memcpy (srvr_addr.sun_path, name, len);
  len += (offsetof (struct sockaddr_un, sun_path));

  if (connect (fd, (struct sockaddr *) &srvr_addr, len) == -1)
    {
      LOG ("can't connect to `%s': %s\n", name, strerror (errno));
      _assuan_release_context (ctx);
      close (fd);
      return ASSUAN_Connect_Failed;
    }

  ctx->inbound.fd = fd;
  ctx->outbound.fd = fd;
  ctx->io = &io;

  /* initial handshake */
  {
    int okay, off;

    err = _assuan_read_from_server (ctx, &okay, &off);
    if (err)
      LOG ("can't connect to server: %s\n", assuan_strerror (err));
    else if (okay != 1)
      {
	LOG ("can't connect to server: `");
	_assuan_log_sanitized_string (ctx->inbound.line);
	fprintf (assuan_get_assuan_log_stream (), "'\n");
	err = ASSUAN_Connect_Failed;
      }
  }

  if (err)
    {
      assuan_disconnect (ctx); 
    }
  else
    *r_ctx = ctx;
  return 0;
}
