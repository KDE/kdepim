/*
    This file is part of KOrganizer.

    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
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

#include <kcmultidialog.h>
#include <ksettings/dialog.h>
#include <kwin.h>

#include <libkdepim/categoryeditdialog.h>

#include "calendarview.h"
#include "koprefsdialog.h"
#include "koprefs.h"
#include "koeventeditor.h"
#include "kotodoeditor.h"
#include "kojournaleditor.h"
#include "searchdialog.h"
#include "filtereditdialog.h"
#ifndef KORG_NOARCHIVE
#include "archivedialog.h"
#endif
#include "koviewmanager.h"
#include "koagendaview.h"
#include "koglobals.h"

#include "kodialogmanager.h"
#include "kodialogmanager.moc"


// FIXME: Handle KOEventViewerDialogs in dialog manager. Pass
// KOPrefs::mCompactDialog.

class KODialogManager::DialogManagerVisitor : public IncidenceBase::Visitor
{
  public:
    DialogManagerVisitor() : mDialogManager( 0 ) {}

    bool act( IncidenceBase *incidence, KODialogManager *manager )
    {
      mDialogManager = manager;
      return incidence->accept( *this );
    }

  protected:
    KODialogManager *mDialogManager;
};

class KODialogManager::EditorDialogVisitor :
      public KODialogManager::DialogManagerVisitor
{
  public:
    EditorDialogVisitor() : DialogManagerVisitor(), mEditor( 0 ) {}
    KOIncidenceEditor *editor() const { return mEditor; }
  protected:
    bool visit( Event * ) { mEditor = mDialogManager->getEventEditor(); return mEditor; }
    bool visit( Todo * ) { mEditor = mDialogManager->getTodoEditor(); return mEditor; }
    bool visit( Journal * ) { mEditor = mDialogManager->getJournalEditor(); return mEditor; }
  protected:
    KOIncidenceEditor *mEditor;
};


KODialogManager::KODialogManager( CalendarView *mainView ) :
  TQObject(), mMainView( mainView )
{
  mOptionsDialog = 0;
  mSearchDialog = 0;
  mArchiveDialog = 0;
  mFilterEditDialog = 0;

  mCategoryEditDialog = new KPIM::CategoryEditDialog( KOPrefs::instance(), mMainView );
  // don't set any specific parent for the dialog, as its kept around and reused
  // in different cases where it should have different parents
  KWin::setMainWindow( mCategoryEditDialog, 0 );
  connect( mainView, TQT_SIGNAL( categoriesChanged() ),
           mCategoryEditDialog, TQT_SLOT( reload() ) );
  KOGlobals::fitDialogToScreen( mCategoryEditDialog );
}

KODialogManager::~KODialogManager()
{
  delete mOptionsDialog;
  delete mSearchDialog;
#ifndef KORG_NOARCHIVE
  delete mArchiveDialog;
#endif
  delete mFilterEditDialog;
}

void KODialogManager::errorSaveIncidence( TQWidget *parent, Incidence *incidence )
{
  KMessageBox::sorry( parent, i18n("Unable to save %1 \"%2\".")
                      .arg( i18n( incidence->type() ) )
                      .arg( incidence->summary() ) );
}

void KODialogManager::showOptionsDialog()
{
  if (!mOptionsDialog) {
#if 0
    mOptionsDialog = new KConfigureDialog();
//    mOptionsDialog = new KConfigureDialog( KConfigureDialog::Configurable );
//    mOptionsDialog = new KConfigureDialog( mMainView );
    connect( mOptionsDialog->dialog(),
             TQT_SIGNAL( configCommitted( const TQCString & ) ),
             mMainView, TQT_SLOT( updateConfig() ) );
#else
    mOptionsDialog = new KCMultiDialog( mMainView, "KorganizerPreferences" );
    connect( mOptionsDialog, TQT_SIGNAL( configCommitted( const TQCString & ) ),
             mMainView, TQT_SLOT( updateConfig( const TQCString& ) ) );
#if 0
    connect( mOptionsDialog, TQT_SIGNAL( applyClicked() ),
             mMainView, TQT_SLOT( updateConfig() ) );
    connect( mOptionsDialog, TQT_SIGNAL( okClicked() ),
             mMainView, TQT_SLOT( updateConfig() ) );
    // @TODO Find a way to do this with KCMultiDialog
    connect(mCategoryEditDialog,TQT_SIGNAL(categoryConfigChanged()),
            mOptionsDialog,TQT_SLOT(updateCategories()));
#endif

    TQStringList modules;

    modules.append( "korganizer_configmain.desktop" );
    modules.append( "korganizer_configtime.desktop" );
    modules.append( "korganizer_configviews.desktop" );
    modules.append( "korganizer_configfonts.desktop" );
    modules.append( "korganizer_configcolors.desktop" );
    modules.append( "korganizer_configgroupscheduling.desktop" );
    modules.append( "korganizer_configgroupautomation.desktop" );
    modules.append( "korganizer_configfreebusy.desktop" );
    modules.append( "korganizer_configplugins.desktop" );
    modules.append( "korganizer_configdesignerfields.desktop" );

    // add them all
    TQStringList::iterator mit;
    for ( mit = modules.begin(); mit != modules.end(); ++mit )
      mOptionsDialog->addModule( *mit );
#endif
  }

  mOptionsDialog->show();
  mOptionsDialog->raise();
}

void KODialogManager::showCategoryEditDialog()
{
  mCategoryEditDialog->show();
}

void KODialogManager::showSearchDialog()
{
  if ( !mSearchDialog ) {
    mSearchDialog = new SearchDialog( mMainView->calendar(), mMainView );
    connect( mSearchDialog, TQT_SIGNAL(showIncidenceSignal(Incidence *,const TQDate &)),
             mMainView, TQT_SLOT(showIncidence(Incidence *,const TQDate &)) );
    connect( mSearchDialog, TQT_SIGNAL(editIncidenceSignal(Incidence *,const TQDate &)),
             mMainView, TQT_SLOT(editIncidence(Incidence *,const TQDate &)) );
    connect( mSearchDialog, TQT_SIGNAL(deleteIncidenceSignal(Incidence *)),
             mMainView, TQT_SLOT(deleteIncidence(Incidence *)) );
    connect( mMainView, TQT_SIGNAL(closingDown()),mSearchDialog,TQT_SLOT(reject()) );
  }
  // make sure the widget is on top again
  mSearchDialog->show();
  mSearchDialog->raise();
}

void KODialogManager::showArchiveDialog()
{
#ifndef KORG_NOARCHIVE
  if (!mArchiveDialog) {
    mArchiveDialog = new ArchiveDialog(mMainView->calendar(),mMainView);
    connect(mArchiveDialog,TQT_SIGNAL(eventsDeleted()),
            mMainView,TQT_SLOT(updateView()));
    connect(mArchiveDialog,TQT_SIGNAL(autoArchivingSettingsModified()),
            mMainView,TQT_SLOT(slotAutoArchivingSettingsModified()));
  }
  mArchiveDialog->show();
  mArchiveDialog->raise();

  // Workaround.
  TQApplication::restoreOverrideCursor();
#endif
}

void KODialogManager::showFilterEditDialog( TQPtrList<CalFilter> *filters )
{
  if ( !mFilterEditDialog ) {
    mFilterEditDialog = new FilterEditDialog( filters, mMainView );
    connect( mFilterEditDialog, TQT_SIGNAL( filterChanged() ),
             mMainView, TQT_SLOT( updateFilter() ) );
    connect( mFilterEditDialog, TQT_SIGNAL( editCategories() ),
             mCategoryEditDialog, TQT_SLOT( show() ) );
    connect( mCategoryEditDialog, TQT_SIGNAL( categoryConfigChanged() ),
             mFilterEditDialog, TQT_SLOT( updateCategoryConfig() ) );
  }
  mFilterEditDialog->show();
  mFilterEditDialog->raise();
}

KOIncidenceEditor *KODialogManager::getEditor( Incidence *incidence )
{
  if ( !incidence ) return 0;
  EditorDialogVisitor v;
  if ( v.act( incidence, this ) ) {
    return v.editor();
  } else
    return 0;
}

KOEventEditor *KODialogManager::getEventEditor()
{
  KOEventEditor *eventEditor = new KOEventEditor( mMainView->calendar(),
                                                  mMainView );
  connectEditor( eventEditor );
  return eventEditor;
}

void KODialogManager::connectTypeAhead( KOEventEditor *editor,
                                        KOrg::AgendaView *agenda )
{
  if ( editor && agenda ) {
    agenda->setTypeAheadReceiver( editor->typeAheadReceiver() );
    connect( editor, TQT_SIGNAL( focusReceivedSignal() ),
             agenda, TQT_SLOT( finishTypeAhead() ) );
  }
}

void KODialogManager::connectEditor( KOIncidenceEditor*editor )
{
  connect( editor, TQT_SIGNAL( deleteIncidenceSignal( Incidence * ) ),
           mMainView, TQT_SLOT( deleteIncidence( Incidence * ) ) );

  connect( mCategoryEditDialog, TQT_SIGNAL( categoryConfigChanged() ),
           editor, TQT_SIGNAL( updateCategoryConfig() ) );
  connect( editor, TQT_SIGNAL( editCategories() ),
           mCategoryEditDialog, TQT_SLOT( show() ) );

  connect( editor, TQT_SIGNAL( dialogClose( Incidence * ) ),
           mMainView, TQT_SLOT( dialogClosing( Incidence * ) ) );
  connect( editor, TQT_SIGNAL( editCanceled( Incidence * ) ),
           mMainView, TQT_SLOT( editCanceled( Incidence * ) ) );
  connect( mMainView, TQT_SIGNAL( closingDown() ), editor, TQT_SLOT( reject() ) );

  connect( editor, TQT_SIGNAL( deleteAttendee( Incidence * ) ),
           mMainView, TQT_SIGNAL( cancelAttendees( Incidence * ) ) );
}

KOTodoEditor *KODialogManager::getTodoEditor()
{
  kdDebug(5850) << k_funcinfo << endl;
  KOTodoEditor *todoEditor = new KOTodoEditor( mMainView->calendar(), mMainView );
  connectEditor( todoEditor );
  return todoEditor;
}

KOJournalEditor *KODialogManager::getJournalEditor()
{
  KOJournalEditor *journalEditor = new KOJournalEditor( mMainView->calendar(), mMainView );
  connectEditor( journalEditor );
  return journalEditor;
}

void KODialogManager::updateSearchDialog()
{
  if (mSearchDialog) mSearchDialog->updateView();
}

