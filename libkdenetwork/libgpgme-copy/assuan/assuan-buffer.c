/* assuan-buffer.c - read and send data
 * Copyright (C) 2001, 2002, 2003, 2004, 2006 Free Software Foundation, Inc.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
 * USA. 
 */

#include <config.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#ifdef HAVE_W32_SYSTEM
#include <process.h>
#endif
#include "assuan-defs.h"


/* Extended version of write(2) to guarantee that all bytes are
   written.  Returns 0 on success or -1 and ERRNO on failure. */
static int
writen (assuan_context_t ctx, const char *buffer, size_t length)
{
  while (length)
    {
      ssize_t nwritten = ctx->io->writefnc (ctx, buffer, length);
      
      if (nwritten < 0)
        {
          if (errno == EINTR)
            continue;
          return -1; /* write error */
        }
      length -= nwritten;
      buffer += nwritten;
    }
  return 0;  /* okay */
}

/* Read an entire line. Returns 0 on success or -1 and ERRNo on
   failure.  EOF is indictated by setting the integer at address
   R_EOF.  */
static int
readline (assuan_context_t ctx, char *buf, size_t buflen,
	  int *r_nread, int *r_eof)
{
  size_t nleft = buflen;
  char *p;

  *r_eof = 0;
  *r_nread = 0;
  while (nleft > 0)
    {
      ssize_t n = ctx->io->readfnc (ctx, buf, nleft);

      if (n < 0)
        {
          if (errno == EINTR)
            continue;
          return -1; /* read error */
        }
      else if (!n)
        {
          *r_eof = 1;
          break; /* allow incomplete lines */
        }
      p = buf;
      nleft -= n;
      buf += n;
      *r_nread += n;

      p = memrchr (p, '\n', n);
      if (p)
        break; /* at least one full line available - that's enough for now */
    }

  return 0;
}


/* Function returns an Assuan error. */
assuan_error_t
_assuan_read_line (assuan_context_t ctx)
{
  char *line = ctx->inbound.line;
  int nread, atticlen;
  int rc;
  char *endp = 0;

  if (ctx->inbound.eof)
    return _assuan_error (-1);

  atticlen = ctx->inbound.attic.linelen;
  if (atticlen)
    {
      memcpy (line, ctx->inbound.attic.line, atticlen);
      ctx->inbound.attic.linelen = 0;

      endp = memchr (line, '\n', atticlen);
      if (endp)
	/* Found another line in the attic.  */
	{
	  rc = 0;
	  nread = atticlen;
	  atticlen = 0;
	}
      else
	/* There is pending data but not a full line.  */
        {
          assert (atticlen < LINELENGTH);
          rc = readline (ctx, line + atticlen,
			 LINELENGTH - atticlen, &nread, &ctx->inbound.eof);
        }
    }
  else
    /* No pending data.  */
    rc = readline (ctx, line, LINELENGTH,
                   &nread, &ctx->inbound.eof);
  if (rc)
    {
      if (ctx->log_fp)
	fprintf (ctx->log_fp, "%s[%u.%d] DBG: <- [Error: %s]\n",
		 assuan_get_assuan_log_prefix (),
                 (unsigned int)getpid (), ctx->inbound.fd,
                 strerror (errno));
      return _assuan_error (ASSUAN_Read_Error);
    }
  if (!nread)
    {
      assert (ctx->inbound.eof);
      if (ctx->log_fp)
	fprintf (ctx->log_fp, "%s[%u.%d] DBG: <- [EOF]\n",
		 assuan_get_assuan_log_prefix (),
                 (unsigned int)getpid (), ctx->inbound.fd);
      return _assuan_error (-1);
    }

  ctx->inbound.attic.pending = 0;
  nread += atticlen;

  if (! endp)
    endp = memchr (line, '\n', nread);

  if (endp)
    {
      unsigned monitor_result;
      int n = endp - line + 1;

      if (n < nread)
	/* LINE contains more than one line.  We copy it to the attic
	   now as handlers are allowed to modify the passed
	   buffer.  */
	{
	  int len = nread - n;
	  memcpy (ctx->inbound.attic.line, endp + 1, len);
	  ctx->inbound.attic.pending = memrchr (endp + 1, '\n', len) ? 1 : 0;
	  ctx->inbound.attic.linelen = len;
	}

      if (endp != line && endp[-1] == '\r')
	endp --;
      *endp = 0;

      ctx->inbound.linelen = endp - line;

      monitor_result = (ctx->io_monitor
                        ? ctx->io_monitor (ctx, 0,
                                           ctx->inbound.line,
                                           ctx->inbound.linelen)
                        : 0);
      if ( (monitor_result & 2) )
        ctx->inbound.linelen = 0;
      
      if (ctx->log_fp && !(monitor_result & 1))
	{
	  fprintf (ctx->log_fp, "%s[%u.%d] DBG: <- ",
		   assuan_get_assuan_log_prefix (),
                   (unsigned int)getpid (), ctx->inbound.fd);
	  if (ctx->confidential)
	    fputs ("[Confidential data not shown]", ctx->log_fp);
	  else
	    _assuan_log_print_buffer (ctx->log_fp,
				      ctx->inbound.line,
				      ctx->inbound.linelen);
	  putc ('\n', ctx->log_fp);
	}
      return 0;
    }
  else
    {
      if (ctx->log_fp)
	fprintf (ctx->log_fp, "%s[%u.%d] DBG: <- [Invalid line]\n",
		 assuan_get_assuan_log_prefix (),
                 (unsigned int)getpid (), ctx->inbound.fd);
      *line = 0;
      ctx->inbound.linelen = 0;
      return _assuan_error (ctx->inbound.eof 
                            ? ASSUAN_Line_Not_Terminated
                            : ASSUAN_Line_Too_Long);
    }
}


