/*
   This file is part of libkdepim.

   Copyright (C) 2005 Con Hennessy <cp.hennessy@iname.com>
                      Tobias Koenig <tokoe@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqtextedit.h>

#include <klocale.h>

#include "sendsmsdialog.h"

SendSMSDialog::SendSMSDialog( const TQString &recipientName, TQWidget *parent, const char *name )
  : KDialogBase( Plain, i18n( "Send SMS" ), Ok | Cancel, Ok, parent, name, true, true )
{
  TQWidget *page = plainPage();

  TQGridLayout *layout = new TQGridLayout( page, 3, 3, marginHint(), spacingHint() );

  layout->addWidget( new TQLabel( i18n( "Message" ), page ), 0, 0 );

  mMessageLength = new TQLabel( "0/160", page );
  mMessageLength->setAlignment( Qt::AlignRight );
  layout->addWidget( mMessageLength, 0, 2 );

  mText = new TQTextEdit( page );
  layout->addMultiCellWidget( mText, 1, 1, 0, 2 );

  layout->addWidget( new TQLabel( i18n( "Recipient:" ), page ), 2, 0 );
  layout->addWidget( new TQLabel( recipientName, page ), 2, 2 );

  setButtonText( Ok, i18n( "Send" ) );

  connect( mText, TQT_SIGNAL( textChanged() ),
           this, TQT_SLOT( updateMessageLength() ) );
  connect( mText, TQT_SIGNAL( textChanged() ),
           this, TQT_SLOT( updateButtons() ) );

  updateButtons();

  mText->setFocus();
}

TQString SendSMSDialog::text() const
{
  return mText->text();
}

void SendSMSDialog::updateMessageLength()
{
  int length = mText->length();

  if( length > 480 )
    mMessageLength->setText( TQString( "%1/%2 (%3)" ).arg( length ).arg( 500 ).arg( 4 ) );
  else if( length > 320 )
    mMessageLength->setText( TQString( "%1/%2 (%3)" ).arg( length ).arg( 480 ).arg( 3 ) );
  else if( length > 160 )
    mMessageLength->setText( TQString( "%1/%2 (%3)" ).arg( length ).arg( 320 ).arg( 2 ) );
  else
    mMessageLength->setText( TQString( "%1/%2" ).arg( length ).arg( 160 ) );
}

void SendSMSDialog::updateButtons()
{
  enableButton( Ok, mText->length() > 0 );
}

#include "sendsmsdialog.moc"
