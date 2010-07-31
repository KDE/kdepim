/*  -*- mode: C++; c-file-style: "gnu" -*-

    This file is part of Kontact.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

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

#include <tqlabel.h>
#include <tqlayout.h>

#include <dcopref.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kparts/part.h>

#include "core.h"
#include "summary.h"
#include "summarywidget.h"

#include <time.h>

SummaryWidget::SummaryWidget( Kontact::Plugin *plugin, TQWidget *parent, const char *name )
  : Kontact::Summary( parent, name ),
    DCOPObject( TQCString("MailSummary") ),
    mPlugin( plugin )
{
  TQVBoxLayout *mainLayout = new TQVBoxLayout( this, 3, 3 );

  TQPixmap icon = KGlobal::iconLoader()->loadIcon( "kontact_mail", KIcon::Desktop,
                                                  KIcon::SizeMedium );
  TQWidget *header = createHeader(this, icon, i18n("E-Mail"));
  mLayout = new TQGridLayout( 1, 3, 3 );

  mainLayout->addWidget(header);
  mainLayout->addLayout(mLayout);

  slotUnreadCountChanged();
  connectDCOPSignal( 0, 0, "unreadCountChanged()", "slotUnreadCountChanged()",
                     false );
}

void SummaryWidget::selectFolder( const TQString& folder )
{
  if ( mPlugin->isRunningStandalone() )
    mPlugin->bringToForeground();
  else
    mPlugin->core()->selectPlugin( mPlugin );
  TQByteArray data;
  TQDataStream arg( data, IO_WriteOnly );
  arg << folder;
  emitDCOPSignal( "kmailSelectFolder(TQString)", data );
}

void SummaryWidget::updateSummary( bool )
{
  // check whether we need to update the message counts
  DCOPRef kmail( "kmail", "KMailIface" );
  const int timeOfLastMessageCountChange =
    kmail.call( "timeOfLastMessageCountChange()" );
  if ( timeOfLastMessageCountChange > mTimeOfLastMessageCountUpdate )
    slotUnreadCountChanged();
}

void SummaryWidget::slotUnreadCountChanged()
{
  DCOPRef kmail( "kmail", "KMailIface" );
  DCOPReply reply = kmail.call( "folderList" );
  if ( reply.isValid() ) {
    TQStringList folderList = reply;
    updateFolderList( folderList );
  }
  else {
    kdDebug(5602) << "Calling kmail->KMailIface->folderList() via DCOP failed."
                  << endl;
  }
  mTimeOfLastMessageCountUpdate = ::time( 0 );
}

void SummaryWidget::updateFolderList( const TQStringList& folders )
{
  mLabels.setAutoDelete( true );
  mLabels.clear();
  mLabels.setAutoDelete( false );

  KConfig config( "kcmkmailsummaryrc" );
  config.setGroup( "General" );

  TQStringList activeFolders;
  if ( !config.hasKey( "ActiveFolders" ) )
    activeFolders << "/Local/inbox";
  else
    activeFolders = config.readListEntry( "ActiveFolders" );

  int counter = 0;
  TQStringList::ConstIterator it;
  DCOPRef kmail( "kmail", "KMailIface" );
  for ( it = folders.begin(); it != folders.end(); ++it ) {
    if ( activeFolders.contains( *it ) ) {
      DCOPRef folderRef = kmail.call( "getFolder(TQString)", *it );
      const int numMsg = folderRef.call( "messages()" );
      const int numUnreadMsg = folderRef.call( "unreadMessages()" );

      if ( numUnreadMsg == 0 ) continue;

      TQString folderPath;
      if ( config.readBoolEntry( "ShowFullPath", true ) )
        folderRef.call( "displayPath()" ).get( folderPath );
      else
        folderRef.call( "displayName()" ).get( folderPath );

      KURLLabel *urlLabel = new KURLLabel( *it, folderPath, this );
      urlLabel->installEventFilter( this );
      urlLabel->setAlignment( AlignLeft );
      urlLabel->show();
      connect( urlLabel, TQT_SIGNAL( leftClickedURL( const TQString& ) ),
               TQT_SLOT( selectFolder( const TQString& ) ) );
      mLayout->addWidget( urlLabel, counter, 0 );
      mLabels.append( urlLabel );

      TQLabel *label =
        new TQLabel( TQString( i18n("%1: number of unread messages "
                                  "%2: total number of messages", "%1 / %2") )
                    .arg( numUnreadMsg ).arg( numMsg ), this );
      label->setAlignment( AlignLeft );
      label->show();
      mLayout->addWidget( label, counter, 2 );
      mLabels.append( label );

      counter++;
    }
  }

  if ( counter == 0 ) {
    TQLabel *label = new TQLabel( i18n( "No unread messages in your monitored folders" ), this );
    label->setAlignment( AlignHCenter | AlignVCenter );
    mLayout->addMultiCellWidget( label, 0, 0, 0, 2 );
    label->show();
    mLabels.append( label );
  }
}

bool SummaryWidget::eventFilter( TQObject *obj, TQEvent* e )
{
  if ( obj->inherits( "KURLLabel" ) ) {
    KURLLabel* label = static_cast<KURLLabel*>( obj );
    if ( e->type() == TQEvent::Enter )
      emit message( i18n( "Open Folder: \"%1\"" ).arg( label->text() ) );
    if ( e->type() == TQEvent::Leave )
      emit message( TQString::null );
  }

  return Kontact::Summary::eventFilter( obj, e );
}

TQStringList SummaryWidget::configModules() const
{
  return TQStringList( "kcmkmailsummary.desktop" );
}

#include "summarywidget.moc"
