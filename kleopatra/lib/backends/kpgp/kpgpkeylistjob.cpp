/*
    kpgpkeylistjob.cpp

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004 Ingo Kloecker <kloecker@kde.org>

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    Libkleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "kpgpkeylistjob.h"

#include <kpgpbase.h>
#include <kpgpkey.h>

#include <gpgmepp/key.h>
#include <gpgmepp/keylistresult.h>

#include <gpgme.h>

#include <qtimer.h>
//Added by qt3to4:
#include <QByteArray>

#include <stdlib.h>
#include <string.h>
#include <assert.h>

Kleo::KpgpKeyListJob::KpgpKeyListJob( Kpgp::Base * pgpBase )
  : KeyListJob( 0, "Kleo::KpgpKeyListJob" ),
    mPgpBase( pgpBase )
{
}

Kleo::KpgpKeyListJob::~KpgpKeyListJob() {
}

// the following function is a verbatim copy from gpgme/key.c
static char *
set_user_id_part (char *tail, const char *buf, size_t len)
{
  while (len && (buf[len - 1] == ' ' || buf[len - 1] == '\t'))
    len--;
  for (; len; len--)
    *tail++ = *buf++;
  *tail++ = 0;
  return tail;
}

// the following function is a verbatim copy from gpgme/key.c
static void
parse_user_id (char *src, char **name, char **email,
	       char **comment, char *tail)
{
  const char *start = NULL;
  int in_name = 0;
  int in_email = 0;
  int in_comment = 0;

  while (*src)
    {
      if (in_email)
	{
	  if (*src == '<')
	    /* Not legal but anyway.  */
	    in_email++;
	  else if (*src == '>')
	    {
	      if (!--in_email && !*email)
		{
		  *email = tail;
		  tail = set_user_id_part (tail, start, src - start);
		}
	    }
	}
      else if (in_comment)
	{
	  if (*src == '(')
	    in_comment++;
	  else if (*src == ')')
	    {
	      if (!--in_comment && !*comment)
		{
		  *comment = tail;
		  tail = set_user_id_part (tail, start, src - start);
		}
	    }
	}
      else if (*src == '<')
	{
	  if (in_name)
	    {
	      if (!*name)
		{
		  *name = tail;
		  tail = set_user_id_part (tail, start, src - start);
		}
	      in_name = 0;
	    }
	  in_email = 1;
	  start = src + 1;
	}
      else if (*src == '(')
	{
	  if (in_name)
	    {
	      if (!*name)
		{
		  *name = tail;
		  tail = set_user_id_part (tail, start, src - start);
		}
	      in_name = 0;
	    }
	  in_comment = 1;
	  start = src + 1;
	}
      else if (!in_name && *src != ' ' && *src != '\t')
	{
	  in_name = 1;
	  start = src;
	}
      src++;
    }

  if (in_name)
    {
      if (!*name)
	{
	  *name = tail;
	  tail = set_user_id_part (tail, start, src - start);
	}
    }

  /* Let unused parts point to an EOS.  */
  tail--;
  if (!*name)
    *name = tail;
  if (!*email)
    *email = tail;
  if (!*comment)
    *comment = tail;
}

gpgme_user_id_t KpgpUserID2GPGMEUserID( const Kpgp::UserID * kUserId )
{
  // inspired by _gpgme_key_append_name

  const QByteArray text = kUserId->text().utf8();
  const int src_len = text.length();

  gpgme_user_id_t uid;
  /* Allocate enough memory for the _gpgme_user_id struct, for the actual user
     id (the text) and for the parsed version. */
  uid = (gpgme_user_id_t) malloc( sizeof( *uid ) + 2 * src_len + 3 );
  memset( uid, 0, sizeof *uid );
  uid->revoked = kUserId->revoked();
  uid->invalid = kUserId->invalid();
  uid->validity = (gpgme_validity_t) kUserId->validity();

  uid->uid = ((char *) uid) + sizeof (*uid);
  char *dst = uid->uid;
  memcpy( dst, text.data(), src_len + 1 );

  dst += src_len + 1;
  parse_user_id( uid->uid, &uid->name, &uid->email,
                 &uid->comment, dst );

  return uid;
}

