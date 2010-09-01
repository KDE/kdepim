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


#include <tqwidget.h>
#include <tqtooltip.h>
#include <tqlayout.h>
#include <tqvbox.h>
#include <tqhbox.h>
#include <tqbuttongroup.h>
#include <tqvgroupbox.h>
#include <tqdatetime.h>
#include <tqlineedit.h>
#include <tqlabel.h>
#include <tqcheckbox.h>
#include <tqpushbutton.h>
#include <tqcombobox.h>
#include <tqspinbox.h>
#include <tqwhatsthis.h>

#include <kglobal.h>
#include <kdialog.h>
#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <ksqueezedtextlabel.h>
#include <kstandarddirs.h>
#include <ktextedit.h>
#include <krestrictedline.h>

#include <libkcal/todo.h>
#include <libkcal/event.h>
#include <libkcal/incidenceformatter.h>
#include <libkcal/resourcecached.h>

#include <libkdepim/kdateedit.h>
#include <libkdepim/categoryselectdialog.h>

#include "koprefs.h"
#include "koglobals.h"

#include "koeditorgeneral.h"
#include "koeditoralarms.h"
#include "koeditorattachments.h"
#include "koeditorgeneral.moc"
#include "kohelper.h"

KOEditorGeneral::KOEditorGeneral( TQObject *parent, const char* name) :
  TQObject( parent, name ), mAttachments(0)
{
  mType = "Event";
  ResourceCached::setEditorWindowOpen(true);
  mAlarmList.setAutoDelete( true );
}

KOEditorGeneral::~KOEditorGeneral()
{
  ResourceCached::setEditorWindowOpen(false);
}


FocusLineEdit::FocusLineEdit( TQWidget *parent )
  : TQLineEdit( parent ), mSkipFirst( true )
{
}

void FocusLineEdit::focusInEvent ( TQFocusEvent *e )
{
  if ( !mSkipFirst ) {
    emit focusReceivedSignal();
  } else {
    mSkipFirst = false;
  }
  TQLineEdit::focusInEvent( e );
}


void KOEditorGeneral::initHeader( TQWidget *parent,TQBoxLayout *topLayout)
{
  TQGridLayout *headerLayout = new TQGridLayout();
  headerLayout->setSpacing( topLayout->spacing() );
  topLayout->addLayout( headerLayout );

  TQString whatsThis = i18n("Sets the Title of this event or to-do.");
  TQLabel *summaryLabel = new TQLabel( i18n("T&itle:"), parent );
  TQWhatsThis::add( summaryLabel, whatsThis );
  TQFont f = summaryLabel->font();
  f.setBold( true );
  summaryLabel->setFont(f);
  headerLayout->addWidget(summaryLabel,1,0);

  mSummaryEdit = new FocusLineEdit( parent );
  TQWhatsThis::add( mSummaryEdit, whatsThis );
  connect( mSummaryEdit, TQT_SIGNAL( focusReceivedSignal() ),
           TQT_SIGNAL( focusReceivedSignal() ) );
  headerLayout->addWidget(mSummaryEdit,1,1);
  summaryLabel->setBuddy( mSummaryEdit );

  mAttendeeSummaryLabel = new TQLabel( parent );
  updateAttendeeSummary( 0 );
  headerLayout->addWidget( mAttendeeSummaryLabel, 1, 2 );

  whatsThis = i18n("Sets where the event or to-do will take place.");
  TQLabel *locationLabel = new TQLabel( i18n("&Location:"), parent );
  TQWhatsThis::add( locationLabel, whatsThis );
  headerLayout->addWidget(locationLabel,2,0);

  mLocationEdit = new TQLineEdit( parent );
  TQWhatsThis::add( mLocationEdit, whatsThis );
  headerLayout->addMultiCellWidget( mLocationEdit, 2, 2, 1, 2 );
  locationLabel->setBuddy( mLocationEdit );

  TQBoxLayout *thirdLineLayout = new TQHBoxLayout();
  headerLayout->addMultiCellLayout( thirdLineLayout, 3, 3, 0, 2 );

  mResourceLabel = new TQLabel( parent );
  mResourceLabel->hide();
  thirdLineLayout->addWidget( mResourceLabel );

  whatsThis = i18n("Allows you to select the categories that this event or to-do belongs to.");
  TQLabel *categoriesLabel = new TQLabel( i18n("Categories:"), parent );
  TQWhatsThis::add( categoriesLabel, whatsThis );
  thirdLineLayout->addWidget( categoriesLabel );
  mCategoriesLabel = new KSqueezedTextLabel( parent );
  TQWhatsThis::add( mCategoriesLabel, whatsThis );
  mCategoriesLabel->setFrameStyle(TQFrame::Panel|TQFrame::Sunken);
  thirdLineLayout->addWidget( mCategoriesLabel );

  mCategoriesButton = new TQPushButton( parent );
  mCategoriesButton->setText(i18n("&Select..."));
  TQWhatsThis::add( mCategoriesButton, whatsThis );
  connect(mCategoriesButton,TQT_SIGNAL(clicked()),TQT_SLOT(selectCategories()));
  thirdLineLayout->addWidget( mCategoriesButton );
}

