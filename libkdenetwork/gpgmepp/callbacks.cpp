/* callbacks.cpp - callback targets for internal use:
   Copyright (C) 2003,2004 Klarälvdalens Datakonsult AB

   This file is part of GPGME++.
 
   GPGME++ is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
 
   GPGME++ is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GPGME; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307 USA.
*/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "callbacks.h"

#include <gpgmepp/interfaces/progressprovider.h>
#include <gpgmepp/interfaces/passphraseprovider.h>
#include <gpgmepp/interfaces/dataprovider.h>
#include <gpgmepp/context.h> // for Error

#include <cassert>
#include <cerrno>
#include <unistd.h>

static inline gpg_error_t makeErrorFromErrno() {
  return gpg_err_make_from_errno( (gpg_err_source_t)22, errno );
}
static inline gpg_error_t makeError( gpg_err_code_t code ) {
  return gpg_err_make( (gpg_err_source_t)22, code );
}

using GpgME::ProgressProvider;
using GpgME::PassphraseProvider;
using GpgME::DataProvider;

void progress_callback( void * opaque, const char * what,
			int type, int current, int total ) {
  ProgressProvider * provider = static_cast<ProgressProvider*>( opaque );
  if ( provider )
    provider->showProgress( what, type, current, total );
}

static void wipe( char * buf, size_t len ) {
  for ( size_t i = 0 ; i < len ; ++i )
    buf[i] = '\0';
}

gpgme_error_t passphrase_callback( void * opaque, const char * uid_hint, const char * desc,
				   int prev_was_bad, int fd ) {
  PassphraseProvider * provider = static_cast<PassphraseProvider*>( opaque );
  bool canceled = false;
  gpgme_error_t err = GPG_ERR_NO_ERROR;
  char * passphrase = provider ? provider->getPassphrase( uid_hint, desc, prev_was_bad, canceled ) : 0 ;
  if ( canceled )
    err = makeError( GPG_ERR_CANCELED );
  else
    if ( passphrase && *passphrase ) {
      size_t passphrase_length = strlen( passphrase );
      size_t written = 0;
      do {
	ssize_t now_written = write( fd, passphrase + written, passphrase_length - written );
	if ( now_written < 0 ) {
	  err = makeErrorFromErrno();
	  break;
	}
	written += now_written;
      } while ( written < passphrase_length );
    }
  
  if ( passphrase && *passphrase )
    wipe( passphrase, strlen( passphrase ) );
  free( passphrase );
  write( fd, "\n", 1 );
  return err;
}



static ssize_t
data_read_callback( void * opaque, void * buf, size_t buflen ) {
  DataProvider * provider = static_cast<DataProvider*>( opaque );
  if ( !provider ) {
    errno = EINVAL;
    return -1;
  }
  return provider->read( buf, buflen );
}

static ssize_t
data_write_callback( void * opaque, const void * buf, size_t buflen ) {
  DataProvider * provider = static_cast<DataProvider*>( opaque );
  if ( !provider ) {
    errno = EINVAL;
    return -1;
  }
  return provider->write( buf, buflen );
}

static off_t
data_seek_callback( void * opaque, off_t offset, int whence ) {
  DataProvider * provider = static_cast<DataProvider*>( opaque );
  if ( !provider ) {
    errno = EINVAL;
    return -1;
  }
  if ( whence != SEEK_SET && whence != SEEK_CUR && whence != SEEK_END ) {
    errno = EINVAL;
    return -1;
  }
  return provider->seek( offset, whence );
}

static void data_release_callback( void * opaque ) {
  DataProvider * provider = static_cast<DataProvider*>( opaque );
  if ( provider )
    provider->release();
}

gpgme_data_cbs data_provider_callbacks = {
  &data_read_callback,
  &data_write_callback,
  &data_seek_callback,
  &data_release_callback
};