/* Read the next line from the client or server and return a pointer
   in *LINE to a buffer holding the line.  LINELEN is the length of
   *LINE.  The buffer is valid until the next read operation on it.
   The caller may modify the buffer.  The buffer is invalid (i.e. must
   not be used) if an error is returned.

   Returns 0 on success or an assuan error code.
   See also: assuan_pending_line().
*/
assuan_error_t
assuan_read_line (assuan_context_t ctx, char **line, size_t *linelen)
{
  assuan_error_t err;

  if (!ctx)
    return _assuan_error (ASSUAN_Invalid_Value);

  err = _assuan_read_line (ctx);
  *line = ctx->inbound.line;
  *linelen = ctx->inbound.linelen;
  return err;
}


/* Return true if a full line is buffered (i.e. an entire line may be
   read without any I/O).  */
int
assuan_pending_line (assuan_context_t ctx)
{
  return ctx && ctx->inbound.attic.pending;
}


assuan_error_t 
_assuan_write_line (assuan_context_t ctx, const char *prefix,
                    const char *line, size_t len)
{
  assuan_error_t rc = 0;
  size_t prefixlen = prefix? strlen (prefix):0;
  unsigned int monitor_result;

  /* Make sure that the line is short enough. */
  if (len + prefixlen + 2 > ASSUAN_LINELENGTH)
    {
      if (ctx->log_fp)
        fprintf (ctx->log_fp, "%s[%u.%d] DBG: -> "
                 "[supplied line too long -truncated]\n",
                 assuan_get_assuan_log_prefix (),
                 (unsigned int)getpid (), ctx->inbound.fd);
      if (prefixlen > 5)
        prefixlen = 5;
      if (len > ASSUAN_LINELENGTH - prefixlen - 2)
        len = ASSUAN_LINELENGTH - prefixlen - 2 - 1;
    }

  monitor_result = (ctx->io_monitor
                    ? ctx->io_monitor (ctx, 1, line, len)
                    : 0);

  /* Fixme: we should do some kind of line buffering.  */
  if (ctx->log_fp && !(monitor_result & 1))
    {
      fprintf (ctx->log_fp, "%s[%u.%d] DBG: -> ",
	       assuan_get_assuan_log_prefix (),
               (unsigned int)getpid (), ctx->inbound.fd);
      if (ctx->confidential)
	fputs ("[Confidential data not shown]", ctx->log_fp);
      else
        {
          if (prefixlen)
            _assuan_log_print_buffer (ctx->log_fp, prefix, prefixlen);
          _assuan_log_print_buffer (ctx->log_fp, line, len);
        }
      putc ('\n', ctx->log_fp);
    }

  if (prefixlen && !(monitor_result & 2))
    {
      rc = writen (ctx, prefix, prefixlen);
      if (rc)
        rc = _assuan_error (ASSUAN_Write_Error);
    }
  if (!rc && !(monitor_result & 2))
    {
      rc = writen (ctx, line, len);
      if (rc)
        rc = _assuan_error (ASSUAN_Write_Error);
      if (!rc)
        {
          rc = writen (ctx, "\n", 1);
          if (rc)
            rc = _assuan_error (ASSUAN_Write_Error);
        }
    }
  return rc;
}


