/*
    This file is part of KOrganizer.

    Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
		large parts of code taken from KOEditorGeneralJournal.cpp:
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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

#include "koeditorgeneraljournal.h"
#include "koeditorgeneral.h"

#include <libkcal/journal.h>

#include <ktextedit.h>
#include <kdateedit.h>
#include <ktimeedit.h>
//#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>

#include <tqgroupbox.h>
#include <tqdatetime.h>
#include <tqcheckbox.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqwhatsthis.h>


KOEditorGeneralJournal::KOEditorGeneralJournal( TQWidget *parent,
                                                const char *name )
  : KOEditorGeneral( parent, name )
{
  setType( "Journal" );
}

KOEditorGeneralJournal::~KOEditorGeneralJournal()
{
}

void KOEditorGeneralJournal::initTitle( TQWidget *parent, TQBoxLayout *topLayout )
{
  TQHBoxLayout *hbox = new TQHBoxLayout( topLayout );

  TQString whatsThis = i18n("Sets the title of this journal.");
  TQLabel *summaryLabel = new TQLabel( i18n("T&itle:"), parent );
  TQWhatsThis::add( summaryLabel, whatsThis );
  TQFont f = summaryLabel->font();
  f.setBold( true );
  summaryLabel->setFont( f );
  hbox->addWidget( summaryLabel );

  mSummaryEdit = new FocusLineEdit( parent );
  TQWhatsThis::add( mSummaryEdit, whatsThis );
  summaryLabel->setBuddy( mSummaryEdit );
  hbox->addWidget( mSummaryEdit );
}


void KOEditorGeneralJournal::initDate( TQWidget *parent, TQBoxLayout *topLayout )
{
//  TQBoxLayout *dateLayout = new TQVBoxLayout(topLayout);
  TQBoxLayout *dateLayout = new TQHBoxLayout( topLayout );

  mDateLabel = new TQLabel( i18n("&Date:"), parent);
  dateLayout->addWidget( mDateLabel );

  mDateEdit = new KDateEdit( parent );
  dateLayout->addWidget( mDateEdit );
  mDateLabel->setBuddy( mDateEdit );

  dateLayout->addStretch();

  mTimeCheckBox = new TQCheckBox( i18n("&Time: "), parent );
  dateLayout->addWidget( mTimeCheckBox );

  mTimeEdit = new KTimeEdit( parent );
  dateLayout->addWidget( mTimeEdit );
  connect( mTimeCheckBox, TQT_SIGNAL(toggled(bool)),
           mTimeEdit, TQT_SLOT(setEnabled(bool)) );

  dateLayout->addStretch();
  setTime( TQTime( -1, -1, -1 ) );
}

void KOEditorGeneralJournal::setDate( const TQDate &date )
{
//  kdDebug(5850) << "KOEditorGeneralJournal::setDate(): Date: " << date.toString() << endl;

  mDateEdit->setDate( date );
}

void KOEditorGeneralJournal::setTime( const TQTime &time )
{
kdDebug()<<"KOEditorGeneralJournal::setTime, time="<<time.toString()<<endl;
  bool validTime = time.isValid();
  mTimeCheckBox->setChecked( validTime );
  mTimeEdit->setEnabled( validTime );
  if ( validTime ) {
kdDebug()<<"KOEditorGeneralJournal::setTime, time is valid"<<endl;
    mTimeEdit->setTime( time );
  }
}

void KOEditorGeneralJournal::initDescription( TQWidget *parent, TQBoxLayout *topLayout )
{
  mDescriptionEdit = new KTextEdit( parent );
  mDescriptionEdit->append("");
  mDescriptionEdit->setReadOnly( false );
  mDescriptionEdit->setOverwriteMode( false );
  mDescriptionEdit->setWordWrap( KTextEdit::WidgetWidth );
  mDescriptionEdit->setTabChangesFocus( true );
  topLayout->addWidget( mDescriptionEdit );
}

void KOEditorGeneralJournal::setDefaults( const TQDate &date )
{
  setDate( date );
}

void KOEditorGeneralJournal::readJournal( Journal *journal, const TQDate &, bool tmpl )
{
  setSummary( journal->summary() );
  if ( !tmpl ) {
    setDate( journal->dtStart().date() );
    if ( !journal->doesFloat() ) {
kdDebug()<<"KOEditorGeneralJournal::readJournal, does not float, time="<<(journal->dtStart().time().toString())<<endl;
      setTime( journal->dtStart().time() );
    } else {
kdDebug()<<"KOEditorGeneralJournal::readJournal, does float"<<endl;
      setTime( TQTime( -1, -1, -1 ) );
    }
  }
  setDescription( journal->description() );
}

void KOEditorGeneralJournal::writeJournal( Journal *journal )
{
//  kdDebug(5850) << "KOEditorGeneralJournal::writeIncidence()" << endl;
  journal->setSummary( mSummaryEdit->text() );
  journal->setDescription( mDescriptionEdit->text() );

  TQDateTime tmpDT( mDateEdit->date(), TQTime(0,0,0) );
  bool hasTime = mTimeCheckBox->isChecked();
  journal->setFloats( !hasTime );
  if ( hasTime ) {
    tmpDT.setTime( mTimeEdit->getTime() );
  }
  journal->setDtStart(tmpDT);

//  kdDebug(5850) << "KOEditorGeneralJournal::writeJournal() done" << endl;
}


void KOEditorGeneralJournal::setDescription( const TQString &text )
{
  mDescriptionEdit->setText( text );
}

void KOEditorGeneralJournal::setSummary( const TQString &text )
{
  mSummaryEdit->setText( text );
}

void KOEditorGeneralJournal::finishSetup()
{
  TQWidget::setTabOrder( mSummaryEdit, mDateEdit );
  TQWidget::setTabOrder( mDateEdit, mTimeCheckBox );
  TQWidget::setTabOrder( mTimeCheckBox, mTimeEdit );
  TQWidget::setTabOrder( mTimeEdit, mDescriptionEdit );
  mSummaryEdit->setFocus();
}

bool KOEditorGeneralJournal::validateInput()
{
//  kdDebug(5850) << "KOEditorGeneralJournal::validateInput()" << endl;

  if (!mDateEdit->date().isValid()) {
    KMessageBox::sorry( 0,
        i18n("Please specify a valid date, for example '%1'.")
        .arg( KGlobal::locale()->formatDate( TQDate::currentDate() ) ) );
    return false;
  }

  return true;
}

#include "koeditorgeneraljournal.moc"
