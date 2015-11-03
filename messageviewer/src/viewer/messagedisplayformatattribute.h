/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#ifndef MESSAGEDISPLAYFORMATATTRIBUTE_H
#define MESSAGEDISPLAYFORMATATTRIBUTE_H

#include <AkonadiCore/attribute.h>
#include "messageviewer_export.h"
#include "messageviewer/viewer.h"
namespace MessageViewer
{
class MessageDisplayFormatAttributePrivate;

class MESSAGEVIEWER_EXPORT MessageDisplayFormatAttribute : public Akonadi::Attribute
{
public:
    explicit MessageDisplayFormatAttribute();
    ~MessageDisplayFormatAttribute();

    MessageDisplayFormatAttribute *clone() const Q_DECL_OVERRIDE;
    QByteArray type() const Q_DECL_OVERRIDE;
    QByteArray serialized() const Q_DECL_OVERRIDE;
    void deserialize(const QByteArray &data) Q_DECL_OVERRIDE;

    void setMessageFormat(Viewer::DisplayFormatMessage format);
    Viewer::DisplayFormatMessage messageFormat() const;

    void setRemoteContent(bool remote);
    bool remoteContent() const;

    bool operator==(const MessageDisplayFormatAttribute &other) const;

private:
    friend class MessageDisplayFormatAttributePrivate;
    MessageDisplayFormatAttributePrivate *const d;
};
}

#endif // MESSAGEDISPLAYFORMATATTRIBUTE_H