assuan_error_t 
assuan_write_line (assuan_context_t ctx, const char *line)
{
  size_t len;
  const char *s;

  if (!ctx)
    return _assuan_error (ASSUAN_Invalid_Value);

  /* Make sure that we never take a LF from the user - this might
     violate the protocol. */
  s = strchr (line, '\n');
  len = s? (s-line) : strlen (line);

  if (ctx->log_fp && s)
    fprintf (ctx->log_fp, "%s[%u.%d] DBG: -> "
             "[supplied line contained a LF - truncated]\n",
             assuan_get_assuan_log_prefix (),
             (unsigned int)getpid (), ctx->inbound.fd);

  return _assuan_write_line (ctx, NULL, line, len);
}



/* Write out the data in buffer as datalines with line wrapping and
   percent escaping.  This function is used for GNU's custom streams. */
int
_assuan_cookie_write_data (void *cookie, const char *buffer, size_t orig_size)
{
  assuan_context_t ctx = cookie;
  size_t size = orig_size;
  char *line;
  size_t linelen;

  if (ctx->outbound.data.error)
    return 0;

  line = ctx->outbound.data.line;
  linelen = ctx->outbound.data.linelen;
  line += linelen;
  while (size)
    {
      unsigned int monitor_result;

      /* Insert data line header. */
      if (!linelen)
        {
          *line++ = 'D';
          *line++ = ' ';
          linelen += 2;
        }
      
      /* Copy data, keep space for the CRLF and to escape one character. */
      while (size && linelen < LINELENGTH-2-2)
        {
          if (*buffer == '%' || *buffer == '\r' || *buffer == '\n')
            {
              sprintf (line, "%%%02X", *(unsigned char*)buffer);
              line += 3;
              linelen += 3;
              buffer++;
            }
          else
            {
              *line++ = *buffer++;
              linelen++;
            }
          size--;
        }
      
      
      monitor_result = (ctx->io_monitor
                        ? ctx->io_monitor (ctx, 1,
                                           ctx->outbound.data.line, linelen)
                        : 0);

      if (linelen >= LINELENGTH-2-2)
        {
          if (ctx->log_fp && !(monitor_result & 1))
            {
	      fprintf (ctx->log_fp, "%s[%u.%d] DBG: -> ",
		       assuan_get_assuan_log_prefix (),
                       (unsigned int)getpid (), ctx->inbound.fd);

              if (ctx->confidential)
                fputs ("[Confidential data not shown]", ctx->log_fp);
              else 
                _assuan_log_print_buffer (ctx->log_fp, 
                                          ctx->outbound.data.line,
                                          linelen);
              putc ('\n', ctx->log_fp);
            }
          *line++ = '\n';
          linelen++;
          if ( !(monitor_result & 2)
               && writen (ctx, ctx->outbound.data.line, linelen))
            {
              ctx->outbound.data.error = _assuan_error (ASSUAN_Write_Error);
              return 0;
            }
          line = ctx->outbound.data.line;
          linelen = 0;
        }
    }

  ctx->outbound.data.linelen = linelen;
  return (int)orig_size;
}


