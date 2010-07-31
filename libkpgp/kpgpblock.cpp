/*
    kpgpblock.cpp

    Copyright (C) 2001,2002 the KPGP authors
    See file AUTHORS.kpgp for details

    This file is part of KPGP, the KDE PGP/GnuPG support library.

    KPGP is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "kpgpblock.h"
#include "kpgp.h"

#include <string.h>

namespace Kpgp {

Block::Block( const TQCString& str )
  : mText(str), mProcessedText(), mError(),
    mSignatureUserId(), mSignatureKeyId(), mSignatureDate(),
    mRequiredKey(), mEncryptedFor(),
    mStatus(0), mHasBeenProcessed(false), mType(NoPgpBlock)
{
  mEncryptedFor.setAutoDelete( true );
}

Block::~Block()
{
}

void
Block::reset()
{
  mProcessedText = TQCString();
  mError = TQCString();
  mSignatureUserId = TQString::null;
  mSignatureKeyId = TQCString();
  mSignatureDate = TQCString();
  mRequiredKey = TQCString();
  mEncryptedFor.clear();
  mStatus = 0;
  mHasBeenProcessed = false;
}

void
Block::clear()
{
  reset();
  mText = TQCString();
  mType = NoPgpBlock;
}

BlockType
Block::determineType() const
{
  if( !strncmp( mText.data(), "-----BEGIN PGP ", 15 ) )
  {
    if( !strncmp( mText.data() + 15, "SIGNED", 6 ) )
      return ClearsignedBlock;
    else if( !strncmp( mText.data() + 15, "SIGNATURE", 9 ) )
      return SignatureBlock;
    else if( !strncmp( mText.data() + 15, "PUBLIC", 6 ) )
      return PublicKeyBlock;
    else if( !strncmp( mText.data() + 15, "PRIVATE", 7 ) ||
             !strncmp( mText.data() + 15, "SECRET", 6 ) )
      return PrivateKeyBlock;
    else if( !strncmp( mText.data() + 15, "MESSAGE", 7 ) )
    {
      if( !strncmp( mText.data() + 22, ", PART", 6 ) )
        return MultiPgpMessageBlock;
      else
        return PgpMessageBlock;
    }
    else if( !strncmp( mText.data() + 15, "ARMORED FILE", 12 ) )
      return PgpMessageBlock;
    else
      return UnknownBlock;
  }
  else
    return NoPgpBlock;
}

bool
Block::decrypt()
{
  Kpgp::Module *pgp = Kpgp::Module::getKpgp();

  if( pgp == 0 )
    return false;

  return pgp->decrypt( *this );
}

bool
Block::verify()
{
  Kpgp::Module *pgp = Kpgp::Module::getKpgp();

  if( pgp == 0 )
    return false;

  return pgp->verify( *this );
}

Kpgp::Result
Block::clearsign( const TQCString& keyId, const TQCString& charset )
{
  Kpgp::Module *pgp = Kpgp::Module::getKpgp();

  if( pgp == 0 )
    return Kpgp::Failure;

  return pgp->clearsign( *this, keyId, charset );
}

Kpgp::Result
Block::encrypt( const TQStringList& receivers, const TQCString& keyId,
                const bool sign, const TQCString& charset )
{
  Kpgp::Module *pgp = Kpgp::Module::getKpgp();

  if( pgp == 0 )
    return Kpgp::Failure;

  return pgp->encrypt( *this, receivers, keyId, sign, charset );
}

} // namespace Kpgp
