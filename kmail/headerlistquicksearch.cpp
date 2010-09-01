/*
    This file is part of KMail, the KDE mail client.
    Copyright (c) 2004 Till Adam <adam@kde.org>

    KMail is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    KMail is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/
#include "headerlistquicksearch.h"

#include <tqapplication.h>
#include <tqlabel.h>
#include <tqcombobox.h>
#include <tqvaluevector.h>
#include <tqtimer.h>

#include <kaction.h>
#include <kiconloader.h>
#include <klistview.h>
#include <klocale.h>
#include <ktoolbarbutton.h>

#include "kmheaders.h"
#include "kmsearchpattern.h"
#include "kmmainwidget.h"

namespace KMail {

HeaderListQuickSearch::HeaderListQuickSearch( TQWidget *parent,
                                              KListView *listView,
                                              KActionCollection *actionCollection,
                                              const char *name )
  : KListViewSearchLine(parent, listView, name), mStatusCombo(0), mStatus(0),  statusList()
{
  KAction *resetQuickSearch = new KAction( i18n( "Reset Quick Search" ),
                                           TQApplication::reverseLayout()
                                           ? "clear_left"
                                           : "locationbar_erase",
                                           0, this,
                                           TQT_SLOT( reset() ),
                                           actionCollection,
                                           "reset_quicksearch" );
  resetQuickSearch->plug( parent );
  resetQuickSearch->setWhatsThis( i18n( "Reset Quick Search\n"
                                        "Resets the quick search so that "
                                        "all messages are shown again." ) );

  TQLabel *label = new TQLabel( i18n("Stat&us:"), parent, "kde toolbar widget" );

  mStatusCombo = new TQComboBox( parent, "quick search status combo box" );
  mStatusCombo->setSizeLimit( 12 );
  mStatusCombo->insertItem( SmallIcon( "run" ), i18n("Any Status") );

  insertStatus( StatusUnread );
  insertStatus( StatusNew );
  insertStatus( StatusImportant );
  insertStatus( StatusReplied );
  insertStatus( StatusForwarded );
  insertStatus( StatusToDo );
  insertStatus( StatusHasAttachment );
  insertStatus( StatusInvitation );
  insertStatus( StatusWatched );
  insertStatus( StatusIgnored );
  mStatusCombo->setCurrentItem( 0 );
  mStatusCombo->installEventFilter( this );
  connect( mStatusCombo, TQT_SIGNAL ( activated( int ) ),
           this, TQT_SLOT( slotStatusChanged( int ) ) );

  label->setBuddy( mStatusCombo );

  KToolBarButton * btn = new KToolBarButton( "mail_find", 0, parent,
                                            0, i18n( "Open Full Search" ) );
  connect( btn, TQT_SIGNAL( clicked() ), TQT_SIGNAL( requestFullSearch() ) );

  /* Disable the signal connected by KListViewSearchLine since it will call
   * itemAdded during KMHeaders::readSortOrder() which will in turn result
   * in getMsgBaseForItem( item ) wanting to access items which are no longer
   * there. Rather rely on KMHeaders::msgAdded and its signal. */
  disconnect(listView, TQT_SIGNAL(itemAdded(TQListViewItem *)),
             this, TQT_SLOT(itemAdded(TQListViewItem *)));
  KMHeaders *headers = static_cast<KMHeaders*>( listView );
  connect( headers, TQT_SIGNAL( msgAddedToListView( TQListViewItem* ) ),
           this, TQT_SLOT( itemAdded( TQListViewItem* ) ) );

}

HeaderListQuickSearch::~HeaderListQuickSearch()
{
}


bool HeaderListQuickSearch::eventFilter( TQObject *watched, TQEvent *event )
{
  if ( watched == mStatusCombo ) {
    KMMainWidget *mainWidget = 0;

    // Travel up the parents list until we find the main widget
    for ( TQWidget *curWidget = parentWidget(); curWidget; curWidget = curWidget->parentWidget() ) {
      mainWidget = ::qt_cast<KMMainWidget *>( curWidget );
      if ( mainWidget )
        break;
    }

    if ( mainWidget ) {
      switch ( event->type() ) {
      case TQEvent::FocusIn:
        mainWidget->setAccelsEnabled( false );
        break;
      case TQEvent::FocusOut:
        mainWidget->setAccelsEnabled( true );
        break;
      default:
        // Avoid compiler warnings
        break;
      }
    }
  }

  // In either case, always return false, we NEVER want to eat the event
  return false;
}


bool HeaderListQuickSearch::itemMatches(const TQListViewItem *item, const TQString &s) const
{
  mCurrentSearchTerm = s; // bit of a hack, but works
  if ( mStatus != 0 ) {
    KMHeaders *headers = static_cast<KMHeaders*>( item->listView() );
    const KMMsgBase *msg = headers->getMsgBaseForItem( item );
    if ( !msg || ! ( msg->status() & mStatus ) )
      return false;
  }

  // The full email address is not visible, but we still want it to be searchable.
  // KListViewSearchLine::itemMatches() only searches in visible columns.
  const HeaderItem *headerItem = static_cast<const HeaderItem*>( item );
  if ( headerItem->from().lower().contains( s.lower() ) ) {
    return true;
  }
  if ( headerItem->to().lower().contains( s.lower() ) ) {
    return true;
  }

  return KListViewSearchLine::itemMatches(item, s);
}

//-----------------------------------------------------------------------------
void HeaderListQuickSearch::reset()
{
  clear();
  mStatusCombo->setCurrentItem( 0 );
  slotStatusChanged( 0 );
}

void HeaderListQuickSearch::slotStatusChanged( int index )
{
  if ( index == 0 )
    mStatus = 0;
  else
    mStatus = KMSearchRuleStatus::statusFromEnglishName( statusList[index - 1] );
  updateSearch();
}

void HeaderListQuickSearch::insertStatus(KMail::StatusValueTypes which)
{
  mStatusCombo->insertItem( SmallIcon( KMail::StatusValues[which].icon ),
    i18n( KMail::StatusValues[ which ].text ) );
  statusList.append( KMail::StatusValues[ which ].text );
}


TQString HeaderListQuickSearch::currentSearchTerm() const
{
    return mCurrentSearchTerm;
}


int HeaderListQuickSearch::currentStatus() const
{
    return mStatus;
}

} // namespace KMail

#include "headerlistquicksearch.moc"
