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

#ifndef MESSAGECOMPOSER_FINALMESSAGE_H
#define MESSAGECOMPOSER_FINALMESSAGE_H

#include "messagecomposer_export.h"

#include <QtCore/QList>
#include <QtCore/QStringList>

#include <kmime/kmime_message.h>
#include <boost/shared_ptr.hpp>

namespace MessageComposer {

class Composer;

/**
*/
class MESSAGECOMPOSER_EXPORT FinalMessage
{
  public:
    typedef QList<FinalMessage*> List;

    KMime::Message::Ptr message() const;
    int transportId() const;
    bool hasCustomHeaders() const;
    QString from() const;
    QStringList to() const;
    QStringList cc() const;
    QStringList bcc() const;

  private:
    // Only used by our friend the Composer.
    friend class Composer;
    explicit FinalMessage( KMime::Message *message = 0 );
    virtual ~FinalMessage();

    class Private;
    Private *const d;
};

}

#endif
