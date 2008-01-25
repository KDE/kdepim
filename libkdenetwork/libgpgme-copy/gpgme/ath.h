/* ath.h - Interfaces for thread-safeness library.
   Copyright (C) 2002, 2003, 2004 g10 Code GmbH

   This file is part of GPGME.
 
   GPGME is free software; you can redistribute it and/or modify it
   under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of
   the License, or (at your option) any later version.
   
   GPGME is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.  */

#ifndef ATH_H
#define ATH_H

#ifdef HAVE_W32_SYSTEM
  /* fixme: Check how we did it in libgcrypt.  */
  struct msghdr { int dummy; };
  typedef int socklen_t;
# include <windows.h>
# include <io.h>

#else /*!HAVE_W32_SYSTEM*/

# ifdef HAVE_SYS_SELECT_H
#  include <sys/select.h>
# else
#  include <sys/time.h>
# endif
# include <sys/types.h>
# include <sys/socket.h>

#endif  /*!HAVE_W32_SYSTEM*/



/* Define _ATH_EXT_SYM_PREFIX if you want to give all external symbols
   a prefix.  */
#define _ATH_EXT_SYM_PREFIX _gpgme_

#ifdef _ATH_EXT_SYM_PREFIX
#define _ATH_PREFIX1(x,y) x ## y
#define _ATH_PREFIX2(x,y) _ATH_PREFIX1(x,y)
#define _ATH_PREFIX(x) _ATH_PREFIX2(_ATH_EXT_SYM_PREFIX,x)
#define ath_mutex_init _ATH_PREFIX(ath_mutex_init)
#define ath_mutex_destroy _ATH_PREFIX(ath_mutex_destroy)
#define ath_mutex_lock _ATH_PREFIX(ath_mutex_lock)
#define ath_mutex_unlock _ATH_PREFIX(ath_mutex_unlock)
#define ath_read _ATH_PREFIX(ath_read)
#define ath_write _ATH_PREFIX(ath_write)
#define ath_select _ATH_PREFIX(ath_select)
#define ath_waitpid _ATH_PREFIX(ath_waitpid)
#define ath_connect _ATH_PREFIX(ath_connect)
#define ath_accept _ATH_PREFIX(ath_accept)
#define ath_sendmsg _ATH_PREFIX(ath_sendmsg)
#define ath_recvmsg _ATH_PREFIX(ath_recvmsg)
#endif


typedef void *ath_mutex_t;
#define ATH_MUTEX_INITIALIZER 0;

/* Functions for mutual exclusion.  */
int ath_mutex_init (ath_mutex_t *mutex);
int ath_mutex_destroy (ath_mutex_t *mutex);
int ath_mutex_lock (ath_mutex_t *mutex);
int ath_mutex_unlock (ath_mutex_t *mutex);

/* Replacement for the POSIX functions, which can be used to allow
   other (user-level) threads to run.  */
ssize_t ath_read (int fd, void *buf, size_t nbytes);
ssize_t ath_write (int fd, const void *buf, size_t nbytes);
ssize_t ath_select (int nfd, fd_set *rset, fd_set *wset, fd_set *eset,
		    struct timeval *timeout);
ssize_t ath_waitpid (pid_t pid, int *status, int options);
int ath_accept (int s, struct sockaddr *addr, socklen_t *length_ptr);
int ath_connect (int s, const struct sockaddr *addr, socklen_t length);
int ath_sendmsg (int s, const struct msghdr *msg, int flags);
int ath_recvmsg (int s, struct msghdr *msg, int flags);

#endif	/* ATH_H */
