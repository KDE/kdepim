/*
    messagepropertydialog.cpp

    Copyright (C) 2003 Michael Goffioul <kdeprint@swing.be>

    This file is part of KTNEF, the KDE TNEF support library/program.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "messagepropertydialog.h"
#include "attachpropertydialog.h"
#include <ktnef/ktnefmessage.h>

#include <k3listview.h>
#include <klocale.h>
#include <KStandardGuiItem>

using namespace KTnef;

MessagePropertyDialog::MessagePropertyDialog( QWidget *parent, KTNEFMessage *msg )
	: KDialog( parent)
{
  setCaption( i18nc( "@title:window", "Message Properties" ) );
  setButtons( KDialog::Close|KDialog::User1 );
  setDefaultButton( KDialog::Close );
  setButtonGuiItem( KDialog::User1,  KStandardGuiItem::save() );
  setModal( true );
	m_message = msg;

	m_listview = new K3ListView( this );
	m_listview->addColumn( i18nc( "@title:column message property name", "Name" ) );
	m_listview->addColumn( i18nc( "@title:column message property value", "Value" ) );
	m_listview->setAllColumnsShowFocus( true );
	setMainWidget( m_listview );

	formatPropertySet( m_message, m_listview );
	connect(this,SIGNAL(user1Clicked()),this,SLOT(slotUser1()));
}

void MessagePropertyDialog::slotUser1()
{
	saveProperty( m_listview, m_message, this );
}

#include "messagepropertydialog.moc"
