/*

stdsoap2.c[pp] 2.7.0d

Runtime environment.

gSOAP XML Web services tools
Copyright (C) 2000-2004, Robert van Engelen, Genivia, Inc., All Rights Reserved.

Contributors:

Wind River Systems, Inc., for the following additions (marked WR[...]):
  - vxWorks compatible
  - Support for IPv6.

--------------------------------------------------------------------------------
gSOAP public license.

The contents of this file are subject to the gSOAP Public License Version 1.3
(the "License"); you may not use this file except in compliance with the
License. You may obtain a copy of the License at
http://www.cs.fsu.edu/~engelen/soaplicense.html
Software distributed under the License is distributed on an "AS IS" basis,
WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
for the specific language governing rights and limitations under the License.

The Initial Developer of the Original Code is Robert A. van Engelen.
Copyright (C) 2000-2004, Robert van Engelen, Genivia, Inc., All Rights Reserved.
--------------------------------------------------------------------------------
GPL license.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place, Suite 330, Boston, MA 02111-1307 USA

Author contact information:
engelen@genivia.com / engelen@acm.org
--------------------------------------------------------------------------------

Installation note:

Win32 build needs winsock.dll (Visual C++ "wsock32.lib")
To do this in Visual C++ 6.0, go to "Project", "settings", select the "Link"
tab (the project file needs to be selected in the file view) and add
"wsock32.lib" to the "Object/library modules" entry

On Mac OS X with gcc (GCC) 3.1 20020420 (prerelease) you MUST compile with
-fstack_check when using -O2 because gcc 3.1 has a bug that smashes the stack
when locally allocated data exceeds 64K.

*/

#include "stdsoap2.h"

#ifdef __cplusplus
SOAP_SOURCE_STAMP("@(#) stdsoap2.cpp ver 2.7.0c 2004-09-27 12:00:00 GMT")
extern "C" {
#else
SOAP_SOURCE_STAMP("@(#) stdsoap2.c ver 2.7.0c 2004-09-27 12:00:00 GMT")
#endif

/* 8bit character representing unknown/nonrepresentable character data (e.g. not supported by current locale) */
#ifndef SOAP_UNKNOWN_CHAR
#define SOAP_UNKNOWN_CHAR (127)
#endif

/*      EOF=-1 */
#define SOAP_LT (soap_wchar)(-2) /* XML character '<' */
#define SOAP_TT (soap_wchar)(-3) /* XML character '</' */
#define SOAP_GT (soap_wchar)(-4) /* XML character '>' */
#define SOAP_QT (soap_wchar)(-5) /* XML character '"' */
#define SOAP_AP (soap_wchar)(-6) /* XML character ''' */

#define soap_blank(c)		((c) >= 0 && (c) <= 32)
#define soap_notblank(c)	((c) > 32)
#define soap_hash_ptr(p)	(((unsigned long)(p) >> 3) & (SOAP_PTRHASH - 1))

static int soap_isxdigit(int);
static soap_wchar soap_char(struct soap*);
static soap_wchar soap_getchunkchar(struct soap*);
static void soap_update_ptrs(struct soap*, char*, char*, long);
static int soap_has_copies(struct soap*, const char*, const char*);
static struct soap_ilist *soap_hlookup(struct soap*, const char*);
static void soap_init_iht(struct soap*);
static void soap_free_iht(struct soap*);
static void soap_init_pht(struct soap*);
static void soap_free_pht(struct soap*);
static int soap_set_error(struct soap*, const char*, const char*, const char*, int);
static const char *soap_set_validation_fault(struct soap*, const char*, const char*);
static int soap_copy_fault(struct soap*, const char*, const char*, const char*);
static int soap_getattrval(struct soap*, char*, size_t, soap_wchar);
static void soap_set_local_namespaces(struct soap*);
static int soap_isnumeric(struct soap*, const char*);
static void *fplugin(struct soap*, const char*);
static const char *soap_decode(char*, size_t, const char*, const char*);

#ifndef WITH_LEAN
static time_t soap_timegm(struct tm*);
#endif

#ifdef SOAP_DEBUG
static void soap_init_logs(struct soap*);
static void soap_close_logfile(struct soap*, int);
static void soap_set_logfile(struct soap*, int, const char*);
#endif

#ifdef WITH_FAST
static int soap_append_lab(struct soap*, const char*, size_t);
#endif

#ifndef WITH_LEANER
static struct soap_multipart *soap_new_multipart(struct soap*, struct soap_multipart**, struct soap_multipart**, char*, size_t);
static int soap_putdimefield(struct soap*, const char*, size_t);
static char *soap_getdimefield(struct soap*, size_t);
static void soap_select_mime_boundary(struct soap*);
static int soap_valid_mime_boundary(struct soap*);
#endif

#ifdef WITH_GZIP
static int soap_getgziphdr(struct soap*);
#endif

#ifdef WITH_OPENSSL
static int ssl_auth_init(struct soap*);
static int ssl_verify_callback(int, X509_STORE_CTX*);
static int ssl_password(char*, int, int, void *);
static const char *ssl_error(struct soap*, int);
/* This callback is included for future references. It should not be deleted
static DH *ssl_tmp_dh(SSL*, int, int);
*/
#endif

static const char *soap_strerror(struct soap*);
static const char *tcp_error(struct soap*);
static const char *http_error(struct soap*, int);
static int http_post(struct soap*, const char*, const char*, int, const char*, const char*, size_t);
static int http_get(struct soap*);
static int http_send_header(struct soap*, const char*);
static int http_post_header(struct soap*, const char*, const char*);
static int http_response(struct soap*, int, size_t);
static int http_parse(struct soap*);
static int http_parse_header(struct soap*, const char*, const char*);
#ifndef MAC_CARBON
static int tcp_gethost(struct soap*, const char *addr, struct in_addr *inaddr);
static int tcp_connect(struct soap*, const char *endpoint, const char *host, int port);
static int tcp_accept(struct soap*, int, struct sockaddr*, int*);
static int tcp_disconnect(struct soap*);
static int tcp_closesocket(struct soap*, SOAP_SOCKET);
static int tcp_shutdownsocket(struct soap*, SOAP_SOCKET, int);
static int fsend(struct soap*, const char*, size_t);
static size_t frecv(struct soap*, char*, size_t);
#endif

/* WR[ */
#ifdef VXWORKS
static int vx_nonblocking = TRUE; /* ioctl argument */
#endif
/* ]WR */

#if defined(PALM) && !defined(PALM_2)
unsigned short errno;
#endif

#ifndef PALM_1
static const char soap_env1[42] = "http://schemas.xmlsoap.org/soap/envelope/";
static const char soap_enc1[42] = "http://schemas.xmlsoap.org/soap/encoding/";
static const char soap_env2[40] = "http://www.w3.org/2003/05/soap-envelope";
static const char soap_enc2[40] = "http://www.w3.org/2003/05/soap-encoding";
static const char soap_rpc[35] = "http://www.w3.org/2003/05/soap-rpc";
#endif

#ifndef PALM_1
const struct soap_double_nan soap_double_nan = {0xFFFFFFFF, 0xFFFFFFFF};
static const char soap_base64o[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char soap_base64i[81] = "\76XXX\77\64\65\66\67\70\71\72\73\74\75XXXXXXX\00\01\02\03\04\05\06\07\10\11\12\13\14\15\16\17\20\21\22\23\24\25\26\27\30\31XXXXXX\32\33\34\35\36\37\40\41\42\43\44\45\46\47\50\51\52\53\54\55\56\57\60\61\62\63";
#endif

static const char soap_padding[3] = "\0\0";
#define SOAP_STR_PADDING (soap_padding)
#define SOAP_STR_EOS (soap_padding)

#ifndef WITH_LEAN
static const struct soap_code_map html_entity_codes[] = /* entities for XHTML parsing */
{ { 160, "nbsp" },
  { 161, "iexcl" },
  { 162, "cent" },
  { 163, "pound" },
  { 164, "curren" },
  { 165, "yen" },
  { 166, "brvbar" },
  { 167, "sect" },
  { 168, "uml" },
  { 169, "copy" },
  { 170, "ordf" },
  { 171, "laquo" },
  { 172, "not" },
  { 173, "shy" },
  { 174, "reg" },
  { 175, "macr" },
  { 176, "deg" },
  { 177, "plusmn" },
  { 178, "sup2" },
  { 179, "sup3" },
  { 180, "acute" },
  { 181, "micro" },
  { 182, "para" },
  { 183, "middot" },
  { 184, "cedil" },
  { 185, "sup1" },
  { 186, "ordm" },
  { 187, "raquo" },
  { 188, "frac14" },
  { 189, "frac12" },
  { 190, "frac34" },
  { 191, "iquest" },
  { 192, "Agrave" },
  { 193, "Aacute" },
  { 194, "Acirc" },
  { 195, "Atilde" },
  { 196, "Auml" },
  { 197, "Aring" },
  { 198, "AElig" },
  { 199, "Ccedil" },
  { 200, "Egrave" },
  { 201, "Eacute" },
  { 202, "Ecirc" },
  { 203, "Euml" },
  { 204, "Igrave" },
  { 205, "Iacute" },
  { 206, "Icirc" },
  { 207, "Iuml" },
  { 208, "ETH" },
  { 209, "Ntilde" },
  { 210, "Ograve" },
  { 211, "Oacute" },
  { 212, "Ocirc" },
  { 213, "Otilde" },
  { 214, "Ouml" },
  { 215, "times" },
  { 216, "Oslash" },
  { 217, "Ugrave" },
  { 218, "Uacute" },
  { 219, "Ucirc" },
  { 220, "Uuml" },
  { 221, "Yacute" },
  { 222, "THORN" },
  { 223, "szlig" },
  { 224, "agrave" },
  { 225, "aacute" },
  { 226, "acirc" },
  { 227, "atilde" },
  { 228, "auml" },
  { 229, "aring" },
  { 230, "aelig" },
  { 231, "ccedil" },
  { 232, "egrave" },
  { 233, "eacute" },
  { 234, "ecirc" },
  { 235, "euml" },
  { 236, "igrave" },
  { 237, "iacute" },
  { 238, "icirc" },
  { 239, "iuml" },
  { 240, "eth" },
  { 241, "ntilde" },
  { 242, "ograve" },
  { 243, "oacute" },
  { 244, "ocirc" },
  { 245, "otilde" },
  { 246, "ouml" },
  { 247, "divide" },
  { 248, "oslash" },
  { 249, "ugrave" },
  { 250, "uacute" },
  { 251, "ucirc" },
  { 252, "uuml" },
  { 253, "yacute" },
  { 254, "thorn" },
  { 255, "yuml" },
  {   0, NULL }
};
#endif

#ifndef WITH_LEAN
static const struct soap_code_map h_error_codes[] =
{
#ifdef HOST_NOT_FOUND   
  { HOST_NOT_FOUND, "Host not found" },
#endif
#ifdef TRY_AGAIN
  { TRY_AGAIN, "Try Again" },
#endif
#ifdef NO_RECOVERY  
  { NO_RECOVERY, "No Recovery" },
#endif
#ifdef NO_DATA
  { NO_DATA, "No Data" },
#endif
#ifdef NO_ADDRESS
  { NO_ADDRESS, "No Address" },
#endif
  { 0, NULL }
};
#endif

#ifndef WITH_LEAN
static const struct soap_code_map h_http_error_codes[] =
{ { 201, "Created" },
  { 202, "Accepted" },
  { 203, "Non-Authoritative Information" },
  { 204, "No Content" },
  { 205, "Reset Content" },
  { 206, "Partial Content" },
  { 300, "Multiple Choices" },
  { 301, "Moved Permanently" },
  { 302, "Found" },
  { 303, "See Other" },
  { 304, "Not Modified" },
  { 305, "Use Proxy" },
  { 307, "Temporary Redirect" },
  { 400, "Bad Request" },
  { 401, "Unauthorized" },
  { 402, "Payment Required" },
  { 403, "Forbidden" },
  { 404, "Not Found" },
  { 405, "Method Not Allowed" },
  { 406, "Not Acceptable" },
  { 407, "Proxy Authentication Required" },
  { 408, "Request Time-out" },
  { 409, "Conflict" },
  { 410, "Gone" },
  { 411, "Length Required" },
  { 412, "Precondition Failed" },
  { 413, "Request Entity Too Large" },
  { 414, "Request-URI Too Large" },
  { 415, "Unsupported Media Type" },
  { 416, "Requested range not satisfiable" },
  { 417, "Expectation Failed" },
  { 500, "Internal Server Error" },
  { 501, "Not Implemented" },
  { 502, "Bad Gateway" },
  { 503, "Service Unavailable" },
  { 504, "Gateway Time-out" },
  { 505, "HTTP Version not supported" },
  {   0, NULL }
};
#endif

#ifdef WITH_OPENSSL
static const struct soap_code_map h_ssl_error_codes[] =
{
#define _SSL_ERROR(e) { e, #e }
  _SSL_ERROR(SSL_ERROR_SSL),
  _SSL_ERROR(SSL_ERROR_ZERO_RETURN),
  _SSL_ERROR(SSL_ERROR_WANT_READ),
  _SSL_ERROR(SSL_ERROR_WANT_WRITE),
  _SSL_ERROR(SSL_ERROR_WANT_CONNECT),
  _SSL_ERROR(SSL_ERROR_WANT_X509_LOOKUP),
  _SSL_ERROR(SSL_ERROR_SYSCALL),
  { 0, NULL }
};
#endif

#ifndef WITH_LEANER
static const struct soap_code_map mime_codes[] =
{ { SOAP_MIME_7BIT,		"7bit" },
  { SOAP_MIME_8BIT,		"8bit" },
  { SOAP_MIME_BINARY,		"binary" },
  { SOAP_MIME_QUOTED_PRINTABLE, "quoted-printable" },
  { SOAP_MIME_BASE64,		"base64" },
  { SOAP_MIME_IETF_TOKEN,	"ietf-token" },
  { SOAP_MIME_X_TOKEN,		"x-token" },
  { 0,				NULL }
};
#endif

#ifdef WIN32
static int tcp_done = 0;
#endif

/******************************************************************************/
#ifndef MAC_CARBON
#ifndef PALM_1
static int
fsend(struct soap *soap, const char *s, size_t n)
{ register int nwritten;
#if defined(__cplusplus) && !defined(WITH_LEAN)
  if (soap->os)
  { soap->os->write(s, n);
    if (soap->os->good())
      return SOAP_OK;
    return SOAP_EOF;
  }
#endif
  while (n)
  { if (soap_valid_socket(soap->socket))
    { 
#ifndef WITH_LEAN
      if (soap->send_timeout)
      { struct timeval timeout;
        fd_set fd;
        if (soap->send_timeout > 0)
        { timeout.tv_sec = soap->send_timeout;
          timeout.tv_usec = 0;
        }
        else
        { timeout.tv_sec = -soap->send_timeout/1000000;
          timeout.tv_usec = -soap->send_timeout%1000000;
        }
        FD_ZERO(&fd);
        FD_SET((SOAP_SOCKET)soap->socket, &fd);
        for (;;)
        { register int r = select((SOAP_SOCKET)(soap->socket + 1), NULL, &fd, &fd, &timeout);
          if (r > 0)
            break;
          if (!r)
          { soap->errnum = 0;
            return SOAP_EOF;
          }
          if (soap_socket_errno != SOAP_EINTR)
          { soap->errnum = soap_socket_errno;
            return SOAP_EOF;
          }
        }
      }
#endif
#ifdef WITH_OPENSSL
      if (soap->ssl)
        nwritten = SSL_write(soap->ssl, s, n);
      else
#endif
#ifndef PALM
        nwritten = send((SOAP_SOCKET)soap->socket, s, n, soap->socket_flags);
#else
        nwritten = send((SOAP_SOCKET)soap->socket, (void*)s, n, soap->socket_flags);
#endif
      if (nwritten <= 0)
      {
#ifdef WITH_OPENSSL
	int err;
        if (soap->ssl && (err = SSL_get_error(soap->ssl, nwritten)) != SSL_ERROR_NONE && err != SSL_ERROR_WANT_READ && err != SSL_ERROR_WANT_WRITE)
          return SOAP_EOF;
#endif
        if (soap_socket_errno != SOAP_EINTR && soap_socket_errno != SOAP_EWOULDBLOCK && soap_socket_errno != SOAP_EAGAIN)
        { soap->errnum = soap_socket_errno;
          return SOAP_EOF;
        }
        nwritten = 0; /* and call write() again */
      }
    }
    else
    {
#ifdef WITH_FASTCGI
      nwritten = fwrite((void*)s, 1, n, stdout);
      fflush(stdout);
#else
#ifdef UNDER_CE
      nwritten = fwrite(s, 1, n, soap->sendfd);
#else
/* WR[ */
#ifdef VXWORKS
#ifdef WMW_RPM_IO
      if (soap->rpmreqid)
          {
          httpBlockPut(soap->rpmreqid, s, n); 
          nwritten = n;
          }
      else
          {
          nwritten = fwrite(s, sizeof(char), n, fdopen(soap->sendfd, "w"));
          }
#else
      nwritten = fwrite(s, sizeof(char), n, fdopen(soap->sendfd, "w"));
#endif /* WMW_RPM_IO */
#else
/* ]WR */
      nwritten = write((SOAP_SOCKET)soap->sendfd, s, n);
/* WR[ */
#endif
/* ]WR */
#endif
#endif
      if (nwritten <= 0)
      { if (soap_errno != SOAP_EINTR && soap_errno != SOAP_EWOULDBLOCK && soap_errno != SOAP_EAGAIN)
        { soap->errnum = soap_errno;
          return SOAP_EOF;
        }
        nwritten = 0; /* and call write() again */
      }
    }
    n -= nwritten;
    s += nwritten;
  }
  return SOAP_OK;
}
#endif
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
int
SOAP_FMAC2
soap_flush_raw(struct soap *soap, const char *s, size_t n)
{ if ((soap->mode & SOAP_IO) == SOAP_IO_STORE)
  { register char *t;
    if (!(t = (char*)soap_push_block(soap, n)))
      return soap->error = SOAP_EOM;
    memcpy(t, s, n);
    if (soap->fpreparesend)
      return soap->fpreparesend(soap, s, n);
    return SOAP_OK;
  }
  if ((soap->mode & SOAP_IO) == SOAP_IO_CHUNK)
  { char t[16];
    sprintf(t, "\r\n%lX\r\n" + (soap->chunksize ? 0 : 2), (unsigned long)n);
    DBGMSG(SENT, t, strlen(t));
    if ((soap->error = soap->fsend(soap, t, strlen(t))))
      return soap->error;
    soap->chunksize += n;
  }
  DBGMSG(SENT, s, n);
  return soap->error = soap->fsend(soap, s, n);
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
int
SOAP_FMAC2
soap_flush(struct soap *soap)
{ if (soap->bufidx)
  {
#ifdef WITH_ZLIB
    if (soap->mode & SOAP_ENC_ZLIB)
    { soap->d_stream.next_in = (Byte*)soap->buf;
      soap->d_stream.avail_in = (unsigned int)soap->bufidx;
#ifdef WITH_GZIP
      soap->z_crc = crc32(soap->z_crc, (Byte*)soap->buf, (unsigned int)soap->bufidx);
#endif
      do
      { DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Deflating %u bytes\n", soap->d_stream.avail_in));
        if (deflate(&soap->d_stream, Z_NO_FLUSH) != Z_OK)
        { DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Unable to deflate: %s\n", soap->d_stream.msg?soap->d_stream.msg:""));
          return soap->error = SOAP_ZLIB_ERROR;
        }
        if (!soap->d_stream.avail_out)
        { if (soap_flush_raw(soap, soap->z_buf, SOAP_BUFLEN))
            return soap->error;
          soap->d_stream.next_out = (Byte*)soap->z_buf;
          soap->d_stream.avail_out = SOAP_BUFLEN;
        }
      } while (soap->d_stream.avail_in);
    }
    else
#endif
    if (soap_flush_raw(soap, soap->buf, soap->bufidx))
      return soap->error;
    soap->bufidx = 0;
  }
  return SOAP_OK;
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
int
SOAP_FMAC2
soap_send_raw(struct soap *soap, const char *s, size_t n)
{ if (!n)
    return SOAP_OK;
  if (soap->mode & SOAP_IO_LENGTH)
  { soap->count += n;
    if (soap->fpreparesend && (soap->mode & SOAP_IO) != SOAP_IO_STORE)
      return soap->fpreparesend(soap, s, n);
    return SOAP_OK;
  }
  if (soap->mode & SOAP_IO)
  { register size_t i = SOAP_BUFLEN - soap->bufidx;
    while (n >= i)
    { memcpy(soap->buf + soap->bufidx, s, i);
      soap->bufidx = SOAP_BUFLEN;
      if (soap_flush(soap))
        return soap->error;
      s += i;
      n -= i;
      i = SOAP_BUFLEN;
    }
    memcpy(soap->buf + soap->bufidx, s, n);
    soap->bufidx += n;
    return SOAP_OK;
  }
  return soap_flush_raw(soap, s, n);
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
int
SOAP_FMAC2
soap_send(struct soap *soap, const char *s)
{ if (s)
    return soap_send_raw(soap, s, strlen(s));
  return SOAP_OK;
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
int
SOAP_FMAC2
soap_send2(struct soap *soap, const char *s1, const char *s2)
{ if (soap_send(soap, s1))
    return soap->error;
  return soap_send(soap, s2);
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
int
SOAP_FMAC2
soap_send3(struct soap *soap, const char *s1, const char *s2, const char *s3)
{ if (soap_send(soap, s1)
   || soap_send(soap, s2))
    return soap->error;
  return soap_send(soap, s3);
}
#endif

/******************************************************************************/
#ifndef MAC_CARBON
#ifndef PALM_1
static size_t
frecv(struct soap *soap, char *s, size_t n)
{ register int r;
  soap->errnum = 0;
#if defined(__cplusplus) && !defined(WITH_LEAN)
  if (soap->is)
  { if (soap->is->good())
      return soap->is->read(s, n).gcount();
    return 0;
  }
#endif
  if (soap_valid_socket(soap->socket))
  { for (;;)
    { 
#ifndef WITH_LEAN
      struct timeval timeout;
      fd_set fd;
      if (soap->recv_timeout)
      { if (soap->recv_timeout > 0)
        { timeout.tv_sec = soap->recv_timeout;
          timeout.tv_usec = 0;
        }
        else
        { timeout.tv_sec = -soap->recv_timeout/1000000;
          timeout.tv_usec = -soap->recv_timeout%1000000;
        }
        FD_ZERO(&fd);
        FD_SET((SOAP_SOCKET)soap->socket, &fd);
        for (;;)
        { r = select((SOAP_SOCKET)(soap->socket + 1), &fd, NULL, &fd, &timeout);
          if (r > 0)
            break;
          if (r == 0)
            return 0;
          if (soap_socket_errno != SOAP_EINTR)
          { soap->errnum = soap_socket_errno;
            return 0;
          }
        }
      }
#endif
#ifdef WITH_OPENSSL
      if (soap->ssl)
      { int err;
	r = SSL_read(soap->ssl, s, n);
        if ((err = SSL_get_error(soap->ssl, r)) == SSL_ERROR_NONE)
          return (size_t)r;
	if (err != SSL_ERROR_WANT_READ && err != SSL_ERROR_WANT_WRITE)
          return 0;
      }
      else
#endif
      { r = recv((SOAP_SOCKET)soap->socket, s, n, soap->socket_flags);
        if (r >= 0)
          return (size_t)r;
        if (soap_socket_errno != SOAP_EINTR && soap_socket_errno != SOAP_EAGAIN)
        { soap->errnum = soap_socket_errno;
          return 0;
        }
      }
#ifndef WITH_LEAN
      { struct timeval timeout;
        fd_set fd;
        timeout.tv_sec = 0;
        timeout.tv_usec = 10000;
        FD_ZERO(&fd);
        FD_SET((SOAP_SOCKET)soap->socket, &fd);
        r = select((SOAP_SOCKET)(soap->socket + 1), &fd, NULL, &fd, &timeout);
        if (r < 0 && soap_socket_errno != SOAP_EINTR)
        { soap->errnum = soap_socket_errno;
          return 0;
        }
      }
#endif
    }
  }
#ifdef WITH_FASTCGI
  return fread(s, 1, n, stdin);
#else
#ifdef UNDER_CE
  return fread(s, 1, n, soap->recvfd);
#else
/* WR[ */
#ifdef WMW_RPM_IO
  if (soap->rpmreqid)
    r = httpBlockRead(soap->rpmreqid, s, n);
  else
    r = read(soap->recvfd, s, n);
  if (r >= 0)
    return r;
  return 0;
#else
/* ]WR */
  r = read((SOAP_SOCKET)soap->recvfd, s, n);
  if (r >= 0)
    return (size_t)r;
  soap->errnum = soap_errno;
  return 0;
/* WR[ */
#endif
/* ]WR */
#endif
#endif
}
#endif
#endif

/******************************************************************************/
#ifndef PALM_1
static soap_wchar
soap_getchunkchar(struct soap *soap)
{ if (soap->bufidx < soap->buflen)
    return soap->buf[soap->bufidx++];
  soap->bufidx = 0;
  soap->buflen = soap->chunkbuflen = soap->frecv(soap, soap->buf, SOAP_BUFLEN);
  DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Read %u bytes\n", (unsigned int)soap->buflen));
  DBGMSG(RECV, soap->buf, soap->buflen);
  if (soap->buflen)
    return soap->buf[soap->bufidx++];
  return EOF;
}
#endif

/******************************************************************************/
#ifndef PALM_1
static int
soap_isxdigit(int c)
{ switch (c)
  { case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
      return 1;
  }
  return 0;
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
int
SOAP_FMAC2
soap_recv_raw(struct soap *soap)
{ register size_t ret;
#ifdef WITH_ZLIB
  if (soap->mode & SOAP_ENC_ZLIB)
  { if (soap->d_stream.next_out == Z_NULL)
      return EOF;
    if (soap->d_stream.avail_in || !soap->d_stream.avail_out)
    { register int r;
      DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Inflating\n"));
      soap->d_stream.next_out = (Byte*)soap->buf;
      soap->d_stream.avail_out = SOAP_BUFLEN;
      r = inflate(&soap->d_stream, Z_NO_FLUSH);
      if (r == Z_OK || r == Z_STREAM_END)
      { soap->bufidx = 0;
        soap->buflen = SOAP_BUFLEN - soap->d_stream.avail_out;
        if (soap->zlib_in == SOAP_ZLIB_GZIP)
          soap->z_crc = crc32(soap->z_crc, (Byte*)soap->buf, (unsigned int)soap->buflen);
        if (r == Z_STREAM_END)
        { DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Inflated %lu->%lu bytes\n", soap->d_stream.total_in, soap->d_stream.total_out));
          soap->z_ratio_in = (float)soap->d_stream.total_in / (float)soap->d_stream.total_out;
          soap->d_stream.next_out = Z_NULL;
        }
        if (soap->buflen)
        { soap->count += soap->buflen;
          return SOAP_OK;
        }
      }
      else if (r != Z_BUF_ERROR)
      { DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Inflate error: %s\n", soap->d_stream.msg?soap->d_stream.msg:""));
        soap->d_stream.next_out = Z_NULL;
        return EOF;
      }
    }
zlib_again:
    if ((soap->mode & SOAP_IO) == SOAP_IO_CHUNK && !soap->chunksize)
    { memcpy(soap->buf, soap->z_buf, SOAP_BUFLEN);
      soap->buflen = soap->z_buflen;
    }
  }
#endif
  if ((soap->mode & SOAP_IO) == SOAP_IO_CHUNK) /* read HTTP chunked transfer */
  { 
chunk_again:
    if (soap->chunksize)
    { soap->buflen = ret = soap->frecv(soap, soap->buf, soap->chunksize > SOAP_BUFLEN ? SOAP_BUFLEN : soap->chunksize);
      DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Getting chunk: read %u bytes\n", (unsigned int)ret));
      DBGMSG(RECV, soap->buf, ret);
      soap->bufidx = 0;
      soap->chunksize -= ret;
    }
    else
    { register soap_wchar c;
      char *t, tmp[8];
      t = tmp;
      if (!soap->chunkbuflen)
      { soap->chunkbuflen = ret = soap->frecv(soap, soap->buf, SOAP_BUFLEN);
        DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Read %u bytes\n", (unsigned int)ret));
        DBGMSG(RECV, soap->buf, ret);
        soap->bufidx = 0;
        if (!ret)
          return EOF;
      }
      else
        soap->bufidx = soap->buflen;
      soap->buflen = soap->chunkbuflen;
      DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Getting chunk size (%u %u)\n", (unsigned int)soap->bufidx, (unsigned int)soap->buflen));
      while (!soap_isxdigit((int)(c = soap_getchunkchar(soap))))
        if ((int)c == EOF)
	  return EOF;
      do
        *t++ = (char)c;
      while (soap_isxdigit((int)(c = soap_getchunkchar(soap))) && t - tmp < 7);
      while ((int)c != EOF && c != '\n')
        c = soap_getchunkchar(soap);
      if ((int)c == EOF)
        return EOF;
      *t = '\0';
      soap->chunksize = soap_strtoul(tmp, &t, 16);
      if (!soap->chunksize)
      { soap->chunkbuflen = 0;
        DBGLOG(TEST, SOAP_MESSAGE(fdebug, "End of chunked message\n"));
	while ((int)c != EOF && c != '\n')
          c = soap_getchunkchar(soap);
        return EOF;
      }
      soap->buflen = soap->bufidx + soap->chunksize;
      DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Moving buf len to %u (%u %s)\n", (unsigned int)soap->buflen, (unsigned int)soap->bufidx, tmp));
      if (soap->buflen > soap->chunkbuflen)
      { soap->buflen = soap->chunkbuflen;
        soap->chunksize -= soap->buflen - soap->bufidx;
        soap->chunkbuflen = 0;
        DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Passed end of buffer for chunked HTTP (%lu bytes left)\n", (unsigned long)(soap->buflen - soap->bufidx)));
      }
      else if (soap->chunkbuflen)
        soap->chunksize = 0;
      ret = soap->buflen - soap->bufidx;
      if (!ret)
        goto chunk_again;
    }
  }
  else
  { soap->bufidx = 0;
    soap->buflen = ret = soap->frecv(soap, soap->buf, SOAP_BUFLEN);
    DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Read %u bytes\n", (unsigned int)ret));
    DBGMSG(RECV, soap->buf, ret);
  }
  if (soap->fpreparerecv && (soap->error = soap->fpreparerecv(soap, soap->buf, ret)))
    return soap->error;
#ifdef WITH_ZLIB
  if (soap->mode & SOAP_ENC_ZLIB)
  { register int r;
    memcpy(soap->z_buf, soap->buf, SOAP_BUFLEN);
    soap->d_stream.next_in = (Byte*)(soap->z_buf + soap->bufidx);
    soap->d_stream.avail_in = (unsigned int)ret;
    soap->d_stream.next_out = (Byte*)soap->buf;
    soap->d_stream.avail_out = SOAP_BUFLEN;
    r = inflate(&soap->d_stream, Z_NO_FLUSH);
    if (r == Z_OK || r == Z_STREAM_END)
    { soap->bufidx = 0;
      soap->z_buflen = soap->buflen;
      soap->buflen = ret = SOAP_BUFLEN - soap->d_stream.avail_out;
      if (soap->zlib_in == SOAP_ZLIB_GZIP)
        soap->z_crc = crc32(soap->z_crc, (Byte*)soap->buf, (unsigned int)soap->buflen);
      DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Inflated %u bytes\n", (unsigned int)ret));
      if (!ret)
        goto zlib_again;
      if (r == Z_STREAM_END)
      { DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Inflated %lu->%lu bytes\n", soap->d_stream.total_in, soap->d_stream.total_out));
        soap->z_ratio_in = (float)soap->d_stream.total_in / (float)soap->d_stream.total_out;
        soap->d_stream.next_out = Z_NULL;
      }
    }
    else
    { DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Unable to inflate: (%d) %s\n", r, soap->d_stream.msg?soap->d_stream.msg:""));
      soap->d_stream.next_out = Z_NULL;
      return EOF;
    }
  }
#endif
  soap->count += ret;
  return !ret;
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
int
SOAP_FMAC2
soap_recv(struct soap *soap)
{ 
#ifndef WITH_LEANER
  if (soap->mode & SOAP_ENC_DIME)
  { if (soap->dime.buflen)
    { char *s;
      int i;
      unsigned char tmp[12];
      DBGLOG(TEST, SOAP_MESSAGE(fdebug, "DIME hdr for chunked DIME is in buffer\n"));
      soap->count += soap->dime.buflen - soap->buflen;
      soap->buflen = soap->dime.buflen;
      DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Skip padding (%ld bytes)\n", -(long)soap->dime.size&3));
      for (i = -(long)soap->dime.size&3; i > 0; i--)
      { soap->bufidx++;
        if (soap->bufidx >= soap->buflen)
          if (soap_recv_raw(soap))
            return EOF;
      }
      DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Get DIME hdr for next chunk\n"));
      s = (char*)tmp;
      for (i = 12; i > 0; i--)
      { *s++ = soap->buf[soap->bufidx++];
        if (soap->bufidx >= soap->buflen)
          if (soap_recv_raw(soap))
            return EOF;
      }
      soap->dime.flags = tmp[0] & 0x7;
      soap->dime.size = ((size_t)tmp[8] << 24) | ((size_t)tmp[9] << 16) | ((size_t)tmp[10] << 8) | ((size_t)tmp[11]);
      DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Get DIME chunk (%u bytes)\n", (unsigned int)soap->dime.size));
      if (soap->dime.flags & SOAP_DIME_CF)
      { DBGLOG(TEST, SOAP_MESSAGE(fdebug, "More chunking\n"));
        soap->dime.chunksize = soap->dime.size;
        if (soap->buflen - soap->bufidx >= soap->dime.size)
        { soap->dime.buflen = soap->buflen;
          soap->buflen = soap->bufidx + soap->dime.chunksize;
        }
        else
          soap->dime.chunksize -= soap->buflen - soap->bufidx;
      }
      else
      { DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Last chunk\n"));
        soap->dime.buflen = 0;
        soap->dime.chunksize = 0;
      }
      soap->count = soap->buflen - soap->bufidx;
      DBGLOG(TEST, SOAP_MESSAGE(fdebug, "%u bytes remaining\n", (unsigned int)soap->count));
      return SOAP_OK;
    }
    if (soap->dime.chunksize)
    { DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Get next DIME hdr for chunked DIME (%u bytes chunk)\n", (unsigned int)soap->dime.chunksize));
      if (soap_recv_raw(soap))
        return EOF;
      if (soap->buflen - soap->bufidx >= soap->dime.chunksize)
      { soap->dime.buflen = soap->buflen;
        soap->count -= soap->buflen - soap->bufidx - soap->dime.chunksize;
        soap->buflen = soap->bufidx + soap->dime.chunksize;
      }
      else
        soap->dime.chunksize -= soap->buflen - soap->bufidx;
      DBGLOG(TEST, SOAP_MESSAGE(fdebug, "%lu bytes remaining, count=%u\n", (unsigned long)(soap->buflen-soap->bufidx), (unsigned int)soap->count));
      return SOAP_OK;
    }
  }
#endif
  return soap_recv_raw(soap);
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
soap_wchar
SOAP_FMAC2
soap_getchar(struct soap *soap)
{ register soap_wchar c;
  if (soap->ahead)
  { c = soap->ahead;
    soap->ahead = 0;
    return c;
  }
  return soap_get1(soap);
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
const struct soap_code_map*
SOAP_FMAC2
soap_code(const struct soap_code_map *map, const char *str)
{ while (map->string)
  { if (!strcmp(str, map->string)) /* case sensitive */
      return map;
    map++;
  }
  return NULL;
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
long
SOAP_FMAC2
soap_int_code(const struct soap_code_map *map, const char *str, long other)
{ while (map->string)
  { if (!soap_tag_cmp(str, map->string)) /* case insensitive */
      return map->code;
    map++;
  }
  return other;
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
const char*
SOAP_FMAC2
soap_str_code(const struct soap_code_map *map, long code)
{ while (map->code != code && map->string)
    map++;
  return map->string;
}
#endif

/******************************************************************************/
#ifndef PALM_1
static soap_wchar
soap_char(struct soap *soap)
{ char tmp[8];
  register int i;
  register soap_wchar c;
  register char *s = tmp;
  for (i = 0; i < 7; i++)
  { c = soap_get1(soap);
    if (c == ';' || (int)c == EOF)
      break;
    *s++ = (char)c;
  }
  *s = '\0';
  if (*tmp == '#')
  { if (tmp[1] == 'x' || tmp[1] == 'X')
      return soap_strtol(tmp + 2, NULL, 16);
    return atol(tmp + 1);
  }
  if (!strcmp(tmp, "lt"))
    return '<';
  if (!strcmp(tmp, "gt"))
    return '>';
  if (!strcmp(tmp, "amp"))
    return '&';
  if (!strcmp(tmp, "quot"))
    return '"';
  if (!strcmp(tmp, "apos"))
    return '\'';
#ifndef WITH_LEAN
  return (soap_wchar)soap_int_code(html_entity_codes, tmp, SOAP_UNKNOWN_CHAR);
#else
  return SOAP_UNKNOWN_CHAR; /* use this to represent unknown code */
#endif
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
soap_wchar
SOAP_FMAC2
soap_get(struct soap *soap)
{ register soap_wchar c;
  c = soap->ahead;
  if (c)
    soap->ahead = 0;
  else
    c = soap_get1(soap);
  for (;;)
  { if (soap->cdata)
    { if (c == ']')
      { c = soap_get1(soap);
        if (c == ']')
        { soap->cdata = 0;
          soap_get1(soap); /* skip > */
          c = soap_get1(soap);
        }
	else
        { soap_revget1(soap);
          return ']';
        }
      }
      else
        return c;
    }
    switch (c)
    { case '<':
        do c = soap_get1(soap);
        while (soap_blank(c));
        if (c == '!' || c == '?' || c == '%')
        { if (c == '!')
          { c = soap_get1(soap);
            if (c == '[')
            { do c = soap_get1(soap);
              while ((int)c != EOF && c != '[');
              if ((int)c == EOF)
                break;
              soap->cdata = 1;
              c = soap_get1(soap);
	      continue;
            }
            if (c == '-' && (c = soap_get1(soap)) == '-')
            { do
              { c = soap_get1(soap);
                if (c == '-' && (c = soap_get1(soap)) == '-')
                  break;
              } while ((int)c != EOF);
            }
          }
          while ((int)c != EOF && c != '>')
            c = soap_get1(soap);
	  if ((int)c == EOF)
	    break;
          c = soap_get1(soap);
          continue;
        }
        if (c == '/')
          return SOAP_TT;
        soap_revget1(soap);
        return SOAP_LT;
      case '>':
        return SOAP_GT;
      case '"':
        return SOAP_QT;
      case '\'':
        return SOAP_AP;
      case '&':
        return soap_char(soap) | 0x80000000;
    }
    break;
  }
  return c;
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
soap_wchar
SOAP_FMAC2
soap_advance(struct soap *soap)
{ register soap_wchar c;
  while ((int)((c = soap_get(soap)) != EOF) && c != SOAP_LT && c != SOAP_TT)
    ;
  return c;
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
soap_wchar
SOAP_FMAC2
soap_skip(struct soap *soap)
{ register soap_wchar c;
  do c = soap_get(soap);
  while (soap_blank(c));
  return c;
}
#endif

/******************************************************************************/
#ifndef WITH_LEANER
#ifndef PALM_1
SOAP_FMAC1
int
SOAP_FMAC2
soap_move(struct soap *soap, long n)
{ DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Moving %ld bytes forward\n", (long)n));
  for (; n > 0; n--)
    if ((int)soap_getchar(soap) == EOF)
      return SOAP_EOF;
  return SOAP_OK;
}
#endif
#endif

/******************************************************************************/
#ifndef WITH_LEANER
#ifndef PALM_1
SOAP_FMAC1
size_t
SOAP_FMAC2
soap_tell(struct soap *soap)
{ return soap->count - soap->buflen + soap->bufidx - (soap->ahead != 0);
}
#endif
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
int
SOAP_FMAC2
soap_pututf8(struct soap *soap, register unsigned long c)
{ char tmp[16];
  if (c > 0 && c < 0x80)
  { *tmp = (char)c;
    return soap_send_raw(soap, tmp, 1);
  }
#ifndef WITH_LEAN
  if (soap->mode & SOAP_XML_CANONICAL)
  { register char *t = tmp;
    if (c < 0x0800)
      *t++ = (char)(0xC0 | ((c >> 6) & 0x1F));
    else
    { if (c < 0x010000)
        *t++ = (char)(0xE0 | ((c >> 12) & 0x0F));
      else
      { if (c < 0x200000)
          *t++ = (char)(0xF0 | ((c >> 18) & 0x07));
        else
        { if (c < 0x04000000)
            *t++ = (char)(0xF8 | ((c >> 24) & 0x03));
          else
          { *t++ = (char)(0xFC | ((c >> 30) & 0x01));
            *t++ = (char)(0x80 | ((c >> 24) & 0x3F));
          }
          *t++ = (char)(0x80 | ((c >> 18) & 0x3F));
        }     
        *t++ = (char)(0x80 | ((c >> 12) & 0x3F));
      }
      *t++ = (char)(0x80 | ((c >> 6) & 0x3F));
    }
    *t++ = (char)(0x80 | (c & 0x3F));
    *t = '\0';
  }
  else
#endif
    sprintf(tmp, "&#%lu;", c);
  return soap_send(soap, tmp);
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
soap_wchar
SOAP_FMAC2
soap_getutf8(struct soap *soap)
{ register soap_wchar c, c1, c2, c3, c4;
  c = soap_get(soap);
  if (c < 0x80 || (soap->mode & SOAP_ENC_LATIN))
    return c;
  c1 = soap_get(soap);
  if (c1 < 0x80)
  { soap_unget(soap, c1);
    return c;
  }
  c1 &= 0x3F;
  if (c < 0xE0)
    return ((soap_wchar)(c & 0x1F) << 6) | c1;
  c2 = (soap_wchar)soap_get1(soap) & 0x3F;
  if (c < 0xF0)
    return ((soap_wchar)(c & 0x0F) << 12) | (c1 << 6) | c2;
  c3 = (soap_wchar)soap_get1(soap) & 0x3F;
  if (c < 0xF8)
    return ((soap_wchar)(c & 0x07) << 18) | (c1 << 12) | (c2 << 6) | c3;
  c4 = (soap_wchar)soap_get1(soap) & 0x3F;
  if (c < 0xFC)
    return ((soap_wchar)(c & 0x03) << 24) | (c1 << 18) | (c2 << 12) | (c3 << 6) | c4;
  return ((soap_wchar)(c & 0x01) << 30) | (c1 << 24) | (c2 << 18) | (c3 << 12) | (c4 << 6) | (soap_wchar)(soap_get1(soap) & 0x3F);
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
int
SOAP_FMAC2
soap_puthex(struct soap *soap, const unsigned char *s, int n)
{ /* TODO: serialize to DOM (as an option) using new soap_s2hex() */
  char d[2];
  register int i;
  for (i = 0; i < n; i++)
  { register int m = *s++;
    d[0] = (char)((m >> 4) + (m > 159 ? '7' : '0'));
    m &= 0x0F;
    d[1] = (char)(m + (m > 9 ? '7' : '0'));
    if (soap_send_raw(soap, d, 2))
      return soap->error;
  }
  return SOAP_OK;
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
unsigned char*
SOAP_FMAC2
soap_gethex(struct soap *soap, int *n)
{
#ifdef WITH_FAST
  soap->labidx = 0;
  for (;;)
  { register char *s;
    register int i, k;
    if (soap_append_lab(soap, NULL, 0))
      return NULL;
    s = soap->labbuf + soap->labidx;
    k = soap->lablen - soap->labidx;
    soap->labidx = soap->lablen;
    for (i = 0; i < k; i++)
    { register char d1, d2;
      register soap_wchar c;
      c = soap_get(soap);
      if (soap_isxdigit(c))
      { d1 = (char)c;
        c = soap_get(soap); 
	if (soap_isxdigit(c))
          d2 = (char)c;
        else 
        { soap->error = SOAP_TYPE;
          return NULL;
        }
      }
      else
      { unsigned char *p;
        soap_unget(soap, c);
        if (n)
          *n = (int)(soap->lablen - k + i);
        p = (unsigned char*)soap_malloc(soap, soap->lablen - k + i);
	if (p)
	  memcpy(p, soap->labbuf, soap->lablen - k + i);
        return p;
      }
      *s++ = ((d1 >= 'A' ? (d1 & 0x7) + 9 : d1 - '0') << 4) + (d2 >= 'A' ? (d2 & 0x7) + 9 : d2 - '0');
    }
  }
#else
  if (soap_new_block(soap))
    return NULL;
  for (;;)
  { register int i;
    register char *s = (char*)soap_push_block(soap, SOAP_BLKLEN);
    if (!s)
    { soap_end_block(soap);
      return NULL;
    }
    for (i = 0; i < SOAP_BLKLEN; i++)
    { register char d1, d2;
      register soap_wchar c = soap_get(soap);
      if (soap_isxdigit(c))
      { d1 = (char)c;
        c = soap_get(soap); 
	if (soap_isxdigit(c))
          d2 = (char)c;
        else 
        { soap_end_block(soap);
	  soap->error = SOAP_TYPE;
          return NULL;
        }
      }
      else
      { unsigned char *p;
        soap_unget(soap, c);
        if (n)
          *n = soap_size_block(soap, i);
        p = (unsigned char*)soap_save_block(soap, NULL, 0);
        return p;
      }
      *s++ = ((d1 >= 'A' ? (d1 & 0x7) + 9 : d1 - '0') << 4) + (d2 >= 'A' ? (d2 & 0x7) + 9 : d2 - '0');
    }
  }
#endif
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
int
SOAP_FMAC2
soap_putbase64(struct soap *soap, const unsigned char *s, int n)
{ register int i;
  register unsigned long m;
  char d[4];
  if (!s)
    return SOAP_OK;
#ifdef WITH_DOM
  if ((soap->mode & SOAP_XML_DOM) && soap->dom)
  { if (!(soap->dom->data = soap_s2base64(soap, s, soap->dom->data, n)))
      return soap->error;
    return SOAP_OK;
  }
#endif
  for (; n > 2; n -= 3, s += 3)
  { m = s[0];
    m = (m << 8) | s[1];
    m = (m << 8) | s[2];
    for (i = 4; i > 0; m >>= 6)
      d[--i] = soap_base64o[m & 0x3F];
    if (soap_send_raw(soap, d, 4))
      return soap->error;
  }
  if (n > 0)
  { m = 0;
    for (i = 0; i < n; i++)
      m = (m << 8) | *s++;
    for (; i < 3; i++)
      m <<= 8;
    for (i++; i > 0; m >>= 6)
      d[--i] = soap_base64o[m & 0x3F];
    for (i = 3; i > n; i--)
      d[i] = '=';
    if (soap_send_raw(soap, d, 4))
      return soap->error;
  }
  return SOAP_OK;
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
unsigned char*
SOAP_FMAC2
soap_getbase64(struct soap *soap, int *n, int malloc_flag)
{ 
#ifdef WITH_FAST
  soap->labidx = 0;
  for (;;)
  { register int i, k;
    register char *s;
    if (soap_append_lab(soap, NULL, 2))
      return NULL;
    s = soap->labbuf + soap->labidx;
    k = 3 * ((soap->lablen - soap->labidx) / 3);
    soap->labidx = 3 * (soap->lablen / 3);
    if (!s)
      return NULL;
    for (i = 0; i < k; i += 3)
    { register unsigned long m = 0;
      register int j = 0;
      do
      { register soap_wchar c = soap_get(soap);
        if (c == '=' || c < 0)
        { unsigned char *p;
          switch (j)
          { case 2:
              *s++ = (char)((m >> 4) & 0xFF);
              i++;
              break;
            case 3:
              *s++ = (char)((m >> 10) & 0xFF);
              *s++ = (char)((m >> 2) & 0xFF);
              i += 2;
          }
          if (n)
            *n = (int)(soap->lablen - k + i);
          p = (unsigned char*)soap_malloc(soap, soap->lablen - k + i);
	  if (p)
	    memcpy(p, soap->labbuf, soap->lablen - k + i);
          if (c >= 0)
          { while ((int)((c = soap_get(soap)) != EOF) && c != SOAP_LT && c != SOAP_TT)
              ;
	  }
          soap_unget(soap, c);
          return p;
        }
        c -= '+';
        if (c >= 0 && c <= 79)
        { m = (m << 6) + soap_base64i[c];
          j++;
        }
      } while (j < 4);
      *s++ = (char)((m >> 16) & 0xFF);
      *s++ = (char)((m >> 8) & 0xFF);
      *s++ = (char)(m & 0xFF);
    }
  }
#else
  if (soap_new_block(soap))
    return NULL;
  for (;;)
  { register int i;
    register char *s = (char*)soap_push_block(soap, 3 * SOAP_BLKLEN); /* must be multiple of 3 */
    if (!s)
    { soap_end_block(soap);
      return NULL;
    }
    for (i = 0; i < SOAP_BLKLEN; i++)
    { register unsigned long m = 0;
      register int j = 0;
      do
      { register soap_wchar c = soap_get(soap);
        if (c == '=' || c < 0)
        { unsigned char *p;
          i *= 3;
          switch (j)
          { case 2:
              *s++ = (char)((m >> 4) & 0xFF);
              i++;
              break;
            case 3:
              *s++ = (char)((m >> 10) & 0xFF);
              *s++ = (char)((m >> 2) & 0xFF);
              i += 2;
          }
          if (n)
	    *n = (int)soap_size_block(soap, i);
          p = (unsigned char*)soap_save_block(soap, NULL, 0);
          if (c >= 0)
          { while ((int)((c = soap_get(soap)) != EOF) && c != SOAP_LT && c != SOAP_TT)
              ;
	  }
          soap_unget(soap, c);
          return p;
        }
        c -= '+';
        if (c >= 0 && c <= 79)
        { m = (m << 6) + soap_base64i[c];
          j++;
        }
      } while (j < 4);
      *s++ = (char)((m >> 16) & 0xFF);
      *s++ = (char)((m >> 8) & 0xFF);
      *s++ = (char)(m & 0xFF);
    }
  }
#endif
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
char *
SOAP_FMAC2
soap_strdup(struct soap *soap, const char *s)
{ char *t = NULL;
  if (s && (t = (char*)soap_malloc(soap, strlen(s) + 1)))
    strcpy(t, s);
  return t;
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
int
SOAP_FMAC2
soap_new_block(struct soap *soap)
{ struct soap_blist *p;
  DBGLOG(TEST, SOAP_MESSAGE(fdebug, "New block sequence (prev=%p)\n", soap->blist));
  if (!(p = (struct soap_blist*)SOAP_MALLOC(sizeof(struct soap_blist))))
    return SOAP_EOM;   
  p->next = soap->blist; 
  p->ptr = NULL;
  p->size = 0;
  soap->blist = p;
  return SOAP_OK;
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
void*
SOAP_FMAC2
soap_push_block(struct soap *soap, size_t n)
{ char *p;
  DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Push block of %u bytes (%u bytes total)\n", (unsigned int)n, (unsigned int)soap->blist->size + (unsigned int)n));
  if (!(p = (char*)SOAP_MALLOC(n + sizeof(char*) + sizeof(size_t))))
  { soap->error = SOAP_EOM;
    return NULL;
  }
  *(char**)p = soap->blist->ptr;
  *(size_t*)(p + sizeof(char*)) = n;
  soap->blist->ptr = p;
  soap->blist->size += n;
  return p + sizeof(char*) + sizeof(size_t);
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
void
SOAP_FMAC2
soap_pop_block(struct soap *soap)
{ char *p;
  if (!soap->blist->ptr)
    return;
  DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Pop block\n"));
  p = soap->blist->ptr;
  soap->blist->size -= *(size_t*)(p + sizeof(char*));
  soap->blist->ptr = *(char**)p;
  SOAP_FREE(p);
}
#endif

/******************************************************************************/
#ifndef PALM_1
static void
soap_update_ptrs(struct soap *soap, char *start, char *end, long offset)
{ int i;
  register struct soap_ilist *ip;
  register struct soap_flist *fp;
  register void *p, **q;
  for (i = 0; i < SOAP_IDHASH; i++)
    for (ip = soap->iht[i]; ip; ip = ip->next)
    { if (ip->ptr && (char*)ip->ptr >= start && (char*)ip->ptr < end)
      { DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Update id='%s' %p -> %p\n", ip->id, ip->ptr, (char*)ip->ptr + offset));
        ip->ptr = (char*)ip->ptr + offset;
      }
      for (q = &ip->link; q; q = (void**)p)
      { p = *q;
        if (p && (char*)p >= start && (char*)p < end)
        { DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Link update id='%s' %p\n", ip->id, p));
          *q = (char*)p + offset;
        }
      }
      for (q = &ip->copy; q; q = (void**)p)
      { p = *q;
        if (p && (char*)p >= start && (char*)p < end)
        { DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Copy chain update id='%s' %p\n", ip->id, p));
          *q = (char*)p + offset;
        }
      }
      for (fp = ip->flist; fp; fp = fp->next)
      { if ((char*)fp->ptr >= start && (char*)fp->ptr < end)
        { DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Copy list update id='%s' %p\n", ip->id, fp));
          fp->ptr = (char*)fp->ptr + offset;
        }
      }
    }
}
#endif

/******************************************************************************/
#ifndef PALM_1
static int
soap_has_copies(struct soap *soap, register const char *start, register const char *end)
{ register int i;
  register struct soap_ilist *ip;
  register struct soap_flist *fp;
  register const char *p;
  for (i = 0; i < SOAP_IDHASH; i++)
  { for (ip = soap->iht[i]; ip; ip = ip->next)
    { for (p = (const char*)ip->copy; p; p = *(const char**)p)
        if (p >= start && p < end)
          return SOAP_ERR;
      for (fp = ip->flist; fp; fp = fp->next)
        if ((const char*)fp->ptr >= start && (const char*)fp->ptr < end)
	  return SOAP_ERR;
    }
  }
  return SOAP_OK;
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
int
SOAP_FMAC2
soap_resolve(struct soap *soap)
{ register int i;
  register struct soap_ilist *ip;
  register struct soap_flist *fp;
  short flag;
  DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Resolving forwarded data\n"));
  for (i = 0; i < SOAP_IDHASH; i++)
  { for (ip = soap->iht[i]; ip; ip = ip->next)
    { if (ip->ptr)
      { register void *p, **q, *r;
        q = (void**)ip->link;
        ip->link = NULL;
        r = ip->ptr;
        DBGLOG(TEST, if (q) SOAP_MESSAGE(fdebug, "Traversing link chain to resolve id='%s'\n", ip->id));
        while (q)
        { p = *q;
          *q = r;
          DBGLOG(TEST,SOAP_MESSAGE(fdebug, "... link %p -> %p\n", q, r));
          q = (void**)p;
        }
      }
      else if (*ip->id == '#')
      { DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Missing data for id='%s'\n", ip->id));
        strcpy(soap->id, ip->id + 1);
        return soap->error = SOAP_MISSING_ID;
      }
    }
  }
  do
  { flag = 0;
    DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Resolution phase\n"));
    for (i = 0; i < SOAP_IDHASH; i++)
    { for (ip = soap->iht[i]; ip; ip = ip->next)
      { if (ip->ptr && !soap_has_copies(soap, (const char*)ip->ptr, (const char*)ip->ptr + ip->size))
        { if (ip->copy)
          { register void *p, **q = (void**)ip->copy;
            DBGLOG(TEST, if (q) SOAP_MESSAGE(fdebug, "Traversing copy chain to resolve id='%s'\n", ip->id));
            ip->copy = NULL;
            do
            { DBGLOG(TEST, SOAP_MESSAGE(fdebug, "... copy %p -> %p (%u bytes)\n", ip->ptr, q, (unsigned int)ip->size));
	      p = *q;
              memcpy(q, ip->ptr, ip->size);
              q = (void**)p;
            } while (q);
	    flag = 1;
	  }
          for (fp = ip->flist; fp; fp = ip->flist)
          { register unsigned int k = fp->level;
	    register void *p = ip->ptr;
            DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Resolving forwarded data type=%d location=%p level=%u,%u id='%s'\n", ip->type, p, ip->level, fp->level, ip->id));
	    while (ip->level < k)
            { register void **q = (void**)soap_malloc(soap, sizeof(void*));  
	      if (!q)
	        return soap->error;
	      *q = p;
              DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Descending one level, new location=%p holds=%p...\n", q, *q));
              p = (void*)q;
              k--;
            }
	    if (fp->fcopy)
	      fp->fcopy(soap, ip->type, fp->type, fp->ptr, p, ip->size);
	    else
	      soap_fcopy(soap, ip->type, fp->type, fp->ptr, p, ip->size);
	    ip->flist = fp->next;
	    SOAP_FREE(fp);
	    flag = 1;
	  }
        }
      }
    }
  } while (flag);
#ifdef SOAP_DEBUG
  for (i = 0; i < SOAP_IDHASH; i++)
  { for (ip = soap->iht[i]; ip; ip = ip->next)
    { if (ip->copy || ip->flist)
      { DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Resolution error: forwarded data for id='%s' could not be propagated, please report this problem to the developers\n", ip->id));
      }
    }
  }
#endif
  DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Resolution done\n"));
  return SOAP_OK;
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
size_t
SOAP_FMAC2
soap_size_block(struct soap *soap, size_t n)
{ if (soap->blist->ptr)
  { soap->blist->size -= *(size_t*)(soap->blist->ptr + sizeof(char*)) - n;
    *(size_t*)(soap->blist->ptr + sizeof(char*)) = n;
  }
  return soap->blist->size;
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
char*
SOAP_FMAC2
soap_first_block(struct soap *soap)
{ char *p, *q, *r;
  p = soap->blist->ptr;
  if (!p)
    return NULL;
  DBGLOG(TEST, SOAP_MESSAGE(fdebug, "First block\n"));
  r = NULL;
  do
  { q = *(char**)p;
    *(char**)p = r;
    r = p;
    p = q;
  } while (p);
  soap->blist->ptr = r;
  return r + sizeof(char*) + sizeof(size_t);
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
char*
SOAP_FMAC2
soap_next_block(struct soap *soap)
{ char *p;
  p = soap->blist->ptr;
  if (p)
  { DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Next block\n"));
    soap->blist->ptr = *(char**)p;
    SOAP_FREE(p);
    if (soap->blist->ptr)
      return soap->blist->ptr + sizeof(char*) + sizeof(size_t);
  }
  return NULL;
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
size_t
SOAP_FMAC2
soap_block_size(struct soap *soap)
{ return *(size_t*)(soap->blist->ptr + sizeof(char*));
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
void
SOAP_FMAC2
soap_end_block(struct soap *soap)
{ struct soap_blist *bp;
  char *p, *q;
  bp = soap->blist;
  if (bp)
  { DBGLOG(TEST, SOAP_MESSAGE(fdebug, "End of block sequence, free all remaining blocks\n"));
    for (p = bp->ptr; p; p = q)
    { q = *(char**)p;
      SOAP_FREE(p);
    }
    soap->blist = bp->next;
    SOAP_FREE(bp);
  }
  DBGLOG(TEST, if (soap->blist) SOAP_MESSAGE(fdebug, "Restore previous block sequence\n"));
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
char*
SOAP_FMAC2
soap_save_block(struct soap *soap, char *p, int flag)
{ register size_t n;
  register char *q, *s;
  DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Save all blocks in contiguous memory space of %u bytes (%p->%p)\n", (unsigned int)soap->blist->size, soap->blist->ptr, p));
  if (soap->blist->size)
  { if (!p)
      p = (char*)soap_malloc(soap, soap->blist->size);
    if (p)
    { for (s = p, q = soap_first_block(soap); q; q = soap_next_block(soap))
      { n = soap_block_size(soap);
        if (flag)
	  soap_update_ptrs(soap, q, q + n, (long)s - (long)q); /* pointers s and q may or may not be related */
        DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Copy %u bytes from %p to %p\n", (unsigned int)n, q, s));
        memcpy(s, q, n);
        s += n;
      }
    }
    else
      soap->error = SOAP_EOM;
  }
  soap_end_block(soap);
  return p;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
char *
SOAP_FMAC2
soap_putsize(struct soap *soap, const char *type, int size)
{ return soap_putsizes(soap, type, &size, 1);
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
char *
SOAP_FMAC2
soap_putsizes(struct soap *soap, const char *type, const int *size, int dim)
{ return soap_putsizesoffsets(soap, type, size, NULL, dim);
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
char *
SOAP_FMAC2
soap_putsizesoffsets(struct soap *soap, const char *type, const int *size, const int *offset, int dim)
{ int i;
  if (!type)
    return NULL;
  if (soap->version == 2)
  { sprintf(soap->type, "%s[%d", type, size[0]);
    for (i = 1; i < dim; i++)
      sprintf(soap->type + strlen(soap->type), " %d", size[i]);
  }
  else
  { if (offset)
    { sprintf(soap->type, "%s[%d", type, size[0] + offset[0]);
      for (i = 1; i < dim; i++)
        sprintf(soap->type + strlen(soap->type), ",%d", size[i] + offset[i]);
    }
    else
    { sprintf(soap->type, "%s[%d", type, size[0]);
      for (i = 1; i < dim; i++)
        sprintf(soap->type + strlen(soap->type), ",%d", size[i]);
    }
    strcat(soap->type, "]");
  }
  return soap->type;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
char *
SOAP_FMAC2
soap_putoffset(struct soap *soap, int offset)
{ return soap_putoffsets(soap, &offset, 1);
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
char *
SOAP_FMAC2
soap_putoffsets(struct soap *soap, const int *offset, int dim)
{ register int i;
  sprintf(soap->arrayOffset, "[%d", offset[0]);
  for (i = 1; i < dim; i++)
    sprintf(soap->arrayOffset + strlen(soap->arrayOffset), ",%d", offset[i]);
  strcat(soap->arrayOffset, "]");
  return soap->arrayOffset;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_size(const int *size, int dim)
{ register int i, n = size[0];
  for (i = 1; i < dim; i++)
    n *= size[i];
  return n;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_getoffsets(const char *attr, const int *size, int *offset, int dim)
{ register int i, j = 0;
  if (offset)
    for (i = 0; i < dim && attr && *attr; i++)
    { attr++;
      j *= size[i];
      j += offset[i] = (int)atol(attr);
      attr = strchr(attr, ',');
    }
  else
    for (i = 0; i < dim && attr && *attr; i++)
    { attr++;
      j *= size[i];
      j += (int)atol(attr);
      attr = strchr(attr, ',');
    }
  return j;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_getsize(const char *attr1, const char *attr2, int *j)
{ register int n, k;
  char *s;
  *j = 0;
  if (!*attr1)
    return -1;
  n = 1;
  do
  { attr1++;
    k = (int)soap_strtol(attr1, &s, 10);
    n *= k;
    if (k < 0 || n > SOAP_MAXARRAYSIZE || s == attr1)
      return -1;
    attr1 = strchr(s, ',');
    if (!attr1)
      attr1 = strchr(s, ' ');
    if (attr2 && *attr2)
    { attr2++;
      *j *= k;
      k = (int)soap_strtol(attr2, &s, 10);
      *j += k;
      if (k < 0)
        return -1;
      attr2 = s;
    }
  } while (attr1 && *attr1 != ']');
  return n - *j;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_getsizes(const char *attr, int *size, int dim)
{ register int i, k, n;
  if (!*attr)
    return -1;
  i = strlen(attr);
  n = 1;
  do
  { for (i = i-1; i >= 0; i--)
      if (attr[i] == '[' || attr[i] == ',' || attr[i] == ' ')
        break;
    k = (int)atol(attr + i + 1);
    n *= size[--dim] = k;
    if (k < 0 || n > SOAP_MAXARRAYSIZE)
      return -1;
  } while (i >= 0 && attr[i] != '[');
  return n;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_getposition(const char *attr, int *pos)
{ register int i, n;
  if (!*attr)
    return -1;
  n = 0;
  i = 1;
  do
  { pos[n++] = (int)atol(attr + i);
    while (attr[i] && attr[i] != ',' && attr[i] != ']')
      i++;
    if (attr[i] == ',')
      i++;
  } while (n < SOAP_MAXDIMS && attr[i] && attr[i] != ']');
  return n;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_push_namespace(struct soap *soap, const char *id, const char *ns)
{ register struct soap_nlist *np;
  register struct Namespace *p;
  np = (struct soap_nlist*)SOAP_MALLOC(sizeof(struct soap_nlist) + strlen(id));
  if (!np)
    return soap->error = SOAP_EOM;
  np->next = soap->nlist;
  soap->nlist = np;
  strcpy(np->id, id);
  np->level = soap->level;
  np->index = -1;
  np->ns = NULL;
  DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Push namespace binding (level=%u) '%s' '%s'\n", soap->level, id, ns));
  p = soap->local_namespaces;
  if (p)
  { register short i = 0;
    for (; p->id; p++, i++)
    { if (p->ns && !strcmp(ns, p->ns))
      { if (p->out)
        { SOAP_FREE(p->out);
          p->out = NULL;
        }
        break;
      }
      if (p->out)
      { if (!SOAP_STRCMP(ns, p->out))
          break;
      }
      else if (p->in)
      { if (!soap_tag_cmp(ns, p->in))
        { if ((p->out = (char*)SOAP_MALLOC(strlen(ns) + 1)))
            strcpy(p->out, ns);
          break;
        }
      }
    }
    if (p && p->id)
    { DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Push OK ('%s' matches '%s' in namespace table)\n", id, p->id));
      np->index = i;
    }
  }
  if (!p || !p->id)
  { DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Push NOT OK: no match found for '%s' in namespace mapping table (added to stack anyway)\n", ns));
    np->ns = (char*)SOAP_MALLOC(strlen(ns) + 1);
    if (!np->ns)
      return soap->error = SOAP_EOM;
    strcpy(np->ns, ns);
  }
  return SOAP_OK;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
void
SOAP_FMAC2
soap_pop_namespace(struct soap *soap)
{ register struct soap_nlist *np;
  while (soap->nlist && soap->nlist->level >= soap->level)
  { np = soap->nlist->next;
    DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Popped namespace binding (level=%u) '%s'\n", soap->level, soap->nlist->id));
    if (soap->nlist->ns)
      SOAP_FREE(soap->nlist->ns);
    SOAP_FREE(soap->nlist);
    soap->nlist = np;
  }
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_match_namespace(struct soap *soap, const char *id1, const char *id2, int n1, int n2) 
{ register struct soap_nlist *np = soap->nlist;
  while (np && (strncmp(np->id, id1, n1) || np->id[n1]))
    np = np->next;
  if (np)
  { if (np->index < 0 || (np->index >= 0 && soap->local_namespaces[np->index].id && (strncmp(soap->local_namespaces[np->index].id, id2, n2) || soap->local_namespaces[np->index].id[n2])))
      return SOAP_NAMESPACE;
    return SOAP_OK;
  }
  if (n1 == 3 && n1 == n2 && !strcmp(id1, "xml") && !strcmp(id1, id2))
    return SOAP_OK;
  return SOAP_SYNTAX_ERROR; 
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_tag_cmp(const char *s, const char *t)
{ for (;;)
  { register int c1 = *s;
    register int c2 = *t;
    if (!c1 || c1 == '"')
      break;
    if (c2 != '-')
    { if (c1 != c2)
      { if (c1 >= 'A' && c1 <= 'Z')
          c1 += 'a' - 'A';
        if (c2 >= 'A' && c2 <= 'Z')
          c2 += 'a' - 'A';
      }
      if (c1 != c2)
      { if (c2 != '*')
          return 1;
	c2 = *++t;
        if (!c2)
	  return 0;
        if (c2 >= 'A' && c2 <= 'Z')
          c2 += 'a' - 'A';
        for (;;)
        { c1 = *s;
	  if (!c1 || c1 == '"')
	    break;
	  if (c1 >= 'A' && c1 <= 'Z')
            c1 += 'a' - 'A';
          if (c1 == c2)
            if (!soap_tag_cmp(s + 1, t + 1))
              return 0;
	  s++;
        }
        break;
      }
    }
    s++;
    t++;
  }
  if (*t == '*' && !t[1])
    return 0;
  return *t;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_match_tag(struct soap *soap, const char *tag1, const char *tag2_)
{ register const char *s, *t;
  if (!tag1 || !tag2_ || !*tag2_)
    return SOAP_OK;

  const char *tag2;
  if ( strncmp( tag2_, "ns1:", 4 ) == 0 ) tag2 = tag2_ + 4;
  else tag2 = tag2_;

  s = strchr(tag1, ':');
  t = strchr(tag2, ':');
  if (t)
  { if (s)
    { if (t[1] && SOAP_STRCMP(s + 1, t + 1))
        return SOAP_TAG_MISMATCH;
      if (t != tag2 && soap_match_namespace(soap, tag1, tag2, s - tag1, t - tag2))
      { DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Tags '%s' and '%s' match but namespaces differ\n", tag1, tag2));
        return SOAP_TAG_MISMATCH;
      }
    } 
    else if (SOAP_STRCMP(tag1, t + 1))
      return SOAP_TAG_MISMATCH;
    else if (t != tag2 && soap_match_namespace(soap, tag1, tag2, 0, t - tag2))
    { DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Tags '%s' and '%s' match but namespaces differ\n", tag1, tag2));
      return SOAP_TAG_MISMATCH;
    }
    DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Tags and (default) namespaces match: '%s' '%s'\n", tag1, tag2));
    return SOAP_OK;
  }
  if (s)
  { if (SOAP_STRCMP(s + 1, tag2))
      return SOAP_TAG_MISMATCH;
  }
  else if (SOAP_STRCMP(tag1, tag2))
    return SOAP_TAG_MISMATCH;
  DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Tags match: '%s' '%s'\n", tag1, tag2));
  return SOAP_OK;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_match_array(struct soap *soap, const char *type)
{ if (*soap->arrayType)
    if (soap_match_tag(soap, soap->arrayType, type)
     && soap_match_tag(soap, soap->arrayType, "xsd:anyType")
     && soap_match_tag(soap, soap->arrayType, "xsd:ur-type")
    )
    { DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Array type mismatch: '%s' '%s'\n", soap->arrayType, type));
      return SOAP_TAG_MISMATCH;
    }
  return SOAP_OK;
}
#endif

/******************************************************************************/

#ifdef WITH_OPENSSL
/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_ssl_server_context(struct soap *soap, unsigned short flags, const char *keyfile, const char *password, const char *cafile, const char *capath, const char *dhfile, const char *randfile, const char *sid)
{ int err;
  soap->keyfile = keyfile;
  soap->password = password;
  soap->cafile = cafile;
  soap->capath = capath;
  if (dhfile)
  { soap->dhfile = dhfile;
    soap->rsa = 0;
  }
  else
  { soap->dhfile = NULL;
    soap->rsa = 1;
  }
  soap->randfile = randfile;
  soap->require_client_auth = (flags & SOAP_SSL_REQUIRE_CLIENT_AUTHENTICATION);
  if (!(err = soap->fsslauth(soap)))
    if (sid)
      SSL_CTX_set_session_id_context(soap->ctx, (unsigned char*)sid, strlen(sid));
  return err; 
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_ssl_client_context(struct soap *soap, unsigned short flags, const char *keyfile, const char *password, const char *cafile, const char *capath, const char *randfile)
{ soap->keyfile = keyfile;
  soap->password = password;
  soap->cafile = cafile;
  soap->capath = capath;
  soap->dhfile = NULL;
  soap->rsa = 0;
  soap->randfile = randfile;
  soap->require_server_auth = (flags & SOAP_SSL_REQUIRE_SERVER_AUTHENTICATION);
  return soap->fsslauth(soap);
}
#endif

/******************************************************************************/
#ifndef PALM_2
static void
ssl_init()
{ static int done = 0;
  if (!done)
  { done = 1;
    SSL_library_init();
#ifndef WITH_LEAN
    SSL_load_error_strings();
#endif
    if (!RAND_load_file("/dev/urandom", 1024))
    { int r;
#ifdef HAVE_RAND_R
      unsigned int s = (unsigned int)time(NULL);
#endif
      char buf[SOAP_BUFLEN];
      RAND_seed(buf, sizeof(buf));
      while (!RAND_status())
      {
#ifdef HAVE_RAND_R
        r = rand_r(&s);
#else
        r = rand();
#endif
        RAND_seed(&r, sizeof(int));
      }
    }
  }
}
#endif

/******************************************************************************/
#ifndef PALM_1
static const char *
ssl_error(struct soap *soap, int ret)
{ int err = SSL_get_error(soap->ssl, ret);
  const char *msg = soap_str_code(h_ssl_error_codes, err);
  if (msg)
    strcpy(soap->msgbuf, msg);
  else
    return ERR_error_string(err, soap->msgbuf);
  if (ERR_peek_error())
  { unsigned long r;
    strcat(soap->msgbuf, "\n");
    while ((r = ERR_get_error()))
      ERR_error_string_n(r, soap->msgbuf + strlen(soap->msgbuf), sizeof(soap->msgbuf) - strlen(soap->msgbuf));
  } 
  else
  { switch (ret)
    { case 0:
        strcpy(soap->msgbuf, "EOF was observed that violates the protocol. The client probably provided invalid authentication information.");
        break;
      case -1:
        sprintf(soap->msgbuf, "Error observed by underlying BIO: %s", strerror(errno));  
        break;
    }
  }
  return soap->msgbuf;
}
#endif

/******************************************************************************/
#ifndef PALM_1
static int
ssl_password(char *buf, int num, int rwflag, void *userdata)
{ if (num < (int)strlen((char*)userdata) + 1)
    return 0;
  return strlen(strcpy(buf, (char*)userdata));
}
#endif

/******************************************************************************/
/* This callback is included for future references. It should not be deleted
#ifndef PALM_2
static DH *
ssl_tmp_dh(SSL *ssl, int is_export, int keylength)
{ static DH *dh512 = NULL;
  static DH *dh1024 = NULL;
  DH *dh;
  switch (keylength)
  { case 512:
      if (!dh512)
      { BIO *bio = BIO_new_file("dh512.pem", "r");
        if (bio)
        { dh512 = PEM_read_bio_DHparams(bio, NULL, NULL, NULL);
          BIO_free(bio);
          return dh512;
        }
      }
      else
        return dh512;
    default:
      if (!dh1024)
      { BIO *bio = BIO_new_file("dh1024.pem", "r");
        if (bio)
        { dh1024 = PEM_read_bio_DHparams(bio, NULL, NULL, NULL);
          BIO_free(bio);
        }
      }
      dh = dh1024;
  }
  return dh;
}
#endif
*/
/******************************************************************************/
#ifndef PALM_1
static int
ssl_auth_init(struct soap *soap)
{ ssl_init();
  if (!soap->ctx)
    if (!(soap->ctx = SSL_CTX_new(SSLv23_method())))
      return soap_set_receiver_error(soap, "SSL error", "Can't setup context", SOAP_SSL_ERROR);
  if (soap->randfile)
  { if (!RAND_load_file(soap->randfile, -1))
      return soap_set_receiver_error(soap, "SSL error", "Can't load randomness", SOAP_SSL_ERROR);
  }
  if (soap->cafile || soap->capath)
    if (!SSL_CTX_load_verify_locations(soap->ctx, soap->cafile, soap->capath))
      return soap_set_receiver_error(soap, "SSL error", "Can't read CA file and/or directory", SOAP_SSL_ERROR);
  if (!SSL_CTX_set_default_verify_paths(soap->ctx))
    return soap_set_receiver_error(soap, "SSL error", "Can't read default CA file and/or directory", SOAP_SSL_ERROR);
  if (soap->keyfile)
  { if (!SSL_CTX_use_certificate_chain_file(soap->ctx, soap->keyfile))
      return soap_set_receiver_error(soap, "SSL error", "Can't read certificate key file", SOAP_SSL_ERROR);
    if (soap->password)
    { SSL_CTX_set_default_passwd_cb_userdata(soap->ctx, (void*)soap->password);
      SSL_CTX_set_default_passwd_cb(soap->ctx, ssl_password);
      if (!SSL_CTX_use_PrivateKey_file(soap->ctx, soap->keyfile, SSL_FILETYPE_PEM))
        return soap_set_receiver_error(soap, "SSL error", "Can't read key file", SOAP_SSL_ERROR);
    }
  }
  if (soap->rsa)
  { RSA *rsa = RSA_generate_key(512, RSA_F4, NULL, NULL);
    if (!SSL_CTX_set_tmp_rsa(soap->ctx, rsa))
    { if (rsa)
        RSA_free(rsa);
      return soap_set_receiver_error(soap, "SSL error", "Can't set RSA key", SOAP_SSL_ERROR);
    }
    RSA_free(rsa);
  }
  else if (soap->dhfile)
  { DH *dh = 0;
    BIO *bio;
    bio = BIO_new_file(soap->dhfile, "r");
    if (!bio)
      return soap_set_receiver_error(soap, "SSL error", "Can't read DH file", SOAP_SSL_ERROR);
    dh = PEM_read_bio_DHparams(bio, NULL, NULL, NULL);
    BIO_free(bio);
    if (SSL_CTX_set_tmp_dh(soap->ctx, dh) < 0)
    { if (dh)
        DH_free(dh);
      return soap_set_receiver_error(soap, "SSL error", "Can't set DH parameters", SOAP_SSL_ERROR);
    }
    DH_free(dh);
  }
  SSL_CTX_set_options(soap->ctx, SSL_OP_ALL | SSL_OP_NO_SSLv2);
  SSL_CTX_set_verify(soap->ctx, soap->require_client_auth ? (SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT) : soap->require_server_auth ? SSL_VERIFY_PEER : SSL_VERIFY_NONE, soap->fsslverify);
#if (OPENSSL_VERSION_NUMBER < 0x00905100L)
  SSL_CTX_set_verify_depth(soap->ctx, 1); 
#else
  SSL_CTX_set_verify_depth(soap->ctx, 9); 
#endif  
  return SOAP_OK;
}
#endif

/******************************************************************************/
#ifndef PALM_1
static int
ssl_verify_callback(int ok, X509_STORE_CTX *store)
{
#ifdef SOAP_DEBUG
  if (!ok) 
  { char data[256];
    X509 *cert = X509_STORE_CTX_get_current_cert(store);
    fprintf(stderr, "SSL Verify error with certificate at depth %d: %s\n", X509_STORE_CTX_get_error_depth(store), X509_verify_cert_error_string(X509_STORE_CTX_get_error(store)));
    X509_NAME_oneline(X509_get_issuer_name(cert), data, sizeof(data));
    fprintf(stderr, "certificate issuer %s\n", data);
    X509_NAME_oneline(X509_get_subject_name(cert), data, sizeof(data));
    fprintf(stderr, "certificate subject %s\n", data);
  }
#endif
  /* return 1 to always continue, but unsafe progress will be terminated by SSL */
  return ok;
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
int
SOAP_FMAC2
soap_ssl_accept(struct soap *soap)
{ int i, r;
  if (!soap_valid_socket(soap->socket))
    return soap_set_receiver_error(soap, "SSL error", "No socket in soap_ssl_accept()", SOAP_SSL_ERROR);
  if (!soap->ssl)
  { soap->ssl = SSL_new(soap->ctx);
    if (!soap->ssl)
      return soap_set_receiver_error(soap, "SSL error", "SSL_new() failed in soap_ssl_accept()", SOAP_SSL_ERROR);
  }
  else
    SSL_clear(soap->ssl);
  soap->imode |= SOAP_ENC_SSL;
  soap->omode |= SOAP_ENC_SSL;
#ifdef WIN32
  { u_long nonblocking = 1;
    ioctlsocket((SOAP_SOCKET)soap->socket, FIONBIO, &nonblocking);
  }
#else
  fcntl((SOAP_SOCKET)soap->socket, F_SETFL, fcntl((SOAP_SOCKET)soap->socket, F_GETFL)|O_NONBLOCK);
#endif
  soap->bio = BIO_new_socket((SOAP_SOCKET)soap->socket, BIO_NOCLOSE);
  SSL_set_bio(soap->ssl, soap->bio, soap->bio);
  i = 100; /* 100 * 0.1 ms retries */
  while ((r = SSL_accept(soap->ssl)) <= 0)
  { int err = SSL_get_error(soap->ssl, r);
    if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE)
    { struct timeval timeout;
      fd_set fd;
      if (i-- <= 0)
        break;
      timeout.tv_sec = 0;
      timeout.tv_usec = 100000;
      FD_ZERO(&fd);
      FD_SET((SOAP_SOCKET)soap->socket, &fd);
      r = select((SOAP_SOCKET)(soap->socket + 1), &fd, NULL, &fd, &timeout);
      if (r < 0 && soap_socket_errno != SOAP_EINTR)
      { soap->errnum = soap_socket_errno;
        return SOAP_EOF;
      }
    }
    else
    { soap->errnum = err;
      break;
    }
  }
#ifdef WIN32
  { u_long blocking = 0;
    ioctlsocket((SOAP_SOCKET)soap->socket, FIONBIO, &blocking);
  }
#else
  fcntl((SOAP_SOCKET)soap->socket, F_SETFL, fcntl((SOAP_SOCKET)soap->socket, F_GETFL)&~O_NONBLOCK);
#endif
  if (r <= 0)
  { soap_set_receiver_error(soap, ssl_error(soap, r), "SSL_accept() failed in soap_ssl_accept()", SOAP_SSL_ERROR);
    soap_closesock(soap);
    return SOAP_SSL_ERROR;
  }
  if (soap->require_client_auth)
  { X509 *peer;
    int err;
    if ((err = SSL_get_verify_result(soap->ssl)) != X509_V_OK)
    { soap_closesock(soap);
      return soap_set_sender_error(soap, X509_verify_cert_error_string(err), "SSL certificate presented by peer cannot be verified in soap_ssl_accept()", SOAP_SSL_ERROR);
    }
    peer = SSL_get_peer_certificate(soap->ssl);
    if (!peer)
    { soap_closesock(soap);
      return soap_set_sender_error(soap, "SSL error", "No SSL certificate was presented by the peer in soap_ssl_accept()", SOAP_SSL_ERROR);
    }
    X509_free(peer);
  }
  return SOAP_OK;
}
#endif

/******************************************************************************/
#endif /* WITH_OPENSSL */

/******************************************************************************/
#ifndef PALM_1
static int
tcp_init(struct soap *soap)
{ soap->errmode = 1;
#ifdef WIN32
  if (tcp_done)
    return 0;
  else
  { WSADATA w;
    if (WSAStartup(MAKEWORD(1, 1), &w))
      return -1;
    tcp_done = 1;
  }
#endif
  return 0;
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
void
SOAP_FMAC2
soap_done(struct soap *soap)
{ 
#ifdef SOAP_DEBUG
  int i;
#endif
  soap_free(soap);
  while (soap->clist)
  { struct soap_clist *p = soap->clist->next;
    SOAP_FREE(soap->clist);
    soap->clist = p;
  }
  soap->keep_alive = 0; /* to force close the socket */
  soap_closesock(soap);
#ifdef WITH_COOKIES
  soap_free_cookies(soap);
#endif
  while (soap->plugins)
  { register struct soap_plugin *p = soap->plugins->next;
    DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Removing plugin '%s'\n", soap->plugins->id));
    if (soap->plugins->fcopy || !soap->copy)
      soap->plugins->fdelete(soap, soap->plugins);
    SOAP_FREE(soap->plugins);
    soap->plugins = p;
  }
  soap->fplugin = fplugin;
  soap->fpost = http_post;
  soap->fget = http_get;
  soap->fposthdr = http_post_header;
  soap->fresponse = http_response;
  soap->fparse = http_parse;
  soap->fparsehdr = http_parse_header;
#ifndef MAC_CARBON
#ifndef WITH_IPV6
  soap->fresolve = tcp_gethost;
#else
  soap->fresolve = NULL;
#endif
  soap->faccept = tcp_accept;
  soap->fopen = tcp_connect;
  soap->fclose = tcp_disconnect;
  soap->fclosesocket = tcp_closesocket;
  soap->fshutdownsocket = tcp_shutdownsocket;
  soap->fsend = fsend;
  soap->frecv = frecv;
  soap->fpoll = soap_poll;
#else
  soap->fpoll = NULL;
#endif
  soap->fprepareinit = NULL;
  soap->fpreparesend = NULL;
  soap->fpreparerecv = NULL;
  soap->fignore = NULL;
  soap->fserveloop = NULL;
#ifdef WITH_OPENSSL
  if (soap->session)
  { SSL_SESSION_free(soap->session);
    soap->session = NULL;
  }
#endif
  if (!soap->copy)
  { if (soap_valid_socket(soap->master))
    { soap->fclosesocket(soap, (SOAP_SOCKET)soap->master);
      soap->master = SOAP_INVALID_SOCKET;
    }
#ifdef WITH_OPENSSL
    if (soap->ctx)
    { SSL_CTX_free(soap->ctx);
      soap->ctx = NULL;
    }
#endif
  }
#ifdef SOAP_DEBUG
  for (i = 0; i < SOAP_MAXLOGS; i++)
  { soap_close_logfile(soap, i);
    if (soap->logfile[i])
    { SOAP_FREE((void*)soap->logfile[i]);
      soap->logfile[i] = NULL;
    }
  }
#endif
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
void
SOAP_FMAC2
soap_cleanup(struct soap *soap)
{ soap_done(soap);
#ifdef WIN32
  if (!tcp_done)
    return;
  tcp_done = 0;
  WSACleanup();
#endif
}
#endif

/******************************************************************************/
#ifndef PALM_1
static const char*
tcp_error(struct soap *soap)
{ register const char *msg = NULL;
  switch (soap->errmode)
  { case 0:
      msg = soap_strerror(soap);
      break;
    case 1:
      msg = "WSAStartup failed";
      break;
    case 2:
    {
#ifndef WITH_LEAN
      msg = soap_str_code(h_error_codes, soap->errnum);
      if (!msg)
#endif
      { sprintf(soap->msgbuf, "TCP error %d", soap->errnum);
        msg = soap->msgbuf;
      }
    }
  }
  return msg;
}
#endif

/******************************************************************************/
#ifndef PALM_1
static const char*
http_error(struct soap *soap, int status)
{ register const char *msg = NULL;
#ifndef WITH_LEAN
  msg = soap_str_code(h_http_error_codes, status);
  if (!msg)
#endif
  { sprintf(soap->msgbuf, "HTTP error %d", status);
    msg = soap->msgbuf;
  }
  return msg;
}
#endif

/******************************************************************************/
/* WR[ */
#ifndef WITH_IPV6
/* ]WR */
#ifndef MAC_CARBON
#ifndef PALM_1
static int
tcp_gethost(struct soap *soap, const char *addr, struct in_addr *inaddr)
{ unsigned long iadd;
  struct hostent hostent, *host = &hostent;
/* WR[ */
#ifdef VXWORKS
  int hostint;
  char *addrcopy = (char*)malloc(strlen(addr) + 1); /*copy of addr. */
  /* inet_addr(), and hostGetByName() expect "char *"; addr is a "const char *". */
  strncpy(addrcopy, addr, strlen(addr)+1);
  iadd = inet_addr(addrcopy);
#else
/* ]WR */
#if defined(_AIXVERSION_431) || defined(TRU64)
  struct hostent_data ht_data;
#endif
  iadd = inet_addr(addr);
/* WR[ */
#endif
/* ]WR */
  if ((int)iadd != -1)
  { memcpy(inaddr, &iadd, sizeof(iadd));
/* WR[ */
#ifdef VXWORKS
    free(addrcopy);
#endif
/* ]WR */
    return SOAP_OK;
  }
#if defined(__GLIBC__)
  if (gethostbyname_r(addr, &hostent, soap->buf, SOAP_BUFLEN, &host, &soap->errnum) < 0)
    host = NULL;
#elif defined(_AIXVERSION_431) || defined(TRU64)
  memset((void*)&ht_data, 0, sizeof(ht_data));
  if (gethostbyname_r(addr, &hostent, &ht_data) < 0)
  { host = NULL;
    soap->errnum = h_errno;
  }
#elif defined(HAVE_GETHOSTBYNAME_R)
  host = gethostbyname_r(addr, &hostent, soap->buf, SOAP_BUFLEN, &soap->errnum);
/* WR[ */
#elif defined(VXWORKS)
  /* If the DNS resolver library resolvLib has been configured in the vxWorks
   * image, a query for the host IP address is sent to the DNS server, if the
   * name was not found in the local host table. */
  hostint = hostGetByName(addrcopy);
  if (hostint == ERROR)
  { host = NULL;
    soap->errnum = soap_errno; 
  }
  free(addrcopy);  /*free() is placed after the error checking to assure that
		    * errno captured is that from hostGetByName() */
/* ]WR */
#else
  if (!(host = gethostbyname(addr)))
    soap->errnum = h_errno;
#endif
  if (!host)
  { DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Host name not found\n"));
    return SOAP_ERR;
  }
/* WR[ */
#ifdef VXWORKS
  inaddr->s_addr = hostint;
#else
/* ]WR */
  memcpy(inaddr, host->h_addr, host->h_length);
/* WR[ */
#endif
/* ]WR */
  return SOAP_OK;
}
#endif
#endif
/* WR[ */
#endif /* WITH_IPV6 */
/* ]WR */

/******************************************************************************/
#ifndef MAC_CARBON
#ifndef PALM_1
static int
tcp_connect(struct soap *soap, const char *endpoint, const char *host, int port)
{ struct sockaddr_in sockaddr;
/* WR[ */
#ifdef WITH_IPV6
  struct addrinfo *addrinfo;
  struct addrinfo hints;
  struct addrinfo resaddr;
  struct sockaddr_storage addrstorage;
  int err;
#endif /* WITH_IPV6 */
/* ]WR */
  register int fd;
#ifndef WITH_LEAN
  int len = SOAP_BUFLEN;
  int set = 1;
#endif
  if (soap_valid_socket(soap->socket))
    soap->fclosesocket(soap, (SOAP_SOCKET)soap->socket);
  soap->socket = SOAP_INVALID_SOCKET;
  if (tcp_init(soap))
  { soap_set_sender_error(soap, tcp_error(soap), "TCP initialization failed in tcp_connect()", SOAP_TCP_ERROR);
    return -1;
  }
  soap->errmode = 0;
/* WR[ */
#ifdef WITH_IPV6
  memset((void*)&hints, 0, sizeof(hints));
  hints.ai_family = PF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  soap->errmode = 2;
  if (soap->proxy_host)
    err = getaddrinfo(soap->proxy_host, soap_int2s(soap, soap->proxy_port), &hints, &addrinfo);
  else
    err = getaddrinfo(host, soap_int2s(soap, port), &hints, &addrinfo);
  if (addrinfo)
  { resaddr = *addrinfo;
    addrstorage = *((struct sockaddr_storage*)addrinfo->ai_addr);
    resaddr.ai_addr = (struct sockaddr*)&addrstorage;
    freeaddrinfo(addrinfo);
  }
  if (err)
  { soap_set_sender_error(soap, gai_strerror(err), 
    "TCP getaddrinfo on proxy host failed in tcp_connect()", SOAP_TCP_ERROR);
    return -1;
  }
  fd = (int)socket(resaddr.ai_family, resaddr.ai_socktype, resaddr.ai_protocol); /* modified to use fd */
  soap->errmode = 0;
#else /* WITH_IPV6 */
/* ]WR */
  fd = (int)socket(AF_INET, SOCK_STREAM, 0);
/* WR[ */
#endif /* WITH_IPV6 */
/* ]WR */
  if (fd < 0)
  { soap->errnum = soap_socket_errno;
    soap_set_sender_error(soap, tcp_error(soap), "TCP socket failed in tcp_connect()", SOAP_TCP_ERROR);
    return -1;
  }
#ifdef SOCKET_CLOSE_ON_EXEC
#ifdef WIN32
#ifndef UNDER_CE
  SetHandleInformation((HANDLE)fd, HANDLE_FLAG_INHERIT, 0);
#endif
#else
  fcntl (fd, F_SETFD, 1);
#endif
#endif
#ifndef WITH_LEAN
  if (soap->connect_flags & SO_LINGER)
  { struct linger linger;
    memset((void*)&linger, 0, sizeof(linger));
    linger.l_onoff = 1;
    linger.l_linger = 0;
    if (setsockopt((SOAP_SOCKET)fd, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(struct linger)))
    { soap->errnum = soap_socket_errno;
      soap_set_sender_error(soap, tcp_error(soap), "TCP setsockopt SO_LINGER failed in tcp_connect()", SOAP_TCP_ERROR);
      soap->fclosesocket(soap, (SOAP_SOCKET)fd);
      return -1;
    }
  }
  if ((soap->connect_flags & ~SO_LINGER) && setsockopt((SOAP_SOCKET)fd, SOL_SOCKET, soap->connect_flags & ~SO_LINGER, (char*)&set, sizeof(int)))
  { soap->errnum = soap_socket_errno;
    soap_set_sender_error(soap, tcp_error(soap), "TCP setsockopt failed in tcp_connect()", SOAP_TCP_ERROR);
    soap->fclosesocket(soap, (SOAP_SOCKET)fd);
    return -1;
  }
  if (soap->keep_alive && setsockopt((SOAP_SOCKET)fd, SOL_SOCKET, SO_KEEPALIVE, (char*)&set, sizeof(int)))
  { soap->errnum = soap_socket_errno;
    soap_set_sender_error(soap, tcp_error(soap), "TCP setsockopt SO_KEEPALIVE failed in tcp_connect()", SOAP_TCP_ERROR);
    soap->fclosesocket(soap, (SOAP_SOCKET)fd);
    return -1;
  }
  if (setsockopt((SOAP_SOCKET)fd, SOL_SOCKET, SO_SNDBUF, (char*)&len, sizeof(int)))
  { soap->errnum = soap_socket_errno;
    soap_set_sender_error(soap, tcp_error(soap), "TCP setsockopt SO_SNDBUF failed in tcp_connect()", SOAP_TCP_ERROR);
    soap->fclosesocket(soap, (SOAP_SOCKET)fd);
    return -1;
  }
  if (setsockopt((SOAP_SOCKET)fd, SOL_SOCKET, SO_RCVBUF, (char*)&len, sizeof(int)))
  { soap->errnum = soap_socket_errno;
    soap_set_sender_error(soap, tcp_error(soap), "TCP setsockopt SO_RCVBUF failed in tcp_connect()", SOAP_TCP_ERROR);
    soap->fclosesocket(soap, (SOAP_SOCKET)fd);
    return -1;
  }
#ifdef TCP_NODELAY
  if (setsockopt((SOAP_SOCKET)fd, IPPROTO_TCP, TCP_NODELAY, (char*)&set, sizeof(int)))
  { soap->errnum = soap_socket_errno;
    soap_set_sender_error(soap, tcp_error(soap), "TCP setsockopt TCP_NODELAY failed in tcp_connect()", SOAP_TCP_ERROR);
    soap->fclosesocket(soap, (SOAP_SOCKET)fd);
    return -1;
  }
#endif
#endif
/* WR[ */
#ifndef WITH_IPV6
/* ]WR */
  memset((void*)&sockaddr, 0, sizeof(sockaddr));
  sockaddr.sin_family = AF_INET;
  DBGLOG(TEST,SOAP_MESSAGE(fdebug, "Open socket %d to host='%s'\n", fd, host));
  soap->errmode = 2;
  if (soap->proxy_host)
  { if (soap->fresolve(soap, soap->proxy_host, &sockaddr.sin_addr))
    { soap_set_sender_error(soap, tcp_error(soap), "TCP get proxy host by name failed in tcp_connect()", SOAP_TCP_ERROR);
      soap->fclosesocket(soap, (SOAP_SOCKET)fd);
      return -1;
    }
    sockaddr.sin_port = htons((short)soap->proxy_port);
  }
  else
  { if (soap->fresolve(soap, host, &sockaddr.sin_addr))
    { soap_set_sender_error(soap, tcp_error(soap), "TCP get host by name failed in tcp_connect()", SOAP_TCP_ERROR);
      soap->fclosesocket(soap, (SOAP_SOCKET)fd);
      return -1;
    }
    sockaddr.sin_port = htons((short)port);
  }
  soap->errmode = 0;
/* WR[ */
#endif /* WITH_IPV6 */
/* ]WR */
#ifndef WITH_LEAN
  if (soap->connect_timeout)
#if defined(WIN32)
  { u_long nonblocking = 1;
    ioctlsocket((SOAP_SOCKET)fd, FIONBIO, &nonblocking);
  }
/* WR[ */
#elif defined(VXWORKS)
  { vx_nonblocking = TRUE;
    ioctl((SOAP_SOCKET)fd, FIONBIO, (int)(&vx_nonblocking)); /* modified to use fd */
  }
/* ]WR */
#else
    fcntl((SOAP_SOCKET)fd, F_SETFL, fcntl((SOAP_SOCKET)fd, F_GETFL)|O_NONBLOCK);
#endif
  else
#if defined(WIN32)
  { u_long blocking = 0;
    ioctlsocket((SOAP_SOCKET)fd, FIONBIO, &blocking);
  }
/* WR[ */
#elif defined(VXWORKS)
  { vx_nonblocking = FALSE;
    ioctl((SOAP_SOCKET)fd, FIONBIO, (int)(&vx_nonblocking)); /* modified to use fd */
  }
/* ]WR */
#else
    fcntl((SOAP_SOCKET)fd, F_SETFL, fcntl((SOAP_SOCKET)fd, F_GETFL)&~O_NONBLOCK);
#endif
#endif
  for (;;)
  { 
/* WR[ */
#ifdef WITH_IPV6
    if (connect((SOAP_SOCKET)fd, resaddr.ai_addr, resaddr.ai_addrlen)) /* modified to use fd */
#else /* WITH_IPV6 */
    if (connect((SOAP_SOCKET)fd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)))
#endif /* WITH_IPV6 */
/* ]WR */
    { 
#ifndef WITH_LEAN
      if (soap->connect_timeout && (soap_socket_errno == SOAP_EINPROGRESS || soap_socket_errno == SOAP_EWOULDBLOCK))
      { struct timeval timeout;
#if defined(SOCKLEN_T)
        SOCKLEN_T n = sizeof(struct sockaddr_in);
#elif defined(__socklen_t_defined) || defined(_SOCKLEN_T) || defined(CYGWIN)
        socklen_t n = sizeof(struct sockaddr_in);
#elif defined(WIN32) || defined(__APPLE__) || defined(HP_UX) || defined(SUN_OS) || defined(OPENSERVER) || defined(TRU64) || defined(VXWORKS)
        int n = sizeof(struct sockaddr_in);
#else
        size_t n = sizeof(struct sockaddr_in);
#endif
        fd_set fds;
        if (soap->connect_timeout > 0)
        { timeout.tv_sec = soap->connect_timeout;
          timeout.tv_usec = 0;
        }
        else
        { timeout.tv_sec = -soap->connect_timeout/1000000;
          timeout.tv_usec = -soap->connect_timeout%1000000;
        }
        FD_ZERO(&fds);
        FD_SET((SOAP_SOCKET)fd, &fds);
        for (;;)
        { int r = select((SOAP_SOCKET)(fd + 1), NULL, &fds, NULL, &timeout);
          if (r > 0)
	    break;
          if (!r)
          { soap->errnum = 0;
            DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Connect timeout\n"));
            soap_set_sender_error(soap, "Timeout", "TCP connect failed in tcp_connect()", SOAP_TCP_ERROR);
            soap->fclosesocket(soap, (SOAP_SOCKET)fd);
            return -1;
          }
          if (soap_socket_errno != SOAP_EINTR)
          { soap->errnum = soap_socket_errno;
            DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Could not connect to host\n"));
            soap_set_sender_error(soap, tcp_error(soap), "TCP connect failed in tcp_connect()", SOAP_TCP_ERROR);
            soap->fclosesocket(soap, (SOAP_SOCKET)fd);
            return -1;
          }
        }
	n = sizeof(soap->errnum);
        if (!getsockopt((SOAP_SOCKET)fd, SOL_SOCKET, SO_ERROR, (char*)&soap->errnum, &n) && !soap->errnum)
          break;
        DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Could not connect to host\n"));
        soap_set_sender_error(soap, tcp_error(soap), "TCP connect failed in tcp_connect()", SOAP_TCP_ERROR);
        soap->fclosesocket(soap, (SOAP_SOCKET)fd);
        return -1;
      }
      else
#endif
      if (soap_socket_errno != SOAP_EINTR)
      { soap->errnum = soap_socket_errno;
        DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Could not connect to host\n"));
        soap_set_sender_error(soap, tcp_error(soap), "TCP connect failed in tcp_connect()", SOAP_TCP_ERROR);
        soap->fclosesocket(soap, (SOAP_SOCKET)fd);
        return -1;
      }
    }  
    else
      break;
  }
#ifndef WITH_LEAN
  if (soap->connect_timeout)
#if defined(WIN32)
  { u_long blocking = 0;
    ioctlsocket((SOAP_SOCKET)fd, FIONBIO, &blocking);
  }
/* WR[ */
#elif defined(VXWORKS)
  { vx_nonblocking = FALSE;
    ioctl((SOAP_SOCKET)fd, FIONBIO, (int)(&vx_nonblocking)); /* modified to use fd */
  }
/* ]WR */
#else
    fcntl((SOAP_SOCKET)fd, F_SETFL, fcntl((SOAP_SOCKET)fd, F_GETFL)&~O_NONBLOCK);
#endif
#endif
  soap->socket = fd;
#ifdef WITH_OPENSSL
  soap->imode &= ~SOAP_ENC_SSL;
  soap->omode &= ~SOAP_ENC_SSL;
  if (!strncmp(endpoint, "https:", 6))
  { int r;
    if (soap->proxy_host)
    { unsigned int k = soap->omode; /* make sure we only parse HTTP */
      size_t n = soap->count; /* save the content length */
      soap->omode &= ~0xFF; /* mask IO and ENC */
      soap->omode |= SOAP_IO_BUFFER;
      soap_begin_send(soap);
      DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Connecting to proxy server\n"));
      sprintf(soap->tmpbuf, "CONNECT %s:%d HTTP/%s", host, port, soap->http_version);
      if ((soap->error = soap->fposthdr(soap, soap->tmpbuf, NULL)))
        return -1;
#ifndef WITH_LEAN
      if (soap->proxy_userid && soap->proxy_passwd && strlen(soap->proxy_userid) + strlen(soap->proxy_passwd) < 761)
      { sprintf(soap->tmpbuf + 262, "%s:%s", soap->proxy_userid, soap->proxy_passwd);
        strcpy(soap->tmpbuf, "Basic ");
        soap_s2base64(soap, (const unsigned char*)(soap->tmpbuf + 262), soap->tmpbuf + 6, strlen(soap->tmpbuf + 262));
        if ((soap->error = soap->fposthdr(soap, "Proxy-Authorization", soap->tmpbuf)))
          return soap->error;
      }
#endif
      if ((soap->error = soap->fposthdr(soap, NULL, NULL))
       || soap_flush(soap))
        return -1;
      soap->omode = k;
      k = soap->imode;
      soap->imode &= ~0xFF; /* mask IO and ENC */
      if (soap_begin_recv(soap))
        return -1;
      soap->imode = k;
      soap->count = n;
      soap_begin_send(soap);
    }
    if (!soap->ctx && (soap->error = soap->fsslauth(soap)))
    { soap_set_sender_error(soap, "SSL error", "SSL authentication failed in tcp_connect(): check password, key file, and ca file.", SOAP_SSL_ERROR);
      return -1;
    }
    soap->ssl = SSL_new(soap->ctx);
    if (!soap->ssl)
    { soap->error = SOAP_SSL_ERROR;
      return -1;
    }
    if (soap->session)
    { if (!strcmp(soap->session_host, host) && soap->session_port == port)
        SSL_set_session(soap->ssl, soap->session);
      SSL_SESSION_free(soap->session);
      soap->session = NULL;
    }
    soap->imode |= SOAP_ENC_SSL;
    soap->omode |= SOAP_ENC_SSL;
    soap->bio = BIO_new_socket((SOAP_SOCKET)fd, BIO_NOCLOSE);
    SSL_set_bio(soap->ssl, soap->bio, soap->bio);
#ifndef WITH_LEAN
    if (soap->connect_timeout)
#ifdef WIN32
    { u_long nonblocking = 1;
      ioctlsocket((SOAP_SOCKET)fd, FIONBIO, &nonblocking);
    }
#else
      fcntl((SOAP_SOCKET)fd, F_SETFL, fcntl((SOAP_SOCKET)fd, F_GETFL)|O_NONBLOCK);
#endif
#endif
    for (;;)
    { if ((r = SSL_connect(soap->ssl)) <= 0)
      { int err = SSL_get_error(soap->ssl, r);
        if (err != SSL_ERROR_NONE && err != SSL_ERROR_WANT_READ && err != SSL_ERROR_WANT_WRITE)
        { soap_set_sender_error(soap, ssl_error(soap, r), "SSL connect failed in tcp_connect()", SOAP_SSL_ERROR);
          return -1;
        }
        if (soap->connect_timeout)
        { struct timeval timeout;
          fd_set fds;
          if (soap->connect_timeout > 0)
          { timeout.tv_sec = soap->connect_timeout;
            timeout.tv_usec = 0;
          }
          else
          { timeout.tv_sec = -soap->connect_timeout/1000000;
            timeout.tv_usec = -soap->connect_timeout%1000000;
          }
          FD_ZERO(&fds);
          FD_SET((SOAP_SOCKET)(soap->socket), &fds);
          for (;;)
          { int r = select((SOAP_SOCKET)(soap->socket + 1), &fds, NULL, &fds, &timeout);
            if (r > 0)
	      break;
            if (!r)
            { soap->errnum = 0;
              DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Connect timeout\n"));
              soap_set_sender_error(soap, "Timeout", "TCP connect failed in tcp_connect()", SOAP_TCP_ERROR);
              return -1;
            }
          }
	  continue;
        }
      }
      break;
    }
#ifndef WITH_LEAN
    if (soap->connect_timeout)
#ifdef WIN32
    { u_long blocking = 0;
      ioctlsocket((SOAP_SOCKET)fd, FIONBIO, &blocking);
    }
#else
      fcntl((SOAP_SOCKET)fd, F_SETFL, fcntl((SOAP_SOCKET)fd, F_GETFL)&~O_NONBLOCK);
#endif
#endif
    if (soap->require_server_auth)
    { X509 *peer;
      int err;
      if ((err = SSL_get_verify_result(soap->ssl)) != X509_V_OK)
      { soap_set_sender_error(soap, X509_verify_cert_error_string(err), "SSL certificate presented by peer cannot be verified in tcp_connect()", SOAP_SSL_ERROR);
        return -1;
      }
      peer = SSL_get_peer_certificate(soap->ssl);
      if (!peer)
      { soap_set_sender_error(soap, "SSL error", "No SSL certificate was presented by the peer in tcp_connect()", SOAP_SSL_ERROR);
        return -1;
      }
      X509_NAME_get_text_by_NID(X509_get_subject_name(peer), NID_commonName, soap->msgbuf, sizeof(soap->msgbuf));
      X509_free(peer);
      if (soap_tag_cmp(soap->msgbuf, host))
      { soap_set_sender_error(soap, "SSL error", "SSL certificate host name mismatch in tcp_connect()", SOAP_SSL_ERROR);
        return -1;
      }
    }
  }
#endif
  return fd;
}
#endif
#endif

/******************************************************************************/
#ifndef MAC_CARBON
#ifndef PALM_1
SOAP_FMAC1
int
SOAP_FMAC2
soap_bind(struct soap *soap, const char *host, int port, int backlog)
{ struct sockaddr_in sockaddr;
/* WR[ */
#ifdef WITH_IPV6
  struct addrinfo *addrinfo;
  struct addrinfo hints;
  struct addrinfo resaddr;
  struct sockaddr_storage addrstorage;
  int err;
#endif /* WITH_IPV6 */
/* ]WR */
#ifndef WITH_LEAN
  int len = SOAP_BUFLEN;
  int set = 1;
#endif
  if (soap_valid_socket(soap->master))
  { soap->fclosesocket(soap, (SOAP_SOCKET)soap->master);
    soap->master = SOAP_INVALID_SOCKET;
  }
  soap->socket = SOAP_INVALID_SOCKET;
  soap->errmode = 1;
  if (tcp_init(soap))
  { soap_set_receiver_error(soap, tcp_error(soap), "TCP init failed in soap_bind()", SOAP_TCP_ERROR);
    return -1;
  }
/* WR[ */
#ifdef WITH_IPV6
  memset((void*)&hints, 0, sizeof(hints));
  hints.ai_family = PF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  soap->errmode = 2;
  if (host)
    err = getaddrinfo(host, soap_int2s(soap, port), &hints, &addrinfo);
  else
    err = getaddrinfo(NULL, soap_int2s(soap, port), &hints, &addrinfo);
  if (NULL != addrinfo)
  {
    resaddr = *addrinfo;
    addrstorage = *((struct sockaddr_storage *) addrinfo->ai_addr);
    resaddr.ai_addr = (struct sockaddr *) &addrstorage;
    freeaddrinfo(addrinfo);
  }
  if (err)
  { soap_set_receiver_error(soap, gai_strerror(err), "TCP getaddrinfo failed in soap_bind()", SOAP_TCP_ERROR);
    return -1;
  }
  soap->errmode = 0;
  if ((soap->master = socket(resaddr.ai_family, resaddr.ai_socktype, resaddr.ai_protocol)) < 0)
  { soap->errnum = soap_socket_errno;
    soap_set_receiver_error(soap, tcp_error(soap), "TCP socket failed in soap_bind()", SOAP_TCP_ERROR);
    return -1;
  }
#else /* WITH_IPV6 */
/* ]WR */
  soap->errmode = 0;
  if ((soap->master = (int)socket(AF_INET, SOCK_STREAM, 0)) < 0)
  { soap->errnum = soap_socket_errno;
    soap_set_receiver_error(soap, tcp_error(soap), "TCP socket failed in soap_bind()", SOAP_TCP_ERROR);
    return -1;
  }
/* WR[ */
#endif /* WITH_IPV6 */
/* ]WR */
#ifdef SOCKET_CLOSE_ON_EXEC
#ifdef WIN32
#ifndef UNDER_CE
  SetHandleInformation((HANDLE)soap->master, HANDLE_FLAG_INHERIT, 0);
#endif
#else
  fcntl (soap->master, F_SETFD, 1);
#endif
#endif
#ifndef WITH_LEAN
  if (soap->bind_flags && setsockopt((SOAP_SOCKET)soap->master, SOL_SOCKET, soap->bind_flags, (char*)&set, sizeof(int)))
  { soap->errnum = soap_socket_errno;
    soap_set_receiver_error(soap, tcp_error(soap), "TCP setsockopt failed in soap_bind()", SOAP_TCP_ERROR);
    return -1;
  }
  if (soap->keep_alive && setsockopt((SOAP_SOCKET)soap->master, SOL_SOCKET, SO_KEEPALIVE, (char*)&set, sizeof(int)))
  { soap->errnum = soap_socket_errno;
    soap_set_receiver_error(soap, tcp_error(soap), "TCP setsockopt SO_KEEPALIVE failed in soap_bind()", SOAP_TCP_ERROR);
    return -1;
  }
  if (setsockopt((SOAP_SOCKET)soap->master, SOL_SOCKET, SO_SNDBUF, (char*)&len, sizeof(int)))
  { soap->errnum = soap_socket_errno;
    soap_set_receiver_error(soap, tcp_error(soap), "TCP setsockopt SO_SNDBUF failed in soap_bind()", SOAP_TCP_ERROR);
    return -1;
  }
  if (setsockopt((SOAP_SOCKET)soap->master, SOL_SOCKET, SO_RCVBUF, (char*)&len, sizeof(int)))
  { soap->errnum = soap_socket_errno;
    soap_set_receiver_error(soap, tcp_error(soap), "TCP setsockopt SO_RCVBUF failed in soap_bind()", SOAP_TCP_ERROR);
    return -1;
  }
#ifdef TCP_NODELAY
  if (setsockopt((SOAP_SOCKET)soap->master, IPPROTO_TCP, TCP_NODELAY, (char*)&set, sizeof(int)))
  { soap->errnum = soap_socket_errno;
    soap_set_receiver_error(soap, tcp_error(soap), "TCP setsockopt TCP_NODELAY failed in soap_bind()", SOAP_TCP_ERROR);
    return -1;
  }
#endif
#endif
/* WR[ */
#ifdef WITH_IPV6
  soap->errmode = 0;
  if (bind(soap->master, resaddr.ai_addr, resaddr.ai_addrlen) || listen(soap->master, backlog))
  { 
    soap->errnum = soap_socket_errno;
    DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Could not bind to host\n"));
    soap_closesock(soap);
    soap_set_receiver_error(soap, tcp_error(soap), "TCP bind failed in soap_bind()", SOAP_TCP_ERROR);
    return -1;
  }  
#else /* WITH_IPV6 */
/* ]WR */
  memset((void*)&sockaddr, 0, sizeof(sockaddr));
  sockaddr.sin_family = AF_INET;
  soap->errmode = 2;
  if (host)
  { if (soap->fresolve(soap, host, &sockaddr.sin_addr))
    { soap_set_receiver_error(soap, tcp_error(soap), "TCP get host by name failed in soap_bind()", SOAP_TCP_ERROR);
      return -1;
    }
  }
  else
    sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  sockaddr.sin_port = htons((short)port);
  soap->errmode = 0;
  if (bind((SOAP_SOCKET)soap->master, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) || listen((SOAP_SOCKET)soap->master, backlog))
  { soap->errnum = soap_socket_errno;
    DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Could not bind to host\n"));
    soap_closesock(soap);
    soap_set_receiver_error(soap, tcp_error(soap), "TCP bind failed in soap_bind()", SOAP_TCP_ERROR);
    return -1;
  }  
/* WR[ */
#endif /* WITH_IPV6 */
/* ]WR */
#ifdef WITH_OPENSSL
  if (!soap->ctx && (soap->error = soap->fsslauth(soap)))
    return -1;
#endif
  return soap->master;
}
#endif
#endif

/******************************************************************************/
#ifndef MAC_CARBON
#ifndef PALM_1
SOAP_FMAC1
int
SOAP_FMAC2
soap_poll(struct soap *soap)
{ 
#ifndef WITH_LEAN
  struct timeval timeout;
  fd_set sfd,rfd;
  int r;
  timeout.tv_sec = 0;
  timeout.tv_usec = 0;
  FD_ZERO(&rfd);
  FD_ZERO(&sfd);
  if (soap->socket >= 0)
  { FD_SET(soap->socket, &rfd);
    FD_SET(soap->socket, &sfd);
    r = select(soap->socket + 1, &rfd, &sfd, NULL, &timeout);
  }
  else if (soap->master >= 0)
  { FD_SET(soap->master, &rfd);
    r = select(soap->master + 1, &rfd, &sfd, NULL, &timeout);
  }
  else
  { FD_SET(soap->sendfd, &sfd);
    FD_SET(soap->recvfd, &rfd);
    r = select((soap->sendfd > soap->recvfd ? soap->sendfd : soap->recvfd) + 1, &rfd, &sfd, NULL, &timeout);
  }
  if (r > 0)
  {
#ifdef WITH_OPENSSL
    if (soap->ssl)
    { if ((soap->socket >= 0) && FD_ISSET(soap->socket, &rfd))
      { char buf = '\0';
	if (SSL_peek(soap->ssl, &buf, 1) <= 0)
  	  return SOAP_EOF;
      }
    }
#endif   
    return SOAP_OK;
  }
  if (r < 0 && (soap_valid_socket(soap->master) || soap_valid_socket(soap->socket)) && soap_socket_errno != SOAP_EINTR)
  { soap->errnum = soap_socket_errno;
    soap_set_receiver_error(soap, tcp_error(soap), "select failed in soap_poll()", SOAP_TCP_ERROR);
    return soap->error = SOAP_TCP_ERROR;
  }
  else
    soap->errnum = soap_errno;
  return SOAP_EOF;
#else
  return SOAP_OK;
#endif
}
#endif
#endif

/******************************************************************************/
#ifndef MAC_CARBON
#ifndef PALM_1
static int
tcp_accept(struct soap *soap, int s, struct sockaddr *a, int *n)
{ int fd;
#if defined(SOCKLEN_T)
  fd = (int)accept((SOAP_SOCKET)s, a, (SOCKLEN_T*)n);
#elif defined(__socklen_t_defined) || defined(_SOCKLEN_T) || defined(CYGWIN)
  fd = (int)accept((SOAP_SOCKET)s, a, (socklen_t*)n);
#elif defined(WIN32) || defined(__APPLE__) || defined(HP_UX) || defined(SUN_OS) || defined(OPENSERVER) || defined(TRU64) || defined(VXWORKS)
  fd = (int)accept((SOAP_SOCKET)s, a, n);
#else
  fd = (int)accept((SOAP_SOCKET)s, a, (size_t*)n);
#endif
#ifdef SOCKET_CLOSE_ON_EXEC
#ifdef WIN32
#ifndef UNDER_CE
  SetHandleInformation((HANDLE)fd, HANDLE_FLAG_INHERIT, 0);
#endif
#else
  fcntl(fd, F_SETFD, FD_CLOEXEC);
#endif
#endif
  return fd;
}
#endif
#endif

/******************************************************************************/
#ifndef MAC_CARBON
#ifndef PALM_1
SOAP_FMAC1
int
SOAP_FMAC2
soap_accept(struct soap *soap)
{ 
/* WR[ */
#ifdef WITH_IPV6
  struct sockaddr_storage sockaddr;
#else /* WITH_IPV6 */
/* ]WR */
  struct sockaddr_in sockaddr;
/* WR[ */
#endif
/* ]WR */
  int n = (int)sizeof(sockaddr);
#ifndef WITH_LEAN
  int len = SOAP_BUFLEN;
  int set = 1;
#endif
  soap->error = SOAP_OK;
  memset((void*)&sockaddr, 0, sizeof(sockaddr));
  soap->socket = SOAP_INVALID_SOCKET;
  soap->errmode = 0;
  if (soap_valid_socket(soap->master))
  { for (;;)
    { 
#ifndef WITH_LEAN
      if (soap->accept_timeout)
      { struct timeval timeout;
        fd_set fd;
        if (soap->accept_timeout > 0)
        { timeout.tv_sec = soap->accept_timeout;
          timeout.tv_usec = 0;
        }
        else
        { timeout.tv_sec = -soap->accept_timeout/1000000;
          timeout.tv_usec = -soap->accept_timeout%1000000;
        }
        FD_ZERO(&fd);
        FD_SET((SOAP_SOCKET)soap->master, &fd);
        for (;;)
        { int r = select((SOAP_SOCKET)(soap->master + 1), &fd, &fd, NULL, &timeout);
          if (r > 0)
            break;
          if (!r)
          { soap->errnum = 0;
            soap_set_receiver_error(soap, "Timeout", "TCP accept failed in soap_accept()", SOAP_TCP_ERROR);
            return -1;
          }
          if (soap_socket_errno != SOAP_EINTR)
          { soap->errnum = soap_socket_errno;
            soap_closesock(soap);
            soap_set_sender_error(soap, tcp_error(soap), "TCP accept failed in soap_accept()", SOAP_TCP_ERROR);
            return -1;
          }
        }
#if defined(WIN32)
	{ u_long nonblocking = 1;
          ioctlsocket((SOAP_SOCKET)soap->master, FIONBIO, &nonblocking);
        }
#elif defined(VXWORKS)
        { vx_nonblocking = TRUE;
          ioctl((SOAP_SOCKET)soap->master, FIONBIO, (int)(&vx_nonblocking));
        }
#else
        fcntl((SOAP_SOCKET)soap->master, F_SETFL, fcntl((SOAP_SOCKET)soap->master, F_GETFL)|O_NONBLOCK);
#endif
      }
      else
#if defined(WIN32)
      { u_long blocking = 0;
        ioctlsocket((SOAP_SOCKET)soap->master, FIONBIO, &blocking);
      }
/* WR[ */
#elif defined(VXWORKS)
      { vx_nonblocking = FALSE;
        ioctl((SOAP_SOCKET)soap->master, FIONBIO, (int)(&vx_nonblocking));
      }
/* ]WR */
#else
        fcntl((SOAP_SOCKET)soap->master, F_SETFL, fcntl((SOAP_SOCKET)soap->master, F_GETFL)&~O_NONBLOCK);
#endif
#endif
      if ((soap->socket = soap->faccept(soap, soap->master, (struct sockaddr*)&sockaddr, &n)) >= 0)
      {
/* WR[ */
#ifdef WITH_IPV6
/* Use soap->host to store the numeric form of the remote host */
        getnameinfo((struct sockaddr*)&sockaddr, n, soap->host, sizeof(soap->host), NULL, 0, NI_NUMERICHOST | NI_NUMERICSERV); 
        DBGLOG(TEST,SOAP_MESSAGE(fdebug, "Accept socket %d from %s\n", soap->socket, soap->host));
        soap->ip = 0; /* info stored in soap->host */
        soap->port = 0; /* info stored in soap->host */
#else /* WITH_IPV6 */
/* ]WR */
        soap->ip = ntohl(sockaddr.sin_addr.s_addr);
        soap->port = (int)ntohs(sockaddr.sin_port); /* does not return port number on some systems */
        DBGLOG(TEST,SOAP_MESSAGE(fdebug, "Accept socket %d at port %d from IP %d.%d.%d.%d\n", soap->socket, soap->port, (int)(soap->ip>>24)&0xFF, (int)(soap->ip>>16)&0xFF, (int)(soap->ip>>8)&0xFF, (int)soap->ip&0xFF));
/* WR[ */
#endif /* WITH_IPV6 */
/* ]WR */
        soap->keep_alive = ((soap->imode & SOAP_IO_KEEPALIVE) != 0);
#ifndef WITH_LEAN
	if (soap->accept_flags & SO_LINGER)
        { struct linger linger;
          memset((void*)&linger, 0, sizeof(linger));
          linger.l_onoff = 1;
          linger.l_linger = 0;
	  if (setsockopt((SOAP_SOCKET)soap->socket, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(struct linger)))
          { soap->errnum = soap_socket_errno;
	    soap_set_receiver_error(soap, tcp_error(soap), "TCP setsockopt SO_LINGER failed in soap_accept()", SOAP_TCP_ERROR);
            return -1;
	  }
        }
        if ((soap->accept_flags & ~SO_LINGER) && setsockopt((SOAP_SOCKET)soap->socket, SOL_SOCKET, soap->accept_flags & ~SO_LINGER, (char*)&set, sizeof(int)))
        { soap->errnum = soap_socket_errno;
          soap_set_receiver_error(soap, tcp_error(soap), "TCP setsockopt failed in soap_accept()", SOAP_TCP_ERROR);
          return -1;
        }
        if (soap->keep_alive && setsockopt((SOAP_SOCKET)soap->socket, SOL_SOCKET, SO_KEEPALIVE, (char*)&set, sizeof(int)))
        { soap->errnum = soap_socket_errno;
          soap_set_receiver_error(soap, tcp_error(soap), "TCP setsockopt SO_KEEPALIVE failed in soap_accept()", SOAP_TCP_ERROR);
          return -1;
        }
        if (setsockopt((SOAP_SOCKET)soap->socket, SOL_SOCKET, SO_SNDBUF, (char*)&len, sizeof(int)))
        { soap->errnum = soap_socket_errno;
          soap_set_receiver_error(soap, tcp_error(soap), "TCP setsockopt SO_SNDBUF failed in soap_accept()", SOAP_TCP_ERROR);
          return -1;
        }
        if (setsockopt((SOAP_SOCKET)soap->socket, SOL_SOCKET, SO_RCVBUF, (char*)&len, sizeof(int)))
        { soap->errnum = soap_socket_errno;
          soap_set_receiver_error(soap, tcp_error(soap), "TCP setsockopt SO_RCVBUF failed in soap_accept()", SOAP_TCP_ERROR);
          return -1;
        }
#ifdef TCP_NODELAY
        if (setsockopt((SOAP_SOCKET)soap->socket, IPPROTO_TCP, TCP_NODELAY, (char*)&set, sizeof(int)))
        { soap->errnum = soap_socket_errno;
          soap_set_receiver_error(soap, tcp_error(soap), "TCP setsockopt TCP_NODELAY failed in soap_accept()", SOAP_TCP_ERROR);
          return -1;
        }
#endif
#endif
        if (soap->accept_timeout)
        {
#if defined(WIN32)
          u_long blocking = 0;
          ioctlsocket((SOAP_SOCKET)soap->master, FIONBIO, &blocking);
          ioctlsocket((SOAP_SOCKET)soap->socket, FIONBIO, &blocking);
/* WR[ */
#elif defined(VXWORKS)
          vx_nonblocking = FALSE;
          ioctl((SOAP_SOCKET)soap->master, FIONBIO, (int)(&vx_nonblocking));
          ioctl((SOAP_SOCKET)soap->socket, FIONBIO, (int)(&vx_nonblocking));
/* ]WR */
#elif defined(PALM)
          fcntl((SOAP_SOCKET)soap->master, F_SETFL, fcntl((SOAP_SOCKET)soap->master, F_GETFL,0)&~O_NONBLOCK);
          fcntl((SOAP_SOCKET)soap->socket, F_SETFL, fcntl((SOAP_SOCKET)soap->socket, F_GETFL,0)&~O_NONBLOCK);
#elif defined(SYMBIAN)
          long blocking = 0;
          ioctl((SOAP_SOCKET)soap->master, 0/*FIONBIO*/, &blocking);
#else
          fcntl((SOAP_SOCKET)soap->master, F_SETFL, fcntl((SOAP_SOCKET)soap->master, F_GETFL)&~O_NONBLOCK);
          fcntl((SOAP_SOCKET)soap->socket, F_SETFL, fcntl((SOAP_SOCKET)soap->socket, F_GETFL)&~O_NONBLOCK);
#endif
	}
        return soap->socket;
      }
      if (soap_socket_errno != SOAP_EINTR && soap_socket_errno != SOAP_EAGAIN)
      { soap->errnum = soap_socket_errno;
        soap_set_receiver_error(soap, tcp_error(soap), "TCP accept failed in soap_accept()", SOAP_TCP_ERROR);
        return -1;
      }
    }
  }
  else
  { soap_set_receiver_error(soap, tcp_error(soap), "TCP no master socket in soap_accept()", SOAP_TCP_ERROR);
    return -1;
  }
}
#endif
#endif

/******************************************************************************/
#ifndef MAC_CARBON
#ifndef PALM_1
static int
tcp_disconnect(struct soap *soap)
{
#ifdef WITH_OPENSSL
  if (soap->ssl)
  { int r, s = 0;
    if (soap->session)
      SSL_SESSION_free(soap->session);
    if (*soap->host)
    { soap->session = SSL_get1_session(soap->ssl);
      if (soap->session)
      { strcpy(soap->session_host, soap->host);
        soap->session_port = soap->port;
      }
    }
    r = SSL_shutdown(soap->ssl);
    if (r != 1)
    { s = ERR_get_error();
      if (s)
      { if (soap_valid_socket(soap->socket))
        { soap->fshutdownsocket(soap, (SOAP_SOCKET)soap->socket, 1);
          soap->socket = SOAP_INVALID_SOCKET;
        }
        r = SSL_shutdown(soap->ssl);
      }
    }
    DBGLOG(TEST, if (s) SOAP_MESSAGE(fdebug, "Shutdown failed: %d\n", SSL_get_error(soap->ssl, r)));
    SSL_free(soap->ssl);
    soap->ssl = NULL;
    if (s)
      return SOAP_SSL_ERROR;
    ERR_remove_state(0);
  }
#endif
  if (soap_valid_socket(soap->socket))
  { DBGLOG(TEST,SOAP_MESSAGE(fdebug, "Closing socket %d\n", soap->socket));
    soap->fshutdownsocket(soap, (SOAP_SOCKET)soap->socket, 2);
    soap->fclosesocket(soap, (SOAP_SOCKET)soap->socket);
    soap->socket = SOAP_INVALID_SOCKET;
  }
  return SOAP_OK;
}
#endif
#endif

/******************************************************************************/
#ifndef MAC_CARBON
#ifndef PALM_1
static int
tcp_closesocket(struct soap *soap, SOAP_SOCKET fd)
{ return closesocket(fd);
}
#endif
#endif

/******************************************************************************/
#ifndef MAC_CARBON
#ifndef PALM_1
static int
tcp_shutdownsocket(struct soap *soap, SOAP_SOCKET fd, int how)
{ return shutdown(fd, how);
}
#endif
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
int
SOAP_FMAC2
soap_closesock(struct soap *soap)
{ register int status = soap->error;
#ifndef MAC_CARBON
  if (status == SOAP_EOF || status == SOAP_TCP_ERROR || status == SOAP_SSL_ERROR || !soap->keep_alive)
  { if ((soap->error = soap->fclose(soap)))
      return soap->error;
    soap->socket = SOAP_INVALID_SOCKET;
  }
#endif
#ifdef WITH_ZLIB
  if (soap->zlib_state == SOAP_ZLIB_DEFLATE)
    deflateEnd(&soap->d_stream);
  else if (soap->zlib_state == SOAP_ZLIB_INFLATE)
    inflateEnd(&soap->d_stream);
  soap->zlib_state = SOAP_ZLIB_NONE;
#endif
  return soap->error = status;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
size_t
SOAP_FMAC2
soap_hash(register const char *s)
{ register size_t h = 0;
  while (*s)
    h = 65599*h + *s++;
  return h % SOAP_IDHASH;
}
#endif

/******************************************************************************/
#ifndef PALM_1
static void
soap_init_pht(struct soap *soap)
{ register int i;
  DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Initializing pointer hashtable\n"));
  for (i = 0; i < (int)SOAP_PTRHASH; i++)
    soap->pht[i] = NULL;
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
struct soap*
SOAP_FMAC2
soap_new()
{ struct soap *soap = (struct soap*)SOAP_MALLOC(sizeof(struct soap));
  if (soap)
    soap_init(soap);
  return soap;
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
struct soap*
SOAP_FMAC2
soap_new1(int mode)
{ return soap_new2(mode, mode);
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
struct soap*
SOAP_FMAC2
soap_new2(int imode, int omode)
{ struct soap *soap = (struct soap*)SOAP_MALLOC(sizeof(struct soap));
  if (soap)
    soap_init2(soap, imode, omode);
  return soap;
}
#endif

/******************************************************************************/
#ifndef PALM_1
static void
soap_free_pht(struct soap *soap)
{ register struct soap_plist *pp, *next;
  register int i;
  DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Free pointer hashtable\n"));
  for (i = 0; i < (int)SOAP_PTRHASH; i++)
  { for (pp = soap->pht[i]; pp; pp = next)
    { next = pp->next;
      SOAP_FREE(pp);
    }
    soap->pht[i] = NULL;
  }
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_embed(struct soap *soap, const void *p, const struct soap_array *a, int n, const char *tag, int type)
{ register int i;
  struct soap_plist *pp;
  if (soap->version != 1)
    soap->encoding = 1;
  if (a)
    i = soap_array_pointer_lookup(soap, p, a, n, type, &pp);
  else
    i = soap_pointer_lookup(soap, p, type, &pp);
  if (i)
  { if (soap_is_embedded(soap, pp)
     || soap_is_single(soap, pp))
      return 0;
    soap_set_embedded(soap, pp);
  }
  return i;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_pointer_lookup(struct soap *soap, const void *p, int type, struct soap_plist **ppp)
{ register struct soap_plist *pp;
  *ppp = NULL;
  if (p)
    for (pp = soap->pht[soap_hash_ptr(p)]; pp; pp = pp->next)
      if (pp->ptr == p && pp->type == type)
      { *ppp = pp;
        DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Lookup location=%p type=%d id=%d\n", p, type, pp->id));
        return pp->id;
      }
  DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Lookup location=%p type=%d: not found\n", p, type));
  return 0;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_pointer_enter(struct soap *soap, const void *p, const struct soap_array *a, int n, int type, struct soap_plist **ppp)
{ register int h;
  register struct soap_plist *pp = *ppp = (struct soap_plist*)SOAP_MALLOC(sizeof(struct soap_plist));
  if (!pp)
    return 0;
  if (a)
    h = soap_hash_ptr(a->__ptr);
  else
    h = soap_hash_ptr(p);
  DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Pointer enter location=%p array=%p size=%d dim=%d type=%d id=%lu\n", p, a?a->__ptr:NULL, a?a->__size:0, n, type, soap->idnum+1));
  pp->next = soap->pht[h];
  pp->type = type;
  pp->mark1 = 0;
  pp->mark2 = 0;
  pp->ptr = p;
  pp->array = a;
  soap->pht[h] = pp;
  pp->id = ++soap->idnum;
  return pp->id;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_array_pointer_lookup(struct soap *soap, const void *p, const struct soap_array *a, int n, int type, struct soap_plist **ppp)
{ register struct soap_plist *pp;
  *ppp = NULL;
  if (!p || !a->__ptr)
    return 0;
  for (pp = soap->pht[soap_hash_ptr(a->__ptr)]; pp; pp = pp->next)
  { if (pp->type == type && pp->array && pp->array->__ptr == a->__ptr)
    { register int i;
      for (i = 0; i < n; i++)
        if (((const int*)&pp->array->__size)[i] != ((const int*)&a->__size)[i])
	  break;
      if (i == n)
      { *ppp = pp;
        DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Array lookup location=%p type=%d id=%d\n", a->__ptr, type, pp->id));
        return pp->id;
      }
    }
  }
  DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Array lookup location=%p type=%d: not found\n", a->__ptr, type));
  return 0;
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
void
SOAP_FMAC2
soap_begin_count(struct soap *soap)
{ soap_clr_attr(soap);
  soap_set_local_namespaces(soap);
#ifndef WITH_LEANER
  if ((soap->mode & SOAP_ENC_DIME) || (soap->omode & SOAP_ENC_DIME))
    soap->mode = soap->omode | SOAP_IO_LENGTH | SOAP_ENC_DIME;
  else
#endif
  { soap->mode = soap->omode;
    if ((soap->mode & SOAP_IO) == SOAP_IO_STORE
     || (((soap->mode & SOAP_IO) == SOAP_IO_CHUNK || (soap->mode & SOAP_ENC_XML)) && !soap->fpreparesend))
      soap->mode &= ~SOAP_IO_LENGTH;
    else
      soap->mode |= SOAP_IO_LENGTH;
  }
  if ((soap->mode & SOAP_ENC_ZLIB) && (soap->mode & SOAP_IO) == SOAP_IO_FLUSH)
  { if (!(soap->mode & SOAP_ENC_DIME))
      soap->mode &= ~SOAP_IO_LENGTH;
    if (soap->mode & SOAP_ENC_XML)
      soap->mode |= SOAP_IO_BUFFER;
    else
      soap->mode |= SOAP_IO_STORE;
  }
  if (!soap->encodingStyle && !(soap->mode & SOAP_XML_GRAPH))
    soap->mode |= SOAP_XML_TREE;
#ifndef WITH_LEANER
  if (soap->mode & SOAP_ENC_MIME)
    soap_select_mime_boundary(soap);
  soap->dime.list = soap->dime.last;	/* keep track of last DIME attachment */
#endif
  soap->count = 0;
  soap->ns = 0;
  soap->null = 0;
  soap->position = 0;
  soap->mustUnderstand = 0;
  soap->encoding = 0;
  soap->part = SOAP_BEGIN;
  soap->idnum = 0;
  soap->dime.count = 0; /* count # of attachments */
  soap->dime.size = 0; /* accumulate total size of attachments */
  DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Begin count phase (socket=%d mode=%x count=%lu)\n", soap->socket, soap->mode, (unsigned long)soap->count));
  if (soap->fprepareinit && (soap->mode & SOAP_IO) != SOAP_IO_STORE)
    soap->fprepareinit(soap);   
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
int
SOAP_FMAC2
soap_begin_send(struct soap *soap)
{ soap->error = SOAP_OK;
  soap_clr_attr(soap);
  soap_set_local_namespaces(soap);
  soap->mode = (soap->omode & ~SOAP_IO_LENGTH) | (soap->mode & SOAP_ENC_DIME);
  if ((soap->mode & SOAP_ENC_ZLIB) && (soap->mode & SOAP_IO) == SOAP_IO_FLUSH)
  { if (soap->mode & SOAP_ENC_XML)
      soap->mode |= SOAP_IO_BUFFER;
    else
      soap->mode |= SOAP_IO_STORE;
  }
  if ((soap->mode & SOAP_IO) == SOAP_IO_FLUSH && soap_valid_socket(soap->socket))
  { if (soap->count || (soap->mode & SOAP_ENC_XML))
      soap->mode |= SOAP_IO_BUFFER;
    else
      soap->mode |= SOAP_IO_STORE;
  }
  if ((soap->mode & SOAP_IO) == SOAP_IO_STORE)
    soap_new_block(soap);
  if (!(soap->mode & SOAP_IO_KEEPALIVE))
    soap->keep_alive = 0;
  if (!soap->encodingStyle && !(soap->mode & SOAP_XML_GRAPH))
    soap->mode |= SOAP_XML_TREE;
#ifndef WITH_LEANER
  if (soap->mode & SOAP_ENC_MIME)
    soap_select_mime_boundary(soap);
#endif
#ifdef WIN32
#ifndef UNDER_CE
#ifndef WITH_FASTCGI
  if (!soap_valid_socket(soap->socket)) /* Set win32 stdout or soap->sendfd to BINARY, e.g. to support DIME */
#ifdef __BORLANDC__
    setmode((SOAP_SOCKET)soap->sendfd, O_BINARY);
#else
    _setmode((SOAP_SOCKET)soap->sendfd, _O_BINARY);
#endif
#endif
#endif
#endif
  if (soap->mode & SOAP_IO)
  { soap->bufidx = 0;
    soap->buflen = 0;
  }
  soap->chunksize = 0;
  soap->ns = 0;
  soap->null = 0;
  soap->position = 0;
  soap->mustUnderstand = 0;
  soap->encoding = 0;
  soap->part = SOAP_BEGIN;
  soap->idnum = 0;
  soap->level = 0;
#ifdef WITH_ZLIB
  soap->z_ratio_out = 1.0;
  if ((soap->mode & SOAP_ENC_ZLIB) && soap->zlib_state != SOAP_ZLIB_DEFLATE)
  {
#ifdef WITH_GZIP
    memcpy(soap->z_buf, "\37\213\10\0\0\0\0\0\0\377", 10);
    soap->d_stream.next_out = (Byte*)soap->z_buf + 10;
    soap->d_stream.avail_out = SOAP_BUFLEN - 10;
    soap->z_crc = crc32(0L, NULL, 0);
    if (deflateInit2(&soap->d_stream, soap->z_level, Z_DEFLATED, -MAX_WBITS, 8, Z_DEFAULT_STRATEGY) != Z_OK)
#else
    soap->d_stream.next_out = (Byte*)soap->z_buf;
    soap->d_stream.avail_out = SOAP_BUFLEN;
    if (deflateInit(&soap->d_stream, soap->z_level) != Z_OK)
#endif
      return soap->error = SOAP_ZLIB_ERROR;
    DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Deflate initialized\n"));
    soap->zlib_state = SOAP_ZLIB_DEFLATE;
  }
#endif
  DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Begin send phase (socket=%d mode=%x count=%lu)\n", soap->socket, soap->mode, (unsigned long)soap->count));
  if (soap->fprepareinit && (soap->mode & SOAP_IO) == SOAP_IO_STORE)
    soap->fprepareinit(soap);   
  return SOAP_OK;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
void
SOAP_FMAC2
soap_embedded(struct soap *soap, const void *p, int t)
{ struct soap_plist *pp;
  if (soap_pointer_lookup(soap, p, t, &pp))
  { pp->mark1 = 1;
    pp->mark2 = 1;
    DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Embedded %p type=%d mark set to 1\n", p, t));
  }
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_reference(struct soap *soap, const void *p, int t)
{ struct soap_plist *pp;
  if (!p || (soap->mode & SOAP_XML_TREE))
    return 1;
  if (soap_pointer_lookup(soap, p, t, &pp))
  { if (pp->mark1 == 0)
    { pp->mark1 = 2;
      pp->mark2 = 2;
    }
  }
  else if (soap_pointer_enter(soap, p, NULL, 0, t, &pp))
  { pp->mark1 = 0;
    pp->mark2 = 0;
  }
  else
    return 1;
  DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Reference %p type=%d (%d %d)\n", p, t, (int)pp->mark1, (int)pp->mark2));
  return pp->mark1;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_array_reference(struct soap *soap, const void *p, const struct soap_array *a, int n, int t)
{ register int i;
  struct soap_plist *pp;
  if (!p)
    return 1;
  i = soap_array_pointer_lookup(soap, p, a, n, t, &pp);
  if (i)
  { if (pp->mark1 == 0)
    { pp->mark1 = 2;
      pp->mark2 = 2;
    }
  }
  else if (!soap_pointer_enter(soap, p, a, n, t, &pp))
    return 1;
  else
  { pp->mark1 = 0;
    pp->mark2 = 0;
  }
  DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Array reference %p ptr=%p dim=%d type=%d (%d %d)\n", p, a->__ptr, n, t, (int)pp->mark1, (int)pp->mark2));
  return pp->mark1;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_embedded_id(struct soap *soap, int id, const void *p, int t)
{ struct soap_plist *pp;
  DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Embedded_id %p type=%d id=%d\n", p, t, id));
  if (soap->mode & SOAP_XML_TREE)
    return id;
  if (soap->version == 1 && soap->encodingStyle && !(soap->mode & SOAP_XML_GRAPH) && soap->part != SOAP_IN_HEADER)
  { if (id < 0)
    { id = soap_pointer_lookup(soap, p, t, &pp);
      if (id)
      { if (soap->mode & SOAP_IO_LENGTH)
          pp->mark1 = 2;
        else
          pp->mark2 = 2;
        DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Embedded_id multiref id=%d %p type=%d = (%d %d)\n", id, p, t, (int)pp->mark1, (int)pp->mark2));
      }
      return -1;
    }
    return id;
  }
  if (id < 0)
    id = soap_pointer_lookup(soap, p, t, &pp);
  else if (id && !soap_pointer_lookup(soap, p, t, &pp))
    return 0;
  if (id && pp)
  { if (soap->mode & SOAP_IO_LENGTH)
      pp->mark1 = 1;
    else
      pp->mark2 = 1;
    DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Embedded_id embedded ref id=%d %p type=%d = (%d %d)\n", id, p, t, (int)pp->mark1, (int)pp->mark2));
  }
  return id;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_is_embedded(struct soap *soap, struct soap_plist *pp)
{ if (!pp)
    return 0;
  DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Is embedded? %d %d\n", (int)pp->mark1, (int)pp->mark2));
  if (soap->version == 1 && soap->encodingStyle && !(soap->mode & SOAP_XML_GRAPH) && soap->part != SOAP_IN_HEADER)
  { if (soap->mode & SOAP_IO_LENGTH)
      return pp->mark1 != 0;
    return pp->mark2 != 0;
  }
  if (soap->mode & SOAP_IO_LENGTH)
    return pp->mark1 == 1;
  return pp->mark2 == 1;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_is_single(struct soap *soap, struct soap_plist *pp)
{ if (soap->part == SOAP_IN_HEADER)
    return 1;
  if (!pp)
    return 0;
  if (soap->mode & SOAP_IO_LENGTH)
    return pp->mark1 == 0;
  return pp->mark2 == 0;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
void
SOAP_FMAC2
soap_set_embedded(struct soap *soap, struct soap_plist *pp)
{ if (!pp)
    return;
  if (soap->mode & SOAP_IO_LENGTH)
    pp->mark1 = 1;
  else
    pp->mark2 = 1;
}
#endif

/******************************************************************************/
#ifndef WITH_LEANER
#ifndef PALM_1
SOAP_FMAC1
int
SOAP_FMAC2
soap_element_dime(struct soap *soap, const char *tag, int id, const void *p, const struct soap_array *a, const char *aid, const char *atype, const char *aoptions, int n, const char *type, int t) 
{ struct soap_plist *pp;
  if (!p || !a->__ptr || (!aid && !atype))
    return soap_element_id(soap, tag, id, p, a, n, type, t);
  DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Attachment tag='%s' id='%s' (%d) type='%s'\n", tag, aid?aid:"", id, atype?atype:""));
  if (id < 0)
    id = soap_array_pointer_lookup(soap, p, a, n, t, &pp);
  if (!aid)
  { sprintf(soap->tmpbuf, soap->dime_id_format, id);
    aid = soap_strdup(soap, soap->tmpbuf);
  }
  if (soap_element_href(soap, tag, 0, "href", aid))
    return soap->error;
  if (soap->mode & SOAP_IO_LENGTH)
  { if (pp->mark1 != 3)
    { struct soap_multipart *content = soap_new_multipart(soap, &soap->dime.first, &soap->dime.last, (char*)a->__ptr, a->__size);
      if (!content)
        return soap->error = SOAP_EOM;
      content->id = aid;
      content->type = atype;
      content->options = aoptions;
      pp->mark1 = 3;
    }
  }
  else
    pp->mark2 = 3;
  return SOAP_OK;
}
#endif
#endif

/******************************************************************************/
#ifndef PALM_1
static void
soap_init_iht(struct soap *soap)
{ register int i;
  DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Initializing ID hashtable\n"));
  for (i = 0; i < SOAP_IDHASH; i++)
    soap->iht[i] = NULL;
}
#endif

/******************************************************************************/
#ifndef PALM_1
static void
soap_free_iht(struct soap *soap)
{ register int i;
  register struct soap_ilist *ip, *p;
  register struct soap_flist *fp, *fq;
  DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Free ID hashtable\n"));
  for (i = 0; i < SOAP_IDHASH; i++)
  { for (ip = soap->iht[i]; ip; ip = p)
    { for (fp = ip->flist; fp; fp = fq)
      { fq = fp->next;
        SOAP_FREE(fp);
      }
      p = ip->next;
      SOAP_FREE(ip);
    }
    soap->iht[i] = NULL;
  }
}
#endif

/******************************************************************************/
#ifndef PALM_2
static struct soap_ilist *
soap_hlookup(struct soap *soap, const char *id)
{ register struct soap_ilist *ip;
  for (ip = soap->iht[soap_hash(id)]; ip; ip = ip->next)
    if (!strcmp(ip->id, id))
      return ip;
  return NULL;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
struct soap_ilist *
SOAP_FMAC2
soap_lookup(struct soap *soap, const char *id)
{ register struct soap_ilist *ip;
  ip = soap_hlookup(soap, id);
#ifndef WITH_LEANER
  if (!ip && *id != '#' && !strchr(id, ':')) /* try content id "cid:" with DIME attachments */
  { char cid[SOAP_TAGLEN];
    strcpy(cid, "cid:");
    strncat(cid + 4, id, sizeof(cid) - 5);
    cid[sizeof(cid) - 1] = '\0';
    ip = soap_hlookup(soap, cid);
  }
#endif
  return ip;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
struct soap_ilist *
SOAP_FMAC2
soap_enter(struct soap *soap, const char *id)
{ register size_t h;
  register struct soap_ilist *ip;
  ip = (struct soap_ilist*)SOAP_MALLOC(sizeof(struct soap_ilist) + strlen(id));
  if (ip)
  { h = soap_hash(id);
    strcpy(ip->id, id);
    ip->next = soap->iht[h];
    soap->iht[h] = ip;
    return ip;
  }
  return NULL;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
void*
SOAP_FMAC2
soap_malloc(struct soap *soap, size_t n)
{ register char *p;
  if (!n)
    return NULL;
  if (!soap)
    return SOAP_MALLOC(n);
  n += (-(long)n) & 7;
  if (!(p = (char*)SOAP_MALLOC(n + sizeof(void*) + sizeof(size_t))))
  { soap->error = SOAP_EOM;
    return NULL;
  }
  /* keep chain of alloced cells for later destruction */
  soap->alloced = 1;
  *(void**)(p + n) = soap->alist;
  *(size_t*)(p + n + sizeof(void*)) = n;
  soap->alist = p + n;
  DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Malloc %u bytes at location %p\n", (unsigned int)n, p));
  return p;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
void
SOAP_FMAC2
soap_dealloc(struct soap *soap, void *p)
{ if (!soap)
    return;
  if (p)
  { register char **q;
    for (q = (char**)&soap->alist; *q; q = *(char***)q)
    { if (p == (void*)(*q - *(size_t*)(*q + sizeof(void*))))
      { *q = **(char***)q;
        DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Freed data at %p\n", p));
        SOAP_FREE(p);
        return;
      }
    }
    soap_delete(soap, p);
  }
  else
  { register char *q;
    while (soap->alist)
    { q = (char*)soap->alist;
      soap->alist = *(void**)q;
      q -= *(size_t*)(q + sizeof(void*));
      SOAP_FREE(q);
    }
    DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Dealloc all data done\n"));
  }
  /* we must assume these were deallocated: */
  soap->action = NULL;
  soap->fault = NULL;
  soap->header = NULL;
  soap->authrealm = NULL;
#ifndef WITH_LEANER
  soap_clr_mime(soap);
#endif
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
void
SOAP_FMAC2
soap_delete(struct soap *soap, void *p)
{ register struct soap_clist **cp = &soap->clist;
  if (p)
  { while (*cp)
    { if (p == (*cp)->ptr)
      { register struct soap_clist *q = *cp;
        *cp = q->next;
        q->fdelete(q);
        SOAP_FREE(q);
        return;
      }
      cp = &(*cp)->next;
    }
    DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Could not dealloc data %p: address not in list\n", p));
  }
  else
  { while (*cp)
    { register struct soap_clist *q = *cp;
      *cp = q->next;
      if (q->ptr == (void*)soap->fault)
        soap->fault = NULL; /* this was deallocated */
      else if (q->ptr == (void*)soap->header)
        soap->header = NULL; /* this was deallocated */
      q->fdelete(q);
      SOAP_FREE(q);
    }
  }
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
struct soap_clist *
SOAP_FMAC2
soap_link(struct soap *soap, void *p, int t, int n, void (*fdelete)(struct soap_clist*))
{ register struct soap_clist *cp;
  if ((cp = (struct soap_clist*)SOAP_MALLOC(sizeof(struct soap_clist))))
  { cp->next = soap->clist;
    cp->type = t;
    cp->size = n; 
    cp->ptr = p;
    cp->fdelete = fdelete;
    soap->clist = cp;
  }
  return cp;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
void
SOAP_FMAC2
soap_unlink(struct soap *soap, const void *p)
{ register char **q;
  register struct soap_clist **cp;
  if (!soap || !p)
    return;
  for (q = (char**)&soap->alist; *q; q = *(char***)q)
  { if (p == (void*)(*q - *(size_t*)(*q + sizeof(void*))))
    { *q = **(char***)q;
      DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Unlinked data %p\n", p));
      return;
    }
  }
  for (cp = &soap->clist; *cp; cp = &(*cp)->next)
  { if (p == (*cp)->ptr)
    { DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Unlinked class instance %p\n", p));
      q = (char**)*cp;
      *cp = (*cp)->next;
      SOAP_FREE(q);
      return;
    }
  }
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_lookup_type(struct soap *soap, const char *id)
{ register struct soap_ilist *ip;
  if (id && *id)
  { ip = soap_lookup(soap, id);
    if (ip)
    { DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Lookup id='%s' type=%d\n", id, ip->type));
      return ip->type;
    }
  }
  DBGLOG(TEST, SOAP_MESSAGE(fdebug, "lookup type id='%s' NOT FOUND! Need to get it from xsi:type\n", id));
  return 0;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
void*
SOAP_FMAC2
soap_id_lookup(struct soap *soap, const char *id, void **p, int t, size_t n, unsigned int k)
{ struct soap_ilist *ip;
  void **q;
  if (!id || !*id)
    return p;
  soap->alloced = 0;
  if (!p)
    p = (void**)soap_malloc(soap, sizeof(void*));
  ip = soap_lookup(soap, id); /* lookup pointer to hash table entry for string id */
  if (!ip)
  { ip = soap_enter(soap, id); /* new hash table entry for string id */
    DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Forwarding first href='%s' type=%d %p (%u bytes)\n", id, t, p, (unsigned int)n));
    ip->type = t;
    ip->size = n; 
    ip->link = p;
    ip->copy = NULL;
    ip->flist = NULL;
    ip->ptr = NULL;
    ip->level = k;
    *p = NULL;
  }
  else if (ip->ptr)
  { DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Resolved href='%s' type=%d location=%p (%u bytes)\n", id, t, ip->ptr, (unsigned int)n));
    if (ip->type != t)
    { strcpy(soap->id, id);
      soap->error = SOAP_HREF;
      DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Type incompatibility: id type=%d href type=%d\n", ip->type, t));
      return NULL;
    }
    while (ip->level < k)
    { q = (void**)soap_malloc(soap, sizeof(void*));  
      if (!q)
        return NULL;
      *p = (void*)q;
      p = q;
      k--;
      DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Descending one level...\n"));
    }
    *p = ip->ptr;
  }
  else if (ip->level > k)
  { DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Resolving level %u pointers to href='%s'\n", ip->level, id));
    while (ip->level > k)
    { void *s, **r = &ip->link;
      q = (void**)ip->link;
      while (q)
      { *r = (void*)soap_malloc(soap, sizeof(void*));
        s = *q;
        *q = *r;
        r = (void**)*r;
        q = (void**)s;
      }
      *r = NULL;
      ip->size = n; 
      ip->copy = NULL;
      ip->level = ip->level - 1;
      DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Descending one level...\n"));
    }
    q = (void**)ip->link;
    ip->link = p;
    *p = (void*)q;
  }
  else
  { DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Forwarded href='%s' type=%d location=%p (%u bytes)\n", id, t, p, (unsigned int)n));
    while (ip->level < k)
    { q = (void**)soap_malloc(soap, sizeof(void*));  
      *p = q;
      p = q;
      k--;
      DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Descending one level...\n"));
    }
    q = (void**)ip->link;
    ip->link = p;
    *p = (void*)q;
  }
  return p;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
void*
SOAP_FMAC2
soap_id_forward(struct soap *soap, const char *href, void *p, int st, int tt, size_t n, unsigned int k, void (*fcopy)(struct soap*, int, int, void*, const void*, size_t))
{ struct soap_ilist *ip;
  if (!p || !href || !*href)
    return p;
  ip = soap_lookup(soap, href); /* lookup pointer to hash table entry for string id */
  if (!ip)
  { ip = soap_enter(soap, href); /* new hash table entry for string id */
    ip->type = st;
    ip->size = n;
    ip->link = NULL;
    ip->copy = NULL;
    ip->ptr = NULL;
    ip->level = 0;
    ip->flist = NULL;
    DBGLOG(TEST,SOAP_MESSAGE(fdebug, "New entry href='%s' type=%d size=%lu level=%d location=%p\n", href, st, (unsigned long)n, k, p));
  }
  else if (ip->type != st || (ip->level == k && ip->size != n))
  { DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Type incompatibility id='%s' expect type=%d size=%lu level=%u got type=%d size=%lu\n", href, ip->type, (unsigned long)ip->size, k, st, (unsigned long)n));
    strcpy(soap->id, href);
    soap->error = SOAP_HREF;
    return NULL;
  }
  if (fcopy || n < sizeof(void*) || *href != '#')
  { register struct soap_flist *fp = (struct soap_flist*)SOAP_MALLOC(sizeof(struct soap_flist));
    if (!fp)
    { soap->error = SOAP_EOM;
      return NULL;
    }
    fp->next = ip->flist;
    fp->type = tt;
    fp->ptr = p;
    fp->level = k;
    if (fcopy)
      fp->fcopy = fcopy;
    else
      fp->fcopy = soap_fcopy;
    ip->flist = fp;
    DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Forwarding type=%d (target type=%d) size=%lu location=%p level=%u href='%s'\n", st, tt, (unsigned long)n, p, k, href));
  }
  else
  { DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Forwarding copying address %p for type=%d href='%s'\n", p, st, href));
    *(void**)p = ip->copy;
    ip->copy = p;
  }
  return p;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
void*
SOAP_FMAC2
soap_id_enter(struct soap *soap, const char *id, void *p, int t, size_t n, unsigned int k, const char *type, const char *arrayType, void *(*finstantiate)(struct soap*, int, const char*, const char*, size_t*))
{ struct soap_ilist *ip;
  DBGLOG(TEST,SOAP_MESSAGE(fdebug, "Enter id='%s' type=%d loc=%p size=%lu level=%u\n", id, t, p, (unsigned long)n, k));
  soap->alloced = 0;
  if (!p)
  { if (finstantiate)
      p = finstantiate(soap, t, type, arrayType, &n);
    else
      p = soap_malloc(soap, n);
    if (p)
      soap->alloced = 1;
  }
  if (!id || !*id)
    return p;
  ip = soap_lookup(soap, id); /* lookup pointer to hash table entry for string id */
  DBGLOG(TEST,SOAP_MESSAGE(fdebug, "Lookup entry id='%s for location=%p'\n", id, p));
  if (!ip)
  { ip = soap_enter(soap, id); /* new hash table entry for string id */
    ip->type = t;
    ip->link = NULL;
    ip->copy = NULL;
    ip->flist = NULL;
    ip->size = n;
    ip->ptr = p;
    ip->level = k;
    DBGLOG(TEST,SOAP_MESSAGE(fdebug, "New entry id='%s' type=%d size=%lu level=%u location=%p\n", id, t, (unsigned long)n, k, p));
  }
  else if ((ip->type != t || (ip->level == k && ip->size != n)) && (ip->copy || ip->flist))
  { DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Type incompatibility id='%s' expect type=%d size=%lu level=%u got type=%d size=%lu\n", id, ip->type, (unsigned long)ip->size, k, t, (unsigned long)n));
    strcpy(soap->id, id);
    soap->error = SOAP_HREF;
    return NULL;
  }
  else if (ip->ptr)
  { DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Multiply defined id='%s'\n", id));
    strcpy(soap->id, id);
    soap->error = SOAP_MULTI_ID;
    return NULL;
  }
  else 
  { ip->size = n;
    ip->ptr = p;
    ip->level = k;
    DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Update entry id='%s' type=%d location=%p size=%lu level=%u\n", id, t, p, (unsigned long)n, k));
  }
  return ip->ptr;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
void
SOAP_FMAC2
soap_fcopy(struct soap *soap, int st, int tt, void *p, const void *q, size_t n)
{ DBGLOG(TEST,SOAP_MESSAGE(fdebug, "Copying data type=%d (target type=%d) %p -> %p (%lu bytes)\n", st, tt, q, p, (unsigned long)n));
  memcpy(p, q, n);
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
int
SOAP_FMAC2
soap_end_send(struct soap *soap)
{ 
#ifndef WITH_LEANER
  if (soap->dime.list)
  { /* SOAP body referenced attachments must appear first */
    soap->dime.last->next = soap->dime.first;
    soap->dime.first = soap->dime.list->next;
    soap->dime.list->next = NULL;
    soap->dime.last = soap->dime.list;
  }
  if (soap_putdime(soap) || soap_putmime(soap))
    return soap->error;
  soap->mime.list = NULL;
  soap->mime.first = NULL;
  soap->mime.last = NULL;
  soap->dime.list = NULL;
  soap->dime.first = NULL;
  soap->dime.last = NULL;
#endif
  DBGLOG(TEST, SOAP_MESSAGE(fdebug, "End send\n"));
  if (soap->mode & SOAP_IO) /* need to flush the remaining data in buffer */
  { if (soap_flush(soap))
#ifdef WITH_ZLIB
    { if (soap->mode & SOAP_ENC_ZLIB && soap->zlib_state == SOAP_ZLIB_DEFLATE)
      { soap->zlib_state = SOAP_ZLIB_NONE;
        deflateEnd(&soap->d_stream);
      }
      return soap->error;
    }
#else
      return soap->error;
#endif
#ifdef WITH_ZLIB
    if (soap->mode & SOAP_ENC_ZLIB)
    { int r;
      soap->d_stream.avail_in = 0;
      do
      { DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Deflating remainder\n"));
        r = deflate(&soap->d_stream, Z_FINISH);
        if (soap->d_stream.avail_out != SOAP_BUFLEN)
        { if (soap_flush_raw(soap, soap->z_buf, SOAP_BUFLEN - soap->d_stream.avail_out))
          { soap->zlib_state = SOAP_ZLIB_NONE;
            deflateEnd(&soap->d_stream);
            return soap->error;
	  }
          soap->d_stream.next_out = (Byte*)soap->z_buf;
          soap->d_stream.avail_out = SOAP_BUFLEN;
        }
      } while (r == Z_OK);
      DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Deflated %lu->%lu bytes\n", soap->d_stream.total_in, soap->d_stream.total_out));
      soap->z_ratio_out = (float)soap->d_stream.total_out / (float)soap->d_stream.total_in;
      soap->mode &= ~SOAP_ENC_ZLIB;
      soap->zlib_state = SOAP_ZLIB_NONE;
      if (deflateEnd(&soap->d_stream) != Z_OK || r != Z_STREAM_END)
      { DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Unable to end deflate: %s\n", soap->d_stream.msg?soap->d_stream.msg:""));
        return soap->error = SOAP_ZLIB_ERROR;
      }
#ifdef WITH_GZIP
      soap->z_buf[0] = soap->z_crc & 0xFF;
      soap->z_buf[1] = (soap->z_crc >> 8) & 0xFF;
      soap->z_buf[2] = (soap->z_crc >> 16) & 0xFF;
      soap->z_buf[3] = (soap->z_crc >> 24) & 0xFF;
      soap->z_buf[4] = soap->d_stream.total_in & 0xFF;
      soap->z_buf[5] = (soap->d_stream.total_in >> 8) & 0xFF;
      soap->z_buf[6] = (soap->d_stream.total_in >> 16) & 0xFF;
      soap->z_buf[7] = (soap->d_stream.total_in >> 24) & 0xFF;
      if (soap_flush_raw(soap, soap->z_buf, 8))
        return soap->error;
      DBGLOG(TEST, SOAP_MESSAGE(fdebug, "gzip crc32=%lu\n", (unsigned long)soap->z_crc));
#endif
    }
#endif
    if ((soap->mode & SOAP_IO) == SOAP_IO_STORE)
    { char *p;
      if (!(soap->mode & SOAP_ENC_XML))
      { soap->mode--;
        DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Sending buffered message of length %u\n", (unsigned int)soap->blist->size));
        if (soap->status >= SOAP_POST)
          soap->error = soap->fpost(soap, soap->endpoint, soap->host, soap->port, soap->path, soap->action, soap->blist->size);
        else if (soap->status != SOAP_STOP)
          soap->error = soap->fresponse(soap, soap->status, soap->blist->size);
        if (soap->error || soap_flush(soap))
          return soap->error;
        soap->mode++;
      }
      for (p = soap_first_block(soap); p; p = soap_next_block(soap))
      { DBGMSG(SENT, p, soap_block_size(soap));
        if ((soap->error = soap->fsend(soap, p, soap_block_size(soap))))
        { soap_end_block(soap);
          return soap->error;
        }
      }
      soap_end_block(soap);
    }
    else if ((soap->mode & SOAP_IO) == SOAP_IO_CHUNK)
    { DBGMSG(SENT, "\r\n0\r\n\r\n", 7);
      if ((soap->error = soap->fsend(soap, "\r\n0\r\n\r\n", 7)))
        return soap->error;
    }
  }
#ifdef WITH_OPENSSL
  if (!soap->ssl && soap_valid_socket(soap->socket) && !soap->keep_alive)
    soap->fshutdownsocket(soap, (SOAP_SOCKET)soap->socket, 1); /* Send TCP FIN */
#else
  if (soap_valid_socket(soap->socket) && !soap->keep_alive)
    soap->fshutdownsocket(soap, (SOAP_SOCKET)soap->socket, 1); /* Send TCP FIN */
#endif
  DBGLOG(TEST, SOAP_MESSAGE(fdebug, "End of send message ok\n"));
  soap->part = SOAP_END;
  soap->count = 0;
  return SOAP_OK;
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
int
SOAP_FMAC2
soap_end_recv(struct soap *soap)
{ soap->part = SOAP_END;
#ifndef WITH_LEANER
  if ((soap->mode & SOAP_ENC_MIME) && soap_getmime(soap))
    return soap->error;
  soap->mime.list = soap->mime.first;
  soap->mime.first = NULL;
  soap->mime.last = NULL;
  soap->dime.list = soap->dime.first;
  soap->dime.first = NULL;
  soap->dime.last = NULL;
#endif
  DBGLOG(TEST,SOAP_MESSAGE(fdebug, "End of receive message ok\n"));
#ifdef WITH_ZLIB
  if (soap->mode & SOAP_ENC_ZLIB)
  { soap->mode &= ~SOAP_ENC_ZLIB;
    memcpy(soap->buf, soap->z_buf, SOAP_BUFLEN);
    soap->bufidx = (char*)soap->d_stream.next_in - soap->z_buf;
    soap->buflen = soap->z_buflen;
    soap->zlib_state = SOAP_ZLIB_NONE;
    if (inflateEnd(&soap->d_stream) != Z_OK)
      return soap->error = SOAP_ZLIB_ERROR;
#ifdef WITH_GZIP
    if (soap->zlib_in == SOAP_ZLIB_GZIP)
    { soap_wchar c;
      short i;
      for (i = 0; i < 8; i++)
      { if ((int)(c = soap_getchar(soap)) == EOF)
          return soap->error = SOAP_EOF;
        soap->z_buf[i] = (char)c;
      }
      if (soap->z_crc != ((uLong)(unsigned char)soap->z_buf[0] | ((uLong)(unsigned char)soap->z_buf[1] << 8) | ((uLong)(unsigned char)soap->z_buf[2] << 16) | ((uLong)(unsigned char)soap->z_buf[3] << 24)))
      { DBGLOG(TEST,SOAP_MESSAGE(fdebug, "Gzip error: crc check failed, message corrupted? (crc32=%lu)\n", (unsigned long)soap->z_crc));
        return soap->error = SOAP_ZLIB_ERROR;
      }
      if (soap->d_stream.total_out != ((uLong)(unsigned char)soap->z_buf[4] | ((uLong)(unsigned char)soap->z_buf[5] << 8) | ((uLong)(unsigned char)soap->z_buf[6] << 16) | ((uLong)(unsigned char)soap->z_buf[7] << 24)))
      { DBGLOG(TEST,SOAP_MESSAGE(fdebug, "Gzip error: incorrect message length\n"));
        return soap->error = SOAP_ZLIB_ERROR;
      }
    }
#endif
  }
#endif
  if ((soap->mode & SOAP_IO) == SOAP_IO_CHUNK)
    while ((int)soap_getchar(soap) != EOF) /* advance to last chunk */
      ;
  if (soap->fdisconnect && (soap->error = soap->fdisconnect(soap)))
    return soap->error;
  return soap_resolve(soap);
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
void
SOAP_FMAC2
soap_free(struct soap *soap)
{ register struct soap_nlist *np;
  register struct soap_attribute *tp;
  register struct Namespace *ns;
  DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Free namespace stack\n"));
  while (soap->nlist)
  { np = soap->nlist->next;
    if (soap->nlist->ns)
      SOAP_FREE(soap->nlist->ns);
    SOAP_FREE(soap->nlist);
    soap->nlist = np;
  }
  DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Free any remaining temp blocks\n"));
  while (soap->blist)
    soap_end_block(soap);
  DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Free attributes\n"));
  while (soap->attributes)
  { tp = soap->attributes->next;
    if (soap->attributes->value)
      SOAP_FREE(soap->attributes->value);
    SOAP_FREE(soap->attributes);
    soap->attributes = tp;
  }
#ifdef WITH_FAST
  if (soap->labbuf)
    SOAP_FREE(soap->labbuf);
  soap->labbuf = NULL;
  soap->lablen = 0;
  soap->labidx = 0;
#endif
  soap_free_pht(soap);
  soap_free_iht(soap);
  ns = soap->local_namespaces;
  if (ns)
  { for (; ns->id; ns++)
    { if (ns->out)
      { SOAP_FREE(ns->out);
	if (soap->encodingStyle == ns->out)
          soap->encodingStyle = SOAP_STR_EOS;
        ns->out = NULL;
      }
      if (soap->encodingStyle == ns->ns)
        soap->encodingStyle = SOAP_STR_EOS;
    }
    SOAP_FREE(soap->local_namespaces);
    soap->local_namespaces = NULL;
  }
}
#endif

/******************************************************************************/
#ifdef SOAP_DEBUG
static void
soap_init_logs(struct soap *soap)
{ int i;
  for (i = 0; i < SOAP_MAXLOGS; i++)
  { soap->logfile[i] = NULL;
    soap->fdebug[i] = NULL;
  }
}
#endif

/******************************************************************************/
#if !defined(WITH_LEAN) || defined(SOAP_DEBUG)
SOAP_FMAC1
void
SOAP_FMAC2
soap_open_logfile(struct soap *soap, int i)
{ if (soap->logfile[i])
    soap->fdebug[i] = fopen(soap->logfile[i], i < 2 ? "ab" : "a");
}
#endif

/******************************************************************************/
#ifdef SOAP_DEBUG
static void
soap_close_logfile(struct soap *soap, int i)
{ if (soap->fdebug[i])
  { fclose(soap->fdebug[i]);
    soap->fdebug[i] = NULL;
  }
}
#endif

/******************************************************************************/
#ifdef SOAP_DEBUG
SOAP_FMAC1
void
SOAP_FMAC2
soap_close_logfiles(struct soap *soap)
{ int i;
  for (i = 0; i < SOAP_MAXLOGS; i++)
    soap_close_logfile(soap, i);
}
#endif

/******************************************************************************/
#ifdef SOAP_DEBUG
static void
soap_set_logfile(struct soap *soap, int i, const char *logfile)
{ char *s = NULL;
  soap_close_logfile(soap, i);
  if (soap->logfile[i])
    SOAP_FREE((void*)soap->logfile[i]);
  if (logfile)
    if ((s = (char*)SOAP_MALLOC(strlen(logfile) + 1)))
      strcpy(s, logfile);
  soap->logfile[i] = s;
}
#endif

/******************************************************************************/
#ifdef SOAP_DEBUG
SOAP_FMAC1
void
SOAP_FMAC2
soap_set_recv_logfile(struct soap *soap, const char *logfile)
{ soap_set_logfile(soap, SOAP_INDEX_RECV, logfile);
}
#endif

/******************************************************************************/
#ifdef SOAP_DEBUG
SOAP_FMAC1
void
SOAP_FMAC2
soap_set_sent_logfile(struct soap *soap, const char *logfile)
{ soap_set_logfile(soap, SOAP_INDEX_SENT, logfile);
}
#endif

/******************************************************************************/
#ifdef SOAP_DEBUG
SOAP_FMAC1
void
SOAP_FMAC2
soap_set_test_logfile(struct soap *soap, const char *logfile)
{ soap_set_logfile(soap, SOAP_INDEX_TEST, logfile);
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
struct soap*
SOAP_FMAC2
soap_copy(struct soap *soap)
{ return soap_copy_context((struct soap*)SOAP_MALLOC(sizeof(struct soap)), soap);
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
struct soap*
SOAP_FMAC2
soap_copy_context(struct soap *copy, struct soap *soap)
{ if (copy)
  { register struct soap_plugin *p;
    memcpy(copy, soap, sizeof(struct soap));
    copy->copy = 1;
    copy->user = NULL;
    copy->userid = NULL;
    copy->passwd = NULL;
    copy->nlist = NULL;
    copy->blist = NULL;
    copy->clist = NULL;
    copy->alist = NULL;
    copy->attributes = NULL;
    copy->local_namespaces = NULL;
    soap_set_local_namespaces(copy);
    soap_init_iht(copy);
    soap_init_pht(copy);
    copy->header = NULL;
    copy->fault = NULL;
    copy->action = NULL;
    *copy->host = '\0';
#ifndef WITH_LEAN
#ifdef WITH_COOKIES
    copy->cookies = soap_copy_cookies(soap);
#else
    copy->cookies = NULL;
#endif
#endif
#ifdef SOAP_DEBUG
    soap_init_logs(copy);
    soap_set_recv_logfile(copy, "RECV.log");
    soap_set_sent_logfile(copy, "SENT.log");
    soap_set_test_logfile(copy, "TEST.log");
#endif
    copy->plugins = NULL;
    for (p = soap->plugins; p; p = p->next)
    { register struct soap_plugin *q = (struct soap_plugin*)SOAP_MALLOC(sizeof(struct soap_plugin));
      if (!q)
        return NULL;
      DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Copying plugin '%s'\n", p->id));
      *q = *p;
      if (p->fcopy && (soap->error = p->fcopy(soap, q, p)))
      { SOAP_FREE(q);
        return NULL;
      }
      q->next = copy->plugins;
      copy->plugins = q;
    }
  }
  else
    soap->error = SOAP_EOM;
  return copy;
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
void
SOAP_FMAC2
soap_init(struct soap *soap)
{ soap->version = 0;
  soap_imode(soap, SOAP_IO_DEFAULT);
  soap_omode(soap, SOAP_IO_DEFAULT);
  soap->copy = 0;
  soap->plugins = NULL;
  soap->user = NULL;
  soap->userid = NULL;
  soap->passwd = NULL;
  soap->fpost = http_post;
  soap->fget = http_get;
  soap->fposthdr = http_post_header;
  soap->fresponse = http_response;
  soap->fparse = http_parse;
  soap->fparsehdr = http_parse_header;
  soap->fconnect = NULL;
  soap->fdisconnect = NULL;
#ifndef MAC_CARBON
#ifndef WITH_IPV6
  soap->fresolve = tcp_gethost;
#else
  soap->fresolve = NULL;
#endif
  soap->faccept = tcp_accept;
  soap->fopen = tcp_connect;
  soap->fclose = tcp_disconnect;
  soap->fclosesocket = tcp_closesocket;
  soap->fshutdownsocket = tcp_shutdownsocket;
  soap->fsend = fsend;
  soap->frecv = frecv;
  soap->fpoll = soap_poll;
#else
  soap->fpoll = NULL;
#endif
  soap->fprepareinit = NULL;
  soap->fpreparesend = NULL;
  soap->fpreparerecv = NULL;
  soap->fignore = NULL;
  soap->fserveloop = NULL;
  soap->fplugin = fplugin;
  soap->fdimereadopen = NULL;
  soap->fdimewriteopen = NULL;
  soap->fdimereadclose = NULL;
  soap->fdimewriteclose = NULL;
  soap->fdimeread = NULL;
  soap->fdimewrite = NULL;
  soap->float_format = "%.8g"; /* .8 preserves single FP precision as much as possible, but might not be very efficient */
  soap->double_format = "%.17lg"; /* .17 preserves double FP precision as much as possible, but might not be very efficient */
  soap->dime_id_format = "cid:id%d"; /* default DIME id format */
  soap->http_version = "1.1";
  soap->actor = NULL;
  soap->max_keep_alive = SOAP_MAXKEEPALIVE;
  soap->keep_alive = 0;
  soap->recv_timeout = 0;
  soap->send_timeout = 0;
  soap->connect_timeout = 0;
  soap->accept_timeout = 0;
  soap->socket_flags = 0;
  soap->connect_flags = 0;
  soap->bind_flags = 0;
  soap->accept_flags = 0;
  soap->ip = 0;
  soap->labbuf = NULL;
  soap->lablen = 0;
  soap->labidx = 0;
  soap->encodingStyle = SOAP_STR_EOS;
#ifndef WITH_NONAMESPACES
  soap->namespaces = namespaces;
#else
  soap->namespaces = NULL;
#endif
  soap->local_namespaces = NULL;
  soap->nlist = NULL;
  soap->blist = NULL;
  soap->clist = NULL;
  soap->alist = NULL;
  soap->attributes = NULL;
  soap->header = NULL;
  soap->fault = NULL;
  soap->master = SOAP_INVALID_SOCKET;
  soap->socket = SOAP_INVALID_SOCKET;
  soap->os = NULL;
  soap->is = NULL;
  soap->dom = NULL;
  soap->dime.list = NULL;
  soap->dime.first = NULL;
  soap->dime.last = NULL;
  soap->mime.list = NULL;
  soap->mime.first = NULL;
  soap->mime.last = NULL;
  soap->mime.boundary = NULL;
  soap->mime.start = NULL;
#ifndef UNDER_CE
  soap->recvfd = 0;
  soap->sendfd = 1;
#else
  soap->recvfd = stdin;
  soap->sendfd = stdout;
#endif 
  soap->host[0] = '\0';
  soap->port = 0;
  soap->action = NULL;
  soap->proxy_host = NULL;
  soap->proxy_port = 8080;
  soap->proxy_userid = NULL;
  soap->proxy_passwd = NULL;
  soap->authrealm = NULL;
  soap->prolog = NULL;
#ifdef WITH_OPENSSL
  soap->fsslauth = ssl_auth_init;
  soap->fsslverify = ssl_verify_callback;
  soap->bio = NULL;
  soap->ssl = NULL;
  soap->ctx = NULL;
  soap->require_server_auth = 0;
  soap->require_client_auth = 0;
  soap->rsa = 0;
  soap->keyfile = NULL;
  soap->password = NULL;
  soap->dhfile = NULL;
  soap->cafile = NULL;
  soap->capath = NULL;
  soap->randfile = NULL;
  soap->session = NULL;
#endif
#ifdef WITH_ZLIB
  soap->zlib_state = SOAP_ZLIB_NONE;
  soap->zlib_in = SOAP_ZLIB_NONE;
  soap->zlib_out = SOAP_ZLIB_NONE;
  soap->d_stream.zalloc = NULL;
  soap->d_stream.zfree = NULL;
  soap->d_stream.opaque = NULL;
  soap->z_level = 6;
#endif
#ifndef WITH_LEAN
  soap->cookies = NULL;
  soap->cookie_domain = NULL;
  soap->cookie_path = NULL;
  soap->cookie_max = 32;
#endif
#ifdef SOAP_DEBUG
  soap_init_logs(soap);
  soap_set_recv_logfile(soap, "RECV.log");
  soap_set_sent_logfile(soap, "SENT.log");
  soap_set_test_logfile(soap, NULL);
#endif
/* WR[ */
#ifdef WMW_RPM_IO
  soap->rpmreqid = NULL;
#endif /* WMW_RPM_IO */
/* ]WR */
#ifdef PALM
  palmNetLibOpen();
#endif
  soap_init_iht(soap);
  soap_init_pht(soap);
  soap_begin(soap);
#ifdef SOAP_DEBUG
  soap_set_test_logfile(soap, "TEST.log");
#endif
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
void
SOAP_FMAC2
soap_init1(struct soap *soap, int mode)
{ soap_init2(soap, mode, mode);
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
void
SOAP_FMAC2
soap_init2(struct soap *soap, int imode, int omode)
{ soap_init(soap);
  soap_imode(soap, imode);
  soap_omode(soap, omode);
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
void
SOAP_FMAC2
soap_begin(struct soap *soap)
{ DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Initializing\n"));
  if (!soap->keep_alive)
  { soap->buflen = 0;
    soap->bufidx = 0;
  }
  soap->keep_alive = (((soap->imode | soap->omode) & SOAP_IO_KEEPALIVE) != 0);
  soap->null = 0;
  soap->position = 0;
  soap->encoding = 0;
  soap->mustUnderstand = 0;
  soap->mode = 0;
  soap->ns = 0;
  soap->part = SOAP_BEGIN;
  soap->alloced = 0;
  soap->count = 0;
  soap->length = 0;
  soap->cdata = 0;
  soap->error = SOAP_OK;
  soap->peeked = 0;
  soap->ahead = 0;
  soap->idnum = 0;
  soap->level = 0;
  soap->endpoint[0] = '\0';
  soap->dime.chunksize = 0;
  soap->dime.buflen = 0;
  soap_free(soap);
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
void
SOAP_FMAC2
soap_end(struct soap *soap)
{ register struct soap_clist *cp;
  soap_free(soap);
  soap_dealloc(soap, NULL);
  while (soap->clist)
  { cp = soap->clist->next;
    SOAP_FREE(soap->clist);
    soap->clist = cp;
  }
  soap_closesock(soap);
#ifdef SOAP_DEBUG
  soap_close_logfiles(soap);
#endif
#ifdef PALM
  palmNetLibClose();
#endif
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
int
SOAP_FMAC2
soap_set_namespaces(struct soap *soap, struct Namespace *p)
{ struct Namespace *ns = soap->local_namespaces;
  struct soap_nlist *np, *nq, *nr;
  unsigned int level = soap->level;
  soap->namespaces = p;
  soap->local_namespaces = NULL;
  soap_set_local_namespaces(soap);
  /* reverse the list */
  np = soap->nlist;
  soap->nlist = NULL;
  if (np)
  { nq = np->next;
    np->next = NULL;
    while (nq)
    { nr = nq->next;
      nq->next = np;
      np = nq;
      nq = nr;
    }
  }
  while (np)
  { soap->level = np->level; /* preserve element nesting level */
    if (np->ns)
    { if (soap_push_namespace(soap, np->id, np->ns))
        return soap->error;
    }
    else if (np->index >= 0 && ns)
    { if (ns[np->index].out)
      { if (soap_push_namespace(soap, np->id, ns[np->index].out))
          return soap->error;
      }
      else if (soap_push_namespace(soap, np->id, ns[np->index].ns))
        return soap->error;
    }
    if (np->ns)
      SOAP_FREE(np->ns);
    nq = np;
    np = np->next;
    SOAP_FREE(nq);
  }
  if (ns)
  { int i;
    for (i = 0; ns[i].id; i++)
    { if (ns[i].out)
      { SOAP_FREE(ns[i].out);
        ns[i].out = NULL;
      }
    }
    SOAP_FREE(ns);
  }
  soap->level = level; /* restore level */
  return SOAP_OK;
}
#endif

/******************************************************************************/
#ifndef PALM_1
static void
soap_set_local_namespaces(struct soap *soap)
{ if (soap->namespaces && !soap->local_namespaces)
  { register const struct Namespace *ns1;
    register struct Namespace *ns2;
    register size_t n = 1;
    for (ns1 = soap->namespaces; ns1->id; ns1++)
      n++;
    if (n > 3)
    { n *= sizeof(struct Namespace);
      ns2 = (struct Namespace*)SOAP_MALLOC(n);
      if (ns2)
      { memcpy(ns2, soap->namespaces, n);
        ns2[0].id = "SOAP-ENV";
        ns2[1].id = "SOAP-ENC";
        ns2[2].id = "xsi";
        if (ns2[0].ns)
        { if (!strcmp(ns2[0].ns, soap_env1))
            soap->version = 1;
          else
            soap->version = 2;
        }
        soap->local_namespaces = ns2;
      }
    }
  }
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_element(struct soap *soap, const char *tag, int id, const char *type)
{ struct Namespace *ns = soap->local_namespaces;
  DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Element begin tag='%s' id='%d' type='%s'\n", tag, id, type?type:""));
/**/
#ifdef WITH_DOM
  if (soap->mode & SOAP_XML_DOM)
  { register struct soap_dom_element *p, *e = (struct soap_dom_element*)soap_malloc(soap, sizeof(struct soap_dom_element));
    e->next = NULL;
    e->prnt = soap->dom;
    e->nstr = NULL;
    e->name = soap_strdup(soap, tag); /* check EOM? */
    e->data = NULL;
    e->type = 0;
    e->node = NULL;
    e->elts = NULL;
    e->atts = NULL;
    if (soap->dom)
    { p = soap->dom->elts;
      if (p)
      { while (p->next)
          p = p->next;
        p->next = e;
      }
      else
        soap->dom->elts = e;
    }
    soap->dom = e;
  }
  else
#endif
{
  soap->level++;
  if (!soap->ns && !(soap->mode & SOAP_XML_CANONICAL))
    if (soap_send(soap, soap->prolog ? soap->prolog : "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"))
      return soap->error;
  if (soap_send_raw(soap, "<", 1)
   || soap_send(soap, tag))
    return soap->error;
}
/**/
  if (!soap->ns)
  { for (ns = soap->local_namespaces; ns && ns->id; ns++)
    { if (*ns->id && (ns->out || ns->ns))
      { sprintf(soap->tmpbuf, "xmlns:%s", ns->id);
        if (soap_attribute(soap, soap->tmpbuf, ns->out ? ns->out : ns->ns))
          return soap->error;
      }
    }   
    soap->ns = 1;
  }
  if (id > 0)
  { sprintf(soap->tmpbuf, "_%d", id);
    if (soap_attribute(soap, "id", soap->tmpbuf))
      return soap->error;
  }
  if (type && *type)
  { if (soap_attribute(soap, "xsi:type", type))
      return soap->error;
  }
  if (soap->null && soap->position > 0)
  { int i;
    sprintf(soap->tmpbuf, "[%d", soap->positions[0]);
    for (i = 1; i < soap->position; i++)
      sprintf(soap->tmpbuf + strlen(soap->tmpbuf), ",%d", soap->positions[i]);
    strcat(soap->tmpbuf, "]");
    if (soap_attribute(soap, "SOAP-ENC:position", soap->tmpbuf))
      return soap->error;
  }
  if (soap->mustUnderstand)
  { if (soap->actor && *soap->actor)
    { if (soap_attribute(soap, soap->version == 2 ? "SOAP-ENV:role" : "SOAP-ENV:actor", soap->actor))
        return soap->error;
    }
    if (soap_attribute(soap, "SOAP-ENV:mustUnderstand", soap->version == 2 ? "true" : "1"))
      return soap->error;
    soap->mustUnderstand = 0;
  }
  if (soap->encoding)
  { if (soap->encodingStyle && soap->local_namespaces)
    { if (!*soap->encodingStyle)
      { if (soap->local_namespaces[1].out)
          soap->encodingStyle = soap->local_namespaces[1].out;
        else
          soap->encodingStyle = soap->local_namespaces[1].ns;
      }
      if (soap_attribute(soap, "SOAP-ENV:encodingStyle", soap->encodingStyle))
        return soap->error;
    }
    soap->encoding = 0;
  }
  soap->null = 0;
  soap->position = 0;
  return SOAP_OK;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_element_begin_out(struct soap *soap, const char *tag, int id, const char *type)
{ if (*tag == '-')
    return SOAP_OK;
  if (soap_element(soap, tag, id, type))
    return soap->error;
  return soap_element_start_end_out(soap, NULL);
}
#endif

/******************************************************************************/
#ifndef PALM_2
#ifndef HAVE_STRRCHR
SOAP_FMAC1
char*
SOAP_FMAC2
soap_strrchr(const char *s, int t)
{ register char *r = NULL;
  while (*s)
    if (*s++ == t)
      r = (char*)s - 1;
  return r;
}
#endif
#endif

/******************************************************************************/
#ifndef PALM_2
#ifndef HAVE_STRTOL
SOAP_FMAC1
long
SOAP_FMAC2
soap_strtol(const char *s, char **t, int b)
{ register long n = 0;
  register int c;
  while (*s > 0 && *s <= 32)
    s++;
  if (b == 10)
  { short neg = 0;
    if (*s == '-')
    { s++;
      neg = 1;
    }
    else if (*s == '+')
      s++;
    while ((c = *s) && c >= '0' && c <= '9')
    { if (n > 214748364)
        break;
      n *= 10;
      n += c - '0';
      s++;
    }
    if (neg)
      n = -n;
  }
  else /* b == 16 and value is always positive */
  { while ((c = *s))
    { if (c >= '0' && c <= '9')
        c -= '0';
      else if (c >= 'A' && c <= 'F')
        c -= 'A' - 10;
      else if (c >= 'a' && c <= 'f')
        c -= 'a' - 10;
      if (n > 0x07FFFFFF)
        break;
      n <<= 4;
      n += c;
      s++;
    }
  }
  if (t)
    *t = (char*)s;
  return n;
}
#endif
#endif

/******************************************************************************/
#ifndef PALM_2
#ifndef HAVE_STRTOUL
SOAP_FMAC1
unsigned long
SOAP_FMAC2
soap_strtoul(const char *s, char **t, int b)
{ unsigned long n = 0;
  register int c;
  while (*s > 0 && *s <= 32)
    s++;
  if (b == 10)
  { if (*s == '+')
      s++;
    while ((c = *s) && c >= '0' && c <= '9')
    { if (n > 429496729)
        break;
      n *= 10;
      n += c - '0';
      s++;
    }
  }
  else /* b == 16 */
  { while ((c = *s))
    { if (c >= '0' && c <= '9')
        c -= '0';
      else if (c >= 'A' && c <= 'F')
        c -= 'A' - 10;
      else if (c >= 'a' && c <= 'f')
        c -= 'a' - 10;
      if (n > 0x0FFFFFFF)
        break;
      n <<= 4;
      n += c;
      s++;
    }
  }
  if (t)
    *t = (char*)s;
  return n;
}
#endif
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
int
SOAP_FMAC2
soap_array_begin_out(struct soap *soap, const char *tag, int id, const char *type, const char *offset)
{ if (soap_element(soap, tag, id, "SOAP-ENC:Array"))
    return soap->error;
  if (soap->version == 2)
  { const char *s;
    s = soap_strrchr(type, '[');
    if ((size_t)(s - type) < sizeof(soap->tmpbuf))
    { strncpy(soap->tmpbuf, type, s - type);
      soap->tmpbuf[s - type] = '\0';
      if (type && *type && (soap_attribute(soap, "SOAP-ENC:itemType", soap->tmpbuf)))
        return soap->error;
      if (s && (soap_attribute(soap, "SOAP-ENC:arraySize", s + 1)))
        return soap->error;
    }
  }
  else
  { if (offset && (soap_attribute(soap, "SOAP-ENC:offset", offset)))
      return soap->error;
    if (type && *type && (soap_attribute(soap, "SOAP-ENC:arrayType", type)))
      return soap->error;
  }
  return soap_element_start_end_out(soap, NULL);
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
int
SOAP_FMAC2
soap_element_start_end_out(struct soap *soap, const char *tag)
{ register struct soap_attribute *tp;
/**/
#ifdef WITH_DOM
  if ((soap->mode & SOAP_XML_DOM) && soap->dom)
  { for (tp = soap->attributes; tp; tp = tp->next)
    { if (tp->visible)
      { register struct soap_dom_attribute *a = (struct soap_dom_attribute*)soap_malloc(soap, sizeof(struct soap_dom_attribute));
	a->next = soap->dom->atts;
        a->nstr = NULL;
        a->name = soap_strdup(soap, tp->name); /* check EOM */
        a->data = soap_strdup(soap, tp->value); /* check EOM */
        a->wide = NULL;
	soap->dom->atts = a;
        tp->visible = 0;
      }
    }
    return SOAP_OK;
  }
#endif
/**/
  for (tp = soap->attributes; tp; tp = tp->next)
  { if (tp->visible)
    { if (soap_send2(soap, " ", tp->name))
        return soap->error;
      if (tp->visible == 2 && tp->value)
        if (soap_send_raw(soap, "=\"", 2)
	 || soap_string_out(soap, tp->value, 1)
	 || soap_send_raw(soap, "\"", 1))
          return soap->error;
      tp->visible = 0;
    }
  }
  if (tag)
  { soap->level--;
#ifndef WITH_LEAN
    if (soap->mode & SOAP_XML_CANONICAL)
    { if (soap_send_raw(soap, ">", 1)
       || soap_element_end_out(soap, tag))
        return soap->error;
      return SOAP_OK;
    }
#endif
    return soap_send_raw(soap, "/>", 2);
  }
  return soap_send_raw(soap, ">", 1);
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
int
SOAP_FMAC2
soap_element_end_out(struct soap *soap, const char *tag)
{ if (*tag == '-')
    return SOAP_OK;
  DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Element ending tag='%s'\n", tag));
/**/
#ifdef WITH_DOM
  if ((soap->mode & SOAP_XML_DOM) && soap->dom)
  { if (soap->dom->prnt)
      soap->dom = soap->dom->prnt;
    return SOAP_OK;
  }
#endif
/**/
  soap->level--;
  if (soap_send_raw(soap, "</", 2)
   || soap_send(soap, tag))
    return soap->error;
  return soap_send_raw(soap, ">", 1);
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
int
SOAP_FMAC2
soap_element_ref(struct soap *soap, const char *tag, int id, int href)
{ register int n = 0;
  if (soap->version == 2)
    n = 1;
  sprintf(soap->href, "#_%d", href);
  return soap_element_href(soap, tag, id, "href" + n, soap->href + n);
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
int
SOAP_FMAC2
soap_element_href(struct soap *soap, const char *tag, int id, const char *ref, const char *val)
{ DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Element '%s' reference %s='%s'\n", tag, ref, val));
  if (soap_element(soap, tag, id, NULL)
   || soap_attribute(soap, ref, val)
   || soap_element_start_end_out(soap, tag))
    return soap->error;
  return SOAP_OK;
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
int
SOAP_FMAC2
soap_element_null(struct soap *soap, const char *tag, int id, const char *type)
{ struct soap_attribute *tp;
  for (tp = soap->attributes; tp; tp = tp->next)
    if (tp->visible)
      break;
  if (tp || (soap->version == 2 && soap->position > 0) || id > 0 || (soap->mode & SOAP_XML_NIL))
  { if (soap_element(soap, tag, id, type))
      return soap->error;
    if (soap->part != SOAP_IN_HEADER && soap->encodingStyle)
      if (soap_attribute(soap, "xsi:nil", "true"))
        return soap->error;
    return soap_element_start_end_out(soap, tag);
  }
  soap->null = 1;
  soap->position = 0;
  soap->mustUnderstand = 0;
  return SOAP_OK;
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
int
SOAP_FMAC2
soap_element_id(struct soap *soap, const char *tag, int id, const void *p, const struct soap_array *a, int n, const char *type, int t) 
{ struct soap_plist *pp;
  if (!p || (a && !a->__ptr))
  { soap_element_null(soap, tag, id, type);
    return -1;
  }
  if (soap->mode & SOAP_XML_TREE)
    return 0;
  if (id < 0)
  { if (a)
      id = soap_array_pointer_lookup(soap, p, a, n, t, &pp);
    else
      id = soap_pointer_lookup(soap, p, t, &pp);
    if (id)
    { if (soap_is_embedded(soap, pp))
      { soap_element_ref(soap, tag, 0, id);
        return -1;
      }
      if (soap_is_single(soap, pp))
        return 0;
      soap_set_embedded(soap, pp);
    }
  }
  return id;
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
int
SOAP_FMAC2
soap_element_result(struct soap *soap, const char *tag)
{ if (soap->version == 2 && soap->encodingStyle)
    if (soap_element(soap, "SOAP-RPC:result", 0, NULL)
     || soap_attribute(soap, "xmlns:SOAP-RPC", soap_rpc)
     || soap_element_start_end_out(soap, NULL)
     || soap_string_out(soap, tag, 0)
     || soap_element_end_out(soap, "SOAP-RPC:result"))
      return soap->error;
  return SOAP_OK;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_attribute(struct soap *soap, const char *name, const char *value)
{ 
/**/
#ifdef WITH_DOM
  if ((soap->mode & SOAP_XML_DOM) && soap->dom)
  { register struct soap_dom_attribute *a = (struct soap_dom_attribute*)soap_malloc(soap, sizeof(struct soap_dom_attribute));
    a->next = soap->dom->atts;
    a->nstr = NULL;
    a->name = soap_strdup(soap, name); /* check EOM */
    a->data = soap_strdup(soap, value); /* check EOM */
    a->wide = NULL;
    soap->dom->atts = a;
    return SOAP_OK;
  }
#endif
/**/
#ifndef WITH_LEAN
  if (soap->mode & SOAP_XML_CANONICAL)
  { if (soap_set_attr(soap, name, value))
      return soap->error;
  }
  else
#endif
  { if (soap_send2(soap, " ", name))
      return soap->error;
    if (value)
      if (soap_send_raw(soap, "=\"", 2)
       || soap_string_out(soap, value, 1)
       || soap_send_raw(soap, "\"", 1))
        return soap->error;
  }
  return SOAP_OK;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_element_begin_in(struct soap *soap, const char *tag, int nillable)
{ if (!soap_peek_element(soap))
  { if (soap->other)
      return soap->error = SOAP_TAG_MISMATCH;
    if (tag && *tag == '-')
      return SOAP_OK;
    if (!(soap->error = soap_match_tag(soap, soap->tag, tag)))
    { soap->peeked = 0;
      if (soap->body)
        soap->level++;
      DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Begin element found (level=%u) '%s'='%s'\n", soap->level, soap->tag, tag?tag:"" ));
      if (!nillable && soap->null && (soap->mode & SOAP_XML_STRICT))
        return soap->error = SOAP_NULL;
    }
  }
  return soap->error;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_element_end_in(struct soap *soap, const char *tag)  
{ register soap_wchar c;
  register char *s;
  register const char *t;
  if (tag && *tag == '-')
    return SOAP_OK;
  soap->level--;
  soap_pop_namespace(soap);
  if (soap->peeked)
  { if (*soap->tag == '\0')
    { soap->peeked = 0;
      if (soap->error == SOAP_NO_TAG || soap->error == SOAP_TAG_END)
        soap->error = SOAP_OK;
    }
    else
      return soap->error = SOAP_TAG_END;
  }
  else
  { while (((c = soap_get(soap)) != SOAP_TT))
    { if ((int)c == EOF)
        return soap->error = SOAP_EOF;
      if (c == SOAP_LT)
        return soap->error = SOAP_TAG_END;
    }
  }
  s = soap->tag;
  do c = soap_get(soap);
  while (soap_blank(c));
  do
  { *s++ = (char)c;
    c = soap_get(soap);
  } while (soap_notblank(c));
  *s = '\0';
  if ((int)c == EOF)
    return soap->error = SOAP_EOF;
  while (soap_blank(c))
    c = soap_get(soap);
  if (c != SOAP_GT)
    return soap->error = SOAP_SYNTAX_ERROR;
  DBGLOG(TEST, SOAP_MESSAGE(fdebug, "End element found (level=%u) '%s'='%s'\n", soap->level, soap->tag, tag?tag:""));
  if (!tag)
    return SOAP_OK;
  if ((s = strchr(soap->tag, ':')))
    s++;
  else
    s = soap->tag;
  if ((t = strchr(tag, ':')))
    t++;
  else
    t = tag;
  if (!SOAP_STRCMP(s, t))
    return SOAP_OK;
  DBGLOG(TEST, SOAP_MESSAGE(fdebug, "End element tag name does not match\n"));
  return soap->error = SOAP_SYNTAX_ERROR;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
const char *
SOAP_FMAC2
soap_attr_value(struct soap *soap, const char *name, int flag)
{ register struct soap_attribute *tp;
  for (tp = soap->attributes; tp; tp = tp->next)
    if (!soap_match_tag(soap, tp->name, name))
      break;
  if (tp && tp->visible == 2)
  { if (flag == 2 && (soap->mode & SOAP_XML_STRICT))
      soap->error = SOAP_PROHIBITED;
    else
      return tp->value;
  }
  else if (flag == 1 && (soap->mode & SOAP_XML_STRICT))
    soap->error = SOAP_REQUIRED;
  return NULL;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_set_attr(struct soap *soap, const char *name, const char *value)
{ register struct soap_attribute *tp;
  DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Set attribute %s='%s'\n", name, value?value:""));
  for (tp = soap->attributes; tp; tp = tp->next)
    if (!strcmp(tp->name, name))
      break;
  if (!tp)
  { DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Allocate attribute %s\n", name));
    if (!(tp = (struct soap_attribute*)SOAP_MALLOC(sizeof(struct soap_attribute) + strlen(name))))
      return soap->error = SOAP_EOM;
    tp->ns = NULL;
#ifndef WITH_LEAN
    if (soap->mode & SOAP_XML_CANONICAL)
    { struct soap_attribute **tpp = &soap->attributes;
      const char *s = strchr(name, ':');
      if (!strncmp(name, "xmlns", 5))
      { for (; *tpp; tpp = &(*tpp)->next)
          if (strncmp((*tpp)->name, "xmlns", 5) || strcmp((*tpp)->name + 5, name + 5) > 0)
            break;
      }
      else if (!s)
      { for (; *tpp; tpp = &(*tpp)->next)
          if (strncmp((*tpp)->name, "xmlns", 5) && ((*tpp)->ns || strcmp((*tpp)->name, name) > 0))
            break;
      }
      else
      { int k;
        for (; *tpp; tpp = &(*tpp)->next)
	{ if (!strncmp((*tpp)->name, "xmlns:", 6) && !strncmp((*tpp)->name + 6, name, s - name) && !(*tpp)->name[6 + s - name])
	  { if (!tp->ns)
            { DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Canonicalization: prefix %s=%p(%s)\n", name, (*tpp)->ns, (*tpp)->ns));
	      tp->ns = (*tpp)->ns;
	    }
	  }
          else if (strncmp((*tpp)->name, "xmlns", 5) && (*tpp)->ns && tp->ns && ((k = strcmp((*tpp)->ns, tp->ns)) > 0 || (!k && strcmp((*tpp)->name, name) > 0)))
            break;
        }
      }
      tp->next = *tpp;
      *tpp = tp;
    }
    else
#endif
    { tp->next = soap->attributes;
      soap->attributes = tp;
    }
    strcpy(tp->name, name);
    tp->value = NULL;
  }
  else if (value && tp->value && tp->size <= strlen(value))
  { DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Free attribute value of %s (free %p)\n", name, tp->value));
    SOAP_FREE(tp->value);
    tp->value = NULL;
    tp->ns = NULL;
  }
  if (value)
  { if (!tp->value)
    { tp->size = strlen(value) + 1;
      if (!(tp->value = (char*)SOAP_MALLOC(tp->size)))
        return soap->error = SOAP_EOM;
      DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Allocate attribute value of %s (%p)\n", tp->name, tp->value));
    }
    strcpy(tp->value, value);
    if (!strncmp(tp->name, "xmlns:", 6))
      tp->ns = tp->value;
    tp->visible = 2;
  }
  else
    tp->visible = 1;
  return SOAP_OK;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
void
SOAP_FMAC2
soap_clr_attr(struct soap *soap)
{ register struct soap_attribute *tp;
#ifndef WITH_LEAN
  if (soap->mode & SOAP_XML_CANONICAL)
  { while (soap->attributes)
    { tp = soap->attributes->next;
      SOAP_FREE(soap->attributes->value);
      SOAP_FREE(soap->attributes);
      soap->attributes = tp;
    }
  }
  else
#endif
  { for (tp = soap->attributes; tp; tp = tp->next)
      tp->visible = 0;
  }
}
#endif

/******************************************************************************/
#ifndef PALM_2
static int
soap_getattrval(struct soap *soap, char *s, size_t n, soap_wchar d)
{ size_t i;
  soap_wchar c;
  for (i = 0; i < n; i++)
  { c = soap_getutf8(soap);
    switch (c)
    {
    case SOAP_TT:
      *s++ = '<';
      soap_unget(soap, '/');
      break;
    case SOAP_LT:
      *s++ = '<';
      break;
    case SOAP_GT:
      if (d == ' ')
      { soap_unget(soap, c);
        *s = '\0';
        return SOAP_OK;
      }
      *s++ = '>';
      break;
    case SOAP_QT:
      if (c == d)
      { *s = '\0';
        return SOAP_OK;
      }
      *s++ = '"';
      break;
    case SOAP_AP:
      if (c == d)
      { *s = '\0';
        return SOAP_OK;
      }
      *s++ = '\'';
      break;
    case '\t':
    case '\n':
    case '\r':
    case ' ':
    case '/':
      if (d == ' ')
      { soap_unget(soap, c);
        *s = '\0';
        return SOAP_OK;
      }
    default:
      if ((int)c == EOF)
        return soap->error = SOAP_EOF;
      *s++ = (char)c;
    }
  }
  return soap->error = SOAP_EOM;
}
#endif

/******************************************************************************/
#ifdef WITH_FAST
#ifndef PALM_2
static int 
soap_append_lab(struct soap *soap, const char *s, size_t n)
{ if (soap->labidx + n >= soap->lablen)
  { register char *t = soap->labbuf;
    DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Enlarging look-aside buffer to append data, old size=%lu", (unsigned long)soap->lablen));
    if (soap->lablen == 0)
      soap->lablen = SOAP_LABLEN;
    while (soap->labidx + n >= soap->lablen)
      soap->lablen <<= 1;
    DBGLOG(TEST, SOAP_MESSAGE(fdebug, ", new size=%lu\n", (unsigned long)soap->lablen));
    soap->labbuf = (char*)SOAP_MALLOC(soap->lablen);
    if (!soap->labbuf)
    { if (t)
        free(t);
      return soap->error = SOAP_EOM;
    }
    if (t && soap->labidx)
    { memcpy(soap->labbuf, t, soap->labidx);
      free(t);
    }
  }
  if (s)
  { memcpy(soap->labbuf + soap->labidx, s, n);
    soap->labidx += n;
  }
  return SOAP_OK;
}
#endif
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_peek_element(struct soap *soap)
{ register struct soap_attribute *tp;
  const char *t;
  register char *s;
  register soap_wchar c;
  register int i;
  if (soap->error == SOAP_NO_TAG || soap->error == SOAP_TAG_END || soap->error == SOAP_TAG_MISMATCH)
    soap->error = SOAP_OK;	/* retry */
  if (soap->peeked)
  { if (*soap->tag == '\0')
      return soap->error = SOAP_NO_TAG;
    return SOAP_OK;
  }
  soap->peeked = 1;
  for (;;)
  { while (((c = soap_getutf8(soap)) != SOAP_LT) && c != SOAP_TT)
    { if ((int)c == EOF)
        return soap->error = SOAP_EOF;
    }
    if (c == SOAP_TT)
    { *soap->tag = '\0';
      return soap->error = SOAP_NO_TAG; /* ending tag found */
    }
    s = soap->tag;
    do c = soap_get(soap);
    while (soap_blank(c));
    i = sizeof(soap->tag);
    while (c != '/' && soap_notblank(c))
    { if (--i > 0)
        *s++ = (char)c;
      c = soap_get(soap);
    }
    while (soap_blank(c))
      c = soap_get(soap);
    *s = '\0';
    if (*soap->tag != '?')
      break;
    DBGLOG(TEST, SOAP_MESSAGE(fdebug, "XML PI <%s?>\n", soap->tag));
    while ((int)c != EOF && c != SOAP_GT && c != '?')
    { s = soap->tmpbuf;
      i = sizeof(soap->tmpbuf) - 2;
      while (c != '=' && c != SOAP_GT && c != '?' && soap_notblank(c))
      { if (--i > 0)
          *s++ = (char)c;
        c = soap_get(soap);
      }
      while (soap_blank(c))
        c = soap_get(soap);
      if (c == '=')
      { *s++ = '=';
	do c = soap_get(soap);
        while (soap_blank(c));
        if (c != SOAP_QT && c != SOAP_AP)
        { soap_unget(soap, c);
          c = ' '; /* blank delimiter */
        }
        if (soap_getattrval(soap, s, i, c) == SOAP_EOM)
	  while (soap_getattrval(soap, soap->tmpbuf, sizeof(soap->tmpbuf), c) == SOAP_EOM)
	    ;
        else if (!strcmp(soap->tag, "?xml") && (!soap_tag_cmp(soap->tmpbuf, "encoding=iso-8859-1") || !soap_tag_cmp(soap->tmpbuf, "encoding=latin1")))
        { DBGLOG(TEST, SOAP_MESSAGE(fdebug, "XML latin1 encoding\n"));
          soap->mode |= SOAP_ENC_LATIN;
        }
      }
      do c = soap_get(soap);
      while (soap_blank(c));
    }
  }
  soap->id[0] = '\0';
  soap->href[0] = '\0';
  soap->type[0] = '\0';
  soap->arrayType[0] = '\0';
  soap->arraySize[0] = '\0';
  soap->arrayOffset[0] = '\0';
  soap->other = 0;
  soap->root = -1;
  soap->position = 0;
  soap->null = 0;
  soap->mustUnderstand = 0;
  soap_clr_attr(soap);
  while ((int)c != EOF && c != SOAP_GT && c != '/')
  { s = soap->tmpbuf;
    i = sizeof(soap->tmpbuf);
    while (c != '=' && c != '/' && soap_notblank(c))
    { if (--i > 0)
        *s++ = (char)c;
      c = soap_get(soap);
    }
    *s = '\0';
    if (i == sizeof(soap->tmpbuf))
      return soap->error = SOAP_SYNTAX_ERROR;
    if (!strncmp(soap->tmpbuf, "xmlns:", 6))
    { soap->tmpbuf[5] = '\0';
      t = soap->tmpbuf + 6;
    }
    else if (!strcmp(soap->tmpbuf, "xmlns"))
      t = SOAP_STR_EOS;
    else
      t = NULL;
    for (tp = soap->attributes; tp; tp = tp->next)
      if (!SOAP_STRCMP(tp->name, soap->tmpbuf))
        break;
    if (!tp)
    { tp = (struct soap_attribute*)SOAP_MALLOC(sizeof(struct soap_attribute) + strlen(soap->tmpbuf));
      if (!tp)
        return soap->error = SOAP_EOM;
      strcpy(tp->name, soap->tmpbuf);
      tp->value = NULL;
      tp->size = 0;
      tp->next = soap->attributes;
      soap->attributes = tp;
    }
    while (soap_blank(c))
      c = soap_get(soap);
    if (c == '=')
    { do c = soap_get(soap);
      while (soap_blank(c));
      if (c != SOAP_QT && c != SOAP_AP)
      { soap_unget(soap, c);
        c = ' '; /* blank delimiter */
      }
      if (soap_getattrval(soap, tp->value, tp->size, c))
      {
#ifdef WITH_FAST
        if (soap->error != SOAP_EOM)
          return soap->error;
        soap->error = SOAP_OK;
	soap->labidx = 0;
	if (soap_append_lab(soap, tp->value, tp->size))
	  return soap->error;
	SOAP_FREE(tp->value);
	for (;;)
	{ if (soap_getattrval(soap, soap->labbuf + soap->labidx, soap->lablen - soap->labidx, c))
	  { if (soap->error != SOAP_EOM)
	      return soap->error;
            soap->error = SOAP_OK;
	    soap->labidx = soap->lablen;
	    if (soap_append_lab(soap, NULL, 0))
	      return soap->error;
	  }
	  else
	    break;
	}
        tp->size = soap->lablen;
	if (!(tp->value = (char*)SOAP_MALLOC(tp->size)))
	  return soap->error = SOAP_EOM;
        memcpy(tp->value, soap->labbuf, soap->lablen);
#else
	size_t n;
        if (soap->error != SOAP_EOM)
          return soap->error;
        soap->error = SOAP_OK;
        if (soap_new_block(soap))
          return soap->error;
        for (;;)
        { if (!(s = (char*)soap_push_block(soap, SOAP_BLKLEN)))
            return soap->error;
          if (soap_getattrval(soap, s, SOAP_BLKLEN, c))
          { if (soap->error != SOAP_EOM)
              return soap->error;
            soap->error = SOAP_OK;
          }
          else
            break;
        }
	n = tp->size + soap->blist->size;
        if (!(s = (char*)SOAP_MALLOC(n)))
          return soap->error = SOAP_EOM;
        if (tp->value)
        { memcpy(s, tp->value, tp->size);
          SOAP_FREE(tp->value);
        }
        soap_save_block(soap, s + tp->size, 0);
        tp->value = s;
        tp->size = n;
#endif
      }
      do c = soap_get(soap);
      while (soap_blank(c));
      tp->visible = 2; /* seen this attribute w/ value */
    }
    else
      tp->visible = 1; /* seen this attribute w/o value */
    if (t && tp->value)
    { if (soap_push_namespace(soap, t, tp->value))
        return soap->error;
      tp->visible = 0;
    }
  }
  if ((int)c == EOF)
    return soap->error = SOAP_EOF;
  for (tp = soap->attributes; tp; tp = tp->next)
  { if (tp->visible && tp->value)
    { if (!strcmp(tp->name, "id"))
      { *soap->id = '#';
        strncpy(soap->id + 1, tp->value, sizeof(soap->id) - 2);
        soap->id[sizeof(soap->id)-1] = '\0';
      }
      else if (!strcmp(tp->name, "href"))
      { strncpy(soap->href, tp->value, sizeof(soap->href) - 1);
        soap->href[sizeof(soap->href)-1] = '\0';
      }
      else if ((soap->version == 2 || (soap->mode & SOAP_XML_GRAPH)) && !strcmp(tp->name, "ref"))
      { *soap->href = '#';
        strncpy(soap->href + 1, tp->value, sizeof(soap->href) - 2);
        soap->href[sizeof(soap->href)-1] = '\0';
      }
      else if (!soap_match_tag(soap, tp->name, "xsi:type"))
      { strncpy(soap->type, tp->value, sizeof(soap->type) - 1);
        soap->type[sizeof(soap->type)-1] = '\0';
      }
      else if (soap->version == 1 && !soap_match_tag(soap, tp->name, "SOAP-ENC:arrayType"))
      { s = soap_strrchr(tp->value, '[');
        if (s && (size_t)(s - tp->value) < sizeof(soap->arrayType))
        { strncpy(soap->arrayType, tp->value, s - tp->value);
          soap->arrayType[s - tp->value] = '\0';
          strncpy(soap->arraySize, s, sizeof(soap->arraySize) - 1);
        }
        else
          strncpy(soap->arrayType, tp->value, sizeof(soap->arrayType) - 1);
        soap->arraySize[sizeof(soap->arrayType)-1] = '\0';
        soap->arrayType[sizeof(soap->arrayType)-1] = '\0';
      }
      else if (soap->version == 2 && !soap_match_tag(soap, tp->name, "SOAP-ENC:itemType"))
        strncpy(soap->arrayType, tp->value, sizeof(soap->arrayType) - 1);
      else if (soap->version == 2 && !soap_match_tag(soap, tp->name, "SOAP-ENC:arraySize"))
        strncpy(soap->arraySize, tp->value, sizeof(soap->arraySize) - 1);
      else if (soap->version == 1 && !soap_match_tag(soap, tp->name, "SOAP-ENC:offset"))
        strncpy(soap->arrayOffset, tp->value, sizeof(soap->arrayOffset));
      else if (soap->version == 1 && !soap_match_tag(soap, tp->name, "SOAP-ENC:position"))
        soap->position = soap_getposition(tp->value, soap->positions);
      else if (soap->version == 1 && !soap_match_tag(soap, tp->name, "SOAP-ENC:root"))
        soap->root = ((!strcmp(tp->value, "1") || !strcmp(tp->value, "true")));
      else if (!soap_match_tag(soap, tp->name, "SOAP-ENV:actor")
            || !soap_match_tag(soap, tp->name, "SOAP-ENV:role"))
      { if ((!soap->actor || strcmp(soap->actor, tp->value))
         && strcmp(tp->value, "http://schemas.xmlsoap.org/soap/actor/next")
         && strcmp(tp->value, "http://www.w3.org/2003/05/soap-envelope/role/next"))
          soap->other = 1;
      }
      else if (!soap_match_tag(soap, tp->name, "SOAP-ENV:mustUnderstand")
            && (!strcmp(tp->value, "1") || !strcmp(tp->value, "true")))
        soap->mustUnderstand = 1;
      else if ((!soap_match_tag(soap, tp->name, "xsi:null")
             || !soap_match_tag(soap, tp->name, "xsi:nil"))
            && (!strcmp(tp->value, "1")
             || !strcmp(tp->value, "true")))
        soap->null = 1;
    }
  }
  if (!(soap->body = (c != '/')))
    do c = soap_get(soap);
    while (soap_blank(c));
  return SOAP_OK;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
void
SOAP_FMAC2
soap_retry(struct soap *soap)
{ soap->peeked = 1;
  soap->error = SOAP_OK;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
void
SOAP_FMAC2
soap_revert(struct soap *soap)
{ if (!soap->peeked)
  { soap->peeked = 1;
    if (soap->body)
      soap->level--;
  }
  DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Reverting last element (level=%u)\n", soap->level));
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_string_out(struct soap *soap, const char *s, int flag)
{ register const char *t;
  register soap_wchar c;
  register soap_wchar mask = 0x80000000;
#ifdef WITH_DOM
  if ((soap->mode & SOAP_XML_DOM) && soap->dom)
  { soap->dom->data = soap_strdup(soap, s); /* check EOM */
    return SOAP_OK;
  }
#endif
  if (soap->mode & SOAP_C_UTFSTRING)
    mask = 0;
  t = s;
  while ((c = *t++))
  { switch (c)
    { 
    case 9:
      if (flag)
      { if (soap_send_raw(soap, s, t - s - 1) || soap_send_raw(soap, "&#x9;", 5))
	  return soap->error;
        s = t;
      }
      break;
    case 10:
      if (flag || !(soap->mode & SOAP_XML_CANONICAL))
      { if (soap_send_raw(soap, s, t - s - 1) || soap_send_raw(soap, "&#xA;", 5))
	  return soap->error;
        s = t;
      }
      break;
    case 13:
      if (soap_send_raw(soap, s, t - s - 1) || soap_send_raw(soap, "&#xD;", 5))
        return soap->error;
      s = t;
      break;
    case '&':
      if (soap_send_raw(soap, s, t - s - 1) || soap_send_raw(soap, "&amp;", 5))
        return soap->error;
      s = t;
      break;
    case '<':
      if (soap_send_raw(soap, s, t - s - 1) || soap_send_raw(soap, "&lt;", 4))
        return soap->error;
      s = t;
      break;
    case '>':
      if (!flag)
      { if (soap_send_raw(soap, s, t - s - 1) || soap_send_raw(soap, "&gt;", 4))
	  return soap->error;
        s = t;
      }
      break;
    case '"':
      if (flag)
      { if (soap_send_raw(soap, s, t - s - 1) || soap_send_raw(soap, "&quot;", 6))
	  return soap->error;
        s = t;
      }
      break;
    default:
#ifdef HAVE_MBTOWC
      if (soap->mode & SOAP_C_MBSTRING)
      { wchar_t wc;
        register int m = mbtowc(&wc, t - 1, MB_CUR_MAX);
        if (m > 0 && wc != c)
        { if (soap_send_raw(soap, s, t - s - 1) || soap_pututf8(soap, wc))
            return soap->error;
	  s = t + m - 1;
	  continue;
        }
      }
#endif
      if (c & mask)
      {
        char S[2];
        S[0] = c;
        S[1] = 0;
        if (soap_send_raw(soap, s, t - s - 1) || soap_send(soap, S) )
          return soap->error;
        s = t;
      }
    }
  }
  return soap_send_raw(soap, s, t - s - 1);
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
char *
SOAP_FMAC2
soap_string_in(struct soap *soap, int flag, long minlen, long maxlen)
{ register char *s;
  char *t = NULL;
  register size_t i;
  register long l = 0;
  register int n = 0;
  register int m = 0;
  register soap_wchar c;
#ifdef HAVE_WCTOMB
  char buf[MB_LEN_MAX > 8 ? MB_LEN_MAX : 8];
#else
  char buf[8];
#endif
#ifdef WITH_CDATA
  if (!flag)
  { register int state = 0;
#ifdef WITH_FAST
    soap->labidx = 0;			/* use look-aside buffer */
#else
    if (soap_new_block(soap))
      return NULL;
#endif
    for (;;)
    { 
#ifdef WITH_FAST
      register size_t k;
      if (soap_append_lab(soap, NULL, 0))	/* allocate more space in look-aside buffer if necessary */
        return NULL;
      s = soap->labbuf + soap->labidx;	/* space to populate */
      k = soap->lablen - soap->labidx;	/* number of bytes available */
      soap->labidx = soap->lablen;	/* claim this space */
#else
      register size_t k = SOAP_BLKLEN;
      if (!(s = (char*)soap_push_block(soap, k)))
        return NULL;
#endif
      for (i = 0; i < k; i++)
      { if (m > 0)
        { *s++ = *t++;	/* copy multibyte characters */
	  m--;
          continue;
        }
        c = soap_getchar(soap);
	if ((int)c == EOF)
          goto end;
        if (c >= 0x80 && !(soap->mode & SOAP_ENC_LATIN))
        { soap_unget(soap, c);
	  c = soap_getutf8(soap);
	  if (soap->mode & SOAP_C_UTFSTRING)
          { if ((c & 0x80000000) && c >= -0x7FFFFF80 && c < SOAP_AP)
            { c &= 0x7FFFFFFF;
              t = buf;
              if (c < 0x0800)
                *t++ = (char)(0xC0 | ((c >> 6) & 0x1F));
              else
              { if (c < 0x010000)
                  *t++ = (char)(0xE0 | ((c >> 12) & 0x0F));
                else
                { if (c < 0x200000)
                    *t++ = (char)(0xF0 | ((c >> 18) & 0x07));
                  else
                  { if (c < 0x04000000)
                      *t++ = (char)(0xF8 | ((c >> 24) & 0x03));
                    else
                    { *t++ = (char)(0xFC | ((c >> 30) & 0x01));
                      *t++ = (char)(0x80 | ((c >> 24) & 0x3F));
                    }
                    *t++ = (char)(0x80 | ((c >> 18) & 0x3F));
                  }     
                  *t++ = (char)(0x80 | ((c >> 12) & 0x3F));
                }
                *t++ = (char)(0x80 | ((c >> 6) & 0x3F));
              }
              *t++ = (char)(0x80 | (c & 0x3F));
	      m = (int)(t - buf) - 1;
              t = buf;
              *s++ = *t++;
              continue;
	    }
          }
        }
	switch (state)
        { case 1:
            if (c == ']')
	      state = 4;
	    *s++ = c;
	    continue;
	  case 2:
	    if (c == '-')
              state = 6;
	    *s++ = c;
	    continue;
	  case 3:
	    if (c == '?')
	      state = 8;
	    *s++ = c;
	    continue;
          /* CDATA */
	  case 4:
	    if (c == ']')
              state = 5;
	    else
	      state = 1;
	    *s++ = c;
	    continue;
	  case 5:
	    if (c == '>')
	      state = 0;
	    else
	      state = 1;
	    *s++ = c;
	    continue;
          /* comment */
          case 6:
            if (c == '-')
	      state = 7;
	    else
	      state = 2;
	    *s++ = c;
	    continue;
          case 7:
            if (c == '>')
	      state = 0;
	    else
	      state = 2;
	    *s++ = c;
	    continue;
          /* PI */
	  case 8:
            if (c == '>')
	      state = 0;
	    else
	      state = 3;
	    *s++ = c;
	    continue;
        }
        switch (c)
        {
        case '/':
          if (n > 0)
          { c = soap_getchar(soap);
            if (c == '>')
              n--;
            soap_unget(soap, c);
          }
          *s++ = '/';
          break;
        case '<':
	  c = soap_getchar(soap);
	  if (c == '/')
	  { if (n == 0)
	    { c = SOAP_TT;
	      goto end;
	    }
	    n--;
	  }
	  else if (c == '!')
	  { c = soap_getchar(soap);
	    if (c == '[')
            { do c = soap_getchar(soap);
              while ((int)c != EOF && c != '[');
              if ((int)c == EOF)
                 goto end;
	      t = (char*)"![CDATA[";
	      m = 8;
	      state = 1;
	    }
            else if (c == '-')
	    { if ((c = soap_getchar(soap)) == '-')
	        state = 2;
	      t = (char*)"!-";
	      m = 2;
	      soap_unget(soap, c);
	    }
	    else
	    { t = (char*)"!";
	      m = 1;
	      soap_unget(soap, c);
	    }
	    *s++ = '<';
	    break;
	  }
	  else if (c == '?')
	    state = 3;
	  else
	    n++;
          soap_unget(soap, c);
          *s++ = '<';
          break;
        case '>':
          *s++ = '>';
          break;
        case '"':
          *s++ = '"';
          break;
        default:
#ifdef HAVE_WCTOMB
          if (soap->mode & SOAP_C_MBSTRING)
          { m = wctomb(buf, c & 0x7FFFFFFF);
            if (m >= 1)
            { t = buf;
              *s++ = *t++;
              m--;
            }
            else
              *s++ = SOAP_UNKNOWN_CHAR;
          }
          else
#endif
            *s++ = (char)(c & 0xFF);
        }
	l++;
        if ((soap->mode & SOAP_XML_STRICT) && maxlen >= 0 && l > maxlen)
        { DBGLOG(TEST,SOAP_MESSAGE(fdebug, "String too long: maxlen=%ld\n", maxlen));
          soap->error = SOAP_LENGTH;
          return NULL;
        }
      }
    }
  }
#endif
#ifdef WITH_FAST
  soap->labidx = 0;			/* use look-aside buffer */
#else
  if (soap_new_block(soap))
    return NULL;
#endif
  for (;;)
  { 
#ifdef WITH_FAST
    register size_t k;
    if (soap_append_lab(soap, NULL, 0))	/* allocate more space in look-aside buffer if necessary */
      return NULL;
    s = soap->labbuf + soap->labidx;	/* space to populate */
    k = soap->lablen - soap->labidx;	/* number of bytes available */
    soap->labidx = soap->lablen;	/* claim this space */
#else
    register size_t k = SOAP_BLKLEN;
    if (!(s = (char*)soap_push_block(soap, k)))
      return NULL;
#endif
    for (i = 0; i < k; i++)
    { if (m > 0)
      { *s++ = *t++;	/* copy multibyte characters */
        m--;
        continue;
      }
      if (soap->mode & SOAP_C_UTFSTRING)
      { if (((c = soap_get(soap)) & 0x80000000) && c >= -0x7FFFFF80 && c < SOAP_AP)
        { c &= 0x7FFFFFFF;
          t = buf;
          if (c < 0x0800)
            *t++ = (char)(0xC0 | ((c >> 6) & 0x1F));
          else
          { if (c < 0x010000)
              *t++ = (char)(0xE0 | ((c >> 12) & 0x0F));
            else
            { if (c < 0x200000)
                *t++ = (char)(0xF0 | ((c >> 18) & 0x07));
              else
              { if (c < 0x04000000)
                  *t++ = (char)(0xF8 | ((c >> 24) & 0x03));
                else
                { *t++ = (char)(0xFC | ((c >> 30) & 0x01));
                  *t++ = (char)(0x80 | ((c >> 24) & 0x3F));
                }
                *t++ = (char)(0x80 | ((c >> 18) & 0x3F));
              }     
              *t++ = (char)(0x80 | ((c >> 12) & 0x3F));
            }
            *t++ = (char)(0x80 | ((c >> 6) & 0x3F));
          }
          *t++ = (char)(0x80 | (c & 0x3F));
	  m = (int)(t - buf) - 1;
          t = buf;
          *s++ = *t++;
          continue;
        }
      }
      else
        c = soap_getutf8(soap);
      switch (c)
      {
      case SOAP_TT:
        if (n == 0)
          goto end;
        n--;
        *s++ = '<';
        t = (char*)"/";
	m = 1;
        break;
      case SOAP_LT:
        n++;
        *s++ = '<';
        break;
      case SOAP_GT:
        *s++ = '>';
        break;
      case SOAP_QT:
        *s++ = '"';
        break;
      case SOAP_AP:
        *s++ = '\'';
        break;
      case '/':
        if (n > 0)
        { c = soap_get(soap);
          if (c == SOAP_GT)
            n--;
          soap_unget(soap, c);
        }
        *s++ = '/';
        break;
      case '<' | 0x80000000:
        if (flag)
          *s++ = '<';
        else
        { *s++ = '&';
          t = (char*)"lt;";
	  m = 3;
        }
        break;
      case '>' | 0x80000000:
        if (flag)
          *s++ = '>';
        else
        { *s++ = '&';
          t = (char*)"gt;";
	  m = 3;
        }
        break;
      case '"' | 0x80000000:
        if (flag)
          *s++ = '"';
        else
        { *s++ = '&';
          t = (char*)"quot;";
	  m = 5;
        }
        break;
      case '\'' | 0x80000000:
        if (flag)
          *s++ = '\'';
        else
        { *s++ = '&';
          t = (char*)"apos;";
	  m = 5;
        }
        break;
      default:
        if ((int)c == EOF)
          goto end;
#ifdef HAVE_WCTOMB
        if (soap->mode & SOAP_C_MBSTRING)
        { m = wctomb(buf, c & 0x7FFFFFFF);
          if (m >= 1)
          { t = buf;
            *s++ = *t++;
            m--;
          }
          else
            *s++ = SOAP_UNKNOWN_CHAR;
        }
        else
#endif
          *s++ = (char)(c & 0xFF);
      }
      l++;
      if ((soap->mode & SOAP_XML_STRICT) && maxlen >= 0 && l > maxlen)
      { DBGLOG(TEST,SOAP_MESSAGE(fdebug, "String too long: maxlen=%ld\n", maxlen));
        soap->error = SOAP_LENGTH;
        return NULL;
      }
    }
  }
end:
  soap_unget(soap, c);
  *s = '\0';
#ifdef WITH_FAST
  t = soap_strdup(soap, soap->labbuf);
#else
  soap_size_block(soap, i+1);
  t = soap_save_block(soap, NULL, 0);
#endif
  if ((soap->mode & SOAP_XML_STRICT) && l < minlen)
  { DBGLOG(TEST,SOAP_MESSAGE(fdebug, "String too short: %ld chars, minlen=%ld\n", l, minlen));
    soap->error = SOAP_LENGTH;
    return NULL;
  }
  if (flag == 2)
    if (soap_s2QName(soap, t, &t))
      return NULL;
  return t;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_wstring_out(struct soap *soap, const wchar_t *s, int flag)
{ const char *t;
  char tmp;
  register soap_wchar c;
#ifdef WITH_DOM
  if ((soap->mode & SOAP_XML_DOM) && soap->dom)
  { soap->dom->wide = NULL; /* soap_malloc() ??? */
    return SOAP_OK;
  }
#endif
  while ((c = *s++))
  { switch (c)
    { 
    case 9:
      if (flag)
        t = "&#x9;";
      else
        t = "\t";
      break;
    case 10:
      if (flag || !(soap->mode & SOAP_XML_CANONICAL))
        t = "&#xA;";
      else
        t = "\n";
      break;
    case 13:
      t = "&#xD;";
      break;
    case '&':
      t = "&amp;";
      break;
    case '<':
      t = "&lt;";
      break;
    case '>':
      if (flag)
        t = ">";
      else
	t = "&gt;";
      break;
    case '"':
      if (flag)
        t = "&quot;";
      else
        t = "\"";
      break;
    default:
      if (c > 0 && c < 0x80)
      { tmp = (char)c;
        if (soap_send_raw(soap, &tmp, 1))
          return soap->error;
      }
      else if (soap_pututf8(soap, (unsigned long)c))
        return soap->error;
      continue;
    }
    if (soap_send(soap, t))
      return soap->error;
  }
  return SOAP_OK;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
wchar_t *
SOAP_FMAC2
soap_wstring_in(struct soap *soap, int flag, long minlen, long maxlen)
{ wchar_t *s;
  register int i, n = 0;
  register long l = 0;
  register soap_wchar c;
  const char *t = NULL;
  if (soap_new_block(soap))
    return NULL;
  for (;;)
  { if (!(s = (wchar_t*)soap_push_block(soap, sizeof(wchar_t)*SOAP_BLKLEN)))
      return NULL;
    for (i = 0; i < SOAP_BLKLEN; i++)
    { if (t)
      { *s++ = (wchar_t)*t++;
        if (!*t)
          t = NULL;
        continue;
      }
      c = soap_getutf8(soap);
      switch (c)
      {
      case SOAP_TT:
        if (n == 0)
          goto end;
        n--;
        *s++ = '<';
        soap_unget(soap, '/');
        break;
      case SOAP_LT:
        n++;
        *s++ = '<';
        break;
      case SOAP_GT:
        *s++ = '>';
        break;
      case SOAP_QT:
        *s++ = '"';
        break;
      case SOAP_AP:
        *s++ = '\'';
        break;
      case '/':
        if (n > 0)
        { c = soap_getutf8(soap);
          if (c == SOAP_GT)
            n--;
          soap_unget(soap, c);
        }
        *s++ = '/';
        break;
      case '<':
        if (flag)
          *s++ = (soap_wchar)'<';
        else
        { *s++ = (soap_wchar)'&';
          t = "lt;";
        }
        break;
      case '>':
        if (flag)
          *s++ = (soap_wchar)'>';
        else
        { *s++ = (soap_wchar)'&';
          t = "gt;";
        }
        break;
      case '"':
        if (flag)
          *s++ = (soap_wchar)'"';
        else
        { *s++ = (soap_wchar)'&';
          t = "quot;";
        }
        break;
      default:
        if ((int)c == EOF)
          goto end;
        *s++ = (wchar_t)c & 0x7FFFFFFF;
      }
      l++;
      if ((soap->mode & SOAP_XML_STRICT) && maxlen >= 0 && l > maxlen)
      { DBGLOG(TEST,SOAP_MESSAGE(fdebug, "String too long: maxlen=%ld\n", maxlen));
        soap->error = SOAP_LENGTH;
        return NULL;
      }
    }
  }
end:
  soap_unget(soap, c);
  *s = '\0';
  soap_size_block(soap, sizeof(wchar_t) * (i + 1));
  if ((soap->mode & SOAP_XML_STRICT) && l < minlen)
  { DBGLOG(TEST,SOAP_MESSAGE(fdebug, "String too short: %ld chars, minlen=%ld\n", l, minlen));
    soap->error = SOAP_LENGTH;
    return NULL;
  }
  return (wchar_t*)soap_save_block(soap, NULL, 0);
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
const char*
SOAP_FMAC2
soap_int2s(struct soap *soap, int n)
{ return soap_long2s(soap, (long)n);
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_outint(struct soap *soap, const char *tag, int id, const int *p, const char *type, int n)
{ if (soap_element_begin_out(soap, tag, soap_embedded_id(soap, id, p, n), type)
   || soap_string_out(soap, soap_long2s(soap, (long)*p), 0))
    return soap->error;
  return soap_element_end_out(soap, tag);
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_s2int(struct soap *soap, const char *s, int *p)
{ if (s)
  { char *r;
    *p = (int)soap_strtol(s, &r, 10);
    if (*r)
      soap->error = SOAP_TYPE;
  }
  return soap->error;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int *
SOAP_FMAC2
soap_inint(struct soap *soap, const char *tag, int *p, const char *type, int t)
{ if (soap_element_begin_in(soap, tag, 0))
    return NULL;
#ifndef WITH_LEAN
  if (*soap->type
   && soap_match_tag(soap, soap->type, type)
   && soap_match_tag(soap, soap->type, ":int")
   && soap_match_tag(soap, soap->type, ":short")
   && soap_match_tag(soap, soap->type, ":byte"))
  { soap->error = SOAP_TYPE;
    soap_revert(soap);
    return NULL;
  }
#endif
  p = (int*)soap_id_enter(soap, soap->id, p, t, sizeof(int), 0, NULL, NULL, NULL);
  if (p)
  { if (soap_s2int(soap, soap_value(soap), p))
      return NULL;
  }
  p = (int*)soap_id_forward(soap, soap->href, p, t, 0, sizeof(int), 0, NULL);
  if (soap->body && soap_element_end_in(soap, tag))
    return NULL;
  return p;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
const char*
SOAP_FMAC2
soap_long2s(struct soap *soap, long n)
{ sprintf(soap->tmpbuf, "%ld", n);
  return soap->tmpbuf;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_outlong(struct soap *soap, const char *tag, int id, const long *p, const char *type, int n)
{ if (soap_element_begin_out(soap, tag, soap_embedded_id(soap, id, p, n), type)
   || soap_string_out(soap, soap_long2s(soap, *p), 0))
    return soap->error;
  return soap_element_end_out(soap, tag);
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_s2long(struct soap *soap, const char *s, long *p)
{ if (s)
  { char *r;
    *p = soap_strtol(s, &r, 10);
    if (*r)
      soap->error = SOAP_TYPE;
  }
  return soap->error;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
long *
SOAP_FMAC2
soap_inlong(struct soap *soap, const char *tag, long *p, const char *type, int t)
{ if (soap_element_begin_in(soap, tag, 0))
    return NULL;
#ifndef WITH_LEAN
  if (*soap->type
   && soap_match_tag(soap, soap->type, type)
   && soap_match_tag(soap, soap->type, ":int")
   && soap_match_tag(soap, soap->type, ":short")
   && soap_match_tag(soap, soap->type, ":byte"))
  { soap->error = SOAP_TYPE;
    soap_revert(soap);
    return NULL;
  }
#endif
  p = (long*)soap_id_enter(soap, soap->id, p, t, sizeof(long), 0, NULL, NULL, NULL);
  if (p)
  { if (soap_s2long(soap, soap_value(soap), p))
      return NULL;
  }
  p = (long*)soap_id_forward(soap, soap->href, p, t, 0, sizeof(long), 0, NULL);
  if (soap->body && soap_element_end_in(soap, tag))
    return NULL;
  return p;
}
#endif

/******************************************************************************/
#ifndef WITH_LEAN
SOAP_FMAC1
const char*
SOAP_FMAC2
soap_LONG642s(struct soap *soap, LONG64 n)
{ sprintf(soap->tmpbuf, SOAP_LONG_FORMAT, n);
  return soap->tmpbuf;
}
#endif

/******************************************************************************/
#ifndef WITH_LEAN
SOAP_FMAC1
int
SOAP_FMAC2
soap_outLONG64(struct soap *soap, const char *tag, int id, const LONG64 *p, const char *type, int n)
{ if (soap_element_begin_out(soap, tag, soap_embedded_id(soap, id, p, n), type)
   || soap_string_out(soap, soap_LONG642s(soap, *p), 0))
    return soap->error;
  return soap_element_end_out(soap, tag);
}
#endif

/******************************************************************************/
#ifndef WITH_LEAN
SOAP_FMAC1
int
SOAP_FMAC2
soap_s2LONG64(struct soap *soap, const char *s, LONG64 *p)
{ if (s && sscanf(s, SOAP_LONG_FORMAT, p) != 1)
    soap->error = SOAP_TYPE;
  return soap->error;
}
#endif

/******************************************************************************/
#ifndef WITH_LEAN
SOAP_FMAC1
LONG64 *
SOAP_FMAC2
soap_inLONG64(struct soap *soap, const char *tag, LONG64 *p, const char *type, int t)
{ if (soap_element_begin_in(soap, tag, 0))
    return NULL;
#ifndef WITH_LEAN
  if (*soap->type
   && soap_match_tag(soap, soap->type, type)
   && soap_match_tag(soap, soap->type, ":integer")
   && soap_match_tag(soap, soap->type, ":positiveInteger")
   && soap_match_tag(soap, soap->type, ":negativeInteger")
   && soap_match_tag(soap, soap->type, ":nonPositiveInteger")
   && soap_match_tag(soap, soap->type, ":nonNegativeInteger")
   && soap_match_tag(soap, soap->type, ":long")
   && soap_match_tag(soap, soap->type, ":int")
   && soap_match_tag(soap, soap->type, ":short")
   && soap_match_tag(soap, soap->type, ":byte"))
  { soap->error = SOAP_TYPE;
    soap_revert(soap);
    return NULL;
  }
#endif
  p = (LONG64*)soap_id_enter(soap, soap->id, p, t, sizeof(LONG64), 0, NULL, NULL, NULL);
  if (p)
  { if (soap_s2LONG64(soap, soap_value(soap), p))
      return NULL;
  }
  p = (LONG64*)soap_id_forward(soap, soap->href, p, t, 0, sizeof(LONG64), 0, NULL);
  if (soap->body && soap_element_end_in(soap, tag))
    return NULL;
  return p;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
const char*
SOAP_FMAC2
soap_byte2s(struct soap *soap, char n)
{ return soap_long2s(soap, (long)n);
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_outbyte(struct soap *soap, const char *tag, int id, const char *p, const char *type, int n)
{ if (soap_element_begin_out(soap, tag, soap_embedded_id(soap, id, p, n), type)
   || soap_string_out(soap, soap_long2s(soap, (long)*p), 0))
    return soap->error;
  return soap_element_end_out(soap, tag);
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_s2byte(struct soap *soap, const char *s, char *p)
{ if (s)
  { long n;
    char *r;
    n = soap_strtol(s, &r, 10);
    if (*r || n < -128 || n > 127)
      soap->error = SOAP_TYPE;
    *p = (char)n;
  }
  return soap->error;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
char *
SOAP_FMAC2
soap_inbyte(struct soap *soap, const char *tag, char *p, const char *type, int t)
{ if (soap_element_begin_in(soap, tag, 0))
    return NULL;
#ifndef WITH_LEAN
  if (*soap->type
   && soap_match_tag(soap, soap->type, type)
   && soap_match_tag(soap, soap->type, ":byte"))
  { soap->error = SOAP_TYPE;
    soap_revert(soap);
    return NULL;
  }
#endif
  p = (char*)soap_id_enter(soap, soap->id, p, t, sizeof(char), 0, NULL, NULL, NULL);
  if (p)
  { if (soap_s2byte(soap, soap_value(soap), p))
      return NULL;
  }
  p = (char*)soap_id_forward(soap, soap->href, p, t, 0, sizeof(char), 0, NULL);
  if (soap->body && soap_element_end_in(soap, tag))
    return NULL;
  return p;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
const char*
SOAP_FMAC2
soap_short2s(struct soap *soap, short n)
{ return soap_long2s(soap, (long)n);
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_outshort(struct soap *soap, const char *tag, int id, const short *p, const char *type, int n)
{ if (soap_element_begin_out(soap, tag, soap_embedded_id(soap, id, p, n), type)
   || soap_string_out(soap, soap_long2s(soap, (long)*p), 0))
    return soap->error;
  return soap_element_end_out(soap, tag);
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_s2short(struct soap *soap, const char *s, short *p)
{ if (s)
  { long n;
    char *r;
    n = soap_strtol(s, &r, 10);
    if (*r || n < -32768 || n > 32767)
      soap->error = SOAP_TYPE;
    *p = (short)n;
  }
  return soap->error;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
short *
SOAP_FMAC2
soap_inshort(struct soap *soap, const char *tag, short *p, const char *type, int t)
{ if (soap_element_begin_in(soap, tag, 0))
    return NULL;
#ifndef WITH_LEAN
  if (*soap->type
   && soap_match_tag(soap, soap->type, type)
   && soap_match_tag(soap, soap->type, ":short")
   && soap_match_tag(soap, soap->type, ":byte"))
  { soap->error = SOAP_TYPE;
    soap_revert(soap);
    return NULL;
  }
#endif
  p = (short*)soap_id_enter(soap, soap->id, p, t, sizeof(short), 0, NULL, NULL, NULL);
  if (p)
  { if (soap_s2short(soap, soap_value(soap), p))
      return NULL;
  }
  p = (short*)soap_id_forward(soap, soap->href, p, t, 0, sizeof(short), 0, NULL);
  if (soap->body && soap_element_end_in(soap, tag))
    return NULL;
  return p;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
const char*
SOAP_FMAC2
soap_float2s(struct soap *soap, float n)
{ const char *s;
  if (soap_isnan((double)n))
    s = "NaN";
  else if (soap_ispinff(n))
    s = "INF";
  else if (soap_isninff(n))
    s = "-INF";
  else
  { sprintf(soap->tmpbuf, soap->float_format, n);
    s = soap->tmpbuf;
  }
  return s;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_outfloat(struct soap *soap, const char *tag, int id, const float *p, const char *type, int n)
{ if (soap_element_begin_out(soap, tag, soap_embedded_id(soap, id, p, n), type)
   || soap_string_out(soap, soap_float2s(soap, *p), 0))
    return soap->error;
  return soap_element_end_out(soap, tag);
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_s2float(struct soap *soap, const char *s, float *p)
{ if (s)
  { if (!soap_tag_cmp(s, "INF"))
      *p = FLT_PINFTY;
    else if (!soap_tag_cmp(s, "+INF"))
      *p = FLT_PINFTY;
    else if (!soap_tag_cmp(s, "-INF"))
      *p = FLT_NINFTY;
    else if (!soap_tag_cmp(s, "NaN"))
      *p = FLT_NAN;
    else
    {
#ifdef HAVE_STRTOD
      char *r;
      *p = (float)strtod(s, &r);
      if (*r)
#endif
#ifdef HAVE_SSCANF
        if (sscanf(s, soap->float_format, p) != 1)
          soap->error = SOAP_TYPE;
#else
        soap->error = SOAP_TYPE;
#endif
    }
  }
  return soap->error;
}
#endif

/******************************************************************************/
#ifndef WITH_LEAN
static int soap_isnumeric(struct soap *soap, const char *type)
{ if (soap_match_tag(soap, soap->type, type)
   && soap_match_tag(soap, soap->type, ":float")
   && soap_match_tag(soap, soap->type, ":double")
   && soap_match_tag(soap, soap->type, ":decimal")
   && soap_match_tag(soap, soap->type, ":integer")
   && soap_match_tag(soap, soap->type, ":positiveInteger")
   && soap_match_tag(soap, soap->type, ":negativeInteger")
   && soap_match_tag(soap, soap->type, ":nonPositiveInteger")
   && soap_match_tag(soap, soap->type, ":nonNegativeInteger")
   && soap_match_tag(soap, soap->type, ":long")
   && soap_match_tag(soap, soap->type, ":int")
   && soap_match_tag(soap, soap->type, ":short")
   && soap_match_tag(soap, soap->type, ":byte")
   && soap_match_tag(soap, soap->type, ":unsignedLong")
   && soap_match_tag(soap, soap->type, ":unsignedInt")
   && soap_match_tag(soap, soap->type, ":unsignedShort")
   && soap_match_tag(soap, soap->type, ":unsignedByte"))
  { soap->error = SOAP_TYPE;
    soap_revert(soap);
    return SOAP_ERR;
  }
  return SOAP_OK;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
float *
SOAP_FMAC2
soap_infloat(struct soap *soap, const char *tag, float *p, const char *type, int t)
{ if (soap_element_begin_in(soap, tag, 0))
    return NULL;
#ifndef WITH_LEAN
  if (*soap->type != '\0' && soap_isnumeric(soap, type))
    return NULL;
#endif
  p = (float*)soap_id_enter(soap, soap->id, p, t, sizeof(float), 0, NULL, NULL, NULL);
  if (p)
  { if (soap_s2float(soap, soap_value(soap), p))
      return NULL;
  }
  p = (float*)soap_id_forward(soap, soap->href, p, t, 0, sizeof(float), 0, NULL);
  if (soap->body && soap_element_end_in(soap, tag))
    return NULL;
  return p;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
const char*
SOAP_FMAC2
soap_double2s(struct soap *soap, double n)
{ const char *s;
  if (soap_isnan(n))
    s = "NaN";
  else if (soap_ispinfd(n))
    s = "INF";
  else if (soap_isninfd(n))
    s = "-INF";
  else
  { sprintf(soap->tmpbuf, soap->double_format, n);
    s = soap->tmpbuf;
  }
  return s;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_outdouble(struct soap *soap, const char *tag, int id, const double *p, const char *type, int n)
{ if (soap_element_begin_out(soap, tag, soap_embedded_id(soap, id, p, n), type)
   || soap_string_out(soap, soap_double2s(soap, *p), 0))
    return soap->error;
  return soap_element_end_out(soap, tag);
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_s2double(struct soap *soap, const char *s, double *p)
{ if (s)
  { if (!soap_tag_cmp(s, "INF"))
      *p = DBL_PINFTY;
    else if (!soap_tag_cmp(s, "+INF"))
      *p = DBL_PINFTY;
    else if (!soap_tag_cmp(s, "-INF"))
      *p = DBL_NINFTY;
    else if (!soap_tag_cmp(s, "NaN"))
      *p = DBL_NAN;
    else
    {
#ifdef HAVE_STRTOD
      char *r;
      *p = strtod(s, &r);
      if (*r)
#endif
#ifdef HAVE_SSCANF
        if (sscanf(s, soap->double_format, p) != 1)
          soap->error = SOAP_TYPE;
#else
        soap->error = SOAP_TYPE;
#endif
    }
  }
  return soap->error;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
double *
SOAP_FMAC2
soap_indouble(struct soap *soap, const char *tag, double *p, const char *type, int t)
{ if (soap_element_begin_in(soap, tag, 0))
    return NULL;
#ifndef WITH_LEAN
  if (*soap->type != '\0' && soap_isnumeric(soap, type))
    return NULL;
#endif
  p = (double*)soap_id_enter(soap, soap->id, p, t, sizeof(double), 0, NULL, NULL, NULL);
  if (p)
  { if (soap_s2double(soap, soap_value(soap), p))
      return NULL;
  }
  p = (double*)soap_id_forward(soap, soap->href, p, t, 0, sizeof(double), 0, NULL);
  if (soap->body && soap_element_end_in(soap, tag))
    return NULL;
  return p;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
const char*
SOAP_FMAC2
soap_unsignedByte2s(struct soap *soap, unsigned char n)
{ return soap_unsignedLong2s(soap, (unsigned long)n);
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_outunsignedByte(struct soap *soap, const char *tag, int id, const unsigned char *p, const char *type, int n)
{ if (soap_element_begin_out(soap, tag, soap_embedded_id(soap, id, p, n), type)
   || soap_string_out(soap, soap_unsignedLong2s(soap, (unsigned long)*p), 0))
    return soap->error;
  return soap_element_end_out(soap, tag);
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_s2unsignedByte(struct soap *soap, const char *s, unsigned char *p)
{ if (s)
  { unsigned long n;
    char *r;
    n = soap_strtoul(s, &r, 10);
    if (*r || n > 255)
      soap->error = SOAP_TYPE;
    *p = (unsigned char)n;
  }
  return soap->error;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
unsigned char *
SOAP_FMAC2
soap_inunsignedByte(struct soap *soap, const char *tag, unsigned char *p, const char *type, int t)
{ if (soap_element_begin_in(soap, tag, 0))
    return NULL;
#ifndef WITH_LEAN
  if (*soap->type
   && soap_match_tag(soap, soap->type, type)
   && soap_match_tag(soap, soap->type, ":unsignedByte"))
  { soap->error = SOAP_TYPE;
    soap_revert(soap);
    return NULL;
  }
#endif
  p = (unsigned char*)soap_id_enter(soap, soap->id, p, t, sizeof(unsigned char), 0, NULL, NULL, NULL);
  if (p)
  { if (soap_s2unsignedByte(soap, soap_value(soap), p))
      return NULL;
  }
  p = (unsigned char*)soap_id_forward(soap, soap->href, p, t, 0, sizeof(unsigned char), 0, NULL);
  if (soap->body && soap_element_end_in(soap, tag))
    return NULL;
  return p;
}
#endif

/******************************************************************************/
#ifndef WITH_LEAN
SOAP_FMAC1
const char*
SOAP_FMAC2
soap_unsignedShort2s(struct soap *soap, unsigned short n)
{ return soap_unsignedLong2s(soap, (unsigned long)n);
}
#endif

/******************************************************************************/
#ifndef WITH_LEAN
SOAP_FMAC1
int
SOAP_FMAC2
soap_outunsignedShort(struct soap *soap, const char *tag, int id, const unsigned short *p, const char *type, int n)
{ if (soap_element_begin_out(soap, tag, soap_embedded_id(soap, id, p, n), type)
   || soap_string_out(soap, soap_unsignedLong2s(soap, (unsigned long)*p), 0))
    return soap->error;
  return soap_element_end_out(soap, tag);
}
#endif

/******************************************************************************/
#ifndef WITH_LEAN
SOAP_FMAC1
int
SOAP_FMAC2
soap_s2unsignedShort(struct soap *soap, const char *s, unsigned short *p)
{ if (s)
  { unsigned long n;
    char *r;
    n = soap_strtoul(s, &r, 10);
    if (*r || n > 65535)
      soap->error = SOAP_TYPE;
    *p = (unsigned short)n;
  }
  return soap->error;
}
#endif

/******************************************************************************/
#ifndef WITH_LEAN
SOAP_FMAC1
unsigned short *
SOAP_FMAC2
soap_inunsignedShort(struct soap *soap, const char *tag, unsigned short *p, const char *type, int t)
{ if (soap_element_begin_in(soap, tag, 0))
    return NULL;
#ifndef WITH_LEAN
  if (*soap->type
   && soap_match_tag(soap, soap->type, type)
   && soap_match_tag(soap, soap->type, ":unsignedShort")
   && soap_match_tag(soap, soap->type, ":unsignedByte"))
  { soap->error = SOAP_TYPE;
    soap_revert(soap);
    return NULL;
  }
#endif
  p = (unsigned short*)soap_id_enter(soap, soap->id, p, t, sizeof(unsigned short), 0, NULL, NULL, NULL);
  if (p)
  { if (soap_s2unsignedShort(soap, soap_value(soap), p))
      return NULL;
  }
  p = (unsigned short*)soap_id_forward(soap, soap->href, p, t, 0, sizeof(unsigned short), 0, NULL);
  if (soap->body && soap_element_end_in(soap, tag))
    return NULL;
  return p;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
const char*
SOAP_FMAC2
soap_unsignedInt2s(struct soap *soap, unsigned int n)
{ return soap_unsignedLong2s(soap, (unsigned long)n);
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_outunsignedInt(struct soap *soap, const char *tag, int id, const unsigned int *p, const char *type, int n)
{ if (soap_element_begin_out(soap, tag, soap_embedded_id(soap, id, p, n), type)
   || soap_string_out(soap, soap_unsignedLong2s(soap, (unsigned long)*p), 0))
    return soap->error;
  return soap_element_end_out(soap, tag);
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_s2unsignedInt(struct soap *soap, const char *s, unsigned int *p)
{ if (s)
  { char *r;
    *p = (unsigned int)soap_strtoul(s, &r, 10);
    if (*r)
      soap->error = SOAP_TYPE;
  }
  return soap->error;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
unsigned int *
SOAP_FMAC2
soap_inunsignedInt(struct soap *soap, const char *tag, unsigned int *p, const char *type, int t)
{ if (soap_element_begin_in(soap, tag, 0))
    return NULL;
#ifndef WITH_LEAN
  if (*soap->type
   && soap_match_tag(soap, soap->type, type)
   && soap_match_tag(soap, soap->type, ":unsignedInt")
   && soap_match_tag(soap, soap->type, ":unsignedShort")
   && soap_match_tag(soap, soap->type, ":unsignedByte"))
  { soap->error = SOAP_TYPE;
    soap_revert(soap);
    return NULL;
  }
#endif
  p = (unsigned int*)soap_id_enter(soap, soap->id, p, t, sizeof(unsigned int), 0, NULL, NULL, NULL);
  if (p)
  { if (soap_s2unsignedInt(soap, soap_value(soap), p))
      return NULL;
  }
  p = (unsigned int*)soap_id_forward(soap, soap->href, p, t, 0, sizeof(unsigned int), 0, NULL);
  if (soap->body && soap_element_end_in(soap, tag))
    return NULL;
  return p;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
const char*
SOAP_FMAC2
soap_unsignedLong2s(struct soap *soap, unsigned long n)
{ sprintf(soap->tmpbuf, "%lu", n);
  return soap->tmpbuf;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_outunsignedLong(struct soap *soap, const char *tag, int id, const unsigned long *p, const char *type, int n)
{ if (soap_element_begin_out(soap, tag, soap_embedded_id(soap, id, p, n), type)
   || soap_string_out(soap, soap_unsignedLong2s(soap, *p), 0))
    return soap->error;
  return soap_element_end_out(soap, tag);
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_s2unsignedLong(struct soap *soap, const char *s, unsigned long *p)
{ if (s)
  { char *r;
    *p = soap_strtoul(s, &r, 10);
    if (*r)
      soap->error = SOAP_TYPE;
  }
  return soap->error;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
unsigned long *
SOAP_FMAC2
soap_inunsignedLong(struct soap *soap, const char *tag, unsigned long *p, const char *type, int t)
{ if (soap_element_begin_in(soap, tag, 0))
    return NULL;
#ifndef WITH_LEAN
  if (*soap->type
   && soap_match_tag(soap, soap->type, type)
   && soap_match_tag(soap, soap->type, ":unsignedInt")
   && soap_match_tag(soap, soap->type, ":unsignedShort")
   && soap_match_tag(soap, soap->type, ":unsignedByte"))
  { soap->error = SOAP_TYPE;
    soap_revert(soap);
    return NULL;
  }
#endif
  p = (unsigned long*)soap_id_enter(soap, soap->id, p, t, sizeof(unsigned long), 0, NULL, NULL, NULL);
  if (p)
  { if (soap_s2unsignedLong(soap, soap_value(soap), p))
      return NULL;
  }
  p = (unsigned long*)soap_id_forward(soap, soap->href, p, t, 0, sizeof(unsigned long), 0, NULL);
  if (soap->body && soap_element_end_in(soap, tag))
    return NULL;
  return p;
}
#endif

/******************************************************************************/
#ifndef WITH_LEAN
SOAP_FMAC1
const char*
SOAP_FMAC2
soap_ULONG642s(struct soap *soap, ULONG64 n)
{ sprintf(soap->tmpbuf, SOAP_ULONG_FORMAT, n);
  return soap->tmpbuf;
}
#endif

/******************************************************************************/
#ifndef WITH_LEAN
SOAP_FMAC1
int
SOAP_FMAC2
soap_outULONG64(struct soap *soap, const char *tag, int id, const ULONG64 *p, const char *type, int n)
{ if (soap_element_begin_out(soap, tag, soap_embedded_id(soap, id, p, n), type)
   || soap_string_out(soap, soap_ULONG642s(soap, *p), 0))
    return soap->error;
  return soap_element_end_out(soap, tag);
}
#endif

/******************************************************************************/
#ifndef WITH_LEAN
SOAP_FMAC1
int
SOAP_FMAC2
soap_s2ULONG64(struct soap *soap, const char *s, ULONG64 *p)
{ if (s && sscanf(s, SOAP_ULONG_FORMAT, p) != 1)
    soap->error = SOAP_TYPE;
  return soap->error;
}
#endif

/******************************************************************************/
#ifndef WITH_LEAN
SOAP_FMAC1
ULONG64 *
SOAP_FMAC2
soap_inULONG64(struct soap *soap, const char *tag, ULONG64 *p, const char *type, int t)
{ if (soap_element_begin_in(soap, tag, 0))
    return NULL;
  if (*soap->type
   && soap_match_tag(soap, soap->type, type)
   && soap_match_tag(soap, soap->type, ":positiveInteger")
   && soap_match_tag(soap, soap->type, ":nonNegativeInteger")
   && soap_match_tag(soap, soap->type, ":unsignedLong")
   && soap_match_tag(soap, soap->type, ":unsignedInt")
   && soap_match_tag(soap, soap->type, ":unsignedShort")
   && soap_match_tag(soap, soap->type, ":unsignedByte"))
  { soap->error = SOAP_TYPE;
    soap_revert(soap);
    return NULL;
  }
  p = (ULONG64*)soap_id_enter(soap, soap->id, p, t, sizeof(ULONG64), 0, NULL, NULL, NULL);
  if (p)
  { if (soap_s2ULONG64(soap, soap_value(soap), p))
      return NULL;
  }
  p = (ULONG64*)soap_id_forward(soap, soap->href, p, t, 0, sizeof(ULONG64), 0, NULL);
  if (soap->body && soap_element_end_in(soap, tag))
    return NULL;
  return p;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_s2string(struct soap *soap, const char *s, char **t)
{ *t = NULL;
  if (s && !(*t = soap_strdup(soap, s)))
    soap->error = SOAP_EOM;
  return soap->error;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_s2QName(struct soap *soap, const char *s, char **t)
{ if (s)
  { struct soap_nlist *np;
    const char *p;
    if (!strncmp(s, "xml:", 4))
    { *t = soap_strdup(soap, s);
      return SOAP_OK;
    }
    np = soap->nlist;
    p = strchr(s, ':');
    if (p)
    { register int n = p - s;
      while (np && (strncmp(np->id, s, n) || np->id[n]))
        np = np->next;
      p++;
    }
    else
    { while (np && *np->id)
        np = np->next;
      p = s;
    }
    if (np)
    { if (np->index >= 0 && soap->local_namespaces)
      { register const char *q = soap->local_namespaces[np->index].id;
        if (q)
        { if ((*t = (char*)soap_malloc(soap, strlen(p) + strlen(q) + 2)))
            sprintf(*t, "%s:%s", q, p);
          return SOAP_OK;
        }
      }
      if (np->ns)
      { if ((*t = (char*)soap_malloc(soap, strlen(p) + strlen(np->ns) + 4)))
          sprintf(*t, "\"%s\":%s", np->ns, p);
        return SOAP_OK;
      }
      DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Namespace prefix of '%s' not defined (index=%d, URI=%s)\n", s, np->index, np->ns?np->ns:""));
      return soap->error = SOAP_NAMESPACE; 
    }
    DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Namespace prefix of '%s' not defined, assuming empty namespace\n", s));
    if ((*t = (char*)soap_malloc(soap, strlen(p) + 4)))
      sprintf(*t, "\"\":%s", p);
  }
  return soap->error;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
const char*
SOAP_FMAC2
soap_QName2s(struct soap *soap, const char *s)
{ struct Namespace *p;
  char *t;
  int n;
  if (!s || *s != '"')
    return s;
  s++;
  if ((p = soap->local_namespaces))
  { for (; p->id; p++)
    { if (p->ns)
        if (!soap_tag_cmp(s, p->ns))
          break;
      if (p->in)
        if (!soap_tag_cmp(s, p->in))
          break;
    }
    if (p && p->id)
    { s = strchr(s, '"');
      if (s)
      { t = (char*)soap_malloc(soap, strlen(p->id) + strlen(s));
        strcpy(t, p->id);
	strcat(t, s + 1);
        return t;
      }
    }
  }
  t = (char*)strchr(s, '"');
  if (t)
    n = t - s;
  else
    n = 0;
  t = soap_strdup(soap, s);
  t[n] = '\0';
  sprintf(soap->tmpbuf, "xmlns:_%lu", soap->idnum++);
  soap_set_attr(soap, soap->tmpbuf, t);
  s = strchr(s, '"');
  if (s)
  { t = (char*)soap_malloc(soap, strlen(soap->tmpbuf) + strlen(s) - 6);
    strcpy(t, soap->tmpbuf + 6);
    strcat(t, s + 1);
  }
  return t;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_outstring(struct soap *soap, const char *tag, int id, char *const*p, const char *type, int n) 
{ id = soap_element_id(soap, tag, id, *p, NULL, 0, type, n);
  if (id < 0
   || soap_element_begin_out(soap, tag, id, type)
   || soap_string_out(soap, *p, 0)
   || soap_element_end_out(soap, tag))
    return soap->error;
  return SOAP_OK;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
char **
SOAP_FMAC2
soap_instring(struct soap *soap, const char *tag, char **p, const char *type, int t, int flag, long minlen, long maxlen)
{ if (soap_element_begin_in(soap, tag, 1))
    return NULL;
  if (!p)
    if (!(p = (char**)soap_malloc(soap, sizeof(char*))))
      return NULL;
  if (soap->body)
  { *p = soap_string_in(soap, flag, minlen, maxlen);
    if (!*p || !(char*)soap_id_enter(soap, soap->id, *p, t, sizeof(char*), 0, NULL, NULL, NULL))
      return NULL;
  }
  else
    *p = NULL;
  p = (char**)soap_id_lookup(soap, soap->href, (void**)p, t, sizeof(char**), 0);
  if (soap->body && soap_element_end_in(soap, tag))
    return NULL;
  return p;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_outwstring(struct soap *soap, const char *tag, int id, wchar_t *const*p, const char *type, int n) 
{ id = soap_element_id(soap, tag, id, *p, NULL, 0, type, n);
  if (id < 0
   || soap_element_begin_out(soap, tag, id, type)
   || soap_wstring_out(soap, *p, 0)
   || soap_element_end_out(soap, tag))
    return soap->error;
  return SOAP_OK;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
wchar_t **
SOAP_FMAC2
soap_inwstring(struct soap *soap, const char *tag, wchar_t **p, const char *type, int t, long minlen, long maxlen)
{ if (soap_element_begin_in(soap, tag, 1))
    return NULL;
  if (!p)
    if (!(p = (wchar_t**)soap_malloc(soap, sizeof(wchar_t*))))
      return NULL;
  if (soap->body)
  { *p = soap_wstring_in(soap, 1, minlen, maxlen);
    if (!*p || !(wchar_t*)soap_id_enter(soap, soap->id, *p, t, sizeof(wchar_t*), 0, NULL, NULL, NULL))
      return NULL;
  }
  else
    *p = NULL;
  p = (wchar_t**)soap_id_lookup(soap, soap->href, (void**)p, t, sizeof(wchar_t**), 0);
  if (soap->body && soap_element_end_in(soap, tag))
    return NULL;
  return p;
}
#endif

/******************************************************************************/
#ifndef WITH_LEAN
static time_t
soap_timegm(struct tm *T)
{
#if defined(HAVE_TIMEGM)
  return timegm(T);
#elif defined(HAVE_GETTIMEOFDAY)
  struct timezone t;
  struct timeval tv;
  memset((void*)&t, 0, sizeof(t));
  gettimeofday(&tv, &t);
  T->tm_min -= t.tz_minuteswest - (t.tz_dsttime != 0)*60;
  T->tm_isdst = 0;
  return mktime(T);
/* WR[ */
  /* The following define was added for VxWorks*/
#elif defined(HAVE_MKTIME)
  /* FOR VXWORKS:
  vxWorks does not seem to have any variable representation of time zones, but
  timezone information can be set in INSTALL_DIR/target/h/private/timeP.h header
  file, by setting the ZONEBUFFER define.  The ZONEBUFFER define follows this 
  format:
    name_of_zone:<(unused)>:time_in_minutes_from_UTC:daylight_start:daylight_end 
  To calculate local time, the value of time_in_minutes_from_UTC is subtracted
  from UTC; time_in_minutes_from_UTC must be positive. Daylight information is 
  expressed as mmddhh (month-day-hour), for example: 
    UTC::0:040102:100102
  */
  return mktime(T);
/* ]WR */
#elif defined(HAVE_FTIME)
  struct timeb t;
  memset((void*)&t, 0, sizeof(t));
  t.timezone = 0;
  t.dstflag = -1;
  ftime(&t);
  T->tm_min -= t.timezone - (t.dstflag != 0)*60;
  T->tm_isdst = 0;
  return mktime(T);
#else
#warning "time_t (de)serialization is not MT safe on this platform"
  time_t t;
  char *tz = getenv("TZ");
  putenv("TZ=UTC");
  tzset();
  t = mktime(T);
  if (tz)
  { char tmp[16];
    strcpy(tmp, "TZ=");
    strncat(tmp, tz, 12);
    tmp[15] = '\0';
    putenv(tmp);
  }
  else
    putenv("TZ=");
  tzset();
  return t;
#endif
}
#endif

/******************************************************************************/
#ifndef WITH_LEAN
SOAP_FMAC1
const char*
SOAP_FMAC2
soap_dateTime2s(struct soap *soap, time_t n)
{ struct tm T;
  struct tm *pT = &T;
#if defined(HAVE_GMTIME_R)
  if (gmtime_r(&n, pT))
    strftime(soap->tmpbuf, sizeof(soap->tmpbuf), "%Y-%m-%dT%H:%M:%SZ", pT);
/* WR[ */
  /* The following defines were added for VxWorks*/
#elif defined(HAVE_PGMTIME_R)
  if (gmtime_r(&n, pT))
    strftime(soap->tmpbuf, sizeof(soap->tmpbuf), "%Y-%m-%dT%H:%M:%SZ", pT);
#elif defined(HAVE_PGMTIME)
  if (gmtime(&n, pT))
    strftime(soap->tmpbuf, sizeof(soap->tmpbuf), "%Y-%m-%dT%H:%M:%SZ", pT);
/* ]WR */
#elif defined(HAVE_GMTIME)
  if ((pT = gmtime(&n)))
    strftime(soap->tmpbuf, sizeof(soap->tmpbuf), "%Y-%m-%dT%H:%M:%SZ", pT);
#elif defined(HAVE_GETTIMEOFDAY)
  struct timezone tz;
  memset((void*)&tz, 0, sizeof(tz));
#if defined(HAVE_LOCALTIME_R)
  if (localtime_r(&n, pT))
  { struct timeval tv;
    gettimeofday(&tv, &tz);
    strftime(soap->tmpbuf, sizeof(soap->tmpbuf), "%Y-%m-%dT%H:%M:%S", pT);
    sprintf(soap->tmpbuf + strlen(soap->tmpbuf), "%+03d:%02d", -tz.tz_minuteswest/60+(tz.tz_dsttime!=0), abs(tz.tz_minuteswest)%60);
  }
#else
  if ((pT = localtime(&n)))
  { struct timeval tv;
    gettimeofday(&tv, &tz);
    strftime(soap->tmpbuf, sizeof(soap->tmpbuf), "%Y-%m-%dT%H:%M:%S", pT);
    sprintf(soap->tmpbuf + strlen(soap->tmpbuf), "%+03d:%02d", -tz.tz_minuteswest/60+(tz.tz_dsttime!=0), abs(tz.tz_minuteswest)%60);
  }
#endif
#elif defined(HAVE_FTIME)
  struct timeb t;
  memset((void*)&t, 0, sizeof(t));
#if defined(HAVE_LOCALTIME_R)
  if (localtime_r(&n, pT))
  { ftime(&t);
    strftime(soap->tmpbuf, sizeof(soap->tmpbuf), "%Y-%m-%dT%H:%M:%S", pT);
    sprintf(soap->tmpbuf + strlen(soap->tmpbuf), "%+03d:%02d", -t.timezone/60+(t.dstflag!=0), abs(t.timezone)%60);
  }
/* WR[ */
  /* The following defines were added for VxWorks*/
#elif defined(HAVE_PLOCALTIME_R)
  if (localtime_r(&n, pT))
  { strftime(soap->tmpbuf, sizeof(soap->tmpbuf), "%Y-%m-%dT%H:%M:%S", pT);
    sprintf(soap->tmpbuf+strlen(soap->tmpbuf), "%+03d:%02d", t.timezone/60, abs(t.timezone)%60);
  }
/* ]WR */
#else
  if ((pT = localtime(&n)))
  { ftime(&t);
    strftime(soap->tmpbuf, sizeof(soap->tmpbuf), "%Y-%m-%dT%H:%M:%S", pT);
    sprintf(soap->tmpbuf + strlen(soap->tmpbuf), "%+03d:%02d", -t.timezone/60+(t.dstflag!=0), abs(t.timezone)%60);
  }
#endif
#elif defined(HAVE_LOCALTIME_R)
  if (localtime_r(&n, pT))
    strftime(soap->tmpbuf, sizeof(soap->tmpbuf), "%Y-%m-%dT%H:%M:%S", pT);
/* WR[ */
  /* The following defines were added for VxWorks*/
#elif defined(HAVE_PLOCALTIME_R)
  if (localtime_r(&n, pT))
    strftime(soap->tmpbuf, sizeof(soap->tmpbuf), "%Y-%m-%dT%H:%M:%S", pT);
/* ]WR */
#else
  if ((pT = localtime(&n)))
    strftime(soap->tmpbuf, sizeof(soap->tmpbuf), "%Y-%m-%dT%H:%M:%S", pT);
#endif
  else
    strcpy(soap->tmpbuf, "1969-12-31T23:59:59Z");
  return soap->tmpbuf;
}
#endif

/******************************************************************************/
#ifndef WITH_LEAN
SOAP_FMAC1
int
SOAP_FMAC2
soap_outdateTime(struct soap *soap, const char *tag, int id, const time_t *p, const char *type, int n)
{ if (soap_element_begin_out(soap, tag, soap_embedded_id(soap, id, p, n), type)
   || soap_string_out(soap, soap_dateTime2s(soap, *p), 0))
    return soap->error;
  return soap_element_end_out(soap, tag);
}
#endif

/******************************************************************************/
#ifndef WITH_LEAN
SOAP_FMAC1
int
SOAP_FMAC2
soap_s2dateTime(struct soap *soap, const char *s, time_t *p)
{ if (s)
  { struct tm T;
    char zone[16];
    memset((void*)&T, 0, sizeof(T));
    zone[sizeof(zone)-1] = '\0';
    sscanf(s, "%d-%d-%dT%d:%d:%d%15s", &T.tm_year, &T.tm_mon, &T.tm_mday, &T.tm_hour, &T.tm_min, &T.tm_sec, zone);
    if (T.tm_year == 1)
      T.tm_year = 70;
    else
      T.tm_year -= 1900;
    T.tm_mon--;
    if (*zone)
    { if (*zone == '.')
      { for (s = zone + 1; *s; s++)
          if (*s < '0' || *s > '9')
            break;
      }
      else
        s = zone;
      if (*s != 'Z')
      { int h = 0, m = 0;
        sscanf(s, "%d:%d", &h, &m);
        T.tm_hour -= h;
        if (h >= 0)
          T.tm_min -= m;
        else
          T.tm_min += m;
      }
      *p = soap_timegm(&T);
    }
    else
      *p = mktime(&T); /* no time zone: suppose it is localtime? */
  }
  return soap->error;
}
#endif

/******************************************************************************/
#ifndef WITH_LEAN
SOAP_FMAC1
time_t *
SOAP_FMAC2
soap_indateTime(struct soap *soap, const char *tag, time_t *p, const char * type, int t)
{ if (soap_element_begin_in(soap, tag, 0))
    return NULL;
  if (*soap->type
   && soap_match_tag(soap, soap->type, type)
   && soap_match_tag(soap, soap->type, ":dateTime"))
  { soap->error = SOAP_TYPE;
    soap_revert(soap);
    return NULL;
  }
  p = (time_t*)soap_id_enter(soap, soap->id, p, t, sizeof(time_t), 0, NULL, NULL, NULL);
  if (p)
  { if (soap_s2dateTime(soap, soap_value(soap), p))
      return NULL;
  }
  p = (time_t*)soap_id_forward(soap, soap->href, p, t, 0, sizeof(time_t), 0, NULL);
  if (soap->body && soap_element_end_in(soap, tag))
    return NULL;
  return p;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_outliteral(struct soap *soap, const char *tag, char *const*p)
{ int i;
  const char *t = NULL;
  if (tag && *tag != '-')
  { if ((t = strchr(tag, ':')))
    { strncpy(soap->tmpbuf, tag, t-tag);
      soap->tmpbuf[t-tag] = '\0';
      for (i = 0; soap->local_namespaces[i].id; i++)
        if (!strcmp(soap->tmpbuf, soap->local_namespaces[i].id))
          break;
      t++;
      sprintf(soap->tmpbuf, "<%s xmlns=\"%s\">", t, soap->local_namespaces[i].ns ? soap->local_namespaces[i].ns : SOAP_STR_EOS);
    }
    else
    { t = tag;
      sprintf(soap->tmpbuf, "<%s>", tag);
    }
    if (soap_send(soap, soap->tmpbuf))
      return soap->error;
  }
  if (p && *p)
  { if (soap_send(soap, *p))
      return soap->error;
  }
  if (t)
  { sprintf(soap->tmpbuf, "</%s>", t);
    return soap_send(soap, soap->tmpbuf);
  }
  return SOAP_OK;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
char **
SOAP_FMAC2
soap_inliteral(struct soap *soap, const char *tag, char **p)
{ if (soap_element_begin_in(soap, tag, 1))
    return NULL;
  if (!p)
    if (!(p = (char**)soap_malloc(soap, sizeof(char*))))
      return NULL;
  if (soap->null)
    *p = NULL;
  else if (soap->body)
    *p = soap_string_in(soap, 0, -1, -1);
  else
    *p = NULL;
  if (soap->body && soap_element_end_in(soap, tag))
    return NULL;
  return p;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_outwliteral(struct soap *soap, const char *tag, wchar_t *const*p)
{ int i;
  const char *t = NULL;
  wchar_t c;
  const wchar_t *s;
  if (tag && *tag != '-')
  { if (tag && (t = strchr(tag, ':')))
    { strncpy(soap->tmpbuf, tag, t-tag);
      soap->tmpbuf[t-tag] = '\0';
      for (i = 0; soap->local_namespaces[i].id; i++)
        if (!strcmp(soap->tmpbuf, soap->local_namespaces[i].id))
          break;
      t++;
      sprintf(soap->tmpbuf, "<%s xmlns=\"%s\">", t, soap->local_namespaces[i].ns ? soap->local_namespaces[i].ns : SOAP_STR_EOS);
    }
    else
    { t = tag;
      sprintf(soap->tmpbuf, "<%s>", tag);
    }
    if (soap_send(soap, soap->tmpbuf))
      return soap->error;
  }
  if (p)
  { s = *p;
    while ((c = *s++))
      if (soap_pututf8(soap, (unsigned char)c))
        return soap->error;
  }
  if (t)
  { sprintf(soap->tmpbuf, "</%s>", t);
    return soap_send(soap, soap->tmpbuf);
  }
  return SOAP_OK;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
wchar_t **
SOAP_FMAC2
soap_inwliteral(struct soap *soap, const char *tag, wchar_t **p)
{ if (soap_element_begin_in(soap, tag, 1))
    return NULL;
  if (!p)
    if (!(p = (wchar_t**)soap_malloc(soap, sizeof(wchar_t*))))
      return NULL;
  if (soap->null)
    *p = NULL;
  else if (soap->body)
    *p = soap_wstring_in(soap, 0, -1, -1);
  else
    *p = NULL;
  if (soap->body && soap_element_end_in(soap, tag))
    return NULL;
  return p;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
const char *
SOAP_FMAC2
soap_value(struct soap *soap)
{ size_t i;
  soap_wchar c = 0;
  char *s = soap->tmpbuf;
  if (!soap->body)
    return SOAP_STR_EOS;
  for (i = 0; i < sizeof(soap->tmpbuf) - 1; i++)
  { c = soap_get(soap);
    if (c == SOAP_TT || (int)c == EOF || soap_blank(c))
      break;
    *s++ = (char)c;
  }
  if ((int)c == EOF || c == SOAP_TT)
    soap_unget(soap, c);
  *s = '\0';
  return soap->tmpbuf; /* return non-null pointer */
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_getline(struct soap *soap, char *s, int len)
{ int i = len;
  soap_wchar c = 0;
  for (;;)
  { while (--i > 0)
    { c = soap_getchar(soap);
      if (c == '\r')
        break;
      if ((int)c == EOF)
        return soap->error = SOAP_EOF;
      *s++ = (char)c;
    }
    c = soap_getchar(soap);
    if (c == '\n')
    { *s = '\0';
      if (i+1 == len) /* empty line: end of HTTP header */
        break;
      c = soap_unget(soap, soap_getchar(soap));
      if (c != ' ' && c != '\t') /* HTTP line continuation? */
        break;
    }
    else if ((int)c == EOF)
      return soap->error = SOAP_EOF;
  }
  return SOAP_OK;
}
#endif

/******************************************************************************/
#ifndef PALM_1
static size_t
soap_count_attachments(struct soap *soap)
{ 
#ifndef WITH_LEANER
  register struct soap_multipart *content;
  register size_t count = soap->count;
  if (soap->mode & SOAP_ENC_DIME)
  { DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Calculating the size of DIME attachments\n"));
    for (content = soap->dime.first; content; content = content->next)
    { count += 12 + ((content->size+3)&(~3));
      if (content->id)
        count += ((strlen(content->id)+3)&(~3));
      if (content->type)
        count += ((strlen(content->type)+3)&(~3));
      if (content->options)
        count += ((((unsigned char)content->options[2] << 8) | ((unsigned char)content->options[3]))+7)&(~3);
      DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Size of DIME attachment %lu bytes\n", (unsigned long)content->size));
    }
  }
  if ((soap->mode & SOAP_ENC_MIME) && soap->mime.boundary)
  { register size_t n = strlen(soap->mime.boundary);
    DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Calculating the size of MIME attachments\n"));
    for (content = soap->mime.first; content; content = content->next)
    { register const char *s;
      /* count \r\n--boundary\r\n */
      count += 6 + n;
      /* count Content-Type: ...\r\n */
      if (content->type)
        count += 16 + strlen(content->type);
      s = soap_str_code(mime_codes, content->encoding);
      /* count Content-Transfer-Encoding: ...\r\n */
      if (s)
        count += 29 + strlen(s);
      /* count Content-ID: ...\r\n */
      if (content->id)
        count += 14 + strlen(content->id);
      /* count Content-Location: ...\r\n */
      if (content->location)
        count += 20 + strlen(content->location);
      /* count Content-Description: ...\r\n */
      if (content->description)
        count += 23 + strlen(content->location);
      /* count \r\n...content */
      count += 2 + content->size;
      DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Size of MIME attachment %lu bytes\n", (unsigned long)content->size));
    }
    /* count \r\n--boundary--\r\n */
    count += 8 + n;
  }
  return count;
#else
  return soap->count;
#endif
}
#endif

/******************************************************************************/
#ifndef WITH_LEANER
#ifndef PALM_1
static int
soap_putdimefield(struct soap *soap, const char *s, size_t n)
{ if (soap_send_raw(soap, s, n))
    return soap->error;
  return soap_send_raw(soap, SOAP_STR_PADDING, -(long)n&3);
}
#endif
#endif

/******************************************************************************/
#ifndef WITH_LEANER
#ifndef PALM_1
SOAP_FMAC1
char *
SOAP_FMAC2
soap_dime_option(struct soap *soap, unsigned short optype, const char *option)
{ size_t n;
  char *s = NULL;
  if (option)
  { n = strlen(option);
    s = (char*)soap_malloc(soap, n + 5);
    if (s)
    { s[0] = optype >> 8;
      s[1] = optype & 0xFF;
      s[2] = n >> 8;
      s[3] = n & 0xFF;
      strcpy(s + 4, option);
    }
  }
  return s;
}
#endif
#endif

/******************************************************************************/
#ifndef WITH_LEANER
#ifndef PALM_1
SOAP_FMAC1
int
SOAP_FMAC2
soap_putdimehdr(struct soap *soap)
{ unsigned char tmp[12];
  size_t optlen = 0, idlen = 0, typelen = 0;
  DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Put DIME header id='%s'\n", soap->dime.id?soap->dime.id:""));
  if (soap->dime.options)
    optlen = (((unsigned char)soap->dime.options[2] << 8) | ((unsigned char)soap->dime.options[3])) + 4;
  if (soap->dime.id)
    idlen = strlen(soap->dime.id);
  if (soap->dime.type)
    typelen = strlen(soap->dime.type);
  tmp[0] = SOAP_DIME_VERSION | (soap->dime.flags & 0x7);
  tmp[1] = soap->dime.flags & 0xF0;
  tmp[2] = optlen >> 8;
  tmp[3] = optlen & 0xFF;
  tmp[4] = idlen >> 8;
  tmp[5] = idlen & 0xFF;
  tmp[6] = typelen >> 8;
  tmp[7] = typelen & 0xFF;
  tmp[8] = soap->dime.size >> 24;
  tmp[9] = (soap->dime.size >> 16) & 0xFF;
  tmp[10] = (soap->dime.size >> 8) & 0xFF;
  tmp[11] = soap->dime.size & 0xFF;
  if (soap_send_raw(soap, (char*)tmp, 12)
   || soap_putdimefield(soap, soap->dime.options, optlen)
   || soap_putdimefield(soap, soap->dime.id, idlen)
   || soap_putdimefield(soap, soap->dime.type, typelen))
    return soap->error;
  return SOAP_OK;
}
#endif
#endif

/******************************************************************************/
#ifndef WITH_LEANER
#ifndef PALM_1
SOAP_FMAC1
int
SOAP_FMAC2
soap_putdime(struct soap *soap)
{ struct soap_multipart *content;
  if (!(soap->mode & SOAP_ENC_DIME))
    return SOAP_OK;
  for (content = soap->dime.first; content; content = content->next)
  { void *handle;
    soap->dime.size = content->size;
    soap->dime.id = content->id;
    soap->dime.type = content->type;
    soap->dime.options = content->options;
    soap->dime.flags = SOAP_DIME_VERSION | SOAP_DIME_MEDIA;
    if (soap->fdimereadopen && ((handle = soap->fdimereadopen(soap, (void*)content->ptr, content->id, content->type, content->options)) || soap->error))
    { size_t size = content->size;
      if (!handle)
      { DBGLOG(TEST, SOAP_MESSAGE(fdebug, "fdimereadopen failed\n"));
        return soap->error;
      }
      if (!content->size && ((soap->mode & SOAP_ENC_XML) || (soap->mode & SOAP_IO) == SOAP_IO_CHUNK || (soap->mode & SOAP_IO) == SOAP_IO_STORE))
      { size_t chunksize = sizeof(soap->tmpbuf);
        DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Chunked streaming DIME\n"));
        do 
        { size = soap->fdimeread(soap, handle, soap->tmpbuf, chunksize);
          DBGLOG(TEST, SOAP_MESSAGE(fdebug, "fdimeread returned %lu bytes\n", (unsigned long)size));
          if (size < chunksize)
 	  { soap->dime.flags &= ~SOAP_DIME_CF;
            if (!content->next)
              soap->dime.flags |= SOAP_DIME_ME;
	  }
          else
            soap->dime.flags |= SOAP_DIME_CF;
	  soap->dime.size = size;
          if (soap_putdimehdr(soap)
	   || soap_putdimefield(soap, soap->tmpbuf, size))
            break;
          if (soap->dime.id)
 	  { soap->dime.flags &= ~(SOAP_DIME_MB | SOAP_DIME_MEDIA);
            soap->dime.id = NULL;
            soap->dime.type = NULL;
            soap->dime.options = NULL;
          }  
        } while (size >= chunksize);
      }
      else
      { if (!content->next)
          soap->dime.flags |= SOAP_DIME_ME;
        if (soap_putdimehdr(soap))
          return soap->error;
        do
        { size_t bufsize;
	  if (size < sizeof(soap->tmpbuf))
            bufsize = size;
          else
            bufsize = sizeof(soap->tmpbuf);
          if (!(bufsize = soap->fdimeread(soap, handle, soap->tmpbuf, bufsize)))
          { DBGLOG(TEST, SOAP_MESSAGE(fdebug, "fdimeread failed: insufficient data (%lu bytes remaining from %lu bytes)\n", (unsigned long)size, (unsigned long)soap->dime.size));
            soap->error = SOAP_EOF;
	    break;
          }
          if (soap_send_raw(soap, soap->tmpbuf, bufsize))
            break;
          size -= bufsize;
        } while (size);
        DBGLOG(TEST, SOAP_MESSAGE(fdebug, "fdimereadclose\n"));
        soap_send_raw(soap, SOAP_STR_PADDING, -(long)soap->dime.size&3);
      }
      DBGLOG(TEST, SOAP_MESSAGE(fdebug, "fdimereadclose\n"));
      if (soap->fdimereadclose)
        soap->fdimereadclose(soap, handle);
    }
    else
    { if (!content->next)
        soap->dime.flags |= SOAP_DIME_ME;
      if (soap_putdimehdr(soap)
       || soap_putdimefield(soap, (char*)content->ptr, content->size))
        return soap->error;
    }
  }
  return SOAP_OK;
}
#endif
#endif

/******************************************************************************/
#ifndef WITH_LEANER
#ifndef PALM_1
static char *
soap_getdimefield(struct soap *soap, size_t n)
{ register soap_wchar c;
  register int i;
  register char *s;
  char *p = NULL;
  if (n)
  { p = (char*)soap_malloc(soap, n + 1);
    if (p)
    { s = p;
      for (i = n; i > 0; i--)
      { if ((int)(c = soap_get1(soap)) == EOF)
        { soap->error = SOAP_EOF;
          return NULL;
        }
        *s++ = (char)c;
      }
      *s = '\0';
      if ((soap->error = soap_move(soap, -(long)n&3)))
        return NULL;
    }
    else
      soap->error = SOAP_EOM;
  }
  return p;
}
#endif
#endif

/******************************************************************************/
#ifndef WITH_LEANER
#ifndef PALM_1
SOAP_FMAC1
int
SOAP_FMAC2
soap_getdimehdr(struct soap *soap)
{ register soap_wchar c;
  register char *s;
  register int i;
  unsigned char tmp[12];
  size_t optlen, idlen, typelen;
  if (!(soap->mode & SOAP_ENC_DIME))
    return soap->error = SOAP_DIME_END;
  DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Get DIME header\n"));
  if (soap->dime.buflen || soap->dime.chunksize)
  { if (soap_move(soap, (long)(soap->dime.size - soap_tell(soap))))
      return soap->error = SOAP_EOF;
    soap_unget(soap, soap_getchar(soap)); /* skip padding and get hdr */
    DBGLOG(TEST, SOAP_MESSAGE(fdebug, "... From chunked\n"));
    return SOAP_OK;
  }
  s = (char*)tmp;
  for (i = 12; i > 0; i--)
  { if ((int)(c = soap_getchar(soap)) == EOF)
      return soap->error = SOAP_EOF;
    *s++ = (char)c;
  }
  if ((tmp[0] & 0xF8) != SOAP_DIME_VERSION)
    return soap->error = SOAP_DIME_MISMATCH;
  soap->dime.flags = (tmp[0] & 0x7) | (tmp[1] & 0xF0);
  optlen = (tmp[2] << 8) | tmp[3];
  idlen = (tmp[4] << 8) | tmp[5];
  typelen = (tmp[6] << 8) | tmp[7];
  soap->dime.size = (tmp[8] << 24) | (tmp[9] << 16) | (tmp[10] << 8) | tmp[11];
  DBGLOG(TEST, SOAP_MESSAGE(fdebug, "DIME size=%lu flags=0x%X\n", (unsigned long)soap->dime.size, soap->dime.flags));
  if (!(soap->dime.options = soap_getdimefield(soap, optlen)) && soap->error)
    return soap->error;
  if (!(soap->dime.id = soap_getdimefield(soap, idlen)) && soap->error)
    return soap->error;
  if (!(soap->dime.type = soap_getdimefield(soap, typelen)) && soap->error)
    return soap->error;
  DBGLOG(TEST, SOAP_MESSAGE(fdebug, "DIME id=%s, type=%s, options=%s\n", soap->dime.id?soap->dime.id:"", soap->dime.type?soap->dime.type:"", soap->dime.options?soap->dime.options+4:""));
  if (soap->dime.flags & SOAP_DIME_ME)
    soap->mode &= ~SOAP_ENC_DIME;
  return SOAP_OK;
}
#endif
#endif

/******************************************************************************/
#ifndef WITH_LEANER
#ifndef PALM_1
SOAP_FMAC1
int
SOAP_FMAC2
soap_getdime(struct soap *soap)
{ struct soap_multipart *content;
  if (soap_getdimehdr(soap))
    return soap->error;
  if (soap->fdimewriteopen && ((soap->dime.ptr = (char*)soap->fdimewriteopen(soap, soap->dime.id, soap->dime.type, soap->dime.options)) || soap->error))
  { const char *id, *type, *options;
    size_t size, n;
    if (!soap->dime.ptr)
      return soap->error;
    id = soap->dime.id;
    type = soap->dime.type;
    options = soap->dime.options;
    for (;;)
    { size = soap->dime.size;
      for (;;)
      { n = soap->buflen - soap->bufidx;
        if (size < n)
          n = size;
        if ((soap->error = soap->fdimewrite(soap, (void*)soap->dime.ptr, soap->buf + soap->bufidx, n)))
          break;
	size -= n;
	if (!size)
	{ soap->bufidx += n;
	  break;
	}
	if (soap_recv(soap))
        { soap->error = SOAP_EOF;
	  goto end;
        }
      }
      if (soap_move(soap, -(long)soap->dime.size&3))
      { soap->error = SOAP_EOF;
	break;
      }
      if (!(soap->dime.flags & SOAP_DIME_CF))
        break;
      if (soap_getdimehdr(soap))
        break;
    }
end:
    if (soap->fdimewriteclose)
      soap->fdimewriteclose(soap, (void*)soap->dime.ptr);
    soap->dime.size = 0;
    soap->dime.id = id;
    soap->dime.type = type;
    soap->dime.options = options;
  }
  else if (soap->dime.flags & SOAP_DIME_CF)
  { const char *id, *type, *options;
    register soap_wchar c;
    register char *s;
    register int i;
    id = soap->dime.id;
    type = soap->dime.type;
    options = soap->dime.options;
    if (soap_new_block(soap))
      return SOAP_EOM;
    for (;;)
    { s = (char*)soap_push_block(soap, soap->dime.size);
      if (!s)
        return soap->error = SOAP_EOM;
      for (i = soap->dime.size; i > 0; i--)
      { if ((int)(c = soap_get1(soap)) == EOF)
          return soap->error = SOAP_EOF;
        *s++ = (char)c;
      }
      if (soap_move(soap, -(long)soap->dime.size&3))
        return soap->error = SOAP_EOF;
      if (!(soap->dime.flags & SOAP_DIME_CF))
        break;
      if (soap_getdimehdr(soap))
        return soap->error;
    }
    soap->dime.size = soap->blist->size++; /* allocate one more for '\0' */
    if (!(soap->dime.ptr = soap_save_block(soap, NULL, 0)))
      return soap->error;
    soap->dime.ptr[soap->dime.size] = '\0'; /* force 0-terminated */
    soap->dime.id = id;
    soap->dime.type = type;
    soap->dime.options = options;
  }
  else
    soap->dime.ptr = soap_getdimefield(soap, soap->dime.size);
  content = soap_new_multipart(soap, &soap->dime.first, &soap->dime.last, soap->dime.ptr, soap->dime.size);
  if (!content)
    return soap->error = SOAP_EOM;
  content->id = soap->dime.id;
  content->type = soap->dime.type;
  content->options = soap->dime.options;
  return soap->error;
}
#endif
#endif

/******************************************************************************/
#ifndef WITH_LEANER
#ifndef PALM_1
SOAP_FMAC1
int
SOAP_FMAC2
soap_getmimehdr(struct soap *soap)
{ struct soap_multipart *content;
  do
  { if (soap_getline(soap, soap->msgbuf, sizeof(soap->msgbuf)))
      return soap->error;
  }
  while (!*soap->msgbuf);
  if (soap->msgbuf[0] == '-' && soap->msgbuf[1] == '-')
  { char *s = soap->msgbuf + strlen(soap->msgbuf) - 1;
    /* remove white space */
    while (soap_blank(*s))
      s--;
    s[1] = '\0';
    if (soap->mime.boundary)
    { if (strcmp(soap->msgbuf + 2, soap->mime.boundary))
        return soap->error = SOAP_MIME_ERROR;
    }
    else
      soap->mime.boundary = soap_strdup(soap, soap->msgbuf + 2);
    if (soap_getline(soap, soap->msgbuf, sizeof(soap->msgbuf)))
      return soap->error;
  }
  if (soap_set_mime_attachment(soap, NULL, 0, SOAP_MIME_NONE, NULL, NULL, NULL, NULL))
    return soap->error = SOAP_EOM;
  content = soap->mime.last;
  for (;;)
  { register char *key = soap->msgbuf;
    register char *val;
    if (!*key)
      break;
    DBGLOG(TEST,SOAP_MESSAGE(fdebug, "MIME header: %s\n", key));
    val = strchr(soap->msgbuf, ':');
    if (val)
    { *val = '\0';
      do val++;
      while (*val && *val <= 32);
      if (!soap_tag_cmp(key, "Content-ID"))
        content->id = soap_strdup(soap, val);
      else if (!soap_tag_cmp(key, "Content-Location"))
        content->location = soap_strdup(soap, val);
      else if (!soap_tag_cmp(key, "Content-Type"))
        content->type = soap_strdup(soap, val);
      else if (!soap_tag_cmp(key, "Content-Description"))
        content->description = soap_strdup(soap, val);
      else if (!soap_tag_cmp(key, "Content-Transfer-Encoding"))
        content->encoding = (enum soap_mime_encoding)soap_int_code(mime_codes, val, (long)SOAP_MIME_NONE);
    }
    if (soap_getline(soap, key, sizeof(soap->msgbuf)))
      return soap->error;
  }
  return SOAP_OK;
}
#endif
#endif

/******************************************************************************/
#ifndef WITH_LEANER
#ifndef PALM_1
SOAP_FMAC1
int
SOAP_FMAC2
soap_getmime(struct soap *soap)
{ register soap_wchar c;
  if (!soap->mime.last)
    return SOAP_OK;
  for (;;)
  { register size_t i, m = 0;
    register char *s;
    struct soap_multipart *content = soap->mime.last;
    DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Parsing MIME content id=%s type=%s\n", content->id?content->id:"", content->type?content->type:""));
    if (soap_new_block(soap))
      return soap->error = SOAP_EOM;
    for (;;)
    { register char *t = NULL;
      if (!(s = (char*)soap_push_block(soap, SOAP_BLKLEN)))
        return soap->error = SOAP_EOM;
      for (i = 0; i < SOAP_BLKLEN; i++)
      { if (m > 0)
        { *s++ = *t++;
	  m--;
	}
	else
        { c = soap_get1(soap);
	  if ((int)c == EOF)
	    return soap->error = SOAP_EOF;
	  if (c == '\r')
	  { t = soap->tmpbuf;
            strcpy(t, "\n--");
            strncat(t, soap->mime.boundary, sizeof(soap->tmpbuf)-3);
            t[sizeof(soap->tmpbuf)-1] = '\0';
	    do c = soap_getchar(soap);
	    while (c == *t++);
	    if ((int)c == EOF)
	      return soap->error = SOAP_EOF;
	    if (!*--t)
	      goto end;
	    *t = (char)c;
	    m = t - soap->tmpbuf + 1;
	    t = soap->tmpbuf;
	    c = '\r';
	  }
	  *s++ = (char)c;
        }
      }
    }
end:
    *s = '\0'; /* force 0-terminated */
    content->size = soap_size_block(soap, i+1)-1;
    content->ptr = soap_save_block(soap, NULL, 0);
    if (c == '-' && soap_getchar(soap) == '-')
      break;
    while (c != '\r' && (int)c != EOF && soap_blank(c))
      c = soap_getchar(soap);
    if (c != '\r' || soap_getchar(soap) != '\n')
      return soap->error = SOAP_MIME_ERROR;
    if (soap_getmimehdr(soap))
      return soap->error;
  }
  do c = soap_getchar(soap);
  while ((int)c != EOF && c != '\r');
  if ((int)c == EOF)
    return soap->error = SOAP_EOF;
  if (soap_getchar(soap) != '\n')
    return soap->error = SOAP_MIME_ERROR;
  return SOAP_OK;
}
#endif
#endif

/******************************************************************************/
#ifndef WITH_LEANER
#ifndef PALM_1
SOAP_FMAC1
int
SOAP_FMAC2
soap_putmimehdr(struct soap *soap, struct soap_multipart *content)
{ const char *s;
  DBGLOG(TEST, SOAP_MESSAGE(fdebug, "MIME attachment type=%s\n", content->type?content->type:""));
  if (soap_send3(soap, "\r\n--", soap->mime.boundary, "\r\n"))
    return soap->error;
  if (content->type && soap_send3(soap, "Content-Type: ", content->type, "\r\n"))
    return soap->error;
  s = soap_str_code(mime_codes, content->encoding);
  if (s && soap_send3(soap, "Content-Transfer-Encoding: ", s, "\r\n"))
    return soap->error;
  if (content->id && soap_send3(soap, "Content-ID: ", content->id, "\r\n"))
    return soap->error;
  if (content->location && soap_send3(soap, "Content-Location: ", content->location, "\r\n"))
    return soap->error;
  if (content->description && soap_send3(soap, "Content-Description: ", content->description, "\r\n"))
    return soap->error;
  return soap_send_raw(soap, "\r\n", 2);
}
#endif
#endif

/******************************************************************************/
#ifndef WITH_LEANER
#ifndef PALM_1
SOAP_FMAC1
int
SOAP_FMAC2
soap_putmime(struct soap *soap)
{ struct soap_multipart *content;
  if (!(soap->mode & SOAP_ENC_MIME) || !soap->mime.boundary)
    return SOAP_OK;
  DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Sending MIME attachments\n"));
  for (content = soap->mime.first; content; content = content->next)
    if (soap_putmimehdr(soap, content)
     || soap_send_raw(soap, content->ptr, content->size))
      return soap->error;
  return soap_send3(soap, "\r\n--", soap->mime.boundary, "--\r\n");
}
#endif
#endif

/******************************************************************************/
#ifndef WITH_LEANER
#ifndef PALM_1
SOAP_FMAC1
void
SOAP_FMAC2
soap_set_dime(struct soap *soap)
{ soap->omode |= SOAP_ENC_DIME;
  soap->dime.first = NULL;
  soap->dime.last = NULL;
}
#endif
#endif

/******************************************************************************/
#ifndef WITH_LEANER
#ifndef PALM_1
SOAP_FMAC1
void
SOAP_FMAC2
soap_set_mime(struct soap *soap, const char *boundary, const char *start)
{ soap->omode |= SOAP_ENC_MIME;
  soap->mime.first = NULL;
  soap->mime.last = NULL;
  soap->mime.boundary = soap_strdup(soap, boundary);
  soap->mime.start = soap_strdup(soap, start);
}
#endif
#endif

/******************************************************************************/
#ifndef WITH_LEANER
#ifndef PALM_1
SOAP_FMAC1
void
SOAP_FMAC2
soap_clr_dime(struct soap *soap)
{ soap->omode &= ~SOAP_ENC_DIME;
  soap->dime.first = NULL;
  soap->dime.last = NULL;
}
#endif
#endif

/******************************************************************************/
#ifndef WITH_LEANER
#ifndef PALM_1
SOAP_FMAC1
void
SOAP_FMAC2
soap_clr_mime(struct soap *soap)
{ soap->omode &= ~SOAP_ENC_MIME;
  soap->mime.first = NULL;
  soap->mime.last = NULL;
  soap->mime.boundary = NULL;
  soap->mime.start = NULL;
}
#endif
#endif

/******************************************************************************/
#ifndef WITH_LEANER
#ifndef PALM_1
static struct soap_multipart*
soap_new_multipart(struct soap *soap, struct soap_multipart **first, struct soap_multipart **last, char *ptr, size_t size)
{ struct soap_multipart *content;
  content = (struct soap_multipart*)soap_malloc(soap, sizeof(struct soap_multipart));
  if (content)
  { content->next = NULL;
    content->ptr = ptr;
    content->size = size;
    content->id = NULL;
    content->type = NULL;
    content->options = NULL;
    content->encoding = SOAP_MIME_NONE;
    content->location = NULL;
    content->description = NULL;
    if (!*first)
      *first = content;
    if (*last)
      (*last)->next = content;
    *last = content;
  }
  return content;
}
#endif
#endif

/******************************************************************************/
#ifndef WITH_LEANER
#ifndef PALM_1
SOAP_FMAC1
int
SOAP_FMAC2
soap_set_dime_attachment(struct soap *soap, char *ptr, size_t size, const char *type, const char *id, unsigned short optype, const char *option)
{ struct soap_multipart *content = soap_new_multipart(soap, &soap->dime.first, &soap->dime.last, ptr, size);
  if (!content)
    return SOAP_EOM;
  content->id = soap_strdup(soap, id);
  content->type = soap_strdup(soap, type);
  content->options = soap_dime_option(soap, optype, option);
  return SOAP_OK;
}
#endif
#endif

/******************************************************************************/
#ifndef WITH_LEANER
#ifndef PALM_1
SOAP_FMAC1
int
SOAP_FMAC2
soap_set_mime_attachment(struct soap *soap, char *ptr, size_t size, enum soap_mime_encoding encoding, const char *type, const char *id, const char *location, const char *description)
{ struct soap_multipart *content = soap_new_multipart(soap, &soap->mime.first, &soap->mime.last, ptr, size);
  if (!content)
    return SOAP_EOM;
  content->id = soap_strdup(soap, id);
  content->type = soap_strdup(soap, type);
  content->encoding = encoding;
  content->location = soap_strdup(soap, location);
  content->description = soap_strdup(soap, description);
  return SOAP_OK;
}
#endif
#endif

/******************************************************************************/
#ifndef WITH_LEANER
#ifndef PALM_1
SOAP_FMAC1
struct soap_multipart*
SOAP_FMAC2
soap_next_multipart(struct soap_multipart *content)
{ if (content)
    return content->next;
  return NULL;
}
#endif
#endif

/******************************************************************************/
#ifndef WITH_LEANER
#ifndef PALM_1
static void
soap_select_mime_boundary(struct soap *soap)
{ while (!soap->mime.boundary || soap_valid_mime_boundary(soap))
  { register char *s = soap->mime.boundary;
    register size_t n = 0;
    if (s)
      n = strlen(s);
    if (n < 16)
    { n = 72;
      s = soap->mime.boundary = (char*)soap_malloc(soap, n);
      if (!s)
        return;
    }
    strcpy(s, "<>");
    s += 2;
    n -= 4;
    while (n)
    { *s++ = soap_base64o[rand()&0x3F];
      n--;
    }
    *s = '\0';
    strcat(s, "<>");
  }
  if (!soap->mime.start)
    soap->mime.start = "<SOAP-ENV:Envelope>";
}
#endif
#endif

/******************************************************************************/
#ifndef WITH_LEANER
#ifndef PALM_1
static int
soap_valid_mime_boundary(struct soap *soap)
{ register struct soap_multipart *content;
  register size_t k = strlen(soap->mime.boundary);
  for (content = soap->mime.first; content; content = content->next)
  { if (content->ptr && content->size >= k)
    { register const char *p = (const char*)content->ptr; 
      register size_t i;
      for (i = 0; i < content->size - k; i++, p++)
        if (!strncmp(p, soap->mime.boundary, k))
          return SOAP_ERR;
    }
  }
  return SOAP_OK;
}
#endif
#endif

/******************************************************************************/

#ifdef WITH_COOKIES
/******************************************************************************/
SOAP_FMAC1
size_t
SOAP_FMAC2
soap_encode_cookie(const char *s, char *t, size_t len)
{ register int c;
  register size_t n = len;
  while ((c = *s++) && --n > 0)
  { if (c > ' ' && c < 128 && c != ';' && c != ',')
      *t++ = c;
    else if (n > 2)
    { *t++ = '%';
      *t++ = (c >> 4) + (c > 159 ? '7' : '0');
      c &= 0xF;
      *t++ = c + (c > 9 ? '7' : '0');
      n -= 2;
    }
    else
      break;
  }
  *t = '\0';
  return len - n;
}

/******************************************************************************/
SOAP_FMAC1
struct soap_cookie*
SOAP_FMAC2
soap_cookie(struct soap *soap, const char *name, const char *domain, const char *path)
{ struct soap_cookie *p;
  size_t n;
  if (!domain)
    domain = soap->cookie_domain;
  if (!path)
    path = soap->cookie_path;
  if (*path == '/')
    path++;
  n = strlen(path);
  for (p = soap->cookies; p; p = p->next)
    if (!strcmp(p->name, name)
     && domain
     && p->domain
     && !strcmp(p->domain, domain)
     && !strncmp(p->path, path, n))
      break;
  return p;
}

/******************************************************************************/
SOAP_FMAC1
struct soap_cookie*
SOAP_FMAC2
soap_set_cookie(struct soap *soap, const char *name, const char *value, const char *domain, const char *path)
{ struct soap_cookie **p, *q;
  int n;
  DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Set cookie: %s=%s domain=%s path=%s\n", name, value?value:"", domain?domain:"", path?path:""));
  if (!domain)
    domain = soap->cookie_domain;
  if (!path)
    path = soap->cookie_path;
  if (!path)
  { soap_set_receiver_error(soap, "Cookie path not set", NULL, SOAP_HTTP_ERROR);
    return NULL;
  }
  if (*path == '/')
    path++;
  q = soap_cookie(soap, name, domain, path);
  if (!q)
  { if ((q = (struct soap_cookie*)SOAP_MALLOC(sizeof(struct soap_cookie))))
    { if ((q->name = (char*)SOAP_MALLOC(strlen(name)+1)))
        strcpy(q->name, name);
      q->value = NULL;
      q->domain = NULL;
      q->path = NULL;
      q->expire = -1;
      q->version = 0;
      q->secure = 0;
      q->env = 0;
      q->modified = 0;
      for (p = &soap->cookies, n = soap->cookie_max; *p && n; p = &(*p)->next, n--)
        if (!strcmp((*p)->name, name) && (*p)->path && strcmp((*p)->path, path) < 0)
          break;
      if (n)
      { q->next = *p;
        *p = q;
      }
      else
      { SOAP_FREE(q->name);
        SOAP_FREE(q);
        q = NULL;
      }
    }
  }
  else
    q->modified = 1;
  if (q)
  { if (q->value)
    { SOAP_FREE(q->value);
      q->value = NULL;
    }
    if (q->domain)
    { SOAP_FREE(q->domain);
      q->domain = NULL;
    }
    if (q->path)
    { SOAP_FREE(q->path);
      q->path = NULL;
    }
    if (value && *value && (q->value = (char*)SOAP_MALLOC(strlen(value)+1)))
      strcpy(q->value, value);
    if (domain && *domain && (q->domain = (char*)SOAP_MALLOC(strlen(domain)+1)))
      strcpy(q->domain, domain);
    if (path && *path && (q->path = (char*)SOAP_MALLOC(strlen(path)+1)))
      strcpy(q->path, path);
    q->session = 1;
  }
  return q;
}

/******************************************************************************/
SOAP_FMAC1
void
SOAP_FMAC2
soap_clr_cookie(struct soap *soap, const char *name, const char *domain, const char *path)
{ struct soap_cookie **p, *q;
  if (!domain)
    domain = soap->cookie_domain;
  if (!domain)
  { soap_set_receiver_error(soap, "Cookie domain not set", SOAP_STR_EOS, SOAP_HTTP_ERROR);
    return;
  }
  if (!path)
    path = soap->cookie_path;
  if (!path)
  { soap_set_receiver_error(soap, "Cookie path not set", SOAP_STR_EOS, SOAP_HTTP_ERROR);
    return;
  }
  if (*path == '/')
    path++;
  for (p = &soap->cookies, q = *p; q; q = *p)
    if (!strcmp(q->name, name) && !strcmp(q->domain, domain) && !strncmp(q->path, path, strlen(q->path)))
    { if (q->value)
        SOAP_FREE(q->value);
      if (q->domain)
        SOAP_FREE(q->domain);
      if (q->path)
        SOAP_FREE(q->path);
      *p = q->next;
      SOAP_FREE(q);
    }
    else
      p = &q->next;
}

/******************************************************************************/
SOAP_FMAC1
char *
SOAP_FMAC2
soap_cookie_value(struct soap *soap, const char *name, const char *domain, const char *path)
{ struct soap_cookie *p;
  if ((p = soap_cookie(soap, name, domain, path)))
    return p->value;
  return NULL;
}

/******************************************************************************/
SOAP_FMAC1
long
SOAP_FMAC2
soap_cookie_expire(struct soap *soap, const char *name, const char *domain, const char *path)
{ struct soap_cookie *p;
  if ((p = soap_cookie(soap, name, domain, path)))
    return p->expire;
  return -1;
}

/******************************************************************************/
SOAP_FMAC1
int
SOAP_FMAC2
soap_set_cookie_expire(struct soap *soap, const char *name, long expire, const char *domain, const char *path)
{ struct soap_cookie *p;
  if ((p = soap_cookie(soap, name, domain, path)))
  { p->expire = expire;
    p->modified = 1;
    return SOAP_OK;
  }
  return SOAP_ERR;
}

/******************************************************************************/
SOAP_FMAC1
int
SOAP_FMAC2
soap_set_cookie_session(struct soap *soap, const char *name, const char *domain, const char *path)
{ struct soap_cookie *p;
  if ((p = soap_cookie(soap, name, domain, path)))
  { p->session = 1;
    p->modified = 1;
    return SOAP_OK;
  }
  return SOAP_ERR;
}

/******************************************************************************/
SOAP_FMAC1
int
SOAP_FMAC2
soap_clr_cookie_session(struct soap *soap, const char *name, const char *domain, const char *path)
{ struct soap_cookie *p;
  if ((p = soap_cookie(soap, name, domain, path)))
  { p->session = 0;
    p->modified = 1;
    return SOAP_OK;
  }
  return SOAP_ERR;
}

/******************************************************************************/
SOAP_FMAC1
int
SOAP_FMAC2
soap_putsetcookies(struct soap *soap)
{ struct soap_cookie *p;
  char *s, tmp[4096];
  const char *t;
  for (p = soap->cookies; p; p = p->next)
  { if (p->modified || !p->env)
    { s = tmp;
      if (p->name)
        s += soap_encode_cookie(p->name, s, tmp-s+4064);
      if (p->value && *p->value)
      { *s++ = '=';
        s += soap_encode_cookie(p->value, s, tmp-s+4064);
      }
      if (p->domain && (int)strlen(p->domain) < tmp-s+4064)
        sprintf(s, ";Domain=%s", p->domain);
      else if (soap->cookie_domain && (int)strlen(soap->cookie_domain) < tmp-s+4064)
        sprintf(s, ";Domain=%s", soap->cookie_domain);
      strcat(s, ";Path=/");
      if (p->path)
        t = p->path;
      else
        t = soap->cookie_path;
      if (t)
      { if (*t == '/')
          t++;
        if ((int)strlen(t) < tmp-s+4064)
          strcat(s, t);
      }
      s += strlen(s);
      if (p->version > 0)
        sprintf(s, ";Version=%u", p->version);
      if (p->expire >= 0)
        sprintf(s, ";Max-Age=%ld", p->expire);
      if (p->secure)
        strcat(s, ";Secure");
      DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Set-Cookie: %s\n", tmp));
      if (soap->fposthdr(soap, "Set-Cookie", tmp))
        return soap->error;
    }
  }
  return SOAP_OK;
}

/******************************************************************************/
SOAP_FMAC1
int
SOAP_FMAC2
soap_putcookies(struct soap *soap, const char *domain, const char *path, int secure)
{ struct soap_cookie **p, *q;
  unsigned int version = 0;
  time_t now = time(NULL);
  char *s, tmp[4096];
  p = &soap->cookies;
  while ((q = *p))
  { if (q->expire && now > q->expire)
    { DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Cookie %s expired\n", q->name));
      SOAP_FREE(q->name);
      if (q->value)
        SOAP_FREE(q->value);
      if (q->domain)
        SOAP_FREE(q->domain);
      if (q->path)
        SOAP_FREE(q->path);
      *p = q->next;
      SOAP_FREE(q);
    }
    else if ((!q->domain || !strcmp(q->domain, domain))
          && (!q->path || !strncmp(q->path, path, strlen(q->path)))
          && (!q->secure || secure))
    { s = tmp;
      if (q->version != version)
      { sprintf(s, "$Version=%u;", q->version);
        version = q->version;
      }
      if (q->name)
        s += soap_encode_cookie(q->name, s, tmp-s+4080);
      if (q->value && *q->value)
      { *s++ = '=';
        s += soap_encode_cookie(q->value, s, tmp-s+4080);
      }
      if (q->path && (int)strlen(q->path) < tmp-s+4080)
      { sprintf(s, ";$Path=/%s", q->path);
        s += strlen(s);
      }
      if (q->domain && (int)strlen(q->domain) < tmp-s+4080)
        sprintf(s, ";$Domain=%s", q->domain);
      DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Cookie: %s\n", tmp));
      if (soap->fposthdr(soap, "Cookie", tmp))
        return soap->error;
      p = &q->next;
    }
    else
      p = &q->next;
  }
  return SOAP_OK;
}

/******************************************************************************/
SOAP_FMAC1
void
SOAP_FMAC2
soap_getcookies(struct soap *soap, const char *val)
{ struct soap_cookie *p = NULL, *q;
  const char *s;
  char *t, tmp[4096]; /* cookie size is up to 4096 bytes [RFC2109] */
  char *domain = NULL;
  char *path = NULL;
  unsigned int version = 0;
  time_t now = time(NULL);
  if (!val)
    return;
  s = val;
  while (*s)
  { s = soap_decode_key(tmp, sizeof(tmp), s);
    if (!soap_tag_cmp(tmp, "$Version"))
    { if ((s = soap_decode_val(tmp, sizeof(tmp), s)))
      { if (p)
          p->version = (int)atol(tmp);
        else
          version = (int)atol(tmp);
      }
    }
    else if (!soap_tag_cmp(tmp, "$Path"))
    { s = soap_decode_val(tmp, sizeof(tmp), s);
      if (*tmp)
      { if ((t = (char*)SOAP_MALLOC(strlen(tmp)+1)))
          strcpy(t, tmp);
      }
      else
        t = NULL;
      if (p)
      { if (p->path)
          SOAP_FREE(p->path);
        p->path = t;
      }
      else
      { if (path)
          SOAP_FREE(path);
        path = t;
      }
    }
    else if (!soap_tag_cmp(tmp, "$Domain"))
    { s = soap_decode_val(tmp, sizeof(tmp), s);
      if (*tmp)
      { if ((t = (char*)SOAP_MALLOC(strlen(tmp)+1)))
          strcpy(t, tmp);
      }
      else
        t = NULL;
      if (p)
      { if (p->domain)
          SOAP_FREE(p->domain);
	p->domain = t;
      }
      else
      { if (domain)
          SOAP_FREE(domain);
        domain = t;
      }
    }
    else if (p && !soap_tag_cmp(tmp, "Path"))
    { if (p->path)
        SOAP_FREE(p->path);
      s = soap_decode_val(tmp, sizeof(tmp), s);
      if (*tmp)
      { if ((p->path = (char*)SOAP_MALLOC(strlen(tmp)+1)))
          strcpy(p->path, tmp);
      }
      else
        p->path = NULL;
    }
    else if (p && !soap_tag_cmp(tmp, "Domain"))
    { if (p->domain)
        SOAP_FREE(p->domain);
      s = soap_decode_val(tmp, sizeof(tmp), s);
      if (*tmp)
      { if ((p->domain = (char*)SOAP_MALLOC(strlen(tmp)+1)))
          strcpy(p->domain, tmp);
      }
      else
        p->domain = NULL;
    }
    else if (p && !soap_tag_cmp(tmp, "Version"))
    { s = soap_decode_val(tmp, sizeof(tmp), s);
      p->version = (unsigned int)atol(tmp);
    }
    else if (p && !soap_tag_cmp(tmp, "Max-Age"))
    { s = soap_decode_val(tmp, sizeof(tmp), s);
      p->expire = now + atol(tmp);
    }
    else if (p && !soap_tag_cmp(tmp, "Expires"))
    { struct tm T;
      char a[3]; 
      static const char mns[] = "anebarprayunulugepctovec";
      s = soap_decode_val(tmp, sizeof(tmp), s);
      if (strlen(tmp) > 20)
      { memset((void*)&T, 0, sizeof(T));
        a[0] = tmp[4];
        a[1] = tmp[5];
        a[2] = '\0';
        T.tm_mday = (int)atol(a);
        a[0] = tmp[8];
        a[1] = tmp[9];
        T.tm_mon = (strstr(mns, a) - mns) / 2;
        a[0] = tmp[11];
        a[1] = tmp[12];
        T.tm_year = 100 + (int)atol(a);
        a[0] = tmp[13];
        a[1] = tmp[14];
        T.tm_hour = (int)atol(a);
        a[0] = tmp[16];
        a[1] = tmp[17];
        T.tm_min = (int)atol(a);
        a[0] = tmp[19];
        a[1] = tmp[20];
        T.tm_sec = (int)atol(a);
        p->expire = soap_timegm(&T);
      }
    }
    else if (p && !soap_tag_cmp(tmp, "Secure"))
      p->secure = 1;
    else
    { if (p)
      { DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Got environment cookie %s=%s domain=%s path=%s expire=%ld secure=%d\n", p->name, p->value?p->value:"", p->domain?p->domain:"", p->path?p->path:"", p->expire, p->secure));
        if ((q = soap_set_cookie(soap, p->name, p->value, p->domain, p->path)))
        { q->version = p->version;
          q->expire = p->expire;
          q->secure = p->secure;
          q->env = 1;
        }
        if (p->name)
          SOAP_FREE(p->name);
        if (p->value)
          SOAP_FREE(p->value);
        if (p->domain)
          SOAP_FREE(p->domain);
        if (p->path)
          SOAP_FREE(p->path);
        SOAP_FREE(p);
      }
      if ((p = (struct soap_cookie*)SOAP_MALLOC(sizeof(struct soap_cookie))))
      { p->name = (char*)SOAP_MALLOC(strlen(tmp)+1);
        strcpy(p->name, tmp);
        s = soap_decode_val(tmp, sizeof(tmp), s);
        if (*tmp)
        { p->value = (char*)SOAP_MALLOC(strlen(tmp)+1);
          strcpy(p->value, tmp);
        }
        else
          p->value = NULL;
        p->domain = domain;
        p->path = path;
        p->expire = 0;
        p->secure = 0;
        p->version = version;
      }
    }
  }
  if (p)
  { DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Got cookie %s=%s domain=%s path=%s expire=%ld secure=%d\n", p->name, p->value?p->value:"", p->domain?p->domain:"", p->path?p->path:"", p->expire, p->secure));
    if ((q = soap_set_cookie(soap, p->name, p->value, p->domain, p->path)))
    { q->version = p->version;
      q->expire = p->expire;
      q->secure = p->secure;
    }
    if (p->name)
      SOAP_FREE(p->name);
    if (p->value)
      SOAP_FREE(p->value);
    if (p->domain)
      SOAP_FREE(p->domain);
    if (p->path)
      SOAP_FREE(p->path);
    SOAP_FREE(p);
  }
  if (domain)
    SOAP_FREE(domain);
  if (path)
    SOAP_FREE(path);
}

/******************************************************************************/
SOAP_FMAC1
int
SOAP_FMAC2
soap_getenv_cookies(struct soap *soap)
{ struct soap_cookie *p;
  const char *s;
  char key[4096], val[4096]; /* cookie size is up to 4096 bytes [RFC2109] */
  if (!(s = getenv("HTTP_COOKIE")))
    return SOAP_ERR;
  do
  { s = soap_decode_key(key, sizeof(key), s);
    s = soap_decode_val(val, sizeof(val), s);
    p = soap_set_cookie(soap, key, val, NULL, NULL);
    if (p)
      p->env = 1;
  } while (*s);
  return SOAP_OK;
}

/******************************************************************************/
SOAP_FMAC1
struct soap_cookie*
SOAP_FMAC2
soap_copy_cookies(struct soap *soap)
{ struct soap_cookie *p, **q, *r;
  q = &r;
  for (p = soap->cookies; p; p = p->next)
  { if (!(*q = (struct soap_cookie*)SOAP_MALLOC(sizeof(struct soap_cookie))))
      return r;
    **q = *p;
    if (p->name)
    { if (((*q)->name = (char*)SOAP_MALLOC(strlen(p->name)+1)))
        strcpy((*q)->name, p->name);
    }
    if (p->value)
    { if (((*q)->value = (char*)SOAP_MALLOC(strlen(p->value)+1)))
        strcpy((*q)->value, p->value);
    }
    if (p->domain)
    { if (((*q)->domain = (char*)SOAP_MALLOC(strlen(p->domain)+1)))
        strcpy((*q)->domain, p->domain);
    }
    if (p->path)
    { if (((*q)->path = (char*)SOAP_MALLOC(strlen(p->path)+1)))
        strcpy((*q)->path, p->path);
    }
    q = &(*q)->next;
  }
  *q = NULL;
  return r;
}

/******************************************************************************/
SOAP_FMAC1
void
SOAP_FMAC2
soap_free_cookies(struct soap *soap)
{ struct soap_cookie *p;
  for (p = soap->cookies; p; p = soap->cookies)
  { soap->cookies = p->next;
    SOAP_FREE(p->name);
    if (p->value)
      SOAP_FREE(p->value);
    if (p->domain)
      SOAP_FREE(p->domain);
    if (p->path)
      SOAP_FREE(p->path);
    SOAP_FREE(p);
  }
}

/******************************************************************************/
#endif /* WITH_COOKIES */

/******************************************************************************/
#ifdef WITH_GZIP
#ifndef PALM_1
static int
soap_getgziphdr(struct soap *soap)
{ int i;
  soap_wchar c, f = 0;
  DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Get gzip header\n"));
  for (i = 0; i < 9; i++)
  { if ((int)(c = soap_get1(soap) == EOF))
      return soap->error = SOAP_EOF;
    if (i == 2)
      f = c;
  }
  if (f & 0x04) /* FEXTRA */
  { for (i = soap_get1(soap) | (soap_get1(soap) << 8); i; i--)
      if ((int)soap_get1(soap) == EOF)
        return soap->error = SOAP_EOF;
  }
  if (f & 0x08) /* FNAME */
    do
      c = soap_get1(soap);
    while (c && (int)c != EOF);
  if ((int)c != EOF && (f & 0x10)) /* FCOMMENT */
    do
      c = soap_get1(soap);
    while (c && (int)f != EOF);
  if ((int)c != EOF && (f & 0x01)) /* FHCRC */
  { if ((int)(c = soap_get1(soap)) != EOF)
      c = soap_get1(soap);
  }
  if ((int)c == EOF)
    return soap->error = SOAP_EOF;
  return SOAP_OK;
}
#endif
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
int
SOAP_FMAC2
soap_begin_recv(struct soap *soap)
{ soap_wchar c;
  soap->error = SOAP_OK;
  soap_free(soap);
  soap_set_local_namespaces(soap);
  soap->version = 0;	/* don't assume we're parsing SOAP content by default */
  soap_free_iht(soap);
  if ((soap->imode & SOAP_IO) == SOAP_IO_CHUNK)
    soap->omode |= SOAP_IO_CHUNK;
  soap->imode &= ~SOAP_IO;
  soap->mode = soap->imode;
  if (!soap->keep_alive)
  { soap->buflen = 0;
    soap->bufidx = 0;
  }
  if (!(soap->mode & SOAP_IO_KEEPALIVE))
    soap->keep_alive = 0;
  soap->ahead = 0;
  soap->peeked = 0;
  soap->level = 0;
  soap->part = SOAP_BEGIN;
  soap->alloced = 0;
  soap->count = 0;
  soap->length = 0;
  soap->cdata = 0;
  *soap->endpoint = '\0';
  soap->userid = NULL;
  soap->passwd = NULL;
  soap->action = NULL;
  soap->authrealm = NULL;
  soap->dime.chunksize = 0;
  soap->dime.buflen = 0;
  soap->dime.list = NULL;
  soap->dime.first = NULL;
  soap->dime.last = NULL;
  soap->mime.list = NULL;
  soap->mime.first = NULL;
  soap->mime.last = NULL;
  soap->mime.boundary = NULL;
  soap->mime.start = NULL;
#ifdef WIN32
#ifndef UNDER_CE
#ifndef WITH_FASTCGI
  if (!soap_valid_socket(soap->socket))
#ifdef __BORLANDC__
    setmode((SOAP_SOCKET)soap->recvfd, O_BINARY);
#else
    _setmode((SOAP_SOCKET)soap->recvfd, _O_BINARY);
#endif
#endif
#endif
#endif
#ifdef WITH_ZLIB
  soap->mode &= ~SOAP_ENC_ZLIB;
  soap->zlib_in = SOAP_ZLIB_NONE;
  soap->zlib_out = SOAP_ZLIB_NONE;
  soap->d_stream.next_in = Z_NULL;
  soap->d_stream.avail_in = 0;
  soap->d_stream.next_out = (Byte*)soap->buf;
  soap->d_stream.avail_out = SOAP_BUFLEN;
  soap->z_ratio_in = 1.0;
  if (soap->fprepareinit)
    soap->fprepareinit(soap);
#endif
  c = soap_getchar(soap);
#ifdef WITH_GZIP
  if (c == 0x1F)
  { if (soap_getgziphdr(soap))
      return soap->error;
    if (inflateInit2(&soap->d_stream, -MAX_WBITS) != Z_OK)
      return soap->error = SOAP_ZLIB_ERROR;
    soap->zlib_state = SOAP_ZLIB_INFLATE;
    soap->mode |= SOAP_ENC_ZLIB;
    soap->zlib_in = SOAP_ZLIB_GZIP;
    soap->z_crc = crc32(0L, NULL, 0);
    DBGLOG(TEST, SOAP_MESSAGE(fdebug, "gzip initialized\n"));
    memcpy(soap->z_buf, soap->buf, SOAP_BUFLEN);
    /* should not chunk over plain transport, so why bother to check? */
    /* if ((soap->mode & SOAP_IO) == SOAP_IO_CHUNK) */
    /*   soap->z_buflen = soap->bufidx; */
    /* else */
    soap->d_stream.next_in = (Byte*)(soap->z_buf + soap->bufidx);
    soap->d_stream.avail_in = soap->buflen - soap->bufidx;
    soap->z_buflen = soap->buflen;
    soap->buflen = soap->bufidx;
    c = soap_getchar(soap);
  }  
#endif
  if (c == '-' && soap_get0(soap) == '-')
    soap->mode |= SOAP_ENC_MIME;
  else if ((c & 0xFFFC) == (SOAP_DIME_VERSION | SOAP_DIME_MB) && (soap_get0(soap) & 0xFFF0) == 0x20)
    soap->mode |= SOAP_ENC_DIME;
  else
  { while (soap_blank(c))
      c = soap_getchar(soap);
  }
  if ((int)c == EOF)
    return soap->error = SOAP_EOF;
  soap_unget(soap, c);
  if (c != '<' && !(soap->mode & (SOAP_ENC_DIME | SOAP_ENC_ZLIB)))
  { soap->mode &= ~SOAP_IO;
    if ((soap->error = soap->fparse(soap)))
    { soap->keep_alive = 0; /* force close later */
      return soap->error;
    }
    if ((soap->mode & SOAP_IO) == SOAP_IO_CHUNK)
    { soap->chunkbuflen = soap->buflen;
      soap->buflen = soap->bufidx;
      soap->chunksize = 0;
    }
    else if (soap->fpreparerecv && soap->buflen != soap->bufidx)
      soap->fpreparerecv(soap, soap->buf + soap->bufidx, soap->buflen - soap->bufidx);
#ifdef WITH_ZLIB
    if (soap->zlib_in)
    { /* fparse should not use soap_unget to push back last char */
#ifdef WITH_GZIP
      c = soap_get1(soap);
      if (c == 0x1F)
      { if (soap_getgziphdr(soap))
          return soap->error;
        if (inflateInit2(&soap->d_stream, -MAX_WBITS) != Z_OK)
          return soap->error = SOAP_ZLIB_ERROR;
        soap->zlib_state = SOAP_ZLIB_INFLATE;
        soap->z_crc = crc32(0L, NULL, 0);
        DBGLOG(TEST, SOAP_MESSAGE(fdebug, "gzip initialized\n"));
      }
      else
      { soap_revget1(soap);
#else
      {
#endif
        if (inflateInit(&soap->d_stream) != Z_OK)
          return soap->error = SOAP_ZLIB_ERROR;
        soap->zlib_state = SOAP_ZLIB_INFLATE;
        DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Inflate initialized\n"));
      }
      soap->mode |= SOAP_ENC_ZLIB;
      memcpy(soap->z_buf, soap->buf, SOAP_BUFLEN);
      soap->d_stream.next_in = (Byte*)(soap->z_buf + soap->bufidx);
      soap->d_stream.avail_in = soap->buflen - soap->bufidx;
      soap->z_buflen = soap->buflen;
      soap->buflen = soap->bufidx;
    }
#endif
  }
#ifndef WITH_LEANER
  if (soap->mode & SOAP_ENC_MIME)
  { if (soap_getmimehdr(soap))
      return soap->error;
    if (soap_get_header_attribute(soap, soap->mime.first->type, "application/dime"))
      soap->mode |= SOAP_ENC_DIME;
  }
  if (soap->mode & SOAP_ENC_DIME)
  { if (soap_getdimehdr(soap))
      return soap->error;
    if (soap->dime.flags & SOAP_DIME_CF)
    { DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Chunked DIME SOAP message\n"));
      soap->dime.chunksize = soap->dime.size;
      if (soap->buflen - soap->bufidx >= soap->dime.chunksize)
      { soap->dime.buflen = soap->buflen;
        soap->buflen = soap->bufidx + soap->dime.chunksize;
      }
      else
        soap->dime.chunksize -= soap->buflen - soap->bufidx;
    }
    soap->count = soap->buflen - soap->bufidx;
  }
#endif
  return SOAP_OK;
}
#endif

/******************************************************************************/
#ifndef PALM_1
static int
http_parse(struct soap *soap)
{ char header[SOAP_HDRLEN], *s;
  unsigned short g = 0, k;
  DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Waiting for response...\n"));
  *soap->endpoint = '\0';
  soap->length = 0;
  do
  { if (soap_getline(soap, soap->msgbuf, sizeof(soap->msgbuf)))
      return soap->error;
    DBGLOG(TEST,SOAP_MESSAGE(fdebug, "HTTP status: %s\n", soap->msgbuf));
    for (;;)
    { if (soap_getline(soap, header, SOAP_HDRLEN))
        return soap->error;
      if (!*header)
        break;
      DBGLOG(TEST,SOAP_MESSAGE(fdebug, "HTTP header: %s\n", header));
      s = strchr(header, ':');
      if (s)
      { *s = '\0';
        do s++;
        while (*s && *s <= 32);
        if ((soap->error = soap->fparsehdr(soap, header, s)))
          return soap->error;
      }
    }
    if ((s = strchr(soap->msgbuf, ' ')))
      k = (unsigned short)soap_strtoul(s, NULL, 10);
    else
      k = 0;
  } while (k == 100);
  DBGLOG(TEST,SOAP_MESSAGE(fdebug, "Finished HTTP header parsing\n"));
  s = strstr(soap->msgbuf, "HTTP/");
  if (s && s[7] != '1')
  { if (soap->keep_alive == 1)
      soap->keep_alive = 0;
    if (k == 0 && (soap->omode & SOAP_IO) == SOAP_IO_CHUNK) /* k == 0 for HTTP request */
    { soap->imode |= SOAP_IO_CHUNK;
      soap->omode = (soap->omode & ~SOAP_IO) | SOAP_IO_STORE;
    }
  }
  if (soap->keep_alive < 0)
    soap->keep_alive = 1;
  DBGLOG(TEST,SOAP_MESSAGE(fdebug, "Keep alive connection = %d\n", soap->keep_alive));
  if (s && (((g = !strncmp(soap->msgbuf, "GET ", 4))) || !strncmp(soap->msgbuf, "POST ", 5)))
  { size_t m = strlen(soap->endpoint);
    size_t n = m + (s - soap->msgbuf) - 5 - (!g);
    if (n >= sizeof(soap->endpoint))
      n = sizeof(soap->endpoint) - 1;
    strncpy(soap->path, soap->msgbuf + 4 + (!g), n - m);
    soap->path[n - m] = '\0';
    strcat(soap->endpoint, soap->path);
    DBGLOG(TEST,SOAP_MESSAGE(fdebug, "Target endpoint='%s'\n", soap->endpoint));
    if (g)
    { soap->error = soap->fget(soap);
      if (soap->error == SOAP_OK)
        soap->error = SOAP_STOP; /* prevents further processing */
      return soap->error;
    }
    return SOAP_OK;
  }
  if (k == 0 || (k >= 200 && k <= 299) || k == 400 || k == 500)
    return SOAP_OK;
  return soap_set_receiver_error(soap, "HTTP error", soap->msgbuf, k);
}
#endif

/******************************************************************************/
#ifndef PALM_1
static int
http_parse_header(struct soap *soap, const char *key, const char *val)
{ if (!soap_tag_cmp(key, "Host"))
  { 
#ifdef WITH_OPENSSL
    if (soap->imode & SOAP_ENC_SSL)
      strcpy(soap->endpoint, "https://");
    else
#endif
      strcpy(soap->endpoint, "http://");
    strncat(soap->endpoint, val, sizeof(soap->endpoint) - 8);
    soap->endpoint[sizeof(soap->endpoint) - 1] = '\0';
  }
#ifndef WITH_LEANER
  else if (!soap_tag_cmp(key, "Content-Type"))
  { if (soap_get_header_attribute(soap, val, "application/dime"))
      soap->mode |= SOAP_ENC_DIME;
    else if (soap_get_header_attribute(soap, val, "multipart/related"))
    { soap->mime.boundary = soap_strdup(soap, soap_get_header_attribute(soap, val, "boundary"));
      soap->mime.start = soap_strdup(soap, soap_get_header_attribute(soap, val, "start"));
      soap->mode |= SOAP_ENC_MIME;
    }
  }
#endif
  else if (!soap_tag_cmp(key, "Content-Length"))
    soap->length = soap_strtoul(val, NULL, 10);
  else if (!soap_tag_cmp(key, "Content-Encoding"))
  { if (!soap_tag_cmp(val, "deflate"))
#ifdef WITH_ZLIB
      soap->zlib_in = SOAP_ZLIB_DEFLATE;
#else
      return SOAP_ZLIB_ERROR;
#endif
    else if (!soap_tag_cmp(val, "gzip"))
#ifdef WITH_GZIP
      soap->zlib_in = SOAP_ZLIB_GZIP;
#else
      return SOAP_ZLIB_ERROR;
#endif
  }
#ifdef WITH_ZLIB
  else if (!soap_tag_cmp(key, "Accept-Encoding"))
  {
#ifdef WITH_GZIP
    if (strchr(val, '*') || soap_get_header_attribute(soap, val, "gzip"))
      soap->zlib_out = SOAP_ZLIB_GZIP;
    else
#endif
    if (strchr(val, '*') || soap_get_header_attribute(soap, val, "deflate"))
      soap->zlib_out = SOAP_ZLIB_DEFLATE;
    else
      soap->zlib_out = SOAP_ZLIB_NONE;
  }
#endif
  else if (!soap_tag_cmp(key, "Transfer-Encoding"))
  { soap->mode &= ~SOAP_IO;
    if (!soap_tag_cmp(val, "chunked"))
      soap->mode |= SOAP_IO_CHUNK;
  }
  else if (!soap_tag_cmp(key, "Connection"))
  { if (!soap_tag_cmp(val, "keep-alive"))
      soap->keep_alive = -soap->keep_alive;
    else if (!soap_tag_cmp(val, "close"))
      soap->keep_alive = 0;
  }
#ifndef WITH_LEAN
  else if (!soap_tag_cmp(key, "Authorization"))
  { if (!soap_tag_cmp(val, "Basic *"))
    { size_t n;
      char *s;
      soap_base642s(soap, val + 6, soap->tmpbuf, sizeof(soap->tmpbuf) - 1, &n);
      soap->tmpbuf[n] = '\0';
      if ((s = strchr(soap->tmpbuf, ':')))
      { *s = '\0';
	soap->userid = soap_strdup(soap, soap->tmpbuf);
	soap->passwd = soap_strdup(soap, s + 1);
      }
    }
  }
  else if (!soap_tag_cmp(key, "WWW-Authenticate"))
    soap->authrealm = soap_strdup(soap, soap_get_header_attribute(soap, val+6, "realm"));
  else if (!soap_tag_cmp(key, "Expect"))
  { if (!soap_tag_cmp(val, "100-continue"))
    { if ((soap->error = soap->fposthdr(soap, "HTTP/1.1 100 Continue", NULL))
       || (soap->error = soap->fposthdr(soap, NULL, NULL)))
        return soap->error;
    }
  }
#endif
  else if (!soap_tag_cmp(key, "SOAPAction"))
  { if (val[0] && val[1])
    { soap->action = soap_strdup(soap, val + 1);
      soap->action[strlen(soap->action) - 1] = '\0';
    }
  }
  else if (!soap_tag_cmp(key, "Location"))
  { strncpy(soap->endpoint, val, sizeof(soap->endpoint));
    soap->endpoint[sizeof(soap->endpoint) - 1] = '\0';
  }
#ifdef WITH_COOKIES
  else if (!soap_tag_cmp(key, "Cookie") || !soap_tag_cmp(key, "Set-Cookie"))
    soap_getcookies(soap, val);
#endif
  return SOAP_OK;
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
const char*
SOAP_FMAC2
soap_get_header_attribute(struct soap *soap, const char *line, const char *key)
{ register const char *s = line;
  if (s)
  { while (*s)
    { register short flag;
      s = soap_decode_key(soap->tmpbuf, sizeof(soap->tmpbuf), s);
      flag = soap_tag_cmp(soap->tmpbuf, key);
      s = soap_decode_val(soap->tmpbuf, sizeof(soap->tmpbuf), s);
      if (!flag)
        return soap->tmpbuf;
    }
  }
  return NULL;
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
const char*
SOAP_FMAC2
soap_decode_key(char *buf, size_t len, const char *val)
{ return soap_decode(buf, len, val, "=,;");
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
const char*
SOAP_FMAC2
soap_decode_val(char *buf, size_t len, const char *val)
{ if (*val != '=')
  { *buf = '\0';
    return val;
  }
  return soap_decode(buf, len, val + 1, ",;");
}
#endif

/******************************************************************************/
#ifndef PALM_1
static const char*
soap_decode(char *buf, size_t len, const char *val, const char *sep)
{ const char *s;
  char *t = buf;
  for (s = val; *s; s++)
    if (*s != ' ' && *s != '\t' && !strchr(sep, *s))
      break;
  if (*s == '"')
  { s++;
    while (*s && *s != '"' && --len)
      *t++ = *s++;
  }
  else
  { while (soap_notblank(*s) && !strchr(sep, *s) && --len)
    { if (*s == '%')
      { *t++ = ((s[1] >= 'A' ? (s[1] & 0x7) + 9 : s[1] - '0') << 4)
              + (s[2] >= 'A' ? (s[2] & 0x7) + 9 : s[2] - '0');
        s += 3;
      }
      else
        *t++ = *s++;
    }
  }
  *t = '\0';
  while (*s && !strchr(sep, *s))
    s++;
  return s;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_envelope_begin_out(struct soap *soap)
{
#ifndef WITH_LEANER
  size_t n = 0;
  if ((soap->mode & SOAP_ENC_MIME) && soap->mime.boundary && soap->mime.start)
  { const char *s;
    if (soap->mode & SOAP_ENC_DIME)
      s = "application/dime";
    else if (soap->version == 2)
      s = "application/soap+xml; charset=utf-8";
    else
      s = "text/xml; charset=utf-8";
    sprintf(soap->tmpbuf, "--%s\r\nContent-Type: %s\r\nContent-Transfer-Encoding: binary\r\nContent-ID: %s\r\n\r\n", soap->mime.boundary, s, soap->mime.start);
    n = strlen(soap->tmpbuf);
    if (soap_send_raw(soap, soap->tmpbuf, n))
      return soap->error;
  }
  if (soap->mode & SOAP_IO_LENGTH)
    soap->dime.size = soap->count;	/* DIME in MIME correction */
  if (!(soap->mode & SOAP_IO_LENGTH) && (soap->mode & SOAP_ENC_DIME))
  { if (soap_putdimehdr(soap))
      return soap->error;
  }
#endif
  soap->part = SOAP_IN_ENVELOPE;
  return soap_element_begin_out(soap, "SOAP-ENV:Envelope", 0, NULL);
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_envelope_end_out(struct soap *soap)
{ if (soap_element_end_out(soap, "SOAP-ENV:Envelope"))
    return soap->error;
#ifndef WITH_LEANER
  if ((soap->mode & SOAP_IO_LENGTH) && (soap->mode & SOAP_ENC_DIME))
  { soap->dime.size = soap->count - soap->dime.size;	/* DIME in MIME correction */
    sprintf(soap->id, soap->dime_id_format, 0);
    soap->dime.id = soap->id;
    if (soap->local_namespaces)
    { if (soap->local_namespaces[0].out)
        soap->dime.type = (char*)soap->local_namespaces[0].out;
      else
        soap->dime.type = (char*)soap->local_namespaces[0].ns;
    }
    soap->dime.options = NULL;
    soap->dime.flags = SOAP_DIME_MB | SOAP_DIME_ABSURI;
    if (!soap->dime.first)
      soap->dime.flags |= SOAP_DIME_ME;
    soap->count += 12 + ((strlen(soap->dime.id)+3)&(~3)) + ((strlen(soap->dime.type)+3)&(~3));
  }
  if (soap->mode & SOAP_ENC_DIME)
    return soap_send_raw(soap, SOAP_STR_PADDING, -(long)soap->dime.size&3);
#endif
  soap->part = SOAP_END_ENVELOPE;
  return SOAP_OK;
} 
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
int
SOAP_FMAC2
soap_envelope_begin_in(struct soap *soap)
{ register struct Namespace *p;
  soap->part = SOAP_IN_ENVELOPE;
  if (soap_element_begin_in(soap, "SOAP-ENV:Envelope", 0))
    return soap->error = SOAP_VERSIONMISMATCH;
  p = soap->local_namespaces;
  if (p)
  { const char *ns = p[0].out;
    if (!ns)
      ns = p[0].ns;
    if (!strcmp(ns, soap_env1))
    { soap->version = 1; /* make sure we use SOAP 1.1 */
      if (p[1].out)
        SOAP_FREE(p[1].out);
      if ((p[1].out = (char*)SOAP_MALLOC(sizeof(soap_enc1))))
        strcpy(p[1].out, soap_enc1);
    }
    else if (!strcmp(ns, soap_env2))
    { soap->version = 2; /* make sure we use SOAP 1.2 */
      if (p[1].out)
        SOAP_FREE(p[1].out);
      if ((p[1].out = (char*)SOAP_MALLOC(sizeof(soap_enc2))))
        strcpy(p[1].out, soap_enc2);
    }
  }
  return SOAP_OK;
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
int
SOAP_FMAC2
soap_envelope_end_in(struct soap *soap)
{ if (soap_element_end_in(soap, "SOAP-ENV:Envelope"))
    return soap->error;
  soap->part = SOAP_END_ENVELOPE;
  return SOAP_OK;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_body_begin_out(struct soap *soap)
{ soap->part = SOAP_IN_BODY;
  if (soap->version == 1)
    soap->encoding = 1;
  if (soap_element(soap, "SOAP-ENV:Body", 0, NULL))
    return soap->error;
  if ((soap->mode & SOAP_XML_SEC) && soap_attribute(soap, "id", "_0"))
    return soap->error;
  return soap_element_start_end_out(soap, NULL);
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_body_end_out(struct soap *soap)
{ if (soap_element_end_out(soap, "SOAP-ENV:Body"))
    return soap->error;
  soap->part = SOAP_IN_BODY;
  return SOAP_OK;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_body_begin_in(struct soap *soap)
{ soap->part = SOAP_IN_BODY;
  return soap_element_begin_in(soap, "SOAP-ENV:Body", 0);
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_body_end_in(struct soap *soap)
{ if (soap_element_end_in(soap, "SOAP-ENV:Body"))
    return soap->error;
  soap->part = SOAP_END_BODY;
  return SOAP_OK;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_recv_header(struct soap *soap)
{ if (soap_getheader(soap) && soap->error == SOAP_TAG_MISMATCH)
    soap->error = SOAP_OK;
  return soap->error;
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
void
SOAP_FMAC2
soap_set_endpoint(struct soap *soap, const char *endpoint)
{ register const char *s;
  register size_t i, n;
  *soap->endpoint = '\0';
  *soap->host = '\0';
  *soap->path = '\0';
  soap->port = 80;
  if (!endpoint || !*endpoint)
    return;
#ifdef WITH_OPENSSL
  if (!strncmp(endpoint, "https:", 6))
    soap->port = 443;
#endif
  strncpy(soap->endpoint, endpoint, sizeof(soap->endpoint) - 1);
  s = strchr(endpoint, ':');
  if (s && s[1] == '/' && s[2] == '/')
    s += 3;
  else
    s = endpoint;
  n = strlen(s);
  if (n >= sizeof(soap->host))
    n = sizeof(soap->host) - 1;
/* WR[ */
#ifdef WITH_IPV6
  if ('[' == s[0])
  { s++;
    for (i = 0; i < n; i++)
    { soap->host[i] = s[i];
      if (']' == s[i])
      {
        s++;
        break; 
      }
    }
  }
  else
  { for (i = 0; i < n; i++)
    { soap->host[i] = s[i];
      if (s[i] == '/' || s[i] == ':')
        break; 
    }
  }
#else /* WITH_IPV6 */
/* ]WR */
  for (i = 0; i < n; i++)
  { soap->host[i] = s[i];
    if (s[i] == '/' || s[i] == ':')
      break; 
  }
/* WR[ */
#endif /* WITH_IPV6 */
/* ]WR */
  soap->host[i] = '\0';
  if (s[i] == ':')
  { soap->port = (int)atol(s + i + 1);
    for (i++; i < n; i++)
      if (s[i] == '/')
        break;
  }
  if (s[i])
  { strncpy(soap->path, s + i + 1, sizeof(soap->path));
    soap->path[sizeof(soap->path) - 1] = '\0';
  }
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
int
SOAP_FMAC2
soap_connect(struct soap *soap, const char *endpoint, const char *action)
{ return soap_connect_command(soap, SOAP_POST, endpoint, action);
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
int
SOAP_FMAC2
soap_connect_command(struct soap *soap, int http_command, const char *endpoint, const char *action)
{ char host[sizeof(soap->host)];
  int port;
  size_t count;
  soap->error = SOAP_OK;
  strcpy(host, soap->host); /* save to compare */
  port = soap->port; /* save to compare */
  soap_set_endpoint(soap, endpoint);
  if (action)
    soap->action = soap_strdup(soap, action);
  if (soap->fconnect)
    if ((soap->error = soap->fconnect(soap, endpoint, soap->host, soap->port)))
      return soap->error;
  if (*soap->host)
  { soap->status = http_command;
    if (!soap_valid_socket(soap->socket) || strcmp(soap->host, host) || soap->port != port)
    { soap->keep_alive = 0; /* force close */
      soap_closesock(soap);
      DBGLOG(TEST,SOAP_MESSAGE(fdebug, "Connect to host='%s' path='%s' port=%d\n", soap->host, soap->path, soap->port));
      soap->socket = soap->fopen(soap, endpoint, soap->host, soap->port);
      if (soap->error)
        return soap->error;
      soap->keep_alive = ((soap->omode & SOAP_IO_KEEPALIVE) != 0);
    }
    else if (!soap->keep_alive || !soap->fpoll || soap->fpoll(soap))
    { soap->keep_alive = 0; /* force close */
      soap_closesock(soap);
      DBGLOG(TEST,SOAP_MESSAGE(fdebug, "Reconnect to host='%s' path='%s' port=%d\n", soap->host, soap->path, soap->port));
      soap->socket = soap->fopen(soap, endpoint, soap->host, soap->port);
      if (soap->error)
        return soap->error;
    }
  }
  if (soap_begin_send(soap))
    return soap->error;
  count = soap_count_attachments(soap);
  if ((soap->mode & SOAP_IO) != SOAP_IO_STORE && !(soap->mode & SOAP_ENC_XML) && endpoint)
  { unsigned int k = soap->mode;
    soap->mode &= ~(SOAP_IO | SOAP_ENC_ZLIB);
    if ((k & SOAP_IO) != SOAP_IO_FLUSH)
      soap->mode |= SOAP_IO_BUFFER;
    if ((soap->error = soap->fpost(soap, endpoint, soap->host, soap->port, soap->path, action, count)))
      return soap->error;
    if ((k & SOAP_IO) == SOAP_IO_CHUNK)
    { if (soap_flush(soap))
        return soap->error;
    }
    soap->mode = k;
  }
  if (http_command != SOAP_POST)
    return soap_end_send(soap);
  return SOAP_OK;
}
#endif

/******************************************************************************/
#ifndef WITH_LEAN
SOAP_FMAC1
char*
SOAP_FMAC2
soap_s2base64(struct soap *soap, const unsigned char *s, char *t, size_t n)
{ register size_t i;
  register unsigned long m;
  register char *p;
  if (!t)
    t = (char*)soap_malloc(soap, (n + 2) / 3 * 4 + 1);
  if (!t)
  { soap->error = SOAP_EOM;
    return NULL;
  }
  p = t;
  t[0] = '\0';
  if (!s)
    return p;
  for (; n > 2; n -= 3, s += 3)
  { m = s[0];
    m = (m << 8) | s[1];
    m = (m << 8) | s[2];
    for (i = 4; i > 0; m >>= 6)
      t[--i] = soap_base64o[m & 0x3F];
    t += 4;
  }
  t[0] = '\0';
  if (n > 0)
  { m = 0;
    for (i = 0; i < n; i++)
      m = (m << 8) | *s++;
    for (; i < 3; i++)
      m <<= 8;
    for (i++; i > 0; m >>= 6)
      t[--i] = soap_base64o[m & 0x3F];
    for (i = 3; i > n; i--)
      t[i] = '=';
    t[4] = '\0';
  }
  return p;
}
#endif

/******************************************************************************/
#ifndef WITH_LEAN
SOAP_FMAC1
const char*
SOAP_FMAC2
soap_base642s(struct soap *soap, const char *s, char *t, size_t l, size_t *n)
{ register int i, j, c;
  register unsigned long m;
  char *p = t;
  if (n)
    *n = 0;
  for (;;)
  { for (i = 0; i < SOAP_BLKLEN; i++)
    { m = 0;
      j = 0;
      while (j < 4)
      { c = *s++;
        if (c == '=' || !c)
        { i *= 3;
          switch (j)
          { case 2:
              *t++ = (char)((m >> 4) & 0xFF);
              i++;
              break;
            case 3:
              *t++ = (char)((m >> 10) & 0xFF);
              *t++ = (char)((m >> 2) & 0xFF);
              i += 2;
          }
          if (n)
	    *n += i;
          return p;
        }
        c -= '+';
        if (c >= 0 && c <= 79)
        { m = (m << 6) + soap_base64i[c];
          j++;
        }
      }
      *t++ = (char)((m >> 16) & 0xFF);
      *t++ = (char)((m >> 8) & 0xFF);
      *t++ = (char)(m & 0xFF);
      if (l < 3)
      { if (n)
	  *n += i;
        return p;
      }
      l -= 3;
    }
    if (n)
      *n += 3 * SOAP_BLKLEN;
  }
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
int
SOAP_FMAC2
soap_puthttphdr(struct soap *soap, int status, size_t count)
{ register const char *s;
  register int err;
  if (status == SOAP_FILE)
    s = soap->http_content;
  else if (status == SOAP_HTML)
    s = "text/html; charset=utf-8";
#ifndef WITH_LEANER
  else if (soap->mode & SOAP_ENC_DIME)
    s = "application/dime";
#endif
  else if (soap->version == 2)
    s = "application/soap+xml; charset=utf-8";
  else
    s = "text/xml; charset=utf-8";
#ifndef WITH_LEANER
  if ((soap->mode & SOAP_ENC_MIME) && soap->mime.boundary && soap->mime.start && soap->status != SOAP_GET)
  { sprintf(soap->tmpbuf, "multipart/related; boundary=\"%s\"; type=%s; start=\"%s\"", soap->mime.boundary, s, soap->mime.start);
    s = soap->tmpbuf;
  }
#endif
  if ((err = soap->fposthdr(soap, "Content-Type", s)))
    return err;
#ifdef WITH_ZLIB
  if (soap->omode & SOAP_ENC_ZLIB)
  {
#ifdef WITH_GZIP
    err = soap->fposthdr(soap, "Content-Encoding", "gzip");
#else
    err = soap->fposthdr(soap, "Content-Encoding", "deflate");
#endif
    if (err)
      return err;
  }
#endif
  if ((soap->omode & SOAP_IO) == SOAP_IO_CHUNK)
    err = soap->fposthdr(soap, "Transfer-Encoding", "chunked");
  else if (count > 0)
  { sprintf(soap->tmpbuf, "%lu", (unsigned long)count);
    err = soap->fposthdr(soap, "Content-Length", soap->tmpbuf);
  }
  if (err)
    return err;
  return soap->fposthdr(soap, "Connection", soap->keep_alive ? "keep-alive" : "close");
}
#endif

/******************************************************************************/
#ifndef PALM_1
static int
http_get(struct soap *soap)
{ return SOAP_GET_METHOD;
}
#endif

/******************************************************************************/
#ifndef PALM_1
static int
http_post(struct soap *soap, const char *endpoint, const char *host, int port, const char *path, const char *action, size_t count)
{ register const char *s;
  register int err;
  if (soap->status == SOAP_GET)
  { s = "GET";
    count = 0;
  }
  else
    s = "POST";
#ifdef PALM
  if (!endpoint || (strncmp(endpoint, "http:", 5) && strncmp(endpoint, "https:", 6) && strncmp(endpoint, "httpg:", 6)) && strncmp(endpoint, "_beam:", 6) && strncmp(endpoint, "_local:", 7) && strncmp(endpoint, "_btobex:", 8))
#else
  if (!endpoint || (strncmp(endpoint, "http:", 5) && strncmp(endpoint, "https:", 6) && strncmp(endpoint, "httpg:", 6)))
#endif
    return SOAP_OK;
  if (soap->proxy_host && strncmp(endpoint, "https:", 6))
    sprintf(soap->tmpbuf, "%s %s HTTP/%s", s, endpoint, soap->http_version);
  else
    sprintf(soap->tmpbuf, "%s /%s HTTP/%s", s, path, soap->http_version);
  if ((err = soap->fposthdr(soap, soap->tmpbuf, NULL)))
    return err;
  if (port != 80)
    sprintf(soap->tmpbuf, "%s:%d", host, port);
  else
    strcpy(soap->tmpbuf, host); 
  if ((err = soap->fposthdr(soap, "Host", soap->tmpbuf))
   || (err = soap->fposthdr(soap, "User-Agent", "gSOAP/2.7"))
   || (err = soap_puthttphdr(soap, SOAP_OK, count)))
    return err;
#ifdef WITH_ZLIB
#ifdef WITH_GZIP
  if ((err = soap->fposthdr(soap, "Accept-Encoding", "gzip, deflate")))
#else
  if ((err = soap->fposthdr(soap, "Accept-Encoding", "deflate")))
#endif
    return err;
#endif
#ifndef WITH_LEAN
  if (soap->userid && soap->passwd && strlen(soap->userid) + strlen(soap->passwd) < 761)
  { sprintf(soap->tmpbuf + 262, "%s:%s", soap->userid, soap->passwd);
    strcpy(soap->tmpbuf, "Basic ");
    soap_s2base64(soap, (const unsigned char*)(soap->tmpbuf + 262), soap->tmpbuf + 6, strlen(soap->tmpbuf + 262));
    if ((err = soap->fposthdr(soap, "Authorization", soap->tmpbuf)))
      return err;
  }
  if (soap->proxy_userid && soap->proxy_passwd && strlen(soap->proxy_userid) + strlen(soap->proxy_passwd) < 761)
  { sprintf(soap->tmpbuf + 262, "%s:%s", soap->proxy_userid, soap->proxy_passwd);
    strcpy(soap->tmpbuf, "Basic ");
    soap_s2base64(soap, (const unsigned char*)(soap->tmpbuf + 262), soap->tmpbuf + 6, strlen(soap->tmpbuf + 262));
    if ((err = soap->fposthdr(soap, "Proxy-Authorization", soap->tmpbuf)))
      return err;
  }
#endif
#ifdef WITH_COOKIES
#ifdef WITH_OPENSSL
  if (soap_putcookies(soap, host, path, soap->ssl != NULL))
    return soap->error;
#else
  if (soap_putcookies(soap, host, path, 0))
    return soap->error;
#endif
#endif
  if (action && soap->version == 1)
  { sprintf(soap->tmpbuf, "\"%s\"", action);
    if ((err = soap->fposthdr(soap, "SOAPAction", soap->tmpbuf)))
      return err;
  }
  return soap->fposthdr(soap, NULL, NULL);
}
#endif

/******************************************************************************/
#ifndef PALM_1
static int
http_send_header(struct soap *soap, const char *s)
{ register const char *t;
  do
  { t = strchr(s, '\n'); /* disallow \n in HTTP headers */
    if (!t)
      t = s + strlen(s);
    if (soap_send_raw(soap, s, t - s))
      return soap->error;
    s = t + 1;
  } while (*t);
  return SOAP_OK;
}
#endif

/******************************************************************************/
#ifndef PALM_1
static int
http_post_header(struct soap *soap, const char *key, const char *val)
{ if (key)
  { if (http_send_header(soap, key))
      return soap->error;
    if (val && (soap_send_raw(soap, ": ", 2) || http_send_header(soap, val)))
      return soap->error;
  }
  return soap_send_raw(soap, "\r\n", 2);
}
#endif

/******************************************************************************/
#ifndef PALM_1
static int
http_response(struct soap *soap, int status, size_t count)
{ register int err;
/* WR[ */
#ifdef WMW_RPM_IO
  if (soap->rpmreqid)
    httpOutputEnable(soap->rpmreqid);
#endif  /* WMW_RPM_IO */
/* ]WR */
  if (!status || status == SOAP_HTML || status == SOAP_FILE)
  { DBGLOG(TEST, SOAP_MESSAGE(fdebug, "OK 200\n"));
/* WR[ */
#ifdef WMW_RPM_IO
    if (soap->rpmreqid || soap_valid_socket(soap->master) || soap_valid_socket(soap->socket)) /* RPM behaves as if standalone */
#else
/* ]WR */
    if (soap_valid_socket(soap->master) || soap_valid_socket(soap->socket)) /* standalone application */
/* WR[ */
#endif /* WMW_RPM_IO */
/* ]WR */
    { sprintf(soap->tmpbuf, "HTTP/%s 200 OK", soap->http_version);
      if ((err = soap->fposthdr(soap, soap->tmpbuf, NULL)))
        return err;
    }
    else if ((err = soap->fposthdr(soap, "Status", "200 OK")))
      return err;
  }
  else if (status > 200 && status < 600)
  { sprintf(soap->tmpbuf, "HTTP/%s %d %s", soap->http_version, status, http_error(soap, status));
    if ((err = soap->fposthdr(soap, soap->tmpbuf, NULL)))
      return err;
    if (status == 401)
    { sprintf(soap->tmpbuf, "Basic realm=\"%s\"", soap->authrealm ? soap->authrealm : "gSOAP Web Service");
      if ((err = soap->fposthdr(soap, "WWW-Authenticate", soap->tmpbuf)))
        return err;
    }
    else if ((status >= 301 && status <= 303) || status == 307)
    { if ((err = soap->fposthdr(soap, "Location", soap->endpoint)))
        return err;
    }
  }
  else
  { const char *s = *soap_faultcode(soap);
    if (soap->version == 2 && !strcmp(s, "SOAP-ENV:Sender"))
      s = "400 Bad Request";
    else
      s = "500 Internal Server Error";
    DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Error %s (status=%d)\n", s, status));
/* WR[ */
#ifdef WMW_RPM_IO
    if (soap->rpmreqid || soap_valid_socket(soap->master) || soap_valid_socket(soap->socket)) /* RPM behaves as if standalone */
#else
/* ]WR */
    if (soap_valid_socket(soap->master) || soap_valid_socket(soap->socket)) /* standalone application */
/* WR[ */
#endif /* WMW_RPM_IO */
/* ]WR */
    { sprintf(soap->tmpbuf, "HTTP/%s %s", soap->http_version, s);
      if ((err = soap->fposthdr(soap, soap->tmpbuf, NULL)))
        return err;
    }
    else if ((err = soap->fposthdr(soap, "Status", s)))
      return err;
  }
  if ((err = soap->fposthdr(soap, "Server", "gSOAP/2.7"))
   || (err = soap_puthttphdr(soap, status, count)))
    return err;
#ifdef WITH_COOKIES
  if (soap_putsetcookies(soap))
    return soap->error;
#endif
  return soap->fposthdr(soap, NULL, NULL);
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
int
SOAP_FMAC2
soap_response(struct soap *soap, int status)
{ register size_t count;
  if (!(soap->omode & (SOAP_ENC_XML | SOAP_IO_STORE /* this tests for chunking too */))
   && (status == SOAP_HTML || status == SOAP_FILE))
  { soap->omode &= ~SOAP_IO;
    soap->omode |= SOAP_IO_STORE;
  }
  soap->status = status;
  count = soap_count_attachments(soap);
  if (soap_begin_send(soap))
    return soap->error;
  if ((soap->mode & SOAP_IO) != SOAP_IO_STORE && !(soap->mode & SOAP_ENC_XML))
  { register int n = soap->mode;
    soap->mode &= ~(SOAP_IO | SOAP_ENC_ZLIB);
    if ((n & SOAP_IO) != SOAP_IO_FLUSH)
      soap->mode |= SOAP_IO_BUFFER;
    if ((soap->error = soap->fresponse(soap, status, count)))
      return soap->error;
    if ((n & SOAP_IO) == SOAP_IO_CHUNK)
    { if (soap_flush(soap))
        return soap->error;
    }
    soap->mode = n;
  }
  return SOAP_OK;
}
#endif

/******************************************************************************/
#ifndef WITH_LEAN
static const char*
soap_set_validation_fault(struct soap *soap, const char *s, const char *t)
{ sprintf(soap->msgbuf, "Validation constraint violation: %s%s in element <%s>", s, t?t:SOAP_STR_EOS, soap->tag);
  return soap->msgbuf;
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
void
SOAP_FMAC2
soap_set_fault(struct soap *soap)
{ const char **c = soap_faultcode(soap);
  const char **s = soap_faultstring(soap);
  if (!*c)
  { if (soap->version == 2)
      *c = "SOAP-ENV:Sender";
    else
      *c = "SOAP-ENV:Client";
  }
  if (*s)
    return;
  switch (soap->error)
  {
#ifndef WITH_LEAN
    case SOAP_CLI_FAULT:
      *s = "Client fault";
      break;
    case SOAP_SVR_FAULT:
      *s = "Server fault";
      break;
    case SOAP_TAG_MISMATCH:
      *s = soap_set_validation_fault(soap, "tag name or namespace mismatch", NULL);
      break;
    case SOAP_TAG_END:
      *s = soap_set_validation_fault(soap, "incorrect end tag", NULL);
      break;
    case SOAP_TYPE:
      *s = soap_set_validation_fault(soap, "data type mismatch ", soap->type);
      break;
    case SOAP_SYNTAX_ERROR:
      *s = "Well-formedness constraint violation";
      break;
    case SOAP_NO_TAG:
      *s = "No XML element tag found";
      break;
    case SOAP_MUSTUNDERSTAND:
      *c = "SOAP-ENV:MustUnderstand";
      sprintf(soap->msgbuf, "The data in element '%s' must be understood but cannot be handled", soap->tag);
      *s = soap->msgbuf;
      break;
    case SOAP_VERSIONMISMATCH:
      *c = "SOAP-ENV:VersionMismatch";
      *s = "SOAP version mismatch or invalid SOAP message";
      break;
    case SOAP_DATAENCODINGUNKNOWN:
      *c = "SOAP-ENV:DataEncodingUnknown";
      *s = "Unsupported SOAP data encoding";
      break;
    case SOAP_NAMESPACE:
      *s = soap_set_validation_fault(soap, "namespace mismatch", NULL);
      break;
    case SOAP_FATAL_ERROR:
      *s = "Fatal error";
      break;
    case SOAP_NO_METHOD:
      sprintf(soap->msgbuf, "Method '%s' not implemented: method name or namespace not recognized", soap->tag);
      *s = soap->msgbuf;
      break;
    case SOAP_GET_METHOD:
      *s = "HTTP GET method not implemented";
      break;
    case SOAP_EOM:
      *s = "Out of memory";
      break;
    case SOAP_IOB:
      *s = "Array index out of bounds";
      break;
    case SOAP_NULL:
      *s = soap_set_validation_fault(soap, "nil not allowed", NULL);
      break;
    case SOAP_MULTI_ID:
      *s = soap_set_validation_fault(soap, "multiple definitions of id ", soap->id);
      break;
    case SOAP_MISSING_ID:
      *s = soap_set_validation_fault(soap, "missing id for ref ", soap->id);
      break;
    case SOAP_HREF:
      *s = soap_set_validation_fault(soap, "incompatible object ref ", soap->id);
      break;
    case SOAP_FAULT:
      break;
    case SOAP_TCP_ERROR:
      *s = tcp_error(soap);
      break;
    case SOAP_HTTP_ERROR:
      *s = "HTTP error";
      break;
    case SOAP_SSL_ERROR:
      *s = "SSL error";
      break;
    case SOAP_PLUGIN_ERROR:
      *s = "Plugin registry error";
      break;
    case SOAP_DIME_MISMATCH:
      *s = "DIME version/transmission error";
      break;
    case SOAP_DIME_END:
      *s = "End of DIME error";
      break;
    case SOAP_DIME_ERROR:
      *s = "DIME format error";
      break;
    case SOAP_MIME_ERROR:
      *s = "MIME format error";
      break;
    case SOAP_ZLIB_ERROR:
#ifdef WITH_ZLIB
      sprintf(soap->msgbuf, "Zlib/gzip error: '%s'", soap->d_stream.msg?soap->d_stream.msg:"");
      *s = soap->msgbuf;
#else
      *s = "Zlib not installed for required message (de)compression";
#endif
      break;
    case SOAP_REQUIRED:
      *s = soap_set_validation_fault(soap, "missing required attribute", NULL);
      break;
    case SOAP_PROHIBITED:
      *s = soap_set_validation_fault(soap, "prohibited attribute present", NULL);
      break;
    case SOAP_OCCURS:
      *s = soap_set_validation_fault(soap, "a min/maxOccurs violation was detected", NULL);
      break;
    case SOAP_LENGTH:
      *s = soap_set_validation_fault(soap, "content length violation", NULL);
      break;
#endif
    case SOAP_EOF:
      sprintf(soap->msgbuf, "End of file or no input: '%s'", soap_strerror(soap));
      *s = soap->msgbuf;
      break;
    default:
      if (soap->error > 200 && soap->error < 600)
      { sprintf(soap->msgbuf, "HTTP Error: '%s'", http_error(soap, soap->error));
        *s = soap->msgbuf;
      }
      else
      { sprintf(soap->msgbuf, "Error code %d", soap->error);
        *s = soap->msgbuf;
      }
    }
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
int
SOAP_FMAC2
soap_send_fault(struct soap *soap)
{ register int status = soap->error;
  if (status == SOAP_STOP)
    return status;
  DBGLOG(TEST,SOAP_MESSAGE(fdebug, "Sending back fault struct for error code %d\n", soap->error));
  soap->keep_alive = 0; /* to terminate connection */
  soap_set_fault(soap);
  if ((status != SOAP_EOF || (!soap->recv_timeout && !soap->send_timeout)) && (!soap->fpoll || soap->fpoll(soap) == SOAP_OK))
  { soap->error = SOAP_OK;
    soap_serializeheader(soap);
    soap_serializefault(soap);
    soap_begin_count(soap);
    if (soap->mode & SOAP_IO_LENGTH)
    { soap_envelope_begin_out(soap);
      soap_putheader(soap);
      soap_body_begin_out(soap);
      soap_putfault(soap);
      soap_body_end_out(soap);
      soap_envelope_end_out(soap);
    }
    if (soap_response(soap, status)
     || soap_envelope_begin_out(soap)
     || soap_putheader(soap)
     || soap_body_begin_out(soap)
     || soap_putfault(soap)
     || soap_body_end_out(soap)
     || soap_envelope_end_out(soap))
      return soap_closesock(soap);
    soap_end_send(soap);
  }
  soap->error = status;
  return soap_closesock(soap);
}
#endif

/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
int
SOAP_FMAC2
soap_recv_fault(struct soap *soap)
{ register int status = soap->error;
  DBGLOG(TEST,SOAP_MESSAGE(fdebug, "Receiving SOAP Fault\n"));
  soap->error = SOAP_OK;
  if (soap_getfault(soap))
  { DBGLOG(TEST,SOAP_MESSAGE(fdebug, "Error: soap_get_soapfault() failed. Is this a SOAP message at all?\n"));
    *soap_faultcode(soap) = (soap->version == 2 ? "SOAP-ENV:Sender" : "SOAP-ENV:Client");
    soap->error = status;
    soap_set_fault(soap);
  }
  else
  { register const char *s = *soap_faultcode(soap);
    if (!soap_match_tag(soap, s, "SOAP-ENV:Server") || !soap_match_tag(soap, s, "SOAP-ENV:Receiver"))
      status = SOAP_SVR_FAULT; 
    else if (!soap_match_tag(soap, s, "SOAP-ENV:Client") || !soap_match_tag(soap, s, "SOAP-ENV:Sender"))
      status = SOAP_CLI_FAULT;
    else if (!soap_match_tag(soap, s, "SOAP-ENV:MustUnderstand"))
      status = SOAP_MUSTUNDERSTAND;
    else if (!soap_match_tag(soap, s, "SOAP-ENV:VersionMismatch"))
      status = SOAP_VERSIONMISMATCH;
    else
    { DBGLOG(TEST,SOAP_MESSAGE(fdebug, "Fault code %s\n", s));
      status = SOAP_FAULT;
    }
    if (soap_body_end_in(soap)
     || soap_envelope_end_in(soap)
     || soap_end_recv(soap))
      return soap_closesock(soap);
    soap->error = status;
  }
  return soap_closesock(soap);
}
#endif

/******************************************************************************/
#ifndef PALM_1
static const char*
soap_strerror(struct soap *soap)
{ int err = soap->errnum;
  if (!err)
    err = soap_errno;
  if (err)
  {
#ifndef UNDER_CE
    return strerror(err);
#else
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, 0, (LPTSTR)&soap->werrorstr, sizeof(soap->werrorstr), NULL);
    wcstombs(soap->errorstr, soap->werrorstr, sizeof(soap->errorstr));
    return soap->errorstr;
#endif
  }
  return "Operation interrupted or timed out";
}
#endif

/******************************************************************************/
#ifndef PALM_2
static int
soap_set_error(struct soap *soap, const char *faultcode, const char *faultstring, const char *faultdetail, int soaperror)
{ *soap_faultcode(soap) = faultcode;
  *soap_faultstring(soap) = faultstring;
  if (faultdetail && *faultdetail)
  { register const char **s = soap_faultdetail(soap);
    if (s)
      *s = faultdetail;
  }
  return soap->error = soaperror;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_set_sender_error(struct soap *soap, const char *faultstring, const char *faultdetail, int soaperror)
{ return soap_set_error(soap, soap->version == 2 ? "SOAP-ENV:Sender" : "SOAP-ENV:Client", faultstring, faultdetail, soaperror);
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_set_receiver_error(struct soap *soap, const char *faultstring, const char *faultdetail, int soaperror)
{ return soap_set_error(soap, soap->version == 2 ? "SOAP-ENV:Receiver" : "SOAP-ENV:Server", faultstring, faultdetail, soaperror);
}
#endif

/******************************************************************************/
#ifndef PALM_2
static int
soap_copy_fault(struct soap *soap, const char *faultcode, const char *faultstring, const char *faultdetail)
{ char *s = NULL, *t = NULL;
  if (faultstring)
    s = soap_strdup(soap, faultstring);
  if (faultdetail)
    t = soap_strdup(soap, faultdetail);
  return soap_set_error(soap, faultcode, s, t, SOAP_FAULT);
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_sender_fault(struct soap *soap, const char *faultstring, const char *faultdetail)
{ return soap_copy_fault(soap, soap->version == 2 ? "SOAP-ENV:Sender" : "SOAP-ENV:Client", faultstring, faultdetail);
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
int
SOAP_FMAC2
soap_receiver_fault(struct soap *soap, const char *faultstring, const char *faultdetail)
{ return soap_copy_fault(soap, soap->version == 2 ? "SOAP-ENV:Receiver" : "SOAP-ENV:Server", faultstring, faultdetail);
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
void
SOAP_FMAC2
soap_print_fault(struct soap *soap, FILE *fd)
{ if (soap->error)
  { const char **s;
    if (!*soap_faultcode(soap))
      soap_set_fault(soap);
    fprintf(fd, "SOAP FAULT: %s\n\"%s\"\n", *soap_faultcode(soap), *soap_faultstring(soap));
    s = soap_faultdetail(soap);
    if (s && *s)
      fprintf(fd, "Detail: %s\n", *s);
  }
}
#endif
 
/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
void
SOAP_FMAC2
soap_print_fault_location(struct soap *soap, FILE *fd)
{ 
#ifndef WITH_LEAN
  int c;
  if (soap->error && soap->buflen > 0)
  { if (soap->bufidx == 0)
      soap->bufidx = 1;
    c = soap->buf[soap->bufidx - 1];
    soap->buf[soap->bufidx - 1] = '\0';
    if (soap->buflen - soap->bufidx > 1024)
      soap->buf[soap->bufidx + 1024] = '\0';
    else
      soap->buf[soap->buflen - 1] = '\0';
    fprintf(fd, "%s%c\n** HERE **\n", soap->buf, c);
    if (soap->bufidx < soap->buflen)
      fprintf(fd, "%s\n", soap->buf + soap->bufidx);
  }
#endif
}
#endif
 
/******************************************************************************/
#ifndef PALM_1
SOAP_FMAC1
int
SOAP_FMAC2
soap_register_plugin_arg(struct soap *soap, int (*fcreate)(struct soap*, struct soap_plugin*, void*), void *arg)
{ register struct soap_plugin *p;
  register int r;
  if (!(p = (struct soap_plugin*)SOAP_MALLOC(sizeof(struct soap_plugin))))
    return soap->error = SOAP_EOM;
  p->id = NULL;
  p->data = NULL;
  p->fcopy = NULL;
  p->fdelete = NULL;
  r = fcreate(soap, p, arg);
  if (!r && p->fdelete)
  { p->next = soap->plugins;
    soap->plugins = p;
    DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Registered '%s' plugin\n", p->id));
    return SOAP_OK;
  }
  DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Could not register plugin '%s': plugin returned error %d (or fdelete callback not set)\n", p->id?p->id:"?", r));
  SOAP_FREE(p);
  return r;
}
#endif

/******************************************************************************/
#ifndef PALM_1
static void *
fplugin(struct soap *soap, const char *id)
{ register struct soap_plugin *p;
  for (p = soap->plugins; p; p = p->next)
    if (p->id == id || !strcmp(p->id, id))
      return p->data;
  return NULL;
}
#endif

/******************************************************************************/
#ifndef PALM_2
SOAP_FMAC1
void *
SOAP_FMAC2
soap_lookup_plugin(struct soap *soap, const char *id)
{ return soap->fplugin(soap, id);
}
#endif

/******************************************************************************/
#ifdef __cplusplus
}
#endif