gpgme_subkey_t KpgpSubkey2GPGMESubKey( const Kpgp::Subkey * kSubkey )
{
  gpgme_subkey_t subkey;

  const QByteArray fpr = kSubkey->fingerprint();
  const unsigned int fpr_len = fpr.length();
  const QByteArray keyId = kSubkey->longKeyID();

  subkey = (gpgme_subkey_t) calloc( 1, sizeof( *subkey ) + fpr_len + 1 );
  subkey->revoked = kSubkey->revoked();
  subkey->expired = kSubkey->expired();
  subkey->disabled = kSubkey->disabled();
  subkey->invalid = kSubkey->invalid();
  subkey->can_encrypt = kSubkey->canEncrypt();
  subkey->can_sign = kSubkey->canSign();
  subkey->can_certify = kSubkey->canCertify();
  subkey->secret = kSubkey->secret();
  subkey->pubkey_algo = (gpgme_pubkey_algo_t) kSubkey->keyAlgorithm();
  subkey->length = kSubkey->keyLength();
  subkey->keyid = subkey->_keyid;
  memcpy( subkey->_keyid, keyId.data(), keyId.length() + 1 );
  subkey->fpr = ((char *) subkey) + sizeof( *subkey );
  memcpy( subkey->fpr, fpr.data(), fpr_len + 1 );
  subkey->timestamp = kSubkey->creationDate();
  subkey->expires = kSubkey->expirationDate();

  return subkey;
}

gpgme_key_t KpgpKey2gpgme_key( const Kpgp::Key * kKey )
{
  gpgme_key_t key;

  key = (gpgme_key_t) calloc( 1, sizeof( *key ) );
  key->revoked = kKey->revoked();
  key->expired = kKey->expired();
  key->disabled = kKey->disabled();
  key->invalid = kKey->invalid();
  key->can_encrypt = kKey->canEncrypt();
  key->can_sign = kKey->canSign();
  key->can_certify = kKey->canCertify();
  key->secret = kKey->secret();
  key->protocol = GPGME_PROTOCOL_OpenPGP;
  key->owner_trust = GPGME_VALIDITY_UNKNOWN; // FIXME?

  Kpgp::UserIDList kUserIDs = kKey->userIDs();
  for ( Kpgp::UserIDListIterator it( kUserIDs ); it.current(); ++it ) {
    gpgme_user_id_t uid = KpgpUserID2GPGMEUserID( *it );
    if ( !key->uids )
      key->uids = uid;
    if ( key->_last_uid )
      key->_last_uid->next = uid;
    key->_last_uid = uid;
  }

  Kpgp::SubkeyList kSubkeys = kKey->subkeys();
  for ( Kpgp::SubkeyListIterator it( kSubkeys ); it.current(); ++it ) {
    gpgme_subkey_t subkey = KpgpSubkey2GPGMESubKey( *it );
    if (!key->subkeys)
      key->subkeys = subkey;
    if (key->_last_subkey)
      key->_last_subkey->next = subkey;
    key->_last_subkey = subkey;
  }

  return key;
}

GpgME::Error Kleo::KpgpKeyListJob::start( const QStringList & patterns,
                                          bool secretOnly ) {
  mPatterns = patterns;
  mSecretOnly = secretOnly;
  QTimer::singleShot( 0, this, SLOT( slotDoIt() ) );
  return GpgME::Error( 0 );
}

void Kleo::KpgpKeyListJob::slotDoIt() {
  std::vector<GpgME::Key> keys;
  GpgME::KeyListResult res = exec( mPatterns, mSecretOnly, keys );
  for ( std::vector<GpgME::Key>::const_iterator it = keys.begin();
        it != keys.end(); ++it )
    emit nextKey( *it );
  emit done();
  emit result( res );
  deleteLater();
}

GpgME::KeyListResult Kleo::KpgpKeyListJob::exec( const QStringList & patterns,
                                                 bool secretOnly,
                                                 std::vector<GpgME::Key> & keys ) {
  Kpgp::KeyList kKeys;
  if ( secretOnly )
    kKeys = mPgpBase->secretKeys( patterns );
  else
    kKeys = mPgpBase->publicKeys( patterns );

  keys.clear();
  for ( Kpgp::KeyListIterator it( kKeys ); it.current(); ++it ) {
    keys.push_back( GpgME::Key( KpgpKey2gpgme_key(*it), true ) );
  }

  _gpgme_op_keylist_result res;
  res.truncated = 0; // key list is not truncated
  res._unused = 0;

  return GpgME::KeyListResult( GpgME::Error( 0 ), res );
}

#include "kpgpkeylistjob.moc"