void KOEditorGeneral::initSecrecy(TQWidget *parent, TQBoxLayout *topLayout)
{
  TQBoxLayout *secrecyLayout = new TQHBoxLayout( topLayout );

  TQLabel *secrecyLabel = new TQLabel(i18n("Acc&ess:"),parent);
  TQString whatsThis = i18n("Sets whether the access to this event or to-do "
  			   "is restricted. Please note that KOrganizer "
			   "currently does not use this setting, so the "
			   "implementation of the restrictions will depend "
			   "on the groupware server. This means that events "
			   "or to-dos marked as private or confidential may "
			   "be visible to others.");
  TQWhatsThis::add( secrecyLabel, whatsThis );
  secrecyLayout->addWidget(secrecyLabel);

  mSecrecyCombo = new TQComboBox(parent);
  TQWhatsThis::add( mSecrecyCombo, whatsThis );
  mSecrecyCombo->insertStringList(Incidence::secrecyList());
  secrecyLayout->addWidget(mSecrecyCombo);
  secrecyLabel->setBuddy( mSecrecyCombo );
}

void KOEditorGeneral::initDescription(TQWidget *parent,TQBoxLayout *topLayout)
{
  mDescriptionEdit = new KTextEdit(parent);
  TQWhatsThis::add( mDescriptionEdit,
		   i18n("Sets the description for this event or to-do. This "
			"will be displayed in a reminder if one is set, "
			"as well as in a tooltip when you hover over the "
			"event.") );
  mDescriptionEdit->append("");
  mDescriptionEdit->setReadOnly(false);
  mDescriptionEdit->setOverwriteMode(false);
  mDescriptionEdit->setWordWrap( KTextEdit::WidgetWidth );
  mDescriptionEdit->setTabChangesFocus( true );;
  topLayout->addWidget(mDescriptionEdit, 4);
}

void KOEditorGeneral::initAlarm( TQWidget *parent, TQBoxLayout *topLayout )
{
  TQBoxLayout *alarmLayout = new TQHBoxLayout( topLayout );

  mAlarmButton = new TQCheckBox( parent );
  TQWhatsThis::add( mAlarmButton, i18n( "Enable reminders for this event or to-do." ) );
  TQToolTip::add( mAlarmButton, i18n( "Enable reminders" ) );
  alarmLayout->addWidget( mAlarmButton );

  mAlarmAdvancedButton = new TQPushButton( parent );
  mAlarmAdvancedButton->setIconSet( KOGlobals::self()->smallIconSet( "bell", 16 ) );
  TQWhatsThis::add( mAlarmAdvancedButton,
                   i18n( "Push this button to create an advanced set of reminders "
                         "for this event or to-do." ) );
  TQToolTip::add( mAlarmAdvancedButton, i18n( "Set an advanced reminder" ) );
  connect( mAlarmAdvancedButton, TQT_SIGNAL(clicked()), TQT_SLOT(editAlarms()) );
  alarmLayout->addWidget( mAlarmAdvancedButton );

  mSimpleAlarmBox = new TQHBox( parent );
  alarmLayout->addWidget( mSimpleAlarmBox );

  TQString whatsThis, toolTip;
  if ( mType == "Event" ) {
    whatsThis = i18n( "Set the time before the event starts when the reminder will be triggered." );
    toolTip = i18n( "Set the start time trigger offset" );
  } else {
    whatsThis = i18n( "Set the time before the to-do is due when the reminder will be triggered." );
    toolTip = i18n( "Set the due time trigger offset" );
  }
  mAlarmTimeEdit = new TQSpinBox( 0, 99999, 1, mSimpleAlarmBox, "alarmTimeEdit" );
  mAlarmTimeEdit->setValue( 0 );
  TQWhatsThis::add( mAlarmTimeEdit, whatsThis );
  TQToolTip::add( mAlarmTimeEdit, toolTip );

  mAlarmIncrCombo = new TQComboBox( false, mSimpleAlarmBox );
  mAlarmIncrCombo->insertItem( i18n("minute(s)") );
  mAlarmIncrCombo->insertItem( i18n("hour(s)") );
  mAlarmIncrCombo->insertItem( i18n("day(s)") );
  TQWhatsThis::add( mAlarmIncrCombo, whatsThis );
  TQToolTip::add( mAlarmIncrCombo, toolTip );

  mAlarmInfoLabel = new TQLabel( parent );
  if ( mType == "Event" ) {
    mAlarmInfoLabel->setText( i18n( "before the start" ) );
  } else {
    mAlarmInfoLabel->setText( i18n( "before the due time" ) );
  }
  alarmLayout->addWidget( mAlarmInfoLabel );

  mAlarmAdvancedButton->setEnabled( false );
  mAlarmTimeEdit->setEnabled( false );
  mAlarmIncrCombo->setEnabled( false );
  mAlarmInfoLabel->setEnabled( false );
  connect( mAlarmButton, TQT_SIGNAL(toggled(bool)), mAlarmAdvancedButton, TQT_SLOT(setEnabled(bool)) );
  connect( mAlarmButton, TQT_SIGNAL(toggled(bool)), mAlarmTimeEdit, TQT_SLOT(setEnabled(bool)) );
  connect( mAlarmButton, TQT_SIGNAL(toggled(bool)), mAlarmIncrCombo, TQT_SLOT(setEnabled(bool)) );
  connect( mAlarmButton, TQT_SIGNAL(toggled(bool)), mAlarmInfoLabel, TQT_SLOT(setEnabled(bool)) );
}

