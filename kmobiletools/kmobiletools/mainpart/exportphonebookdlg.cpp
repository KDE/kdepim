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
#include "exportphonebookdlg.h"

#include <klocale.h>
#include <q3buttongroup.h>
#include <kfiledialog.h>
#include <kabc/vcardformat.h>
#include <kabc/stdaddressbook.h>

#define SAVE_TO_FILE 1
#define APPEND_TO_KABC 0

exportPhonebookDlg::exportPhonebookDlg(ContactsList *addresseeList, QWidget *parent, const char *name)
    : KDialog(parent)
{
    setCaption(i18n("Export Phonebook"));
    setButtons(Ok|Cancel);
    p_addresseeList=addresseeList;
    ui.setupUi(mainWidget());
    connect(this, SIGNAL(okClicked()), SLOT(slotOk()));
}


exportPhonebookDlg::~exportPhonebookDlg()
{
}


#include "exportphonebookdlg.moc"


/*!
    \fn exportPhonebookDlg::slotOk()
 */
void exportPhonebookDlg::slotOk()
{
    KABC::AddressBook *addressbook;
    if ( ui.exportOptions->selectedId() == SAVE_TO_FILE )
    {
        QString saveFileName=KFileDialog::getSaveFileName(KUrl("kfiledialog://vcf"), " text/directory", this);
        if(saveFileName.isNull()) return;
        QFile abcfile(saveFileName);
        KABC::VCardFormat vcard;
        addressbook=new KABC::AddressBook();
        for(KABC::Addressee::List::Iterator it=p_addresseeList->begin(); it != p_addresseeList->end(); ++it)
            vcard.save((*it), &abcfile);
        
/*        KABC::Resource* resource=new KABC::ResourceFile( saveFileName );
        resource->setReadOnly (false);
        addressbook=new KABC::AddressBook();
        addressbook->addResource(resource);
//         addressbook->load();
//         addressbook->clear();
        KABC::Ticket *saveTicket=addressbook->requestSaveTicket();

        addressbook->save( saveTicket );
//         addressbook->releaseSaveTicket( saveTicket );
        resource->close();*/
    }
    if ( ui.exportOptions->selectedId() == APPEND_TO_KABC )
    {
        addressbook=KABC::StdAddressBook::self();
        for(KABC::Addressee::List::Iterator it=p_addresseeList->begin(); it != p_addresseeList->end(); ++it)
            addressbook->insertAddressee( *it );
        addressbook->save(addressbook->requestSaveTicket() );
    }
}
