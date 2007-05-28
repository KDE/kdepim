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
#include "editaddressee.h"

#include <libkmobiletools/engine.h>

#include <klocale.h>
#include <kpushbutton.h>
#include <q3buttongroup.h>
#include <knuminput.h>
#include <klineedit.h>
#include <qradiobutton.h>
#include <kcombobox.h>
#include <k3listview.h>
#include <q3ptrlist.h>
#include <kmessagebox.h>

editAddressee::editAddressee(int phoneManufacturer, int pbslots, int index, QWidget *parent, const char *name)
    : KDialog(parent)
{
    setCaption(i18n("Add New Contact"));
    setButtons(Ok | Cancel);
    ui.setupUi(mainWidget());
    setupWidgets(phoneManufacturer, pbslots, index);
}

editAddressee::editAddressee(const KABC::Addressee &_addressee, int phoneManufacturer, int pbslots, int index, QWidget *parent, const char *name)
    : KDialog(parent), addressee(_addressee)
{
    setCaption(i18n("Edit Contact"));
    setButtons(Ok | Cancel);
    ui.setupUi(mainWidget());
    setupWidgets(phoneManufacturer, pbslots, index);
    if(addressee.isEmpty()) done(QDialog::Rejected);

    int abMemSlot=addressee.custom("KMobileTools","memslot").toInt();
    for(int i=0; i<ui.cb_memslot->count(); i++ )
    {
        if(abMemSlot==KMobileTools::Engine::PB_SIM && ui.cb_memslot->itemText(i)==PB_SIM_TEXT) ui.cb_memslot->setCurrentIndex(i);
        if(abMemSlot==KMobileTools::Engine::PB_Phone && ui.cb_memslot->itemText(i)==PB_PHONE_TEXT) ui.cb_memslot->setCurrentIndex(i);
        if(abMemSlot==KMobileTools::Engine::PB_DataCard && ui.cb_memslot->itemText(i)==PB_DATACARD_TEXT) ui.cb_memslot->setCurrentIndex(i);
    }
    KABC::PhoneNumber::List::ConstIterator it;
    KABC::PhoneNumber::List list=addressee.phoneNumbers();
    for(it=list.begin(); it!=list.end(); ++it)
        new K3ListViewItem( ui.lv_numbers, (*it).number(), (*it).typeLabel() );
    ui.txt_cname->setText( addressee.formattedName() );
}


editAddressee::~editAddressee()
{
}

