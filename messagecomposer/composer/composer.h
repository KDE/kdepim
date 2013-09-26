/*
  Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

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

#ifndef MESSAGECOMPOSER_COMPOSER_H
#define MESSAGECOMPOSER_COMPOSER_H

#include "messagecomposer/job/jobbase.h"
#include "kleo/enum.h"

#include "messagecomposer_export.h"
#include <QtCore/QByteArray>
#include <QtCore/QStringList>

#include <kmime/kmime_message.h>

#include <messagecore/attachment/attachmentpart.h>

#include <vector>
#include <gpgme++/key.h>

namespace boost {
  template <typename T> class shared_ptr;
}

namespace MessageComposer {

class ComposerPrivate;
class GlobalPart;
class InfoPart;
class TextPart;

/**
  The message composer.
*/
class MESSAGECOMPOSER_EXPORT Composer : public JobBase
{
  Q_OBJECT

  public:
    explicit Composer( QObject *parent = 0 );
    virtual ~Composer();

    QList<KMime::Message::Ptr> resultMessages() const;

    GlobalPart *globalPart() const;
    InfoPart *infoPart() const;
    TextPart *textPart() const;
    MessageCore::AttachmentPart::List attachmentParts() const;
    void addAttachmentPart(MessageCore::AttachmentPart::Ptr part , bool autoresizeImage = false);
    void addAttachmentParts( const MessageCore::AttachmentPart::List &parts, bool autoresizeImage = false );
    void removeAttachmentPart( MessageCore::AttachmentPart::Ptr part );

    // if the message and attachments should not be encrypted regardless of settings
    void setNoCrypto( bool noCrypto );
    void setSignAndEncrypt( const bool doSign, const bool doEncrypt );
    void setMessageCryptoFormat( Kleo::CryptoMessageFormat format );
    void setSigningKeys( std::vector<GpgME::Key>& signers );
    void setEncryptionKeys(const QList<QPair<QStringList, std::vector<GpgME::Key> > > &data );

    /// Sets if this message being composed is an auto-saved message
    ///  if so, might need different handling, such as no crypto attachments.
    void setAutoSave( bool isAutoSave );
    bool autoSave() const;

    bool finished() const;

  public Q_SLOTS:
    virtual void start();

  protected Q_SLOTS:
    virtual void slotResult( KJob *job );

  private:
    Q_DECLARE_PRIVATE( Composer )

    Q_PRIVATE_SLOT( d_func(), void doStart() )
    Q_PRIVATE_SLOT( d_func(), void skeletonJobFinished(KJob*) )
    Q_PRIVATE_SLOT( d_func(), void contentJobFinished(KJob*) )
    Q_PRIVATE_SLOT( d_func(), void attachmentsFinished(KJob*) )
};

}

#endif
