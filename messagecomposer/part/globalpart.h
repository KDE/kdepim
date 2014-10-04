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

#ifndef MESSAGECOMPOSER_GLOBALPART_H
#define MESSAGECOMPOSER_GLOBALPART_H

#include "messagepart.h"

#include <QtCore/QByteArray>
#include <QtCore/QList>

namespace MessageComposer
{

class MESSAGECOMPOSER_EXPORT GlobalPart : public MessageComposer::MessagePart
{
    Q_OBJECT

public:
    explicit GlobalPart(QObject *parent = 0);
    virtual ~GlobalPart();

    // default true
    bool isGuiEnabled() const;
    void setGuiEnabled(bool enabled);
    QWidget *parentWidgetForGui() const;
    void setParentWidgetForGui(QWidget *widget);

    bool isFallbackCharsetEnabled() const;
    void setFallbackCharsetEnabled(bool enabled);
    QList<QByteArray> charsets(bool forceFallback = false) const;
    void setCharsets(const QList<QByteArray> &charsets);

    bool is8BitAllowed() const;
    void set8BitAllowed(bool allowed);

    // default is false
    bool MDNRequested() const;
    void setMDNRequested(bool requestMDN);

private:
    class Private;
    Private *const d;
};

} // namespace MessageComposer

#endif