void editAddressee::setupWidgets( int phoneManufacturer,  int pbslots, int  )
{
    switch( phoneManufacturer ){
        case KMobileTools::Engine::Motorola:
            ui.cb_type->addItem(KABC::PhoneNumber::typeLabel(KABC::PhoneNumber::Cell)   );
            ui.cb_type->addItem(KABC::PhoneNumber::typeLabel(KABC::PhoneNumber::Pref)   );
            ui.cb_type->addItem(KABC::PhoneNumber::typeLabel(KABC::PhoneNumber::Work)   );
            ui.cb_type->addItem(KABC::PhoneNumber::typeLabel(KABC::PhoneNumber::Home)   );
            ui.cb_type->addItem(KABC::PhoneNumber::typeLabel(KABC::PhoneNumber::Fax)   );
            ui.cb_type->addItem(KABC::PhoneNumber::typeLabel(KABC::PhoneNumber::Pager)   );
            ui.cb_type->addItem(KABC::PhoneNumber::typeLabel(KABC::PhoneNumber::Msg)   );
            break;
        case KMobileTools::Engine::SonyEricsson:
            ui.cb_type->addItem(KABC::PhoneNumber::typeLabel(KABC::PhoneNumber::Cell)   );
            ui.cb_type->addItem(KABC::PhoneNumber::typeLabel(KABC::PhoneNumber::Pref)   );
            ui.cb_type->addItem(KABC::PhoneNumber::typeLabel(KABC::PhoneNumber::Work)   );
            ui.cb_type->addItem(KABC::PhoneNumber::typeLabel(KABC::PhoneNumber::Home)   );
            ui.cb_type->addItem(KABC::PhoneNumber::typeLabel(KABC::PhoneNumber::Fax)   );
            break;
        case KMobileTools::Engine::Nokia:
            ui.cb_type->addItem(KABC::PhoneNumber::typeLabel(KABC::PhoneNumber::Cell)   );
            ui.cb_type->addItem(KABC::PhoneNumber::typeLabel(KABC::PhoneNumber::Pref)   );
            ui.cb_type->addItem(KABC::PhoneNumber::typeLabel(KABC::PhoneNumber::Work)   );
            ui.cb_type->addItem(KABC::PhoneNumber::typeLabel(KABC::PhoneNumber::Home)   );
            ui.cb_type->addItem(KABC::PhoneNumber::typeLabel(KABC::PhoneNumber::Fax)   );
break;
        default:
            ui.cb_type->addItem(KABC::PhoneNumber::typeLabel(KABC::PhoneNumber::Pref)   );
    }
    connect(ui.b_add, SIGNAL(clicked()), this, SLOT(slotAddClicked() ) );
    connect(ui.b_del, SIGNAL(clicked()), this, SLOT(slotDelClicked() ) );
    connect(ui.txt_cname, SIGNAL(textChanged(const QString &)), this, SLOT(slotCheckIsOk() ) );
    slotCheckIsOk();
    if(pbslots & KMobileTools::Engine::PB_SIM) ui.cb_memslot->addItem( PB_SIM_TEXT );
    if(pbslots & KMobileTools::Engine::PB_Phone) ui.cb_memslot->addItem( PB_PHONE_TEXT );
    if(pbslots & KMobileTools::Engine::PB_DataCard) ui.cb_memslot->addItem( PB_DATACARD_TEXT );
    resize(500,400);
}


#include "editaddressee.moc"

void editAddressee::done(int r)
{
    Q3ListViewItemIterator it( ui.lv_numbers );
    addressee=KABC::Addressee();
    addressee.setNameFromString( ui.txt_cname->text() );
//     if(mainWidget->index_2->isChecked() ) addressee->insertCustom("KMobileTools","index", QString::number(mainWidget->i_index->value() ) );
//     else addressee->insertCustom("KMobileTools","index",QString("0") );
    while ( it.current() ) {
        KABC::PhoneNumber::TypeList typelist=KABC::PhoneNumber::typeList();
        KABC::PhoneNumber::TypeList::ConstIterator nit;
        for ( nit = typelist.begin(); nit != typelist.end(); ++nit )
            if(KABC::PhoneNumber::typeLabel(*nit) ==  it.current()->text(1) ) 
                addressee.insertPhoneNumber(KABC::PhoneNumber(it.current()->text(0), *nit) );
        ++it;
    }
    addressee.insertCustom("KMobileTools","memslot",QString::number(pbSlot() ) );
    KDialog::done(r);
}


/*!
    \fn editAddressee::slotDelClicked()
 */
void editAddressee::slotDelClicked()
{
    if ( ! (ui.lv_numbers->selectedItem () ) ) return;
    delete ui.lv_numbers->selectedItem ();
    slotCheckIsOk();
}


/*!
    \fn editAddressee::slotAddClicked()
 */
void editAddressee::slotAddClicked()
{
    new K3ListViewItem( ui.lv_numbers, ui.txt_number->text(), ui.cb_type->currentText() );
    slotCheckIsOk();
}

void editAddressee::slotCheckIsOk()
{
    if( (ui.lv_numbers->childCount() ) &&
         ui.txt_cname->text().length() )
        enableButton(Ok, true);
    else enableButton(Ok, false);
}



/*!
    \fn editAddressee::pbSlot()
 */
int editAddressee::pbSlot()
{
    if( ui.cb_memslot->currentText() == PB_SIM_TEXT ) return KMobileTools::Engine::PB_SIM;
    if( ui.cb_memslot->currentText() == PB_PHONE_TEXT) return KMobileTools::Engine::PB_Phone;
    if( ui.cb_memslot->currentText() == PB_DATACARD_TEXT ) return KMobileTools::Engine::PB_DataCard;
    return -1;
}
