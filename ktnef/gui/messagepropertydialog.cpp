/*
    messagepropertydialog.cpp

    Copyright (C) 2003 Michael Goffioul <goffioul@imec.be>

    This file is part of KTNEF, the KDE TNEF support library/program.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 */

#include "messagepropertydialog.h"
#include "attachpropertydialog.h"
#include "ktnef/ktnefmessage.h"

#include <klistview.h>
#include <klocale.h>

MessagePropertyDialog::MessagePropertyDialog( QWidget *parent, KTNEFMessage *msg )
	: KDialogBase( parent, "MessagePropertyDialog", true, i18n( "Message Properties" ),
			KDialogBase::Close|KDialogBase::User1, KDialogBase::Close, false,
			KStdGuiItem::save() )
{
	m_message = msg;

	m_listview = new KListView( this );
	m_listview->addColumn( i18n( "Name" ) );
	m_listview->addColumn( i18n( "Value" ) );
	m_listview->setAllColumnsShowFocus( true );
	setMainWidget( m_listview );

	formatPropertySet( m_message, m_listview );
}

void MessagePropertyDialog::slotUser1()
{
	saveProperty( m_listview, m_message, this );
}

#include "messagepropertydialog.moc"
