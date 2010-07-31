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
#include "config.h"

#ifdef KDEPIM_NEW_DISTRLISTS
#include <libkdepim/distributionlist.h>
#endif

#include <kabc/addressbook.h>

#include <kapplication.h>
#include <kinputdialog.h>
#include <klistbox.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <tqlabel.h>
#include <tqlayout.h>
#include <tqpushbutton.h>
#include <tqregexp.h>
#include <tqvalidator.h>

KPIM::DistributionListPickerDialog::DistributionListPickerDialog( KABC::AddressBook* book, TQWidget* parent ) : KDialogBase( parent, 0, true, TQString(), Ok | Cancel | User1 ), m_book( book )
{
    Q_ASSERT( m_book );
    setModal( true );
    enableButton( Ok, false );
    setButtonText( User1, i18n( "Add New Distribution List" ) );
    TQWidget* main = new TQWidget( this );
    TQGridLayout* layout = new TQGridLayout( main );
    layout->setSpacing( KDialog::spacingHint() );
    m_label = new TQLabel( main );
    layout->addWidget( m_label, 0, 0 );
    m_listBox = new KListBox( main );
    layout->addWidget( m_listBox, 1, 0 );
    connect( m_listBox, TQT_SIGNAL( highlighted( const TQString& ) ),
             this, TQT_SLOT( entrySelected( const TQString& ) ) ); 
    connect( m_listBox, TQT_SIGNAL( selected( const TQString& ) ),
             this, TQT_SLOT( entrySelected( const TQString& ) ) ); 
    setMainWidget( main );
#ifdef KDEPIM_NEW_DISTRLISTS
    typedef TQValueList<KPIM::DistributionList> DistListList;
    const DistListList lists = KPIM::DistributionList::allDistributionLists( m_book );
    for ( DistListList::ConstIterator it = lists.begin(); it != lists.end(); ++it )
    {
        m_listBox->insertItem ( (*it).name() );
    }
#endif
}

void KPIM::DistributionListPickerDialog::entrySelected( const TQString& name )
{
    actionButton( Ok )->setEnabled( !name.isNull() );
}

void KPIM::DistributionListPickerDialog::setLabelText( const TQString& text )
{
    m_label->setText( text );
}

void KPIM::DistributionListPickerDialog::slotUser1()
{
#ifdef KDEPIM_NEW_DISTRLISTS
    const TQValueList<KPIM::DistributionList> lists = KPIM::DistributionList::allDistributionLists( m_book );
    TQStringList listNames;
    for ( TQValueList<KPIM::DistributionList>::ConstIterator it = lists.begin(); it != lists.end(); ++it ) 
    {
        listNames += (*it).name();
    }

    bool validName = false;
    do
    {
        TQRegExpValidator validator( TQRegExp( "\\S+.*" ), 0 );
        const TQString name = KInputDialog::getText( i18n( "Enter Name" ), i18n( "Enter a name for the new distribution list:" ), TQString(), 0, this, 0, &validator ).stripWhiteSpace();
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
            TQListBoxItem* item = m_listBox->findItem( name );
            m_listBox->setSelected( item, true );
        }
        else
        {
            KMessageBox::error( this, i18n( "A distribution list with the the name %1 already exists. Please choose another name" ).arg( name ), i18n( "Name Exists" ) );
        }
    }
    while ( !validName );
#endif
}


void KPIM::DistributionListPickerDialog::slotOk()
{
    TQListBoxItem* item = m_listBox->selectedItem();
    m_selectedDistributionList = item ? item->text() : TQString();
    KDialogBase::slotOk();
}

void KPIM::DistributionListPickerDialog::slotCancel()
{
    m_selectedDistributionList = TQString();
    KDialogBase::slotCancel();
}

TQString KPIM::DistributionListPickerDialog::selectedDistributionList() const
{
    return m_selectedDistributionList;
}

#include "distributionlistpicker.moc"

