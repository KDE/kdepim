/*
  This file is part of KOrganizer.

  Copyright (c) 1997, 1998 Preston Brown <pbrown@kde.org>
  Copyright (c) 2000-2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "journaleditor.h"
#include "editorconfig.h"
#ifdef HAVE_QT3SUPPORT
#include "editordetails.h"
#endif
#include "editorgeneraljournal.h"

#include <calendarsupport/incidencechanger.h>
#include <calendarsupport/utils.h>

#include <Akonadi/CollectionComboBox>

#include <KLocale>

#include <QTabWidget>
#include <QVBoxLayout>

using namespace KCalCore;
using namespace IncidenceEditors;

JournalEditor::JournalEditor( QWidget *parent )
  : IncidenceEditor( QString(),
                     QStringList() << KCalCore::Journal::journalMimeType(),
                     parent )
{
  mInitialJournal = Journal::Ptr( new Journal() );
}

JournalEditor::~JournalEditor()
{
  emit dialogClose( mIncidence );
}

void JournalEditor::init()
{
  setupGeneral();
  setupAttendeesTab();

  connect( mGeneral, SIGNAL(openCategoryDialog()),
           SIGNAL(editCategories()) );
  connect( this, SIGNAL(updateCategoryConfig()),
           mGeneral, SIGNAL(updateCategoryConfig()) );

}

void JournalEditor::setupGeneral()
{
  mGeneral = new EditorGeneralJournal( this );

  QFrame *topFrame = new QFrame();
  mTabWidget->addTab( topFrame, i18nc( "@title general journal settings", "General" ) );

  QVBoxLayout *topLayout = new QVBoxLayout( topFrame );
  mGeneral->initTitle( topFrame, topLayout );
  mGeneral->initDate( topFrame, topLayout );
  mGeneral->initDescription( topFrame, topLayout );
  mGeneral->initCategories( topFrame, topLayout );

  mGeneral->finishSetup();
}

void JournalEditor::newJournal()
{
  init();
  mIncidence = Akonadi::Item();
  loadDefaults();
  setCaption( i18nc( "@title:window", "New Journal" ) );
}

void JournalEditor::setTexts( const QString &summary, const QString &description,
                                bool richDescription )
{
  if ( description.isEmpty() && summary.contains( '\n' ) ) {
    mGeneral->setDescription( summary, false );
    int pos = summary.indexOf( '\n' );
    mGeneral->setSummary( summary.left( pos ) );
  } else {
    mGeneral->setSummary( summary );
    mGeneral->setDescription( description, richDescription );
  }
}

void JournalEditor::loadDefaults()
{
  setDate( QDate::currentDate() );
  setTime( QTime::currentTime() );
}

bool JournalEditor::processInput()
{
  kDebug();
  if ( !validateInput() || !mChanger ) {
    return false;
  }

  if ( CalendarSupport::hasJournal( mIncidence ) ) {
    bool rc = true;
    Journal::Ptr oldJournal( CalendarSupport::journal( mIncidence )->clone() );
    Journal::Ptr journal( CalendarSupport::journal( mIncidence )->clone() );

    fillJournal( journal );

    if ( *oldJournal == *journal ) {
      // Don't do anything
    } else {
      journal = CalendarSupport::journal( mIncidence );
      journal->startUpdates(); //merge multiple mIncidence->updated() calls into one
      fillJournal( journal );
      rc = mChanger->changeIncidence( oldJournal,
                                      mIncidence,
                                      CalendarSupport::IncidenceChanger::NOTHING_MODIFIED,
                                      this );
      journal->endUpdates();
    }
    return rc;
  } else {
    Journal::Ptr j( new Journal );
    j->setOrganizer( Person::Ptr( new Person( EditorConfig::instance()->fullName(),
                                              EditorConfig::instance()->email() ) ) );
    fillJournal( j );
    //PENDING(AKONADI_PORT) review: mIncidence will be != the newly created item
    mIncidence.setPayload( j );
    Akonadi::Collection col = mCalSelector->currentCollection();
    if ( !mChanger->addIncidence( j, col, this ) ) {
      mIncidence = Akonadi::Item();
      return false;
    }
  }
  return true;
}

void JournalEditor::deleteJournal()
{
  if ( CalendarSupport::hasJournal( mIncidence ) ) {
    emit deleteIncidenceSignal( mIncidence );
  }

  emit dialogClose( mIncidence );
  reject();
}

void JournalEditor::setDate( const QDate &date )
{
  mGeneral->setDate( date );
}

void JournalEditor::setTime( const QTime &time )
{
  mGeneral->setTime( time );
}

bool JournalEditor::incidenceModified()
{
  Journal::Ptr newJournal;
  Journal::Ptr oldJournal;
  bool modified;

  if ( CalendarSupport::hasJournal( mIncidence ) ) { // modification
    oldJournal = CalendarSupport::journal( mIncidence );
  } else { // new one
    oldJournal = mInitialJournal;
  }

  newJournal = Journal::Ptr( oldJournal->clone() );
  fillJournal( newJournal );
  modified = !( *newJournal == *oldJournal );
  return modified;
}

bool JournalEditor::read( const Akonadi::Item &item, const QDate &date, bool tmpl )
{
  const Journal::Ptr journal = CalendarSupport::journal( item );
  if ( !journal ) {
    return false;
  }

  mGeneral->readJournal( journal, date, tmpl );
#ifdef HAVE_QT3SUPPORT
  Incidence::Ptr inc = journal.staticCast<Incidence>();
  mDetails->readIncidence( inc );
#endif

  return true;
}

void JournalEditor::fillJournal( Journal::Ptr &journal )
{
  mGeneral->fillJournal( journal );
#ifdef HAVE_QT3SUPPORT
  Incidence::Ptr inc = journal.staticCast<Incidence>();
  mDetails->fillIncidence( inc );
#endif
}

bool JournalEditor::validateInput()
{
  if ( !mGeneral->validateInput() ) {
    return false;
  }
#ifdef HAVE_QT3SUPPORT
  if ( !mDetails->validateInput() ) {
    return false;
  }
#endif
  return true;
}

void JournalEditor::modified()
{
  // Play dumb, just reload the Journal. This dialog has become so complicated
  // that there is no point in trying to be smart here...
  readIncidence( mIncidence, QDate(), true );
}

void JournalEditor::show()
{
  fillJournal( mInitialJournal );
  IncidenceEditor::show();
}

#include "journaleditor.moc"
