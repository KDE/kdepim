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
#include <tqwidgetstack.h>
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

#include <libkdepim/kdateedit.h>
#include <libkdepim/categoryselectdialog.h>

#include "koprefs.h"
#include "koglobals.h"
#include "kocore.h"

#include "koeditorgeneral.h"
#include "koeditoralarms.h"
#include "koeditorattachments.h"
#include "koeditorgeneral.moc"
#include "kohelper.h"

KOEditorGeneral::KOEditorGeneral(TQObject* parent, const char* name) :
  TQObject( parent, name ), mAttachments(0)
{
  KOCore::self()->setEditorWindowOpen(true);
  mAlarmList.setAutoDelete( true );
}

KOEditorGeneral::~KOEditorGeneral()
{
  KOCore::self()->setEditorWindowOpen(false);
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

#if 0
  mOwnerLabel = new TQLabel(i18n("Owner:"),parent);
  headerLayout->addMultiCellWidget(mOwnerLabel,0,0,0,1);
#endif

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

void KOEditorGeneral::initAlarm(TQWidget *parent,TQBoxLayout *topLayout)
{
  TQBoxLayout *alarmLayout = new TQHBoxLayout(topLayout);

  mAlarmBell = new TQLabel(parent);
  mAlarmBell->setPixmap(KOGlobals::self()->smallIcon("bell"));
  alarmLayout->addWidget( mAlarmBell );


  mAlarmStack = new TQWidgetStack( parent );
  alarmLayout->addWidget( mAlarmStack );

  mAlarmInfoLabel = new TQLabel( i18n("No reminders configured"), mAlarmStack );
  mAlarmStack->addWidget( mAlarmInfoLabel, AdvancedAlarmLabel );

  TQHBox *simpleAlarmBox = new TQHBox( mAlarmStack );
  mAlarmStack->addWidget( simpleAlarmBox, SimpleAlarmPage );

  mAlarmButton = new TQCheckBox(i18n("&Reminder:"), simpleAlarmBox );
  TQWhatsThis::add( mAlarmButton,
       i18n("Activates a reminder for this event or to-do.") );

  TQString whatsThis = i18n("Sets how long before the event occurs "
                           "the reminder will be triggered.");
  mAlarmTimeEdit = new TQSpinBox( 0, 99999, 1, simpleAlarmBox, "alarmTimeEdit" );
  mAlarmTimeEdit->setValue( 0 );
  TQWhatsThis::add( mAlarmTimeEdit, whatsThis );

  mAlarmIncrCombo = new TQComboBox( false, simpleAlarmBox );
  TQWhatsThis::add( mAlarmIncrCombo, whatsThis );
  mAlarmIncrCombo->insertItem( i18n("minute(s)") );
  mAlarmIncrCombo->insertItem( i18n("hour(s)") );
  mAlarmIncrCombo->insertItem( i18n("day(s)") );
//  mAlarmIncrCombo->setMinimumHeight(20);
  connect(mAlarmButton, TQT_SIGNAL(toggled(bool)), mAlarmTimeEdit, TQT_SLOT(setEnabled(bool)));
  connect(mAlarmButton, TQT_SIGNAL(toggled(bool)), mAlarmIncrCombo, TQT_SLOT(setEnabled(bool)));
  mAlarmTimeEdit->setEnabled( false );
  mAlarmIncrCombo->setEnabled( false );

  mAlarmEditButton = new TQPushButton( i18n("Advanced"), parent );
  mAlarmEditButton->setEnabled( false );
  alarmLayout->addWidget( mAlarmEditButton );
  connect( mAlarmButton, TQT_SIGNAL(toggled(bool)), mAlarmEditButton, TQT_SLOT(setEnabled( bool)));
  connect( mAlarmEditButton, TQT_SIGNAL( clicked() ),
      TQT_SLOT( editAlarms() ) );

}

void KOEditorGeneral::initAttachments(TQWidget *parent,TQBoxLayout *topLayout)
{
  mAttachments = new KOEditorAttachments( KDialog::spacingHint(), parent );
  connect( mAttachments, TQT_SIGNAL( openURL( const KURL & ) ) ,
           this, TQT_SIGNAL( openURL( const KURL & ) ) );
  topLayout->addWidget( mAttachments, 1 );
}

void KOEditorGeneral::addAttachments( const TQStringList &attachments,
                                      const TQStringList &mimeTypes,
                                      bool inlineAttachments )
{
  TQStringList::ConstIterator it;
  uint i = 0;
  for ( it = attachments.begin(); it != attachments.end(); ++it, ++i ) {
    TQString mimeType;
    if ( mimeTypes.count() > i )
      mimeType = mimeTypes[ i ];
    mAttachments->addAttachment( *it, mimeType, !inlineAttachments );
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
  if ( mAlarmStack->id( mAlarmStack->visibleWidget() ) == SimpleAlarmPage ) {
    mAlarmList.clear();
    Alarm *al = alarmFromSimplePage();
    if ( al ) {
      mAlarmList.append( al );
    }
  }

  KOEditorAlarms *dlg = new KOEditorAlarms( &mAlarmList, mAlarmEditButton );
  if ( dlg->exec() != KDialogBase::Cancel ) {
    updateAlarmWidgets();
  }
}


void KOEditorGeneral::enableAlarm( bool enable )
{
  mAlarmStack->setEnabled( enable );
  mAlarmEditButton->setEnabled( enable );
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
#if 0
  mOwnerLabel->setText(i18n("Owner: ") + KOPrefs::instance()->fullName());
#endif

  mAlarmList.clear();
  updateDefaultAlarmTime();
  updateAlarmWidgets();

  mSecrecyCombo->setCurrentItem(Incidence::SecrecyPublic);
  mAttachments->setDefaults();
}

void KOEditorGeneral::updateDefaultAlarmTime()
{
  // FIXME: Implement a KPrefsComboItem to solve this in a clean way.
// FIXME: Use an int value for minutes instead of 5 hardcoded values
  int alarmTime;
  int a[] = { 1,5,10,15,30 };
  int index = KOPrefs::instance()->mAlarmTime;
  if (index < 0 || index > 4) {
    alarmTime = 0;
  } else {
    alarmTime = a[index];
  }
  mAlarmTimeEdit->setValue(alarmTime);
}

void KOEditorGeneral::updateAlarmWidgets()
{
  if ( mAlarmList.isEmpty() ) {
    mAlarmStack->raiseWidget( SimpleAlarmPage );
    if (KOPrefs::instance()->mAlarmsEnabledByDefault == true) {
      mAlarmButton->setChecked( true );
    }
    else {
      mAlarmButton->setChecked( false );
    }
    mAlarmEditButton->setEnabled( false );
  } else if ( mAlarmList.count() > 1 ) {
    mAlarmStack->raiseWidget( AdvancedAlarmLabel );
    mAlarmInfoLabel->setText( i18n("1 advanced reminder configured",
                                   "%n advanced reminders configured",
                                   mAlarmList.count() ) );
    mAlarmEditButton->setEnabled( true );
  } else {
    Alarm *alarm = mAlarmList.first();
    // Check if its the trivial type of alarm, which can be
    // configured with a simply spin box...

    if ( alarm->type() == Alarm::Display && alarm->text().isEmpty()
         && alarm->repeatCount() == 0 && !alarm->hasTime()
         && alarm->hasStartOffset() && alarm->startOffset().asSeconds() < 0 )  {
      mAlarmStack->raiseWidget( SimpleAlarmPage );
      mAlarmButton->setChecked( true );
      int offset = alarm->startOffset().asSeconds();

      offset = offset / -60; // make minutes
      int useoffset = offset;
      if (offset % (24*60) == 0) { // divides evenly into days?
        useoffset = offset / (24*60);
        mAlarmIncrCombo->setCurrentItem(2);
      } else if (offset % 60 == 0) { // divides evenly into hours?
        useoffset = offset / 60;
        mAlarmIncrCombo->setCurrentItem(1);
      }
      mAlarmTimeEdit->setValue( useoffset );
    } else {
      mAlarmStack->raiseWidget( AdvancedAlarmLabel );
      mAlarmInfoLabel->setText( i18n("1 advanced reminder configured") );
      mAlarmEditButton->setEnabled( true );
    }
  }
}

void KOEditorGeneral::readIncidence(Incidence *event, Calendar *calendar)
{
  mSummaryEdit->setText(event->summary());
  mLocationEdit->setText(event->location());

  mDescriptionEdit->setText(event->description());

#if 0
  // organizer information
  mOwnerLabel->setText(i18n("Owner: ") + event->organizer().fullName() );
#endif

  mSecrecyCombo->setCurrentItem(event->secrecy());

  // set up alarm stuff
  mAlarmList.clear();
  Alarm::List::ConstIterator it;
  Alarm::List alarms = event->alarms();
  for( it = alarms.begin(); it != alarms.end(); ++it ) {
    Alarm *al = new Alarm( *(*it) );
    al->setParent( 0 );
    mAlarmList.append( al );
  }
  updateDefaultAlarmTime();
  updateAlarmWidgets();

  setCategories(event->categories());

  mAttachments->readIncidence( event );

  TQString resLabel = KOHelper::resourceLabel( calendar, event );
  if ( !resLabel.isEmpty() ) {
    mResourceLabel->setText( i18n( "Calendar: %1" ).arg( resLabel ) );
    mResourceLabel->show();
  }
}

Alarm *KOEditorGeneral::alarmFromSimplePage() const
{
  if ( mAlarmButton->isChecked() ) {
    Alarm *alarm = new Alarm( 0 );
    alarm->setDisplayAlarm("");
    alarm->setEnabled(true);
    TQString tmpStr = mAlarmTimeEdit->text();
    int j = mAlarmTimeEdit->value() * -60;
    if (mAlarmIncrCombo->currentItem() == 1)
      j = j * 60;
    else if (mAlarmIncrCombo->currentItem() == 2)
      j = j * (60 * 24);
    alarm->setStartOffset( j );
    return alarm;
  } else {
    return 0;
  }
}
void KOEditorGeneral::writeIncidence(Incidence *event)
{
//  kdDebug(5850) << "KOEditorGeneral::writeEvent()" << endl;

  event->setSummary(mSummaryEdit->text());
  event->setLocation(mLocationEdit->text());
  event->setDescription(mDescriptionEdit->text());
  event->setCategories(mCategories);
  event->setSecrecy(mSecrecyCombo->currentItem());

  // alarm stuff
  event->clearAlarms();
  if ( mAlarmStack->id( mAlarmStack->visibleWidget() ) == SimpleAlarmPage ) {
    Alarm *al = alarmFromSimplePage();
    if ( al ) {
      al->setParent( event );
      event->addAlarm( al );
    }
  } else {
    // simply assign the list of alarms
    Alarm::List::ConstIterator it;
    for( it = mAlarmList.begin(); it != mAlarmList.end(); ++it ) {
      Alarm *al = new Alarm( *(*it) );
      al->setParent( event );
      al->setEnabled( true );
      event->addAlarm( al );
    }
  }
  mAttachments->writeIncidence( event );
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
    mAttendeeSummaryLabel->setText( "No attendees" );
  else
    mAttendeeSummaryLabel->setText( i18n( "One attendee", "%n attendees", count ) );
}
