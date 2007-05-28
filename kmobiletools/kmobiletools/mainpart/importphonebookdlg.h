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
#ifndef IMPORTPHONEBOOKDLG_H
#define IMPORTPHONEBOOKDLG_H

#include <kdialog.h>
#include <kurlrequester.h>
#include <kabc/addressee.h>
#include <kabc/addresseelist.h>
#include <kabc/addressbook.h>

#include <qcheckbox.h>


#include "ui_importphonebook.h"

/**
@author Marco Gulino
*/
class importPhonebookDlg : public KDialog
{
Q_OBJECT
public:
    explicit importPhonebookDlg(int availPBSlots, QWidget *parent = 0, const char *name = 0);

    ~importPhonebookDlg();

    QString fileUrl() { return ui.fileUrl->url().url(); }
    KABC::Addressee::List addresseeList() { return p_addresseeList; }
    int kmobiletoolsFormat() { return b_kmobiletoolsFormat; }
    QStringList phoneNumbers(const KABC::PhoneNumber::List &list);
    void loadAddressBook(KABC::AddressBook *addressBook);
    bool deletePhoneBook() {return ui.clearPhoneBook->isChecked(); }

private:
    Ui::importPhonebook ui;
    KABC::AddresseeList p_addresseeList;
    bool b_kmobiletoolsFormat;
    int i_availPBSlots;
    void setListViewItemSlot(int memslot, Q3ListViewItem *item);

public slots:
    void slotUrlChanged(const QString&);
    void slotOk();
    void slotLoadABC();
    void enableButtons();
    void slotToDataCard();
    void slotToPhone();
    void slotToSim();
    void slotDontImport();
    void slotImportFromChanged( int );
};

#endif
