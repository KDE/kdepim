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
#include "importphonebookdlg.h"

#include <klocale.h>
#include <kabc/stdaddressbook.h>
#include <kabc/vcardformat.h>
#include <kdebug.h>
#include <qlabel.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <k3listview.h>
#include <q3buttongroup.h>
#include <kmessagebox.h>

#include <libkmobiletools/engine.h>

#define COL_MEMSLOT 2
#define COL_INTMEMSLOT 3
#define COL_ID 4

importPhonebookDlg::importPhonebookDlg(int availPBSlots, QWidget *parent, const char *name)
    : KDialog(parent)
{
    setCaption(i18n("Import Addressbook"));
    setButtons(Ok| Cancel);
    ui.setupUi(mainWidget());
    ui.abcLoad->setIcon(KIcon("key_enter"));
//     connect(ui.fileUrl, SIGNAL(textChanged(const QString&)), this, SLOT(slotUrlChanged(const QString& )));
    connect(ui.fileUrl, SIGNAL(urlSelected(const QString&)), this, SLOT(slotUrlChanged(const QString& )));
    connect(ui.fileUrl, SIGNAL(returnPressed(const QString&)), this, SLOT(slotUrlChanged(const QString& )));
    connect(ui.lv_abc, SIGNAL(selectionChanged() ), this, SLOT(enableButtons() ) );
    connect(ui.abcLoad, SIGNAL(clicked() ), this, SLOT(slotLoadABC() ) );
    connect(ui.toDataCard, SIGNAL(clicked() ), this, SLOT(slotToDataCard() ) );
    connect(ui.toPhone, SIGNAL(clicked() ), this, SLOT(slotToPhone() ) );
    connect(ui.toSim, SIGNAL(clicked() ), this, SLOT(slotToSim() ) );
    connect(ui.dontimport, SIGNAL(clicked() ), this, SLOT(slotDontImport() ) );
    connect(ui.importFrom, SIGNAL(clicked( int ) ), this, SLOT(slotImportFromChanged(int ) ) );
    connect(this, SIGNAL(okClicked()), SLOT(slotOk()));
    enableButtonOk(false);
    enableButtons();
    i_availPBSlots=availPBSlots;
    resize(550,450);
    slotImportFromChanged( ui.importFrom->selectedId () );
}


importPhonebookDlg::~importPhonebookDlg()
{
}


#include "importphonebookdlg.moc"


/*!
    \fn importPhonebookDlg::slotUrlChanged(const QString&)
 */
void importPhonebookDlg::slotUrlChanged(const QString& text)
{
    /** @TODO abc loading to be fixed
    KABC::AddressBook *res_abc=new KABC::AddressBook();
    KABC::Resource *resfile=new KABC::Resource();
    QFile abcfile(text);
    KABC::VCardFormat vcard;
    if(!abcfile.exists() || ! vcard.loadAll(res_abc, resfile, abcfile) ) return;
    loadAddressBook(res_abc); */
    /*
    res_abc->addResource (resfile);
    if( ! res_abc->load() || !res_abc->allAddressees().count() >0 )
    {
        p_addresseeList=KABC::AddresseeList();
        ui.formatLabel->setText( i18n("File format not recognized or file not found.") );
        enableButtonOk(false);
        return;
    }
    loadAddressBook(res_abc);*/
}

void importPhonebookDlg::slotDontImport()
{
    Q3ListViewItemIterator it( ui.lv_abc, Q3ListViewItemIterator::Selected );
    while( it.current() ){
        setListViewItemSlot( -1, it.current() );
        ++it;
    }
}

/*!
    \fn importPhonebookDlg::slotOk()
 */
void importPhonebookDlg::slotOk()
{
    if(p_addresseeList.isEmpty()) return;
    if( ui.clearPhoneBook->isChecked() )
        if(KMessageBox::warningYesNo(this, i18n("This will erase your mobile phonebook.\nAre you sure you want to continue?") ) == KMessageBox::No ) return;
    /// TODO make some more checks to avoid dummy user actions
}


/*!
    \fn importPhonebookDlg::slotLoadABC()
 */
void importPhonebookDlg::slotLoadABC()
{
    slotUrlChanged( ui.fileUrl->url().url() );
}


/*!
    \fn importPhonebookDlg::phoneNumbers(QValueList<KABC::PhoneNumber> list)
 */
QStringList importPhonebookDlg::phoneNumbers(const KABC::PhoneNumber::List &list)
{
    QStringList retval;
    KABC::PhoneNumber::List::ConstIterator it;
    for ( it = list.begin(); it != list.end(); ++it )
            retval.append( (*it).number() );
    return retval;
}


/*!
    \fn importPhonebookDlg::enableButtons()
 */
