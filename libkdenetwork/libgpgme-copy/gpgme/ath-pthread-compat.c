/* ath-pthread.c - pthread module for self-adapting thread-safeness library
 *      Copyright (C) 2002 g10 Code GmbH
 *
 * This file is part of GPGME.
 *
 * GPGME is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GPGME is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include <stdlib.h>
#include <errno.h>
#include <pthread.h>

#include "ath.h"

/* Need to include pthread_create in our check, as the GNU C library
   has the pthread_mutex_* functions in their public interface.  */
#pragma weak pthread_create
#pragma weak pthread_mutex_init
#pragma weak pthread_mutex_destroy
#pragma weak pthread_mutex_lock
#pragma weak pthread_mutex_unlock

/* The lock we take while checking for lazy lock initialization.  */
static pthread_mutex_t check_init_lock = PTHREAD_MUTEX_INITIALIZER;

/* Initialize the mutex *PRIV.  If JUST_CHECK is true, only do this if
   it is not already initialized.  */
static int
mutex_pthread_init (void **priv, int just_check)
{
  int err = 0;

  if (just_check)
    pthread_mutex_lock (&check_init_lock);
  if (!*priv || !just_check)
    {
      pthread_mutex_t *lock = malloc (sizeof (pthread_mutex_t));
      if (!lock)
	err = ENOMEM;
      if (!err)
	{
	  err = pthread_mutex_init (lock, NULL);
	  if (err)
	    free (lock);
	  else
	    *priv = lock;
	}
    }
  if (just_check)
    pthread_mutex_unlock (&check_init_lock);
  return err;
}


static int
mutex_pthread_destroy (void *priv)
{
  int err = pthread_mutex_destroy ((pthread_mutex_t *) priv);
  free (priv);
  return err;
}


static struct ath_ops ath_pthread_ops =
  {
    mutex_pthread_init,
    mutex_pthread_destroy,
    (int (*) (void *)) pthread_mutex_lock,
    (int (*) (void *)) pthread_mutex_unlock,
    NULL,	/* read */
    NULL,	/* write */
    NULL,	/* select */
    NULL,	/* waitpid */
    NULL,	/* accept */
    NULL,	/* connect */
    NULL,	/* sendmsg */
    NULL	/* recvmsg */
  };


struct ath_ops *
ath_pthread_available (void)
{
  /* Need to include pthread_create in our check, as the GNU C library
     has the pthread_mutex_* functions in their public interface.  */
  if (pthread_create
      && pthread_mutex_init && pthread_mutex_destroy
      && pthread_mutex_lock && pthread_mutex_unlock)
    return &ath_pthread_ops;
  else
    return 0;
}