void KOEditorGeneral::initAttachments(TQWidget *parent,TQBoxLayout *topLayout)
{
  mAttachments = new KOEditorAttachments( KDialog::spacingHint(), parent );
  connect( mAttachments, TQT_SIGNAL( openURL( const KURL & ) ) ,
           this, TQT_SIGNAL( openURL( const KURL & ) ) );
  topLayout->addWidget( mAttachments, 1 );
}

void KOEditorGeneral::setType( const TQCString &type )
{
  // must be "Event", "Todo", "Journal", etc.
  mType = type;
}

void KOEditorGeneral::addAttachments( const TQStringList &attachments,
                                      const TQStringList &mimeTypes,
                                      bool inlineAttachments )
{
  TQStringList::ConstIterator it;
  uint i = 0;
  for ( it = attachments.begin(); it != attachments.end(); ++it, ++i ) {
    if ( !(*it).isEmpty() ) {
      TQString mimeType;
      if ( mimeTypes.count() > i ) {
        mimeType = mimeTypes[ i ];
      }
      mAttachments->addUriAttachment( *it, mimeType, TQString(), inlineAttachments );
    }
  }
}

void KOEditorGeneral::selectCategories()
{
  KPIM::CategorySelectDialog *categoryDialog = new KPIM::CategorySelectDialog( KOPrefs::instance(), mCategoriesButton	 );
  KOGlobals::fitDialogToScreen( categoryDialog );
  categoryDialog->setSelected( mCategories );

  connect(categoryDialog, TQT_SIGNAL(editCategories()), this, TQT_SIGNAL(openCategoryDialog()));
  connect(this, TQT_SIGNAL(updateCategoryConfig()), categoryDialog, TQT_SLOT(updateCategoryConfig()));

  if ( categoryDialog->exec() ) {
    setCategories( categoryDialog->selectedCategories() );
  }
  delete categoryDialog;
}


void KOEditorGeneral::editAlarms()
{
  if ( mAlarmIsSimple ) {
    mAlarmList.clear();
    Alarm *al = alarmFromSimplePage( 0 );
    if ( al ) {
      mAlarmList.append( al );
    }
  }

  KOEditorAlarms *dlg = new KOEditorAlarms( mType, &mAlarmList, mAlarmAdvancedButton );
  if ( dlg->exec() != KDialogBase::Cancel ) {
    if ( mType == "Event" ) {
      Event *e = new Event;
      Alarm::List::ConstIterator it;
      for( it = mAlarmList.begin(); it != mAlarmList.end(); ++it ) {
        Alarm *a = (*it)->clone();
        a->setParent( e );
        e->addAlarm( a );
      }
      updateAlarmWidgets( e );
      delete e;
    } else {
      Todo *t = new Todo;
      Alarm::List::ConstIterator it;
      for( it = mAlarmList.begin(); it != mAlarmList.end(); ++it ) {
        Alarm *a = (*it)->clone();
        a->setParent( t );
        t->addAlarm( a );
      }
      updateAlarmWidgets( t );
      delete t;
    }
  }
}