void importPhonebookDlg::enableButtons()
{
    Q3ListViewItemIterator it( ui.lv_abc, Q3ListViewItemIterator::Selected );
    if(it.current() )
    {
        if( i_availPBSlots & KMobileTools::Engine::PB_SIM) ui.toSim->setEnabled(true);
        if( i_availPBSlots & KMobileTools::Engine::PB_Phone) ui.toPhone->setEnabled(true);
        if( i_availPBSlots & KMobileTools::Engine::PB_DataCard) ui.toDataCard->setEnabled(true);
    }
    else {
        ui.toSim->setEnabled(false);
        ui.toPhone->setEnabled(false);
        ui.toDataCard->setEnabled(false);
    }
}


/*!
    \fn importPhonebookDlg::slotToDataCard()
 */
void importPhonebookDlg::slotToDataCard()
{
    Q3ListViewItemIterator it( ui.lv_abc, Q3ListViewItemIterator::Selected );
    while( it.current() ){
        setListViewItemSlot( KMobileTools::Engine::PB_DataCard, it.current() );
        ++it;
    }
}


/*!
    \fn importPhonebookDlg::slotToPhone()
 */
void importPhonebookDlg::slotToPhone()
{
    Q3ListViewItemIterator it( ui.lv_abc, Q3ListViewItemIterator::Selected );
    while( it.current() ){
        setListViewItemSlot( KMobileTools::Engine::PB_Phone, it.current() );
        ++it;
    }
}


/*!
    \fn importPhonebookDlg::slotToSim()
 */
void importPhonebookDlg::slotToSim()
{
    Q3ListViewItemIterator it( ui.lv_abc, Q3ListViewItemIterator::Selected );
    while( it.current() ){
        setListViewItemSlot( KMobileTools::Engine::PB_SIM, it.current() );
        ++it;
    }
}

void importPhonebookDlg::setListViewItemSlot(int memslot, Q3ListViewItem *item)
{
    switch( memslot ){
        case -1:
            item->setText(COL_MEMSLOT, i18nc( "Do not import phonenumber", "Skip"));
            break;
        case KMobileTools::Engine::PB_SIM:
            item->setText(COL_MEMSLOT, i18nc( "Short SIM memory slot descriptor", "SIM"));
            break;
        case KMobileTools::Engine::PB_DataCard:
            item->setText(COL_MEMSLOT, i18nc( "Short Datacard memory slot descriptor", "DataCard" ));
            break;
//         case KMobileTools::Engine::PB_Phone:
        default: // Let's make PB_Phone the default choice if memslot isn't found ;-)
            item->setText(COL_MEMSLOT, i18nc( "Short Phone memory slot descriptor", "Phone" ));
            break;
    }
    item->setText(COL_INTMEMSLOT, QString::number(memslot));
    KABC::Addressee::List::Iterator it;
    for( it=p_addresseeList.begin(); it!=p_addresseeList.end(); ++it)
        if( (*it).uid() == item->text(COL_ID) ) (*it).insertCustom("KMobileTools", "memslot", QString::number(memslot) );
}



/*!
    \fn importPhonebookDlg::slotImportFromChanged()
 */
void importPhonebookDlg::slotImportFromChanged( int id )
{
    if(id)
    {
        ui.fileUrl->setEnabled(true);
        ui.abcLoad->setEnabled(true);
    } else
    {
        ui.fileUrl->setEnabled(false);
        ui.abcLoad->setEnabled(false);
        loadAddressBook(KABC::StdAddressBook::self());
    }
}

void importPhonebookDlg::loadAddressBook(KABC::AddressBook *addressBook)
{
    if(! addressBook->allAddressees().count() ) return;
    ui.lv_abc->clear();
    enableButtonOk(true);
    p_addresseeList=KABC::AddresseeList(addressBook->allAddressees() );
    KLocalizedString str = ki18n("%2 addressbook, with %1 contacts.");
    str = str.subs( p_addresseeList.count() );
    if(p_addresseeList.first().custom("KMobileTools","memslot").toInt() )
    {
        str = str.subs( "KMobileTools" );
        b_kmobiletoolsFormat=true;
    }
    else
    {
        str = str.subs( "KDE" ) ;
        b_kmobiletoolsFormat=false;
    }
    ui.formatLabel->setText( str.toString() );
    K3ListViewItem *item;
    KABC::Addressee::List::ConstIterator it;
    for ( it = p_addresseeList.begin(); it != p_addresseeList.end(); ++it )
    {
        item=new K3ListViewItem( ui.lv_abc, (*it).formattedName(), phoneNumbers( (*it).phoneNumbers() ).join(",") );
        setListViewItemSlot( (*it).custom("KMobileTools","memslot").toInt(), item );
        item->setText(COL_ID, (*it).uid() );
    }
}

