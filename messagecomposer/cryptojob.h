/*
  Copyright (C) 2009 Klaralvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
  Copyright (c) 2009 Leo Franchi <lfranchi@kde.org>
  
  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
*/

#ifndef MESSAGECOMPOSER_CRYPTOJOB_H
#define MESSAGECOMPOSER_CRYPTOJOB_H

#include "contentjobbase.h"
#include "infopart.h"
#include "messagecomposer_export.h"
#include "kleo/enum.h"

#include <gpgme++/key.h>
#include <vector>

namespace KMime {
  class Content;

}

namespace GpgME {
  class SigningResult;
  class Error;
}


namespace Message {

class CryptoJobPrivate;

/**
  Signs and encrypts a message contents.
  Subjob should be set to main message contents job.
*/
class MESSAGECOMPOSER_EXPORT CryptoJob : public ContentJobBase
{
  Q_OBJECT

  public:
    CryptoJob( QObject *parent = 0 );
    virtual ~CryptoJob();

    void setInfoPart( InfoPart* part );
    void setContent( KMime::Content* content );
    void setCryptoMessageFormat( Kleo::CryptoMessageFormat format);
    void setSignEncrypt( bool sign, bool encrypt );
    void setEncryptionItems( QStringList recipients, std::vector<GpgME::Key> keys );
    void setSigningKeys( std::vector<GpgME::Key>& signers );

  protected Q_SLOTS:
    virtual void doStart();
    virtual void process();

  private:
    Q_DECLARE_PRIVATE( CryptoJob )
};

}

#endif
