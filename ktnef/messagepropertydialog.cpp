/*
  This file is part of KTnef.

  Copyright (C) 2003 Michael Goffioul <kdeprint@swing.be>
  Copyright (c) 2012 Allen Winter <winter@kde.org>

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

#include <KTNEF/KTNEFMessage>

#include <KLocale>
#include <KStandardGuiItem>

#include <QTreeWidget>

MessagePropertyDialog::MessagePropertyDialog( QWidget *parent, KTNEFMessage *msg )
  : KDialog( parent )
{
  mMessage = msg;

  setCaption(i18n( "Message Properties" ));
  mListView = new QTreeWidget( this );
  const QStringList headerLabels =
    ( QStringList( i18nc( "@title:column property name", "Name" ) )
        << i18nc( "@title:column property value", "Value" ) );
  mListView->setHeaderLabels( headerLabels );
  mListView->setAllColumnsShowFocus( true );
  mListView->setWordWrap( true );
  mListView->setAllColumnsShowFocus( true );
  mListView->setRootIsDecorated( false );
  setMainWidget( mListView );
  setButtonGuiItem(KDialog::User1,KStandardGuiItem::save());
  AttachPropertyDialog::formatPropertySet( mMessage, mListView );
}

void MessagePropertyDialog::slotUser1()
{
  AttachPropertyDialog::saveProperty( mListView, mMessage, this );
}

#include "messagepropertydialog.moc"
