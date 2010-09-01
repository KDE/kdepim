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

#include <tqcursor.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqtooltip.h>

#include <kdialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kparts/part.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>
#include <kurllabel.h>
#include <libkcal/resourcecalendar.h>
#include <libkcal/resourcelocal.h>
#include <libkcal/todo.h>
#include <libkcal/incidenceformatter.h>
#include <libkdepim/kpimprefs.h>

#include "korganizeriface_stub.h"

#include "core.h"
#include "plugin.h"
#include "todoplugin.h"

#include "korganizer/stdcalendar.h"
#include "korganizer/koglobals.h"
#include "korganizer/incidencechanger.h"

#include "todosummarywidget.h"

TodoSummaryWidget::TodoSummaryWidget( TodoPlugin *plugin,
                                      TQWidget *parent, const char *name )
  : Kontact::Summary( parent, name ), mPlugin( plugin )
{
  TQVBoxLayout *mainLayout = new TQVBoxLayout( this, 3, 3 );

  TQPixmap icon = KGlobal::iconLoader()->loadIcon( "kontact_todo",
                   KIcon::Desktop, KIcon::SizeMedium );
  TQWidget *header = createHeader( this, icon, i18n( "To-do" ) );
  mainLayout->addWidget( header );

  mLayout = new TQGridLayout( mainLayout, 7, 4, 3 );
  mLayout->setRowStretch( 6, 1 );

  mCalendar = KOrg::StdCalendar::self();

  connect( mCalendar, TQT_SIGNAL( calendarChanged() ), TQT_SLOT( updateView() ) );
  connect( mPlugin->core(), TQT_SIGNAL( dayChanged( const TQDate& ) ),
           TQT_SLOT( updateView() ) );

  updateView();
}

TodoSummaryWidget::~TodoSummaryWidget()
{
}

void TodoSummaryWidget::updateView()
{
  mLabels.setAutoDelete( true );
  mLabels.clear();
  mLabels.setAutoDelete( false );

  KConfig config( "kcmkorgsummaryrc" );
  config.setGroup( "Todo" );
  bool showAllTodos = config.readBoolEntry( "ShowAllTodos", false );

  KIconLoader loader( "kdepim" );

  TQLabel *label = 0;
  int counter = 0;

  TQDate currentDate = TQDate::currentDate();
  KCal::Todo::List todos = mCalendar->todos();
  if ( todos.count() > 0 ) {
    TQPixmap pm = loader.loadIcon( "todo", KIcon::Small );
    KCal::Todo::List::ConstIterator it;
    for ( it = todos.begin(); it != todos.end(); ++it ) {
      KCal::Todo *todo = *it;

      bool accepted = false;
      TQString stateText;

      // show all incomplete todos
      if ( showAllTodos && !todo->isCompleted())
        accepted = true;

      // show uncomplete todos from the last days
      if ( todo->hasDueDate() && !todo->isCompleted() &&
           todo->dtDue().date() < currentDate ) {
        accepted = true;
        stateText = i18n( "overdue" );
      }

      // show todos which started somewhere in the past and has to be finished in future
      if ( todo->hasStartDate() && todo->hasDueDate() &&
           todo->dtStart().date() < currentDate &&
           currentDate < todo->dtDue().date() ) {
        accepted = true;
        stateText = i18n( "in progress" );
      }

      // all todos which start today
      if ( todo->hasStartDate() && todo->dtStart().date() == currentDate ) {
        accepted = true;
        stateText = i18n( "starts today" );
      }

      // all todos which end today
      if ( todo->hasDueDate() && todo->dtDue().date() == currentDate ) {
        accepted = true;
        stateText = i18n( "ends today" );
      }

      if ( !accepted )
        continue;

      label = new TQLabel( this );
      label->setPixmap( pm );
      label->setSizePolicy( TQSizePolicy::Maximum, TQSizePolicy::Maximum );
      mLayout->addWidget( label, counter, 0 );
      mLabels.append( label );

      label = new TQLabel( TQString::number( todo->percentComplete() ) + "%", this );
      label->setAlignment( AlignHCenter | AlignVCenter );
      label->setSizePolicy( TQSizePolicy::Maximum, TQSizePolicy::Maximum );
      mLayout->addWidget( label, counter, 1 );
      mLabels.append( label );

      TQString sSummary = todo->summary();
      if ( todo->relatedTo() ) { // show parent only, not entire ancestry
        sSummary = todo->relatedTo()->summary() + ":" + todo->summary();
      }
      KURLLabel *urlLabel = new KURLLabel( this );
      urlLabel->setText( sSummary );
      urlLabel->setURL( todo->uid() );
      urlLabel->installEventFilter( this );
      urlLabel->setTextFormat( Qt::RichText );
      mLayout->addWidget( urlLabel, counter, 2 );
      mLabels.append( urlLabel );

      connect( urlLabel, TQT_SIGNAL( leftClickedURL( const TQString& ) ),
               this, TQT_SLOT( viewTodo( const TQString& ) ) );
      connect( urlLabel, TQT_SIGNAL( rightClickedURL( const TQString& ) ),
               this, TQT_SLOT( popupMenu( const TQString& ) ) );

      TQString tipText( KCal::IncidenceFormatter::toolTipStr( mCalendar, todo, currentDate, true ) );
      if ( !tipText.isEmpty() ) {
        TQToolTip::add( urlLabel, tipText );
      }

      label = new TQLabel( stateText, this );
      label->setAlignment( AlignLeft | AlignVCenter );
      label->setSizePolicy( TQSizePolicy::Maximum, TQSizePolicy::Maximum );
      mLayout->addWidget( label, counter, 3 );
      mLabels.append( label );

      counter++;
    }
  }

  if ( counter == 0 ) {
    TQLabel *noTodos = new TQLabel( i18n( "No to-dos pending" ), this );
    noTodos->setAlignment( AlignHCenter | AlignVCenter );
    mLayout->addWidget( noTodos, 0, 1 );
    mLabels.append( noTodos );
  }

  for ( label = mLabels.first(); label; label = mLabels.next() )
    label->show();
}

