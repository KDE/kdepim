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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef GNOKIICONFIG_H
#define GNOKIICONFIG_H

#include <qobject.h>

#include <gnokiiconfigui.h>

class GnokiiConfig : public GnokiiConfigUI
{
Q_OBJECT
public:
    GnokiiConfig( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~GnokiiConfig();

    void setValues(const QString &model, const QString &connection, const QString &port, const QString &baud);
    void getValues(QString &model, QString &connection, QString &port, QString &baud) const;

private slots:
    void slotCheckValues();
    void slotCheckValues(const QString &);
};

#endif
