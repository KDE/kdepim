/*
    This file is part of KAddressBook.
    Copyright (c) 2007 Klaralvdalens Datakonsult AB <frank@kdab.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "distributionlistpicker.h"

#include <libkdepim/distributionlist.h>

#include <kabc/addressbook.h>

#include <kapplication.h>
#include <kinputdialog.h>
#include <klistbox.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <qpushbutton.h>
#include <qregexp.h>
#include <qvalidator.h>

KPIM::DistributionListPickerDialog::DistributionListPickerDialog( KABC::AddressBook* book, QWidget* parent ) : KDialogBase( parent ), m_book( book )
{
    Q_ASSERT( m_book );
    setModal( true );
    showButton( Ok, true );
    showButton( Cancel, true );
    showButton( User1, true );
    enableButton( Ok, false );
    setButtonText( User1, i18n( "Add New Distribution List" ) );
    m_listBox = new KListBox( this );
    connect( m_listBox, SIGNAL( highlighted( const QString& ) ),
             this, SLOT( entrySelected( const QString& ) ) ); 
    connect( m_listBox, SIGNAL( selected( const QString& ) ),
             this, SLOT( entrySelected( const QString& ) ) ); 

    setMainWidget( m_listBox );
}

void KPIM::DistributionListPickerDialog::entrySelected( const QString& name )
{
    actionButton( Ok )->setEnabled( !name.isNull() );
    m_selectedDistributionList = name;
}

void KPIM::DistributionListPickerDialog::slotUser1()
{
    const QValueList<KPIM::DistributionList> lists = KPIM::DistributionList::allDistributionLists( m_book );
    QStringList listNames;
    for ( QValueList<KPIM::DistributionList>::ConstIterator it = lists.begin(); it != lists.end(); ++it ) 
    {
        listNames += (*it).name();
    }

    bool validName = false;
    do
    {
        QRegExpValidator validator( QRegExp( ".+" ), 0 );
        const QString name = KInputDialog::getText( i18n( "Enter Name" ), i18n( "Enter a name for the new distribution list:" ), QString(), 0, this, 0, &validator ).stripWhiteSpace();
        if ( name.isEmpty() )
            return;

        validName = !listNames.contains( name );

        if ( validName )
        {
            KPIM::DistributionList list;
            list.setName( name );
            list.setUid( KApplication::randomString( 10 ) );
            m_book->insertAddressee( list );

            m_listBox->insertItem( name );
            QListBoxItem* item = m_listBox->findItem( name );
            m_listBox->setSelected( item, true );
        }
        else
        {
            KMessageBox::error( this, i18n( "A distribution list with the the name %1 already exists. Please choose another name" ).arg( name ), i18n( "Name Exists" ) );
        }
    }
    while ( !validName );
}


void KPIM::DistributionListPickerDialog::slotOk()
{
    KDialogBase::slotOk();
}

void KPIM::DistributionListPickerDialog::slotCancel()
{
    m_selectedDistributionList = QString();
    KDialogBase::slotCancel();
}

QString KPIM::DistributionListPickerDialog::selectedDistributionList() const
{
    return m_selectedDistributionList;
}

#include "distributionlistpicker.moc"