void KOEditorGeneral::enableAlarm( bool enable )
{
  mAlarmAdvancedButton->setEnabled( enable );
}

void KOEditorGeneral::toggleAlarm( bool on )
{
  mAlarmButton->setChecked( on );
}

void KOEditorGeneral::setCategories( const TQStringList &categories )
{
  mCategoriesLabel->setText( categories.join(",") );
  mCategories = categories;
}

void KOEditorGeneral::setDefaults(bool /*allDay*/)
{
  mAlarmList.clear();
  updateDefaultAlarmTime();
  updateAlarmWidgets( 0 );

  mSecrecyCombo->setCurrentItem( Incidence::SecrecyPublic );
  mAttachments->setDefaults();
}

void KOEditorGeneral::updateDefaultAlarmTime()
{
  int reminderTime = KOPrefs::instance()->mReminderTime;
  int index = KOPrefs::instance()->mReminderTimeUnits;
  if ( index < 0 || index > 2 ) {
    index = 0;
  }
  mAlarmTimeEdit->setValue( reminderTime );
  mAlarmIncrCombo->setCurrentItem( index );
}

bool KOEditorGeneral::isSimpleAlarm( Alarm *alarm ) const
{
  // Check if its the trivial type of alarm, which can be
  // configured with a simply spin box...

  bool simple = false;
  if ( alarm->type() == Alarm::Display && alarm->text().isEmpty() &&
       alarm->repeatCount() == 0 && !alarm->hasTime() ) {
    if ( mType == "Event" &&
         alarm->hasStartOffset() && alarm->startOffset().asSeconds() <= 0 ) {
      simple = true;
    }
    if ( mType == "Todo" &&
         alarm->hasEndOffset() && alarm->endOffset().asSeconds() <= 0 ) {
      simple = true;
    }
  }
  return simple;
}

static TQString etc = i18n( "elipsis", "..." );
void KOEditorGeneral::updateAlarmWidgets( Incidence *incidence )
{
  uint maxLen = 75; //TODO: compute from the font and dialog width

  if ( incidence ) {
    mAlarmButton->setChecked( incidence->isAlarmEnabled() );
  }

  if ( mAlarmList.isEmpty() ) {
    mAlarmIsSimple = true;
    mSimpleAlarmBox->show();
    bool on;
    if ( mType == "Event" ) {
      on = KOPrefs::instance()->defaultEventReminders();
    } else if ( mType == "Todo" ) {
      on = KOPrefs::instance()->defaultTodoReminders();
    } else {
      on = false;
    }
    mAlarmButton->setChecked( on );
    mAlarmAdvancedButton->setEnabled( on );
  } else if ( mAlarmList.count() > 1 ) {
    mAlarmIsSimple = false;
    mAlarmAdvancedButton->setEnabled( true );
    mSimpleAlarmBox->hide();
    if ( incidence ) {
      TQString remStr = IncidenceFormatter::reminderStringList( incidence ).join( ", " );
      if ( remStr.length() > maxLen ) {
        maxLen -= etc.length();
        remStr = remStr.left( maxLen );
        remStr += etc;
      }
      mAlarmInfoLabel->setText( i18n( "Triggers %1" ).arg( remStr ) );
    }
  } else {  // alarm count is 1
    Alarm *alarm = mAlarmList.first();
    if ( isSimpleAlarm( alarm ) ) {
      mAlarmIsSimple = true;
      mSimpleAlarmBox->show();
      int offset;
      if ( mType == "Event" ) {
        offset = alarm->startOffset().asSeconds();
        mAlarmInfoLabel->setText( i18n( "before the start" ) );
      }
      if ( mType == "Todo" ) {
        if ( alarm->hasStartOffset() ) {
          offset = alarm->startOffset().asSeconds();
          mAlarmInfoLabel->setText( i18n( "before the start" ) );
        } else {
          offset = alarm->endOffset().asSeconds();
          mAlarmInfoLabel->setText( i18n( "before the due time" ) );
        }
      }
      offset = offset / -60; // make minutes
      int useoffset = offset;
      if ( offset == 0 ) {
        mAlarmIncrCombo->setCurrentItem( 0 ); // use minute units for 0 offset
      } else if (offset % (24*60) == 0) { // divides evenly into days?
        useoffset = offset / (24*60);
        mAlarmIncrCombo->setCurrentItem(2);
      } else if (offset % 60 == 0) { // divides evenly into hours?
        useoffset = offset / 60;
        mAlarmIncrCombo->setCurrentItem(1);
      }
      mAlarmTimeEdit->setValue( useoffset );
    } else {
      mAlarmIsSimple = false;
      mAlarmAdvancedButton->setEnabled( true );
      mSimpleAlarmBox->hide();
      if ( incidence ) {
        TQString remStr = IncidenceFormatter::reminderStringList( incidence ).first();
        mAlarmInfoLabel->setText( i18n( "Triggers %1" ).arg( remStr ) );
      }
    }
  }
}

