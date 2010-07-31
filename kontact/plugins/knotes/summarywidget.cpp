/*
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

#include <tqobject.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqtooltip.h>

#include <dcopclient.h>
#include <dcopref.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kurllabel.h>
#include <kstandarddirs.h>

#include <knotes/resourcenotes.h>
#include <knotes/resourcemanager.h>

#include "core.h"
#include "plugin.h"

#include "summarywidget.h"

KNotesSummaryWidget::KNotesSummaryWidget( Kontact::Plugin *plugin,
                                          TQWidget *parent, const char *name )
  : Kontact::Summary( parent, name ), mLayout( 0 ), mPlugin( plugin )
{
  TQVBoxLayout *mainLayout = new TQVBoxLayout( this, 3, 3 );

  TQPixmap icon = KGlobal::iconLoader()->loadIcon( "kontact_notes",
                   KIcon::Desktop, KIcon::SizeMedium );
  TQWidget* header = createHeader( this, icon, i18n( "Notes" ) );
  mainLayout->addWidget( header );

  mLayout = new TQGridLayout( mainLayout, 7, 3, 3 );
  mLayout->setRowStretch( 6, 1 );

  mCalendar = new KCal::CalendarLocal( TQString::fromLatin1("UTC") );
  KNotesResourceManager *manager = new KNotesResourceManager();

  TQObject::connect( manager, TQT_SIGNAL( sigRegisteredNote( KCal::Journal* ) ),
                    this, TQT_SLOT( addNote( KCal::Journal* ) ) );
  TQObject::connect( manager, TQT_SIGNAL( sigDeregisteredNote( KCal::Journal* ) ),
                    this, TQT_SLOT( removeNote( KCal::Journal* ) ) );
  manager->load();


  updateView();
}

void KNotesSummaryWidget::updateView()
{
  mNotes = mCalendar->journals();
  TQLabel *label;

  for ( label = mLabels.first(); label; label = mLabels.next() )
    label->deleteLater();
  mLabels.clear();

  KIconLoader loader( "knotes" );

  int counter = 0;
  TQPixmap pm = loader.loadIcon( "knotes", KIcon::Small );

  KCal::Journal::List::Iterator it;
  if ( mNotes.count() ) {
    for (it = mNotes.begin(); it != mNotes.end(); ++it) {

      // Fill Note Pixmap Field
      label = new TQLabel( this );
      label->setPixmap( pm );
      label->setMaximumWidth( label->minimumSizeHint().width() );
      label->setAlignment( AlignVCenter );
      mLayout->addWidget( label, counter, 0 );
      mLabels.append( label );

      // File Note Summary Field
      TQString newtext = (*it)->summary();

      KURLLabel *urlLabel = new KURLLabel( (*it)->uid(), newtext, this );
      urlLabel->installEventFilter( this );
      urlLabel->setTextFormat(RichText);
      urlLabel->setAlignment( urlLabel->alignment() | Qt::WordBreak );
      mLayout->addWidget( urlLabel, counter, 1 );
      mLabels.append( urlLabel );

      if ( !(*it)->description().isEmpty() ) {
        TQToolTip::add( urlLabel, (*it)->description().left( 80 ) );
      }

      connect( urlLabel, TQT_SIGNAL( leftClickedURL( const TQString& ) ),
               this, TQT_SLOT( urlClicked( const TQString& ) ) );
      counter++;
    }

  } else {
      TQLabel *noNotes = new TQLabel( i18n( "No Notes Available" ), this );
      noNotes->setAlignment( AlignHCenter | AlignVCenter );
      mLayout->addWidget( noNotes, 0, 1 );
      mLabels.append( noNotes );
  }

  for ( label = mLabels.first(); label; label = mLabels.next() )
    label->show();
}

void KNotesSummaryWidget::urlClicked( const TQString &/*uid*/ )
{
  if ( !mPlugin->isRunningStandalone() )
    mPlugin->core()->selectPlugin( mPlugin );
  else
    mPlugin->bringToForeground();
}

bool KNotesSummaryWidget::eventFilter( TQObject *obj, TQEvent* e )
{
  if ( obj->inherits( "KURLLabel" ) ) {
    KURLLabel* label = static_cast<KURLLabel*>( obj );
    if ( e->type() == TQEvent::Enter )
      emit message( i18n( "Read Note: \"%1\"" ).arg( label->text() ) );
    if ( e->type() == TQEvent::Leave )
      emit message( TQString::null );
  }

  return Kontact::Summary::eventFilter( obj, e );
}

void KNotesSummaryWidget::addNote( KCal::Journal *j )
{
  mCalendar->addJournal( j );
  updateView();
}

void KNotesSummaryWidget::removeNote( KCal::Journal *j )
{
  mCalendar->deleteJournal( j );
  updateView();
}


#include "summarywidget.moc"
