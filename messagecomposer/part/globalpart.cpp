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

#include "globalpart.h"

using namespace MessageComposer;

class GlobalPart::Private
{
public:
    Private()
        : guiEnabled(true),
          parentWidgetForGui(0),
          fallbackCharsetEnabled(false),
          allow8Bit(false),
          MDNRequested(false)
    {
    }

    bool guiEnabled;
    QWidget *parentWidgetForGui;
    bool fallbackCharsetEnabled;
    QList<QByteArray> charsets;
    bool allow8Bit;
    bool MDNRequested;
};

GlobalPart::GlobalPart(QObject *parent)
    : MessagePart(parent)
    , d(new Private)
{
}

GlobalPart::~GlobalPart()
{
    delete d;
}

bool GlobalPart::isGuiEnabled() const
{
    return d->guiEnabled;
}

void GlobalPart::setGuiEnabled(bool enabled)
{
    d->guiEnabled = enabled;
}

QWidget *GlobalPart::parentWidgetForGui() const
{
    return d->parentWidgetForGui;
}

void GlobalPart::setParentWidgetForGui(QWidget *widget)
{
    d->parentWidgetForGui = widget;
}

bool GlobalPart::isFallbackCharsetEnabled() const
{
    return d->fallbackCharsetEnabled;
}

void GlobalPart::setFallbackCharsetEnabled(bool enabled)
{
    d->fallbackCharsetEnabled = enabled;
}

QList<QByteArray> GlobalPart::charsets(bool forceFallback) const
{
    QList<QByteArray> ret = d->charsets;
    if (d->fallbackCharsetEnabled || forceFallback) {
        ret << "us-ascii";
        ret << "utf-8";
    }
    return ret;
}

void GlobalPart::setCharsets(const QList<QByteArray> &charsets)
{
    d->charsets = charsets;
}

bool GlobalPart::is8BitAllowed() const
{
    return d->allow8Bit;
}

void GlobalPart::set8BitAllowed(bool allowed)
{
    d->allow8Bit = allowed;
}

bool GlobalPart::MDNRequested() const
{
    return d->MDNRequested;
}

void GlobalPart::setMDNRequested(bool requestMDN)
{
    d->MDNRequested = requestMDN;
}