/* Write out any buffered data 
   This function is used for GNU's custom streams */
int
_assuan_cookie_write_flush (void *cookie)
{
  assuan_context_t ctx = cookie;
  char *line;
  size_t linelen;
  unsigned int monitor_result;

  if (ctx->outbound.data.error)
    return 0;

  line = ctx->outbound.data.line;
  linelen = ctx->outbound.data.linelen;
  line += linelen;

  monitor_result = (ctx->io_monitor
                    ? ctx->io_monitor (ctx, 1,
                                       ctx->outbound.data.line, linelen)
                    : 0);
  
  if (linelen)
    {
      if (ctx->log_fp && !(monitor_result & 1))
	{
	  fprintf (ctx->log_fp, "%s[%u.%d] DBG: -> ",
		   assuan_get_assuan_log_prefix (),
                   (unsigned int)getpid (), ctx->inbound.fd);
	  if (ctx->confidential)
	    fputs ("[Confidential data not shown]", ctx->log_fp);
	  else
	    _assuan_log_print_buffer (ctx->log_fp,
				      ctx->outbound.data.line, linelen);
	  putc ('\n', ctx->log_fp);
	}
      *line++ = '\n';
      linelen++;
      if ( !(monitor_result & 2)
           && writen (ctx, ctx->outbound.data.line, linelen))
        {
          ctx->outbound.data.error = _assuan_error (ASSUAN_Write_Error);
          return 0;
        }
      ctx->outbound.data.linelen = 0;
    }
  return 0;
}


/**
 * assuan_send_data:
 * @ctx: An assuan context
 * @buffer: Data to send or NULL to flush
 * @length: length of the data to send/
 * 
 * This function may be used by the server or the client to send data
 * lines.  The data will be escaped as required by the Assuan protocol
 * and may get buffered until a line is full.  To force sending the
 * data out @buffer may be passed as NULL (in which case @length must
 * also be 0); however when used by a client this flush operation does
 * also send the terminating "END" command to terminate the reponse on
 * a INQUIRE response.  However, when assuan_transact() is used, this
 * function takes care of sending END itself.
 * 
 * Return value: 0 on success or an error code
 **/

assuan_error_t
assuan_send_data (assuan_context_t ctx, const void *buffer, size_t length)
{
  if (!ctx)
    return _assuan_error (ASSUAN_Invalid_Value);
  if (!buffer && length)
    return _assuan_error (ASSUAN_Invalid_Value);

  if (!buffer)
    { /* flush what we have */
      _assuan_cookie_write_flush (ctx);
      if (ctx->outbound.data.error)
        return ctx->outbound.data.error;
      if (!ctx->is_server)
        return assuan_write_line (ctx, "END");
    }
  else
    {
      _assuan_cookie_write_data (ctx, buffer, length);
      if (ctx->outbound.data.error)
        return ctx->outbound.data.error;
    }

  return 0;
}

assuan_error_t
assuan_sendfd (assuan_context_t ctx, int fd)
{
  /* It is explicitly allowed to use (NULL, -1) as a runtime test to
     check whether descriptor passing is available. */
  if (!ctx && fd == -1)
#ifdef USE_DESCRIPTOR_PASSING
    return 0;
#else
    return _assuan_error (ASSUAN_Not_Implemented);
#endif

  if (! ctx->io->sendfd)
    return set_error (ctx, Not_Implemented,
		      "server does not support sending and receiving "
		      "of file descriptors");
  return ctx->io->sendfd (ctx, fd);
}

assuan_error_t
assuan_receivefd (assuan_context_t ctx, int *fd)
{
  if (! ctx->io->receivefd)
    return set_error (ctx, Not_Implemented,
		      "server does not support sending and receiving "
		      "of file descriptors");
  return ctx->io->receivefd (ctx, fd);
}
