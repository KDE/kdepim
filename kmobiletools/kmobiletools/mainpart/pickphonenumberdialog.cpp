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
#include "pickphonenumberdialog.h"
#include "engineslist.h"

#include <qlayout.h>
#include <kcombobox.h>
#include <klocale.h>
#include <kabc/stdaddressbook.h>
#include <k3listview.h>
#include <q3listview.h>
#include <kabc/addressee.h>
#include <kdebug.h>

#include "engine.h"
#include "enginedata.h"
#include "devicesconfig.h"
#include "contactslist.h"

using namespace KMobileTools;

class PickPhoneNumberDialogPrivate {
    public:
        PickPhoneNumberDialogPrivate()
    : addresseeList(NULL)
        {}
        Ui::PickPhoneNumber ui;
        KMobileTools::ContactsList *addresseeList;
        QStringList s_selectedNumbers;
};

PickPhoneNumberDialog::PickPhoneNumberDialog(QWidget *parent, const char *name)
    : KDialog(parent), d(new PickPhoneNumberDialogPrivate)
{
    setObjectName( QLatin1String( name ) );
    setCaption( i18n("Pick Phonenumber") );
    setButtons( Ok|Cancel );
    setDefaultButton( Ok );

    d->ui.setupUi(mainWidget());
    d->ui.searchLine->addTreeWidget(d->ui.lv_Phone_Numbers);
    d->ui.pbSource->addItem(i18n("KDE Addressbook"));
    d->ui.pbSource->addItems( KMobileTools::EnginesList::instance()->namesList( true ) );
    resize(450,600);
    connect(d->ui.pbSource, SIGNAL(activated(int)), this, SLOT(slotSourceChanged( int ) ));
    connect(d->ui.lv_Phone_Numbers, SIGNAL(doubleClicked( const QModelIndex &) ), this, SLOT(doubleClick( const QModelIndex &) ) );
    connect(d->ui.lv_Phone_Numbers, SIGNAL(clicked( const QModelIndex &) ), this, SLOT(click( const QModelIndex & ) ) );
    KMobileTools::EnginesList *enginelist=KMobileTools::EnginesList::instance();
    connect(enginelist, SIGNAL(phonebookUpdated()), this, SLOT(updatePhonebook()) );
    if(name)
    {
        d->ui.pbSource->setCurrentItem( DEVCFG(name)->devicename() );
        slotSourceChanged( d->ui.pbSource->currentIndex() );
    } else slotSourceChanged( 0 );
}


PickPhoneNumberDialog::~PickPhoneNumberDialog()
{
    delete d;
}


#include "pickphonenumberdialog.moc"


/*!
    \fn PickPhoneNumberDialog::slotSourceChanged(int)
 */
void PickPhoneNumberDialog::slotSourceChanged(int index)
{
    if(!index)
        d->addresseeList=new KMobileTools::ContactsList(KABC::StdAddressBook::self()->  allAddressees () );
    else
    {
        KMobileTools::Engine *engine= KMobileTools::EnginesList::instance()->find( d->ui.pbSource->itemText(index), true );
        if(engine) d->addresseeList=engine->constEngineData()->contactsList(); else d->addresseeList=0;
    }
    updateNumbersList();
}


void PickPhoneNumberDialog::updatePhonebook()
{
    kDebug() << "PickPhoneNumberDialog::updatePhonebook()\n";
    KMobileTools::Engine *engine= KMobileTools::EnginesList::instance()->find( d->ui.pbSource->currentText(), true );
    if(!engine)
    {
        d->addresseeList=0;
        return;
    }
    d->addresseeList=engine->constEngineData()->contactsList();
    updateNumbersList();
}
/*!
    \fn PickPhoneNumberDialog::updateNumbersList()
 */
void PickPhoneNumberDialog::updateNumbersList()
{
    /** @TODO reimplement with model/view
    ui.lv_Phone_Numbers->reset();
    if(!addresseeList) return;
    KABC::Addressee curItem;
    ContactsListIterator it(*addresseeList);
    QListViewItem *newItem;
    KABC::PhoneNumber::List numbersList;
    while ( it.hasNext() )
    {
        curItem=it.next();
        numbersList=curItem.phoneNumbers();
        if( numbersList.count()==0 ) continue;
        KABC::PhoneNumber::List::ConstIterator pit;
        newItem=new QListViewItem(ui.lv_Phone_Numbers, curItem.formattedName ());
        for(pit=numbersList.begin(); pit!=numbersList.end(); ++pit)
            new QListViewItem(newItem, (*pit).typeLabel(), (*pit).number(), curItem.formattedName () );
        newItem->setOpen(false);
        newItem->setSelectable(false);
    }
     * */
}


/*!
    \fn PickPhoneNumberDialog::doubleClick( QListViewItem *, const QPoint &, int )
 */
void PickPhoneNumberDialog::doubleClick( const QModelIndex & index )
{
    /** @TODO reimplement with model/view
    if(!item) return;
    if(item->childCount() )
    {
        if(item->isOpen() ) item->setOpen( false); else item->setOpen( true );
        return;
    }
    s_selectedNumbers.clear();
    s_selectedNumbers+= item->text( 1);
    done(Accepted);*/
}

const QStringList PickPhoneNumberDialog::selectedNumbers() {
    return d->s_selectedNumbers;
}


/*!
    \fn PickPhoneNumberDialog::click( QListViewItem *, const QPoint &, int )
 */
void PickPhoneNumberDialog::click( const QModelIndex & index )
{
    /** @TODO reimplement with model/view
    if(!item) return;
    if(item->childCount() )
    {
        if(item->isOpen() ) item->setOpen( false); else item->setOpen( true );
        return;
    }
    Q3ListViewItemIterator it( ui.lv_Phone_Numbers, Q3ListViewItemIterator::Selected );
    while(it.current() )
    {
        s_selectedNumbers+=(*it)->text(1);
        ++it;
    }*/
}
