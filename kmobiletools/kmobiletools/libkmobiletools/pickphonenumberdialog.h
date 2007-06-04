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
#ifndef PICKPHONENUMBERDIALOG_H
#define PICKPHONENUMBERDIALOG_H

#include <kdialog.h>
#include <kabc/addressee.h>

#include <libkmobiletools/kmobiletools_export.h>
#include <libkmobiletools/ui_ui_pickphonenumber.h>

#include <klistwidgetsearchline.h>

/**
	@author Marco Gulino <marco@kmobiletools.org>
*/

class PickPhoneNumberDialogPrivate;

class KMOBILETOOLS_EXPORT PickPhoneNumberDialog : public KDialog
{
Q_OBJECT
public:
    explicit PickPhoneNumberDialog(QWidget *parent = 0, const char *name = 0);

    ~PickPhoneNumberDialog();
    const QStringList selectedNumbers();
    private:
        PickPhoneNumberDialogPrivate *const d;


public Q_SLOTS:
    void slotSourceChanged(int);
    void updateNumbersList();
    void updatePhonebook();
    void click( const QModelIndex & index ) ;
    void doubleClick( const QModelIndex & index );
};

#endif
