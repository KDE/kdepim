/*  This file is part of the KDE mobile library.
    Copyright (C) 2004 Helge Deller <deller@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef GNOKIICONFIG_H
#define GNOKIICONFIG_H

#include <tqobject.h>

#include <gnokiiconfigui.h>

class GnokiiConfig : public GnokiiConfigUI
{
Q_OBJECT
public:
    GnokiiConfig( TQWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~GnokiiConfig();

    void setValues(const TQString &model, const TQString &connection, const TQString &port, const TQString &baud);
    void getValues(TQString &model, TQString &connection, TQString &port, TQString &baud) const;

private slots:
    void slotCheckValues();
    void slotCheckValues(const TQString &);
};

#endif