void TodoSummaryWidget::viewTodo( const TQString &uid )
{
  mPlugin->core()->selectPlugin( "kontact_todoplugin" );//ensure loaded
  KOrganizerIface_stub iface( "korganizer", "KOrganizerIface" );
  iface.editIncidence( uid );
}

void TodoSummaryWidget::removeTodo( const TQString &uid )
{
  mPlugin->core()->selectPlugin( "kontact_todoplugin" );//ensure loaded
  KOrganizerIface_stub iface( "korganizer", "KOrganizerIface" );
  iface.deleteIncidence( uid, false );
}

void TodoSummaryWidget::completeTodo( const TQString &uid )
{
  KCal::Todo *todo = mCalendar->todo( uid );
  IncidenceChanger *changer = new IncidenceChanger( mCalendar, this );
  if ( !todo->isReadOnly() && changer->beginChange( todo, 0, TQString() ) ) {
    KCal::Todo *oldTodo = todo->clone();
    todo->setCompleted( TQDateTime::currentDateTime() );
    changer->changeIncidence( oldTodo, todo, KOGlobals::COMPLETION_MODIFIED, this );
    changer->endChange( todo, 0, TQString() );
    delete oldTodo;
    updateView();
  }
}

void TodoSummaryWidget::popupMenu( const TQString &uid )
{
  KPopupMenu popup( this );
  TQToolTip::remove( this );
  popup.insertItem( i18n( "&Edit To-do..." ), 0 );
  popup.insertItem( KGlobal::iconLoader()->loadIcon( "editdelete", KIcon::Small),
                    i18n( "&Delete To-do" ), 1 );
  KCal::Todo *todo = mCalendar->todo( uid );
  if ( !todo->isCompleted() ) {
    popup.insertItem( KGlobal::iconLoader()->loadIcon( "checkedbox", KIcon::Small),
                      i18n( "&Mark To-do Completed" ), 2 );
  }

  switch ( popup.exec( TQCursor::pos() ) ) {
    case 0:
      viewTodo( uid );
      break;
    case 1:
      removeTodo( uid );
      break;
    case 2:
      completeTodo( uid );
      break;
  }
}

bool TodoSummaryWidget::eventFilter( TQObject *obj, TQEvent* e )
{
  if ( obj->inherits( "KURLLabel" ) ) {
    KURLLabel* label = static_cast<KURLLabel*>( obj );
    if ( e->type() == TQEvent::Enter )
      emit message( i18n( "Edit To-do: \"%1\"" ).arg( label->text() ) );
    if ( e->type() == TQEvent::Leave )
      emit message( TQString::null );
  }

  return Kontact::Summary::eventFilter( obj, e );
}

TQStringList TodoSummaryWidget::configModules() const
{
  return TQStringList( "kcmtodosummary.desktop" );
}

#include "todosummarywidget.moc"
