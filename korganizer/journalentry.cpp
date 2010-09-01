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

//
// Journal Entry

#include <tqlabel.h>
#include <tqlayout.h>
#include <tqcheckbox.h>
#include <tqwhatsthis.h>
#include <tqtooltip.h>
#include <tqtoolbutton.h>

#include <kdebug.h>
#include <kdialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <ktextedit.h>
#include <ktimeedit.h>
#include <klineedit.h>
#include <kactivelabel.h>
#include <kstdguiitem.h>
#include <kmessagebox.h>

#include <libkcal/journal.h>
#include <libkcal/calendar.h>

#include "kodialogmanager.h"
#include "incidencechanger.h"
#include "koglobals.h"

#include "journalentry.h"
#include "journalentry.moc"
#ifndef KORG_NOPRINTER
#include "kocorehelper.h"
#include "calprinter.h"
#endif

class JournalTitleLable : public KActiveLabel
{
public:
  JournalTitleLable( TQWidget *parent, const char *name=0 ) : KActiveLabel( parent, name ) {}

  void openLink( const TQString &/*link*/ ) {}
};


JournalDateEntry::JournalDateEntry( Calendar *calendar, TQWidget *parent ) :
  TQVBox( parent ), mCalendar( calendar )
{
//kdDebug(5850)<<"JournalEntry::JournalEntry, parent="<<parent<<endl;
  mChanger = 0;

  mTitle = new JournalTitleLable( this );
  mTitle->setMargin(2);
  mTitle->setSizePolicy( TQSizePolicy::Expanding, TQSizePolicy::Fixed );
  connect( mTitle, TQT_SIGNAL( linkClicked( const TQString & ) ),
           this, TQT_SLOT( emitNewJournal() ) );
}

JournalDateEntry::~JournalDateEntry()
{
}

void JournalDateEntry::setDate(const TQDate &date)
{
  TQString dtstring = TQString( "<qt><center><b><i>%1</i></b>  " )
                     .arg( KGlobal::locale()->formatDate(date) );

  dtstring += " <font size=\"-1\"><a href=\"#\">" +
              i18n("[Add Journal Entry]") +
              "</a></font></center></qt>";

  mTitle->setText( dtstring );
  mDate = date;
  emit setDateSignal( date );
}

void JournalDateEntry::clear()
{
  TQValueList<JournalEntry*> values( mEntries.values() );

  TQValueList<JournalEntry*>::Iterator it = values.begin();
  for ( ; it != values.end(); ++it ) {
    delete (*it);
  }
  mEntries.clear();
}

// should only be called by the KOJournalView now.
void JournalDateEntry::addJournal( Journal *j )
{
  TQMap<Journal*,JournalEntry*>::Iterator pos = mEntries.find( j );
  if ( pos != mEntries.end() ) return;

  JournalEntry *entry = new JournalEntry( j, this );
  entry->show();
  entry->setDate( mDate );
  entry->setIncidenceChanger( mChanger );

  mEntries.insert( j, entry );
  connect( this, TQT_SIGNAL( setIncidenceChangerSignal( IncidenceChangerBase * ) ),
           entry, TQT_SLOT( setIncidenceChanger( IncidenceChangerBase * ) ) );
  connect( this, TQT_SIGNAL( setDateSignal( const TQDate & ) ),
           entry, TQT_SLOT( setDate( const TQDate & ) ) );
  connect( this, TQT_SIGNAL( flushEntries() ),
           entry, TQT_SLOT( flushEntry() ) );
  connect( entry, TQT_SIGNAL( deleteIncidence( Incidence* ) ),
           this, TQT_SIGNAL( deleteIncidence( Incidence* ) ) );
  connect( entry, TQT_SIGNAL( editIncidence( Incidence*, const TQDate& ) ),
           this, TQT_SIGNAL( editIncidence( Incidence*, const TQDate& ) ) );
}

Journal::List JournalDateEntry::journals() const
{
  TQValueList<Journal*> jList( mEntries.keys() );
  Journal::List l;
  TQValueList<Journal*>::Iterator it = jList.begin();
  for ( ; it != jList.end(); ++it ) {
    l.append( *it );
  }
  return l;
}

void JournalDateEntry::setIncidenceChanger( IncidenceChangerBase *changer )
{
  mChanger = changer;
  emit setIncidenceChangerSignal( changer );
}

void JournalDateEntry::emitNewJournal()
{
  emit newJournal( 0/*ResourceCalendar*/, TQString()/*subResource*/, mDate );
}

void JournalDateEntry::journalEdited( Journal *journal )
{
  TQMap<Journal*,JournalEntry*>::Iterator pos = mEntries.find( journal );
  if ( pos == mEntries.end() ) return;

  pos.data()->setJournal( journal );

}

