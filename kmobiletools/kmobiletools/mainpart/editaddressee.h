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
#ifndef EDITADDRESSEE_H
#define EDITADDRESSEE_H

#include "ui_editaddressee_ui.h"

#include <kdialog.h>
#include <kabc/addressee.h>
/**
@author Marco Gulino
*/
class editAddressee : public KDialog
{
Q_OBJECT
public:
    explicit editAddressee(int phoneManufacturer,  int pbslots=0, int index=0, QWidget *parent = 0, const char *name = 0);
    explicit editAddressee(const KABC::Addressee &_addressee, int phoneManufacturer,  int pbslots=0, int index=0, QWidget *parent = 0, const char *name = 0);

    ~editAddressee();

    KABC::Addressee getAddressee() { return addressee; }
    int pbSlot();
private:
    Ui::editAddressee_ui ui;
    KABC::Addressee addressee;
    void setupWidgets( int phoneManufacturer,  int pbslots=0, int index=0 );

public slots:
    void slotAddClicked();
    void slotDelClicked();
    void slotCheckIsOk();
protected:
    void done(int r);
};

#endif
