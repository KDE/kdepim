/*
    This file is part of KOrganizer.

    Copyright (c) 1997, 1998 Preston Brown <pbrown@kde.org>
    Copyright (c) 2000-2003 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include <tqtooltip.h>
#include <tqframe.h>
#include <tqpixmap.h>
#include <tqlayout.h>
#include <tqdatetime.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <libkcal/calendarlocal.h>
#include <libkcal/calendarresources.h>
#include <libkcal/resourcecalendar.h>

#include "koprefs.h"
#include "koeditorattachments.h"
#include "kogroupware.h"
#include "kodialogmanager.h"
#include "incidencechanger.h"

#include "koeditorgeneraltodo.h"
#include "koeditordetails.h"
#include "koeditorrecurrence.h"

#include "kotodoeditor.h"
#include "kocore.h"

KOTodoEditor::KOTodoEditor( Calendar *calendar, TQWidget *parent ) :
  KOIncidenceEditor( TQString::null, calendar, parent )
{
  mTodo = 0;
  mCalendar = 0;
  mRelatedTodo = 0;
}

KOTodoEditor::~KOTodoEditor()
{
  emit dialogClose( mTodo );
}

void KOTodoEditor::init()
{
  kdDebug(5850) << k_funcinfo << endl;
  setupGeneral();
  setupRecurrence();
  setupAttendeesTab();
//  setupAttachmentsTab();

  connect( mGeneral, TQT_SIGNAL( dateTimeStrChanged( const TQString & ) ),
           mRecurrence, TQT_SLOT( setDateTimeStr( const TQString & ) ) );
  connect( mGeneral, TQT_SIGNAL( signalDateTimeChanged( const TQDateTime &, const TQDateTime & ) ),
           mRecurrence, TQT_SLOT( setDateTimes( const TQDateTime &, const TQDateTime & ) ) );

  connect( mGeneral, TQT_SIGNAL( openCategoryDialog() ),
           TQT_SIGNAL( editCategories() ) );

  connect( mDetails, TQT_SIGNAL(updateAttendeeSummary(int)),
           mGeneral, TQT_SLOT(updateAttendeeSummary(int)) );
}

void KOTodoEditor::reload()
{
  if ( mTodo ) readTodo( mTodo, mCalendar );
}

void KOTodoEditor::setupGeneral()
{
  mGeneral = new KOEditorGeneralTodo(this);

  if (KOPrefs::instance()->mCompactDialogs) {
    TQFrame *topFrame = addPage(i18n("General"));

    TQBoxLayout *topLayout = new TQVBoxLayout(topFrame);
    topLayout->setMargin(marginHint());
    topLayout->setSpacing(spacingHint());

    mGeneral->initHeader( topFrame, topLayout );
    mGeneral->initTime(topFrame,topLayout);
    TQHBoxLayout *priorityLayout = new TQHBoxLayout( topLayout );
    mGeneral->initPriority(topFrame,priorityLayout);
    topLayout->addStretch(1);

    TQFrame *topFrame2 = addPage(i18n("Details"));

    TQBoxLayout *topLayout2 = new TQVBoxLayout(topFrame2);
    topLayout2->setMargin(marginHint());
    topLayout2->setSpacing(spacingHint());

    TQHBoxLayout *completionLayout = new TQHBoxLayout( topLayout2 );
    mGeneral->initCompletion(topFrame2,completionLayout);

    mGeneral->initAlarm(topFrame,topLayout);
 
    mGeneral->initSecrecy( topFrame2, topLayout2 );
    mGeneral->initDescription(topFrame2,topLayout2);
  } else {
    TQFrame *topFrame = addPage(i18n("&General"));

    TQBoxLayout *topLayout = new TQVBoxLayout(topFrame);
    topLayout->setSpacing(spacingHint());

    mGeneral->initHeader( topFrame, topLayout );
    mGeneral->initTime(topFrame,topLayout);
    mGeneral->initStatus(topFrame,topLayout);
    TQBoxLayout *alarmLineLayout = new TQHBoxLayout(topLayout);
    mGeneral->initAlarm(topFrame,alarmLineLayout);
    alarmLineLayout->addStretch( 1 );
    mGeneral->initDescription(topFrame,topLayout);
    mGeneral->initAttachments(topFrame,topLayout);
    connect( mGeneral, TQT_SIGNAL( openURL( const KURL& ) ),
             this, TQT_SLOT( openURL( const KURL& ) ) );
    connect( this, TQT_SIGNAL( signalAddAttachments( const TQStringList&, const TQStringList&, bool ) ),
             mGeneral, TQT_SLOT( addAttachments( const TQStringList&, const TQStringList&, bool ) ) );
  }
  mGeneral->enableAlarm( true );

  mGeneral->finishSetup();
}

void KOTodoEditor::setupRecurrence()
{
  TQFrame *topFrame = addPage( i18n("Rec&urrence") );

  TQBoxLayout *topLayout = new TQVBoxLayout( topFrame );

  mRecurrence = new KOEditorRecurrence( topFrame );
  topLayout->addWidget( mRecurrence );

  mRecurrence->setEnabled( false );
  connect(mGeneral,TQT_SIGNAL(dueDateEditToggle( bool ) ),
          mRecurrence, TQT_SLOT( setEnabled( bool ) ) );
}

void KOTodoEditor::editIncidence(Incidence *incidence, Calendar *calendar)
{
  kdDebug(5850) << k_funcinfo << endl;
  Todo *todo=dynamic_cast<Todo*>(incidence);
  if (todo)
  {
    init();

    mTodo = todo;
    mCalendar = calendar;
    readTodo( mTodo, mCalendar );
  }

  setCaption( i18n("Edit To-do") );
}

void KOTodoEditor::newTodo()
{
  kdDebug(5850) << k_funcinfo << endl;
  init();
  mTodo = 0;
  mCalendar = 0;
  setCaption( i18n("New To-do") );
  loadDefaults();
}

void KOTodoEditor::setTexts( const TQString &summary, const TQString &description )
{
  if ( description.isEmpty() && summary.contains("\n") ) {
    mGeneral->setDescription( summary );
    int pos = summary.find( "\n" );
    mGeneral->setSummary( summary.left( pos ) );
  } else {
    mGeneral->setSummary( summary );
    mGeneral->setDescription( description );
  }
}



void KOTodoEditor::loadDefaults()
{
  kdDebug(5850) << k_funcinfo << endl;
  setDates( TQDateTime::currentDateTime().addDays(7), true, 0 );
  mGeneral->toggleAlarm( true );
}

bool KOTodoEditor::processInput()
{
  if ( !validateInput() ) return false;

  if ( mTodo ) {
    bool rc = true;
    Todo *oldTodo = mTodo->clone();
    Todo *todo = mTodo->clone();

    kdDebug(5850) << "KOTodoEditor::processInput() write event." << endl;
    writeTodo( todo );
    kdDebug(5850) << "KOTodoEditor::processInput() event written." << endl;

    if( *mTodo == *todo )
      // Don't do anything
      kdDebug(5850) << "Todo not changed\n";
    else {
      kdDebug(5850) << "Todo changed\n";
      //IncidenceChanger::assignIncidence( mTodo, todo );
      writeTodo( mTodo );
      mChanger->changeIncidence( oldTodo, mTodo );
    }
    delete todo;
    delete oldTodo;
    return rc;

  } else {
    mTodo = new Todo;
    mTodo->setOrganizer( Person( KOPrefs::instance()->fullName(),
                         KOPrefs::instance()->email() ) );

    writeTodo( mTodo );

    if ( !mChanger->addIncidence( mTodo, this ) ) {
      delete mTodo;
      mTodo = 0;
      return false;
    }
  }

  return true;

}

void KOTodoEditor::deleteTodo()
{
  if (mTodo)
    emit deleteIncidenceSignal( mTodo );
  emit dialogClose(mTodo);
  reject();
}

void KOTodoEditor::setDates( const TQDateTime &due, bool allDay, Todo *relatedEvent )
{
  mRelatedTodo = relatedEvent;

  // inherit some properties from parent todo
  if ( mRelatedTodo ) {
    mGeneral->setCategories( mRelatedTodo->categories() );
  }
  if ( !due.isValid() && mRelatedTodo && mRelatedTodo->hasDueDate() ) {
    mGeneral->setDefaults( mRelatedTodo->dtDue(), allDay );
  } else {
    mGeneral->setDefaults( due, allDay );
  }

  mDetails->setDefaults();
  if ( mTodo )
    mRecurrence->setDefaults( mTodo->dtStart(), due, false );
  else
    mRecurrence->setDefaults( TQDateTime::currentDateTime(), due, false );
}

void KOTodoEditor::readTodo( Todo *todo, Calendar *calendar )
{
  if ( !todo ) return;
//   mRelatedTodo = todo->relatedTo();
  kdDebug(5850)<<"read todo"<<endl;
  mGeneral->readTodo( todo, calendar );
  mDetails->readEvent( todo );
  mRecurrence->readIncidence( todo );

  createEmbeddedURLPages( todo );
  readDesignerFields( todo );
}

void KOTodoEditor::writeTodo( Todo *todo )
{
  Incidence *oldIncidence = todo->clone();

  mRecurrence->writeIncidence( todo );
  mGeneral->writeTodo( todo );
  mDetails->writeEvent( todo );

  if ( *(oldIncidence->recurrence()) != *(todo->recurrence() ) ) {
    todo->setDtDue( todo->dtDue(), true );
    if ( todo->hasStartDate() )
      todo->setDtStart( todo->dtStart() );
  }
  writeDesignerFields( todo );

  // set related incidence, i.e. parent to-do in this case.
  if ( mRelatedTodo ) {
    todo->setRelatedTo( mRelatedTodo );
  }

  cancelRemovedAttendees( todo );
}

bool KOTodoEditor::validateInput()
{
  if ( !mGeneral->validateInput() ) return false;
  if ( !mRecurrence->validateInput() ) return false;
  if ( !mDetails->validateInput() ) return false;
  return true;
}

int KOTodoEditor::msgItemDelete()
{
  return KMessageBox::warningContinueCancel(this,
      i18n("This item will be permanently deleted."),
      i18n("KOrganizer Confirmation"), KStdGuiItem::del() );
}

void KOTodoEditor::modified (int /*modification*/)
{
  // Play dump, just reload the todo. This dialog has become so complicated that
  // there is no point in trying to be smart here...
  reload();
}

void KOTodoEditor::loadTemplate( /*const*/ CalendarLocal& cal )
{
  Todo::List todos = cal.todos();
  if ( todos.count() == 0 ) {
    KMessageBox::error( this,
        i18n("Template does not contain a valid to-do.") );
  } else {
    readTodo( todos.first(), 0 );
  }
}

void KOTodoEditor::slotSaveTemplate( const TQString &templateName )
{
  Todo *todo = new Todo;
  writeTodo( todo );
  saveAsTemplate( todo, templateName );
}

TQStringList& KOTodoEditor::templates() const
{
  return KOPrefs::instance()->mTodoTemplates;
}

#include "kotodoeditor.moc"