void JournalDateEntry::journalDeleted( Journal *journal )
{
  TQMap<Journal*,JournalEntry*>::Iterator pos = mEntries.find( journal );
  if ( pos == mEntries.end() ) return;

  delete pos.data();
}





JournalEntry::JournalEntry( Journal* j, TQWidget *parent ) :
  TQWidget( parent ), mJournal( j )
{
//kdDebug(5850)<<"JournalEntry::JournalEntry, parent="<<parent<<endl;
  mDirty = false;
  mWriteInProgress = false;
  mChanger = 0;
  mReadOnly = false;

  mLayout = new TQGridLayout( this );
  mLayout->setSpacing( KDialog::spacingHint() );
  mLayout->setMargin( KDialog::marginHint() );

  TQString whatsThis = i18n("Sets the Title of this journal entry.");

  mTitleLabel = new TQLabel( i18n("&Title: "), this );
  mLayout->addWidget( mTitleLabel, 0, 0 );
  mTitleEdit = new KLineEdit( this );
  mLayout->addWidget( mTitleEdit, 0, 1 );
  mTitleLabel->setBuddy( mTitleEdit );

  TQWhatsThis::add( mTitleLabel, whatsThis );
  TQWhatsThis::add( mTitleEdit, whatsThis );

  mTimeCheck = new TQCheckBox( i18n("Ti&me: "), this );
  mLayout->addWidget( mTimeCheck, 0, 2 );
  mTimeEdit = new KTimeEdit( this );
  mLayout->addWidget( mTimeEdit, 0, 3 );
  connect( mTimeCheck, TQT_SIGNAL(toggled(bool)),
           this, TQT_SLOT(timeCheckBoxToggled(bool)) );
  TQWhatsThis::add( mTimeCheck, i18n("Determines whether this journal entry has "
                                    "a time associated with it") );
  TQWhatsThis::add( mTimeEdit, i18n( "Sets the time associated with this journal "
                                    "entry" ) );

  mDeleteButton = new TQToolButton( this, "deleteButton" );
  TQPixmap pix = KOGlobals::self()->smallIcon( "editdelete" );
  mDeleteButton->setPixmap( pix );
  mDeleteButton->setSizePolicy( TQSizePolicy::Fixed, TQSizePolicy::Fixed );
  TQToolTip::add( mDeleteButton, i18n("Delete this journal entry") );
  TQWhatsThis::add( mDeleteButton, i18n("Delete this journal entry") );
  mLayout->addWidget( mDeleteButton, 0, 4 );
  connect( mDeleteButton, TQT_SIGNAL(pressed()), this, TQT_SLOT(deleteItem()) );

  mEditButton = new TQToolButton( this, "editButton" );
  mEditButton->setPixmap( KOGlobals::self()->smallIcon( "edit" ) );
  mEditButton->setSizePolicy( TQSizePolicy::Fixed, TQSizePolicy::Fixed );
  TQToolTip::add( mEditButton, i18n("Edit this journal entry") );
  TQWhatsThis::add( mEditButton, i18n("Opens an editor dialog for this journal entry") );
  mLayout->addWidget( mEditButton, 0, 5 );
  connect( mEditButton, TQT_SIGNAL(clicked()), this, TQT_SLOT( editItem() ) );

#ifndef KORG_NOPRINTER
  mPrintButton = new TQToolButton( this, "printButton" );
  mPrintButton->setPixmap( KOGlobals::self()->smallIcon( "printer1" ) );
  mPrintButton->setSizePolicy( TQSizePolicy::Fixed, TQSizePolicy::Fixed );
  TQToolTip::add( mPrintButton, i18n("Print this journal entry") );
  TQWhatsThis::add( mPrintButton, i18n("Opens the print dialog for this journal entry") );
  mLayout->addWidget( mPrintButton, 0, 6 );
  connect( mPrintButton, TQT_SIGNAL(clicked()), this, TQT_SLOT( printItem() ) );
#endif
  mEditor = new KTextEdit(this);
  mLayout->addMultiCellWidget( mEditor, 1, 2, 0, 6 );

  connect( mTitleEdit, TQT_SIGNAL(textChanged( const TQString& )), TQT_SLOT(setDirty()) );
  connect( mTimeCheck, TQT_SIGNAL(toggled(bool)), TQT_SLOT(setDirty()) );
  connect( mTimeEdit, TQT_SIGNAL(timeChanged(TQTime)), TQT_SLOT(setDirty()) );
  connect( mEditor, TQT_SIGNAL(textChanged()), TQT_SLOT(setDirty()) );

  mEditor->installEventFilter(this);

  readJournal( mJournal );
  mDirty = false;
}

JournalEntry::~JournalEntry()
{
  writeJournal();
}

