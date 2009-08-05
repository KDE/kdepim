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

#include "attachmentpart.h"
#include "finalmessage.h"
#include "jobbase.h"
#include "messagecomposer_export.h"

#include <QtCore/QByteArray>
#include <QtCore/QStringList>

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

    FinalMessage::List messages() const;

    GlobalPart *globalPart();
    InfoPart *infoPart();
    TextPart *textPart();
    AttachmentPart::List attachmentParts();
    // takes ownership
    void addAttachmentPart( AttachmentPart *part );
    // sets parent to 0
    void removeAttachmentPart( AttachmentPart *part, bool del = true );

  public Q_SLOTS:
    virtual void start();

  private:
    Q_DECLARE_PRIVATE( Composer )

    Q_PRIVATE_SLOT( d_func(), void doStart() )
    Q_PRIVATE_SLOT( d_func(), void skeletonJobFinished(KJob*) )
    Q_PRIVATE_SLOT( d_func(), void beforeCryptoJobFinished(KJob*) )
};

}

#endif
