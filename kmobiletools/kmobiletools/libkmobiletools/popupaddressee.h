/***************************************************************************
   Copyright (C) 2007
   by Marco Gulino <marco@kmobiletools.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/
#ifndef POPUPADDRESSEE_H
#define POPUPADDRESSEE_H

#include <libkmobiletools/kmobiletools_export.h>

#include <kmenu.h>

#include <kabc/addressee.h>

/**
@author Marco Gulino
*/
class KMOBILETOOLS_EXPORT popupAddressee : public KMenu
{
Q_OBJECT
public:
    explicit popupAddressee(const QString &deviceName, const KABC::Addressee &_addressee, QWidget *parent = 0, bool ro=false);

    ~popupAddressee();

private:
    KABC::Addressee addressee;

private slots:
    void slotEdit();
    void slotDelete();
    signals:
        void editClicked(const KABC::Addressee&);
        void delContact();
};

#endif