void JournalEntry::deleteItem()
{
/*  KMessageBox::ButtonCode *code = KMessageBox::warningContinueCancel(this,
      i18n("The journal \"%1\" on %2 will be permanently deleted.")
               .arg( mJournal->summary() )
               .arg( mJournal->dtStartStr() ),
  i18n("KOrganizer Confirmation"), KStdGuiItem::del() );
  if ( code == KMessageBox::Yes ) {*/
    if ( mJournal )
      emit deleteIncidence( mJournal );
//   }
}

void JournalEntry::editItem()
{
  writeJournal();
  if ( mJournal ) {
    emit editIncidence( mJournal, mJournal->dtStart().date() );
  }
}

void JournalEntry::printItem()
{
#ifndef KORG_NOPRINTER
  writeJournal();
  if ( mJournal ) {
    KOCoreHelper helper;
    CalPrinter printer( this, 0, &helper );
    connect( this, TQT_SIGNAL(configChanged()), &printer, TQT_SLOT(updateConfig()) );

    Incidence::List selectedIncidences;
    selectedIncidences.append( mJournal );

    printer.print( KOrg::CalPrinterBase::Incidence,
                 TQDate(), TQDate(), selectedIncidences );
  }
#endif
}

void JournalEntry::setReadOnly( bool readonly )
{
  mReadOnly = readonly;
  mTitleEdit->setReadOnly( mReadOnly );
  mEditor->setReadOnly( mReadOnly );
  mTimeCheck->setEnabled( !mReadOnly );
  mTimeEdit->setEnabled( !mReadOnly && mTimeCheck->isChecked() );
  mDeleteButton->setEnabled( !mReadOnly );
}


void JournalEntry::setDate(const TQDate &date)
{
  writeJournal();
  mDate = date;
}

void JournalEntry::setJournal(Journal *journal)
{
  if ( !mWriteInProgress )
    writeJournal();
  if ( !journal ) return;

  mJournal = journal;
  readJournal( journal );

  mDirty = false;
}

void JournalEntry::setDirty()
{
  mDirty = true;
  kdDebug(5850) << "JournalEntry::setDirty()" << endl;
}

bool JournalEntry::eventFilter( TQObject *o, TQEvent *e )
{
//  kdDebug(5850) << "JournalEntry::event received " << e->type() << endl;

  if ( e->type() == TQEvent::FocusOut || e->type() == TQEvent::Hide ||
       e->type() == TQEvent::Close ) {
    writeJournal();
  }
  return TQWidget::eventFilter( o, e );    // standard event processing
}


void JournalEntry::readJournal( Journal *j )
{
  mJournal = j;
  mTitleEdit->setText( mJournal->summary() );
  bool hasTime = !mJournal->doesFloat();
  mTimeCheck->setChecked( hasTime );
  mTimeEdit->setEnabled( hasTime );
  if ( hasTime ) {
    mTimeEdit->setTime( mJournal->dtStart().time() );
  }
  mEditor->setText( mJournal->description() );
  setReadOnly( mJournal->isReadOnly() );
}

void JournalEntry::writeJournalPrivate( Journal *j )
{
  j->setSummary( mTitleEdit->text() );
  bool hasTime = mTimeCheck->isChecked();
  TQTime tm( mTimeEdit->getTime() );
  j->setDtStart( TQDateTime( mDate, hasTime?tm:TQTime(0,0,0) ) );
  j->setFloats( !hasTime );
  j->setDescription( mEditor->text() );
}

void JournalEntry::writeJournal()
{
//  kdDebug(5850) << "JournalEntry::writeJournal()" << endl;

  if ( mReadOnly || !mDirty || !mChanger ) {
    kdDebug(5850)<<"Journal either read-only, unchanged or no changer object available"<<endl;
    return;
  }
  bool newJournal = false;
  mWriteInProgress = true;

  Journal *oldJournal = 0;

  if ( !mJournal ) {
    newJournal = true;
    mJournal = new Journal;
    writeJournalPrivate( mJournal );
    if ( !mChanger->addIncidence( mJournal, 0, TQString(), this ) ) {
      KODialogManager::errorSaveIncidence( this, mJournal );
      delete mJournal;
      mJournal = 0;
    }
  } else {
    oldJournal = mJournal->clone();
    if ( mChanger->beginChange( mJournal, 0, TQString() ) ) {
      writeJournalPrivate( mJournal );
      mChanger->changeIncidence( oldJournal, mJournal, KOGlobals::DESCRIPTION_MODIFIED, this );
      mChanger->endChange( mJournal, 0, TQString() );
    }
    delete oldJournal;
  }
  mDirty = false;
  mWriteInProgress = false;
}

void JournalEntry::flushEntry()
{
  if (!mDirty) return;

  writeJournal();
}

void JournalEntry::timeCheckBoxToggled(bool on)
{
  mTimeEdit->setEnabled(on);
  if(on)
    mTimeEdit->setFocus();
}
