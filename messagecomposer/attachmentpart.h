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

#ifndef MESSAGECOMPOSER_ATTACHMENTPART_H
#define MESSAGECOMPOSER_ATTACHMENTPART_H

#include "messagepart.h"

#include <QtCore/QList>

class KUrl;

namespace MessageComposer {

/** setOverrideTransferEncoding for an AttachmentPart means setting the CTE for the sub-Content
  representing this attachment */
class MESSAGECOMPOSER_EXPORT AttachmentPart : public MessagePart
{
  Q_OBJECT

  public:
    typedef QList<AttachmentPart*> List;

    explicit AttachmentPart( QObject *parent = 0 );
    virtual ~AttachmentPart();

    KUrl url() const;
    void setUrl( const KUrl &url );
    
    bool isDataLoaded() const;
    bool loadData();

    // TODO handle mime type; charset for textual types, etc.

  private:
    class Private;
    Private *const d;
};

} // namespace MessageComposer

#endif // MESSAGECOMPOSER_INFOPART_H
