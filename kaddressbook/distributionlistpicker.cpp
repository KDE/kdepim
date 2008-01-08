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
#include <klocale.h>
#include <kmessagebox.h>
#include <KRandom>

#include <QGridLayout>
#include <QLabel>
#include <QList>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QRegExp>
#include <QRegExpValidator>

KPIM::DistributionListPickerDialog::DistributionListPickerDialog( KABC::AddressBook* book, QWidget* parent ) : KDialog( parent ), m_book( book )
{
    Q_ASSERT( m_book );
    setModal( true );
    setButtons( Ok | Cancel | User1 );
    enableButton( Ok, false );
    setButtonText( User1, i18n( "Add New Distribution List" ) );
    QWidget* main = new QWidget( this );
    QGridLayout* layout = new QGridLayout( main );
    layout->setSpacing( KDialog::spacingHint() );
    m_label = new QLabel( main );
    layout->addWidget( m_label, 0, 0 );
    m_listWidget = new QListWidget( main );
    layout->addWidget( m_listWidget, 1, 0 );
    connect( m_listWidget, SIGNAL( currentTextChanged( QString ) ),
             this, SLOT( entrySelected( QString ) ) );
    connect( this, SIGNAL( okClicked() ),
             this, SLOT( slotOk() ) );
    connect( this, SIGNAL( cancelClicked() ),
             this, SLOT( slotCancel() ) );
    connect( this, SIGNAL( user1Clicked() ),
             this, SLOT( slotUser1() ) );
                      
    setMainWidget( main );
    Q_FOREACH ( const KPIM::DistributionList i,
                KPIM::DistributionList::allDistributionLists( m_book ) )
    {
        m_listWidget->addItem( i.name() );
    }
}

void KPIM::DistributionListPickerDialog::entrySelected( const QString& name )
{
    enableButton( Ok, !name.isNull() );
}

void KPIM::DistributionListPickerDialog::setLabelText( const QString& text )
{
    m_label->setText( text );
}

void KPIM::DistributionListPickerDialog::slotUser1()
{
    QStringList listNames;
    Q_FOREACH( const KPIM::DistributionList i, 
               KPIM::DistributionList::allDistributionLists( m_book ) )
    {
        listNames += i.name();
    }

    bool validName = false;
    do
    {
        QRegExpValidator validator( QRegExp( "\\S+.*" ), 0 );
        const QString name = KInputDialog::getText( i18n( "Enter Name" ), i18n( "Enter a name for the new distribution list:" ), QString(), 0, this, &validator ).trimmed();
        validName = !listNames.contains( name );

        if ( validName )
        {
            KPIM::DistributionList list;
            list.setName( name );
            list.setUid( KRandom::randomString( 10 ) );
            m_book->insertAddressee( list );

            QListWidgetItem* const item = new QListWidgetItem( name );
            m_listWidget->addItem( item );
            item->setSelected( true );
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
    const QList<QListWidgetItem*> items = m_listWidget->selectedItems();
    Q_ASSERT( items.count() <= 1 );
    m_selectedDistributionList = items.isEmpty() ? QString() : items.first()->text();
    accept();
}

void KPIM::DistributionListPickerDialog::slotCancel()
{
    m_selectedDistributionList = QString();
    reject();
}

QString KPIM::DistributionListPickerDialog::selectedDistributionList() const
{
    return m_selectedDistributionList;
}

#include "distributionlistpicker.moc"