void KOEditorGeneral::readIncidence( Incidence *incidence, Calendar *calendar )
{
  mSummaryEdit->setText( incidence->summary() );
  mLocationEdit->setText( incidence->location() );
  mDescriptionEdit->setText( incidence->description() );

  mSecrecyCombo->setCurrentItem( incidence->secrecy() );

  // set up alarm stuff
  mAlarmList.clear();
  Alarm::List::ConstIterator it;
  Alarm::List alarms = incidence->alarms();
  for( it = alarms.begin(); it != alarms.end(); ++it ) {
    Alarm *al = new Alarm( *(*it) );
    al->setParent( 0 );
    mAlarmList.append( al );
  }
  updateDefaultAlarmTime();
  updateAlarmWidgets( incidence );

  setCategories( incidence->categories() );

  mAttachments->readIncidence( incidence );

  TQString resLabel = IncidenceFormatter::resourceString( calendar, incidence );
  if ( !resLabel.isEmpty() ) {
    mResourceLabel->setText( i18n( "Calendar: %1" ).arg( resLabel ) );
    mResourceLabel->show();
  }
}

Alarm *KOEditorGeneral::alarmFromSimplePage( Incidence *incidence ) const
{
  if ( mAlarmButton->isChecked() ) {
    Alarm *alarm = new Alarm( 0 );
    alarm->setDisplayAlarm( "" );
    alarm->setEnabled(true);
    TQString tmpStr = mAlarmTimeEdit->text();
    int j = mAlarmTimeEdit->value() * -60;
    if ( mAlarmIncrCombo->currentItem() == 1 ) {
      j = j * 60;
    } else if ( mAlarmIncrCombo->currentItem() == 2 ) {
      j = j * (60 * 24);
    }
    if ( mType == "Event" ) {
      alarm->setStartOffset( j );
    }
    if ( mType == "Todo" ) {
      Todo *todo = static_cast<Todo *>( incidence );
      if ( todo && todo->hasStartDate() && !todo->hasDueDate() ) {
        alarm->setStartOffset( j );
      } else {
        alarm->setEndOffset( j );
      }
    }
    return alarm;
  } else {
    return 0;
  }
}
void KOEditorGeneral::writeIncidence( Incidence *incidence )
{
  incidence->setSummary(mSummaryEdit->text());
  incidence->setLocation(mLocationEdit->text());
  incidence->setDescription(mDescriptionEdit->text());
  incidence->setCategories(mCategories);
  incidence->setSecrecy(mSecrecyCombo->currentItem());

  // alarm stuff
  incidence->clearAlarms();
  if ( mAlarmIsSimple ) {
    Alarm *al = alarmFromSimplePage( incidence );
    if ( al ) {
      al->setParent( incidence );
      al->setEnabled( mAlarmButton->isChecked() );
      incidence->addAlarm( al );
    }
  } else {
    // simply assign the list of alarms
    Alarm::List::ConstIterator it;
    for( it = mAlarmList.begin(); it != mAlarmList.end(); ++it ) {
      Alarm *al = new Alarm( *(*it) );
      al->setParent( incidence );
      al->setEnabled( mAlarmButton->isChecked() );
      incidence->addAlarm( al );
    }
  }
  mAttachments->writeIncidence( incidence );
}

void KOEditorGeneral::setSummary( const TQString &text )
{
  mSummaryEdit->setText( text );
}

void KOEditorGeneral::setDescription( const TQString &text )
{
  mDescriptionEdit->setText( text );
}

TQObject *KOEditorGeneral::typeAheadReceiver() const
{
  return mSummaryEdit;
}

void KOEditorGeneral::updateAttendeeSummary(int count)
{
  if ( count <= 0 )
    mAttendeeSummaryLabel->setText( i18n("No attendees") );
  else
    mAttendeeSummaryLabel->setText( i18n( "One attendee", "%n attendees", count ) );
}
