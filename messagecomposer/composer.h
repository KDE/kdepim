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
#include "behaviour.h"
#include "finalmessage.h"
#include "job.h"
#include "messagecomposer_export.h"

#include <QtCore/QByteArray>
#include <QtCore/QStringList>

#include <KDE/KCompositeJob>

namespace MessageComposer {

class InfoPart;
class TextPart;

/**
  The message composer.
*/
class MESSAGECOMPOSER_EXPORT Composer : public KCompositeJob
{
  Q_OBJECT

  public:
    // TODO figure out how to share Job::Error (See PLAN).

    explicit Composer( QObject *parent = 0 );
    virtual ~Composer();

    Behaviour &behaviour();
    void setBehaviour( const Behaviour &beh );
    QWidget *parentWidget() const; // TODO make this part of Behaviour??
    void setParentWidget( QWidget *widget );
    FinalMessage::List messages() const;

    InfoPart *infoPart();
    //void setInfoPart( InfoPart *part, bool delOldPart = true );
    TextPart *textPart();
    //void setTextPart( TextPart *part, bool delOldPart = true );
    // TODO figure out how to make this extensible...
    // and how to handle d->composer in MessagePart...
    AttachmentPart::List attachmentParts();
    void addAttachmentPart( AttachmentPart *part );
    void removeAttachmentPart( AttachmentPart *part, bool del = true );

  public Q_SLOTS:
    virtual void start();

  private:
    class Private;
    friend class Private;
    Private *const d;

    Q_PRIVATE_SLOT( d, void doStart() )
    Q_PRIVATE_SLOT( d, void skeletonJobFinished(KJob*) )
    Q_PRIVATE_SLOT( d, void beforeCryptoJobFinished(KJob*) )
};

}

#endif
