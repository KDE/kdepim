/*
    This file is part of KOrganizer.

    Copyright (c) 2000-2003 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include <tqlayout.h>
#include <tqlabel.h>
#include <tqgroupbox.h>
#include <tqbuttongroup.h>
#include <tqlineedit.h>
#include <tqslider.h>
#include <tqfile.h>
#include <tqcombobox.h>
#include <tqhbox.h>
#include <tqvbox.h>
#include <tqgrid.h>
#include <tqspinbox.h>
#include <tqcheckbox.h>
#include <tqradiobutton.h>
#include <tqpushbutton.h>
#include <tqstrlist.h>
#include <tqlistview.h>
#include <tqtabwidget.h>
#include <tqwhatsthis.h>

#include <kcolorbutton.h>
#include <kcombobox.h>
#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <knuminput.h>
#include <kemailsettings.h>
#include <kcalendarsystem.h>
#include <ktrader.h>
#include <kpushbutton.h>
#include <kocore.h>
#include <kstandarddirs.h>
#include <ksimpleconfig.h>
#include <kholidays.h>
#include <kurlrequester.h>

#include <libkcal/calendarresources.h>

#if defined(USE_SOLARIS)
#include <sys/param.h>

#define ZONEINFODIR    "/usr/share/lib/zoneinfo"
#define INITFILE       "/etc/default/init"
#endif

#include "koprefs.h"

#include "koprefsdialog.h"
#include "kogroupwareprefspage.h"
#include "ktimeedit.h"
#include "koglobals.h"
#include "stdcalendar.h"
#include <kdepimmacros.h>


KOPrefsDialogMain::KOPrefsDialogMain( TQWidget *parent, const char *name )
  : KPrefsModule( KOPrefs::instance(), parent, name )
{
  TQBoxLayout *topTopLayout = new TQVBoxLayout( this );

  TQVBox *topFrame = new TQVBox( this );
  topTopLayout->addWidget( topFrame );

  topFrame->setSpacing( KDialog::spacingHint() );

  KPrefsWidBool *emailControlCenter =
      addWidBool( KOPrefs::instance()->emailControlCenterItem(), topFrame );
  connect(emailControlCenter->checkBox(),TQT_SIGNAL(toggled(bool)),
          TQT_SLOT(toggleEmailSettings(bool)));

  mUserEmailSettings = new TQGrid( 2, topFrame );

  addWidString( KOPrefs::instance()->userNameItem(), mUserEmailSettings );
  addWidString( KOPrefs::instance()->userEmailItem(), mUserEmailSettings );

  TQGroupBox *saveGroup = new TQGroupBox(1,Horizontal,i18n("Saving Calendar"),
                                           topFrame);

  addWidBool( KOPrefs::instance()->htmlWithSaveItem(), saveGroup );

  KPrefsWidBool *autoSave = addWidBool( KOPrefs::instance()->autoSaveItem(), saveGroup );

  TQHBox *intervalBox = new TQHBox( saveGroup );
  addWidInt( KOPrefs::instance()->autoSaveIntervalItem(), intervalBox );
  connect( autoSave->checkBox(), TQT_SIGNAL( toggled( bool ) ),
           intervalBox, TQT_SLOT( setEnabled( bool ) ) );
  intervalBox->setSpacing( KDialog::spacingHint() );
  new TQWidget( intervalBox );

  addWidBool( KOPrefs::instance()->confirmItem(), topFrame );
  addWidRadios( KOPrefs::instance()->destinationItem(), topFrame);
  addWidRadios( KOPrefs::instance()->defaultEmailAttachMethodItem(), topFrame );

  topTopLayout->addStretch( 1 );

  load();
}

void KOPrefsDialogMain::toggleEmailSettings( bool on )
{
  mUserEmailSettings->setEnabled( !on );
/*  if (on) {
    KEMailSettings settings;
    mNameEdit->setText( settings.getSetting(KEMailSettings::RealName) );
    mEmailEdit->setText( settings.getSetting(KEMailSettings::EmailAddress) );
  } else {
    mNameEdit->setText( KOPrefs::instance()->mName );
    mEmailEdit->setText( KOPrefs::instance()->mEmail );
  }*/
}

extern "C"
{
  KDE_EXPORT KCModule *create_korganizerconfigmain( TQWidget *parent, const char * )
  {
    return new KOPrefsDialogMain( parent, "kcmkorganizermain" );
  }
}


class KOPrefsDialogTime : public KPrefsModule
{
  public:
    KOPrefsDialogTime( TQWidget *parent, const char *name )
      : KPrefsModule( KOPrefs::instance(), parent, name )
    {
      TQBoxLayout *topTopLayout = new TQVBoxLayout( this );

      TQWidget *topFrame = new TQWidget( this );
      topTopLayout->addWidget( topFrame );

      TQGridLayout *topLayout = new TQGridLayout(topFrame,6,2);
      topLayout->setSpacing( KDialog::spacingHint() );

      TQHBox *timeZoneBox = new TQHBox( topFrame );
      topLayout->addMultiCellWidget( timeZoneBox, 0, 0, 0, 1 );

      TQLabel *timeZoneLabel = new TQLabel( i18n("Timezone:"), timeZoneBox );
      TQString whatsThis = i18n( "Select your timezone from the list of "
                                "locations on this drop down box. If your city "
                                "is not listed, select one which shares the "
                                "same timezone. KOrganizer will automatically "
                                "adjust for daylight savings." );
      TQWhatsThis::add( timeZoneLabel, whatsThis );
      mTimeZoneCombo = new TQComboBox( timeZoneBox );

      connect( mTimeZoneCombo, TQT_SIGNAL( activated( int ) ),
               TQT_SLOT( slotWidChanged() ) );

      FILE *f;
      char tempstring[101] = "Unknown";
      TQString sCurrentlySet(i18n("Unknown"));
      int nCurrentlySet = 0;
      TQStringList list;

      // read the currently set time zone
    #if defined(USE_SOLARIS)       // MARCO
        char buf[MAXPATHLEN];

        snprintf(buf, MAXPATHLEN,
                "/bin/fgrep 'TZ=' %s | /bin/head -n 1 | /bin/cut -b 4-",
                INITFILE);

        if (f = popen(buf, "r"))
          {
           if (fgets(buf, MAXPATHLEN - 1, f) != NULL)
             {
               buf[strlen(buf) - 1] = '\0';
               sCurrentlySet = TQString(buf);
             }
           pclose(f);
          }
    #else
      if((f = fopen("/etc/timezone", "r")) != NULL) {
        // get the currently set timezone
        fgets(tempstring, 100, f);
        tempstring[strlen(tempstring) - 1] = '\0';
        sCurrentlySet = TQString(tempstring);
        fclose(f);
      }
    #endif // !USE_SOLARIS

      mTimeZoneCombo->insertItem(i18n("[No selection]"));

      // Read all system time zones
    #if defined(USE_SOLARIS)       // MARCO
        snprintf(buf, MAXPATHLEN,
               "/bin/find %s \\( -name src -prune \\) -o -type f -print | /bin/cut -b %d-",
               ZONEINFODIR, strlen(ZONEINFODIR) + 2);

        if (f = popen(buf, "r"))
          {
           while(fgets(buf, MAXPATHLEN - 1, f) != NULL)
             {
               buf[strlen(buf) - 1] = '\0';
               list.append(buf);
             }
           pclose(f);
          }

    #else
      f = popen("grep -e  ^[^#] /usr/share/zoneinfo/zone.tab | cut -f 3","r");
      if (!f) return;
      while(fgets(tempstring, 100, f) != NULL) {
        tempstring[strlen(tempstring)-1] = '\0';
        list.append(i18n(tempstring));
        tzonenames << tempstring;
      }
      pclose(f);
    #endif // !USE_SOLARIS
      list.sort();

      mTimeZoneCombo->insertStringList(list);

        // find the currently set time zone and select it
      for ( int i = 0; i < mTimeZoneCombo->count(); ++i )
        {
          if (mTimeZoneCombo->text(i) == sCurrentlySet)
            {
             nCurrentlySet = i;
             break;
            }
        }

      mTimeZoneCombo->setCurrentItem(nCurrentlySet);
      TQWhatsThis::add( mTimeZoneCombo, whatsThis );

      // holiday region selection
      TQHBox *holidayRegBox = new TQHBox( topFrame );
      topLayout->addMultiCellWidget( holidayRegBox, 1, 1, 0, 1 );

      TQLabel *holidayLabel = new TQLabel( i18n( "Use holiday region:" ), holidayRegBox );
      whatsThis = i18n( "Select from which region you want to use the "
                        "holidays here. Defined holidays are shown as "
                        "non-working days in the date navigator, the "
                        "agenda view, etc." );
      TQWhatsThis::add( holidayLabel, whatsThis );

      mHolidayCombo = new TQComboBox( holidayRegBox );
      connect( mHolidayCombo, TQT_SIGNAL( activated( int ) ),
               TQT_SLOT( slotWidChanged() ) );

      TQWhatsThis::add( mHolidayCombo, whatsThis );

      TQString currentHolidayName;
      TQStringList holidayList;
      TQStringList countryList = KHolidays::locations();
      TQStringList::ConstIterator it;

      for ( it = countryList.begin(); it != countryList.end(); ++it ) {
        TQString countryFile = locate( "locale",
                                      "l10n/" + (*it) + "/entry.desktop" );
        TQString regionName;
        if ( !countryFile.isEmpty() ) {
          KSimpleConfig cfg( countryFile );
          cfg.setGroup( "KCM Locale" );
          regionName = cfg.readEntry( "Name" );
        }
        if (regionName.isEmpty()) regionName = (*it);

        holidayList << regionName;
        mRegionMap[regionName] = (*it); //store region for saving to config file

        if ( KOGlobals::self()->holidays()
             && ((*it) == KOGlobals::self()->holidays()->location()) )
          currentHolidayName = regionName;
      }
      holidayList.sort();
      holidayList.push_front( i18n("(None)") );  //be able to disable holidays

      mHolidayCombo->insertStringList(holidayList);

      for (int i=0; i < mHolidayCombo->count(); ++i) {
        if ( mHolidayCombo->text(i) == currentHolidayName ) {
          mHolidayCombo->setCurrentItem(i);
          break;
        }
      }

      KPrefsWidTime *dayBegins =
        addWidTime( KOPrefs::instance()->dayBeginsItem(), topFrame );
      topLayout->addWidget( dayBegins->label(), 2, 0 );
      topLayout->addWidget( dayBegins->timeEdit(), 2, 1 );

      KPrefsWidTime *defaultTime =
        addWidTime( KOPrefs::instance()->startTimeItem(), topFrame );
      topLayout->addWidget( defaultTime->label(), 3, 0);
      topLayout->addWidget( defaultTime->timeEdit(), 3, 1);

      KPrefsWidDuration *defaultDuration =
        addWidDuration( KOPrefs::instance()->defaultDurationItem(), topFrame );
      topLayout->addWidget( defaultDuration->label(), 4, 0 );
      topLayout->addWidget( defaultDuration->timeEdit(), 4, 1 );

      TQGroupBox *remindersGroupBox = new TQGroupBox( 1, Horizontal,
                                                    i18n( "Reminders" ),
                                                    topFrame );
      topLayout->addMultiCellWidget( remindersGroupBox, 5, 5, 0, 1 );

      TQHBox *remindersBox = new TQHBox( remindersGroupBox );
      new TQLabel( i18n( "Default reminder time:" ), remindersBox );

      mReminderTimeSpin  = new KIntSpinBox( remindersBox );
      connect( mReminderTimeSpin, TQT_SIGNAL(valueChanged(int)), TQT_SLOT(slotWidChanged()) );

      mReminderUnitsCombo = new KComboBox( remindersBox );
      connect( mReminderUnitsCombo, TQT_SIGNAL(activated(int)), TQT_SLOT(slotWidChanged()) );
      mReminderUnitsCombo->insertItem( i18n( "minute(s)" ) );
      mReminderUnitsCombo->insertItem( i18n( "hour(s)" ) );
      mReminderUnitsCombo->insertItem( i18n( "day(s)" ) );

      TQHBox *audioFileRemindersBox = new TQHBox( remindersGroupBox );

      TQCheckBox *cb = addWidBool( KOPrefs::instance()->defaultAudioFileRemindersItem(),
                                  audioFileRemindersBox )->checkBox();
      cb->setText( TQString::null );

      if ( KOPrefs::instance()->audioFilePathItem()->value().isEmpty() ) {
        TQString defAudioFile = KGlobal::dirs()->findResourceDir( "sound", "KDE-Sys-Warning.ogg");
        KOPrefs::instance()->audioFilePathItem()->setValue( defAudioFile + "KDE-Sys-Warning.ogg"  );
      }
      TQString filter = i18n( "*.ogg *.wav *.mp3 *.wma *.flac *.aiff *.raw *.au *.ra|"
                             "Audio Files (*.ogg *.wav *.mp3 *.wma *.flac *.aiff *.raw *.au *.ra)" );
      KURLRequester *rq = addWidPath( KOPrefs::instance()->audioFilePathItem(),
                                      audioFileRemindersBox, filter )->urlRequester();
      rq->setEnabled( cb->isChecked() );
      connect( cb, TQT_SIGNAL(toggled(bool)),
               rq, TQT_SLOT(setEnabled( bool)) );

      TQHBox *eventRemindersBox = new TQHBox( remindersGroupBox );
      addWidBool( KOPrefs::instance()->defaultEventRemindersItem(), eventRemindersBox )->checkBox();

      TQHBox *todoRemindersBox = new TQHBox( remindersGroupBox );
      addWidBool( KOPrefs::instance()->defaultTodoRemindersItem(), todoRemindersBox )->checkBox();

      TQLabel *alarmDefaultLabel = new TQLabel( i18n( "Enable reminders by default:" ), topFrame);
      topLayout->addWidget( alarmDefaultLabel, 6, 0 );
      mAlarmTimeDefaultCheckBox = new TQCheckBox( topFrame );
      topLayout->addWidget( mAlarmTimeDefaultCheckBox, 6, 1 );
      connect( mAlarmTimeDefaultCheckBox, TQT_SIGNAL( toggled( bool ) ),
               TQT_SLOT( slotWidChanged() ) );

      TQGroupBox *workingHoursGroup = new TQGroupBox(1,Horizontal,
                                                   i18n("Working Hours"),
                                                   topFrame);
      topLayout->addMultiCellWidget( workingHoursGroup, 7, 7, 0, 1 );

      TQHBox *workDaysBox = new TQHBox( workingHoursGroup );
      // Respect start of week setting
      int weekStart=KGlobal::locale()->weekStartDay();
      for ( int i = 0; i < 7; ++i ) {
        const KCalendarSystem *calSys = KOGlobals::self()->calendarSystem();
        TQString weekDayName = calSys->weekDayName( (i + weekStart + 6)%7 + 1, true );
        if ( KOPrefs::instance()->mCompactDialogs ) {
          weekDayName = weekDayName.left( 1 );
        }
        int index = ( i + weekStart + 6 ) % 7;
        mWorkDays[ index ] = new TQCheckBox( weekDayName, workDaysBox );
        TQWhatsThis::add( mWorkDays[ index ],
                         i18n( "Check this box to make KOrganizer mark the "
                               "working hours for this day of the week. "
                               "If this is a work day for you, check "
                               "this box, or the working hours will not be "
                               "marked with color." ) );

        connect( mWorkDays[ index ], TQT_SIGNAL( stateChanged( int ) ),
               TQT_SLOT( slotWidChanged() ) );
      }

      TQHBox *workStartBox = new TQHBox(workingHoursGroup);
      addWidTime( KOPrefs::instance()->workingHoursStartItem(), workStartBox );

      TQHBox *workEndBox = new TQHBox(workingHoursGroup);
      addWidTime( KOPrefs::instance()->workingHoursEndItem(), workEndBox );


      addWidBool( KOPrefs::instance()->excludeHolidaysItem(),
                  workingHoursGroup );

      topLayout->setRowStretch(8,1);

      load();
    }

  protected:
    void usrReadConfig()
    {
      setCombo( mTimeZoneCombo,
                i18n( KOPrefs::instance()->mTimeZoneId.utf8() ) );

      mReminderTimeSpin->setValue( KOPrefs::instance()->mReminderTime );
      mReminderUnitsCombo->setCurrentItem( KOPrefs::instance()->mReminderTimeUnits );

      for ( int i = 0; i < 7; ++i ) {
        mWorkDays[i]->setChecked( (1<<i) & (KOPrefs::instance()->mWorkWeekMask) );
      }
    }

    void usrWriteConfig()
    {
      // Find untranslated selected zone
      TQStringList::Iterator tz;
      for ( tz = tzonenames.begin(); tz != tzonenames.end(); ++tz )
        if (mTimeZoneCombo->currentText() == i18n((*tz).utf8()))
          break;
      if (tz != tzonenames.end())
        KOPrefs::instance()->mTimeZoneId = (*tz);
      else
        KOPrefs::instance()->mTimeZoneId = mTimeZoneCombo->currentText();

      KOPrefs::instance()->mHolidays = ( mHolidayCombo->currentItem() == 0 ) ?  // (None)
                                       TQString::null :
                                       mRegionMap[mHolidayCombo->currentText()];

      KOPrefs::instance()->mReminderTime = mReminderTimeSpin->value();
      KOPrefs::instance()->mReminderTimeUnits = mReminderUnitsCombo->currentItem();

      int mask = 0;
      for ( int i = 0; i < 7; ++i ) {
        if (mWorkDays[i]->isChecked()) mask = mask | (1<<i);
      }
      KOPrefs::instance()->mWorkWeekMask = mask;
      KOPrefs::instance()->writeConfig();
    }

    void setCombo( TQComboBox *combo, const TQString &text,
                   const TQStringList *tags = 0 )
    {
      if (tags) {
        int i = tags->findIndex(text);
        if (i > 0) combo->setCurrentItem(i);
      } else {
        for(int i=0;i<combo->count();++i) {
          if (combo->text(i) == text) {
            combo->setCurrentItem(i);
            break;
          }
        }
      }
    }

  private:
    TQComboBox    *mTimeZoneCombo;
    TQStringList   tzonenames;
    TQComboBox    *mHolidayCombo;
    TQMap<TQString,TQString> mRegionMap;
    KIntSpinBox  *mReminderTimeSpin;
    KComboBox    *mReminderUnitsCombo;
    TQCheckBox    *mAlarmTimeDefaultCheckBox;
    TQCheckBox    *mWorkDays[7];
};

extern "C"
{
  KCModule *create_korganizerconfigtime( TQWidget *parent, const char * )
  {
    KGlobal::locale()->insertCatalogue( "timezones" );
    return new KOPrefsDialogTime( parent, "kcmkorganizertime" );
  }
}


class KOPrefsDialogViews : public KPrefsModule
{
  public:
    KOPrefsDialogViews( TQWidget *parent, const char *name )
      : KPrefsModule( KOPrefs::instance(), parent, name )
    {
      TQBoxLayout *topTopLayout = new TQVBoxLayout( this );

      TQWidget *topFrame = new TQWidget( this );
      topTopLayout->addWidget( topFrame );

      TQBoxLayout *topLayout = new TQVBoxLayout( topFrame );
      topLayout->setSpacing( KDialog::spacingHint() );

      KPrefsWidBool *enableToolTips =
          addWidBool( KOPrefs::instance()->enableToolTipsItem(), topFrame );
      topLayout->addWidget( enableToolTips->checkBox() );

      KPrefsWidBool *showTodosAgenda =
          addWidBool( KOPrefs::instance()->showAllDayTodoItem(), topFrame );
      topLayout->addWidget( showTodosAgenda->checkBox() );

      /*** Date Navigator Group ***/
      TQGroupBox *dateNavGroup = new TQGroupBox( 1, Horizontal,
                                               i18n("Date Navigator"),
                                               topFrame );
      addWidBool( KOPrefs::instance()->dailyRecurItem(), dateNavGroup );
      addWidBool( KOPrefs::instance()->weeklyRecurItem(), dateNavGroup );
      addWidBool( KOPrefs::instance()->weekNumbersShowWorkItem(), dateNavGroup );
      topLayout->addWidget( dateNavGroup );


      /*** Agenda View Group ***/
      TQGroupBox *agendaGroup = new TQGroupBox( 1, Horizontal,
                                              i18n("Agenda View"),
                                              topFrame );

      TQHBox *hourSizeBox = new TQHBox( agendaGroup );
      KPrefsWidInt *hourSize =
          addWidInt( KOPrefs::instance()->hourSizeItem(), hourSizeBox );
      hourSize->spinBox()->setSuffix(i18n("suffix in the hour size spin box", " pixel"));
      // horizontal spacer:
      new TQWidget( hourSizeBox );

      TQHBox *nextDaysBox = new TQHBox( agendaGroup );
      KPrefsWidInt *nextDays =
        addWidInt( KOPrefs::instance()->nextXDaysItem(), nextDaysBox );
      nextDays->spinBox()->setSuffix(i18n("suffix in the N days spin box", " days"));
      new TQWidget( nextDaysBox );

      KPrefsWidBool *marcusBainsEnabled =
          addWidBool( KOPrefs::instance()->marcusBainsEnabledItem(), agendaGroup );

      KPrefsWidBool *marcusBainsShowSeconds =
          addWidBool( KOPrefs::instance()->marcusBainsShowSecondsItem(), agendaGroup );
      connect( marcusBainsEnabled->checkBox(), TQT_SIGNAL( toggled( bool ) ),
               marcusBainsShowSeconds->checkBox(), TQT_SLOT( setEnabled( bool ) ) );

      addWidBool( KOPrefs::instance()->selectionStartsEditorItem(), agendaGroup );

      addWidCombo( KOPrefs::instance()->agendaViewColorsItem(), agendaGroup );

      addWidCombo( KOPrefs::instance()->agendaViewCalendarDisplayItem(), agendaGroup );

      topLayout->addWidget( agendaGroup );

      /*** Month View Group ***/
      TQGroupBox *monthGroup = new TQGroupBox( 1, Horizontal,
                                             i18n("Month View"),
                                             topFrame );
      addWidBool( KOPrefs::instance()->enableMonthScrollItem(), monthGroup );
      addWidBool( KOPrefs::instance()->fullViewMonthItem(), monthGroup );
      addWidCombo( KOPrefs::instance()->monthItemColorsItem(), monthGroup );
      topLayout->addWidget( monthGroup );


      /*** Todo View Group ***/
      TQGroupBox *todoGroup = new TQGroupBox( 1, Horizontal,
                                            i18n("To-do View"),
                                            topFrame );
      addWidBool( KOPrefs::instance()->fullViewTodoItem(), todoGroup );
      addWidBool( KOPrefs::instance()->recordTodosInJournalsItem(), todoGroup );
      topLayout->addWidget( todoGroup );

      topLayout->addStretch( 1 );

      load();
    }
};

extern "C"
{
  KCModule *create_korganizerconfigviews( TQWidget *parent, const char * )
  {
    return new KOPrefsDialogViews( parent, "kcmkorganizerviews" );
  }
}


class KOPrefsDialogFonts : public KPrefsModule
{
  public:
    KOPrefsDialogFonts( TQWidget *parent, const char *name )
      : KPrefsModule( KOPrefs::instance(), parent, name )
    {
      TQBoxLayout *topTopLayout = new TQVBoxLayout( this );

      TQWidget *topFrame = new TQWidget( this );
      topTopLayout->addWidget( topFrame );

      TQGridLayout *topLayout = new TQGridLayout(topFrame,5,3);
      topLayout->setSpacing( KDialog::spacingHint() );

      KPrefsWidFont *timeBarFont =
          addWidFont( KOPrefs::instance()->timeBarFontItem(), topFrame,
                      KGlobal::locale()->formatTime( TQTime( 12, 34 ) ) );
      topLayout->addWidget(timeBarFont->label(),0,0);
      topLayout->addWidget(timeBarFont->preview(),0,1);
      topLayout->addWidget(timeBarFont->button(),0,2);

      KPrefsWidFont *monthViewFont =
          addWidFont( KOPrefs::instance()->monthViewFontItem(), topFrame,
                      KGlobal::locale()->formatTime(TQTime(12,34)) + " " +
                      i18n("Event text") );

      topLayout->addWidget(monthViewFont->label(),1,0);
      topLayout->addWidget(monthViewFont->preview(),1,1);
      topLayout->addWidget(monthViewFont->button(),1,2);

      KPrefsWidFont *agendaViewFont =
          addWidFont( KOPrefs::instance()->agendaViewFontItem(),
                      topFrame, i18n("Event text") );
      topLayout->addWidget(agendaViewFont->label(),2,0);
      topLayout->addWidget(agendaViewFont->preview(),2,1);
      topLayout->addWidget(agendaViewFont->button(),2,2);

      KPrefsWidFont *marcusBainsFont =
          addWidFont( KOPrefs::instance()->marcusBainsFontItem(), topFrame,
                      KGlobal::locale()->formatTime( TQTime( 12, 34, 23 ) ) );
      topLayout->addWidget(marcusBainsFont->label(),3,0);
      topLayout->addWidget(marcusBainsFont->preview(),3,1);
      topLayout->addWidget(marcusBainsFont->button(),3,2);

      topLayout->setColStretch(1,1);
      topLayout->setRowStretch(4,1);

      load();
    }
};

extern "C"
{
  KCModule *create_korganizerconfigfonts( TQWidget *parent, const char * )
  {
    return new KOPrefsDialogFonts( parent, "kcmkorganizerfonts" );
  }
}


KOPrefsDialogColors::KOPrefsDialogColors( TQWidget *parent, const char *name )
      : KPrefsModule( KOPrefs::instance(), parent, name )
{
  TQBoxLayout *topTopLayout = new TQVBoxLayout( this );

  TQWidget *topFrame = new TQWidget( this );
  topTopLayout->addWidget( topFrame );

  TQGridLayout *topLayout = new TQGridLayout(topFrame,7,2);
  topLayout->setSpacing( KDialog::spacingHint() );

  // Holiday Color
  KPrefsWidColor *holidayColor =
      addWidColor( KOPrefs::instance()->holidayColorItem(), topFrame );
  topLayout->addWidget(holidayColor->label(),0,0);
  topLayout->addWidget(holidayColor->button(),0,1);

  // Highlight Color
  KPrefsWidColor *highlightColor =
      addWidColor( KOPrefs::instance()->highlightColorItem(), topFrame );
  topLayout->addWidget(highlightColor->label(),1,0);
  topLayout->addWidget(highlightColor->button(),1,1);

  // agenda view background color
  KPrefsWidColor *agendaBgColor =
      addWidColor( KOPrefs::instance()->agendaBgColorItem(), topFrame );
  topLayout->addWidget(agendaBgColor->label(),2,0);
  topLayout->addWidget(agendaBgColor->button(),2,1);

  // working hours color
  KPrefsWidColor *workingHoursColor =
      addWidColor( KOPrefs::instance()->workingHoursColorItem(), topFrame );
  topLayout->addWidget(workingHoursColor->label(),3,0);
  topLayout->addWidget(workingHoursColor->button(),3,1);

  // Todo due today color
  KPrefsWidColor *todoDueTodayColor =
      addWidColor( KOPrefs::instance()->todoDueTodayColorItem(), topFrame );
  topLayout->addWidget(todoDueTodayColor->label(),4,0);
  topLayout->addWidget(todoDueTodayColor->button(),4,1);

  // Todo overdue color
  KPrefsWidColor *todoOverdueColor =
      addWidColor( KOPrefs::instance()->todoOverdueColorItem(), topFrame );
  topLayout->addWidget(todoOverdueColor->label(),5,0);
  topLayout->addWidget(todoOverdueColor->button(),5,1);

  // "No Category" color
  KPrefsWidColor *unsetCategoryColor =
    addWidColor( KOPrefs::instance()->unsetCategoryColorItem(), topFrame );
  topLayout->addWidget( unsetCategoryColor->label(), 6, 0 );
  topLayout->addWidget( unsetCategoryColor->button(), 6, 1 );

  // categories colors
  TQGroupBox *categoryGroup = new TQGroupBox(1,Horizontal,i18n("Categories"),
                                           topFrame);
  topLayout->addMultiCellWidget(categoryGroup,7,7,0,1);


  mCategoryCombo = new TQComboBox(categoryGroup);
  mCategoryCombo->insertStringList(KOPrefs::instance()->mCustomCategories);
  TQWhatsThis::add( mCategoryCombo,
                   i18n( "Select here the event category you want to modify. "
                         "You can change the selected category color using "
                         "the button below." ) );
  connect(mCategoryCombo,TQT_SIGNAL(activated(int)),TQT_SLOT(updateCategoryColor()));

  mCategoryButton = new KColorButton(categoryGroup);
  TQWhatsThis::add( mCategoryButton,
                   i18n( "Choose here the color of the event category selected "
                         "using the combo box above." ) );
  connect(mCategoryButton,TQT_SIGNAL(changed(const TQColor &)),TQT_SLOT(setCategoryColor()));
  updateCategoryColor();

  // resources colors
  TQGroupBox *resourceGroup = new TQGroupBox(1,Horizontal,i18n("Resources"),
                                           topFrame);
  topLayout->addMultiCellWidget(resourceGroup,8,8,0,1);

  mResourceCombo = new TQComboBox(resourceGroup);
  TQWhatsThis::add( mResourceCombo,
                   i18n( "Select here resource you want to modify. "
                         "You can change the selected resource color using "
                         "the button below." ) );
  connect(mResourceCombo,TQT_SIGNAL(activated(int)),TQT_SLOT(updateResourceColor()));

  mResourceButton = new KColorButton(resourceGroup);
  TQWhatsThis::add( mResourceButton,
                   i18n( "Choose here the color of the resource selected "
                         "using the combo box above." ) );
  connect(mResourceButton,TQT_SIGNAL(changed(const TQColor &)),TQT_SLOT(setResourceColor()));
  updateResources();

  topLayout->setRowStretch(9,1);

  load();
}

void KOPrefsDialogColors::usrWriteConfig()
{
  TQDictIterator<TQColor> itCat(mCategoryDict);
  while (itCat.current()) {
    KOPrefs::instance()->setCategoryColor(itCat.currentKey(),*itCat.current());
    ++itCat;
  }

  TQDictIterator<TQColor> itRes(mResourceDict);
  while (itRes.current()) {
    KOPrefs::instance()->setResourceColor(itRes.currentKey(),*itRes.current());
    ++itRes;
  }
}

void KOPrefsDialogColors::usrReadConfig()
{
  updateCategories();
  updateResources();
}

void KOPrefsDialogColors::updateCategories()
{
  mCategoryCombo->clear();
  mCategoryCombo->insertStringList(KOPrefs::instance()->mCustomCategories);
  updateCategoryColor();
}

void KOPrefsDialogColors::setCategoryColor()
{
  mCategoryDict.replace(mCategoryCombo->currentText(), new TQColor(mCategoryButton->color()));
  slotWidChanged();
}

void KOPrefsDialogColors::updateCategoryColor()
{
  TQString cat = mCategoryCombo->currentText();
  TQColor *color = mCategoryDict.find(cat);
  if (!color) {
    color = KOPrefs::instance()->categoryColor(cat);
  }
  if (color) {
    mCategoryButton->setColor(*color);
  }
}

void KOPrefsDialogColors::updateResources()
{
  mResourceCombo->clear();
  mResourceIdentifier.clear();
  kdDebug( 5850) << "KOPrefsDialogColors::updateResources()" << endl;

  KCal::CalendarResourceManager *manager = KOrg::StdCalendar::self()->resourceManager();

  kdDebug(5850) << "Loading Calendar resources...:" << endl;
  KCal::CalendarResourceManager::Iterator it;
  for( it = manager->begin(); it != manager->end(); ++it ) {
    if ( !(*it)->subresources().isEmpty() ) {
      TQStringList subresources = (*it)->subresources();
      for ( uint i = 0; i < subresources.count(); ++i ) {
        TQString resource = subresources[ i ];
        if ( (*it)->subresourceActive( resource ) ) {
          mResourceCombo->insertItem( (*it)->labelForSubresource( resource ) );
          mResourceIdentifier.append( resource );
        }
      }
    }

    mResourceCombo->insertItem( (*it)->resourceName() );
    mResourceIdentifier.append( (*it)->identifier() );
  }

  updateResourceColor();
}

void KOPrefsDialogColors::setResourceColor()
{
  kdDebug( 5850) << "KOPrefsDialogColors::setResorceColor()" << endl;

  mResourceDict.replace( mResourceIdentifier[mResourceCombo->currentItem()],
    new TQColor( mResourceButton->color() ) );
  slotWidChanged();
}

void KOPrefsDialogColors::updateResourceColor()
{
  kdDebug( 5850 ) << "KOPrefsDialogColors::updateResourceColor()" << endl;
  TQString res= mResourceIdentifier[mResourceCombo->currentItem()];
  TQColor *color = mCategoryDict.find(res);
  if( !color )  {
    color = KOPrefs::instance()->resourceColor( res );
  }
  if( color ) {
    mResourceButton->setColor(*color);
  }
}
extern "C"
{
  KCModule *create_korganizerconfigcolors( TQWidget *parent, const char * )
  {
    return new KOPrefsDialogColors( parent, "kcmkorganizercolors" );
  }
}


KOPrefsDialogGroupScheduling::KOPrefsDialogGroupScheduling( TQWidget *parent, const char *name )
  : KPrefsModule( KOPrefs::instance(), parent, name )
{
  TQBoxLayout *topTopLayout = new TQVBoxLayout( this );

  TQWidget *topFrame = new TQWidget( this );
  topTopLayout->addWidget( topFrame );

  TQGridLayout *topLayout = new TQGridLayout(topFrame,6,2);
  topLayout->setSpacing( KDialog::spacingHint() );

  KPrefsWidBool *useGroupwareBool =
      addWidBool( KOPrefs::instance()->useGroupwareCommunicationItem(),
      topFrame );
  topLayout->addMultiCellWidget(useGroupwareBool->checkBox(),0,0,0,1);
  // FIXME: This radio button should only be available when KMail is chosen
//   connect(thekmailradiobuttonupthere,TQT_SIGNAL(toggled(bool)),
//           useGroupwareBool->checkBox(), TQT_SLOT(enabled(bool)));

  KPrefsWidBool *bcc =
      addWidBool( KOPrefs::instance()->bccItem(), topFrame );
  topLayout->addMultiCellWidget(bcc->checkBox(),1,1,0,1);

  KPrefsWidRadios *mailClientGroup =
      addWidRadios( KOPrefs::instance()->mailClientItem(), topFrame );
  topLayout->addMultiCellWidget(mailClientGroup->groupBox(),2,2,0,1);


#if 0
  KPrefsWidRadios *schedulerGroup =
      addWidRadios(i18n("Scheduler Mail Client"),KOPrefs::instance()->mIMIPScheduler,
                   topFrame);
  schedulerGroup->addRadio("Dummy"); // Only for debugging
  schedulerGroup->addRadio(i18n("Mail client"));

  topLayout->addMultiCellWidget(schedulerGroup->groupBox(),0,0,0,1);
#endif

  TQLabel *aMailsLabel = new TQLabel(i18n("Additional email addresses:"),topFrame);
  TQString whatsThis = i18n( "Add, edit or remove additional e-mails addresses "
                            "here. These email addresses are the ones you "
                            "have in addition to the one set in personal "
                            "preferences. If you are an attendee of one event, "
                            "but use another email address there, you need to "
                            "list this address here so KOrganizer can "
                            "recognize it as yours." );
  TQWhatsThis::add( aMailsLabel, whatsThis );
  topLayout->addMultiCellWidget(aMailsLabel,3,3,0,1);
  mAMails = new TQListView(topFrame);
  TQWhatsThis::add( mAMails, whatsThis );

  mAMails->addColumn(i18n("Email"),300);
  topLayout->addMultiCellWidget(mAMails,4,4,0,1);

  TQLabel *aEmailsEditLabel = new TQLabel(i18n("Additional email address:"),topFrame);
  whatsThis = i18n( "Edit additional e-mails addresses here. To edit an "
                    "address select it from the list above "
                    "or press the \"New\" button below. These email "
                    "addresses are the ones you have in addition to the "
                    "one set in personal preferences." );
  TQWhatsThis::add( aEmailsEditLabel, whatsThis );
  topLayout->addWidget(aEmailsEditLabel,5,0);
  aEmailsEdit = new TQLineEdit(topFrame);
  TQWhatsThis::add( aEmailsEdit, whatsThis );
  aEmailsEdit->setEnabled(false);
  topLayout->addWidget(aEmailsEdit,5,1);

  TQPushButton *add = new TQPushButton(i18n("New"),topFrame,"new");
  whatsThis = i18n( "Press this button to add a new entry to the "
                    "additional e-mail addresses list. Use the edit "
                    "box above to edit the new entry." );
  TQWhatsThis::add( add, whatsThis );
  topLayout->addWidget(add,6,0);
  TQPushButton *del = new TQPushButton(i18n("Remove"),topFrame,"remove");
  TQWhatsThis::add( del, whatsThis );
  topLayout->addWidget(del,6,1);

  //topLayout->setRowStretch(2,1);
  connect(add, TQT_SIGNAL( clicked() ), this, TQT_SLOT(addItem()) );
  connect(del, TQT_SIGNAL( clicked() ), this, TQT_SLOT(removeItem()) );
  connect(aEmailsEdit,TQT_SIGNAL( textChanged(const TQString&) ), this,TQT_SLOT(updateItem()));
  connect(mAMails,TQT_SIGNAL(selectionChanged(TQListViewItem *)),TQT_SLOT(updateInput()));

  load();
}

void KOPrefsDialogGroupScheduling::usrReadConfig()
{
  mAMails->clear();
  for ( TQStringList::Iterator it = KOPrefs::instance()->mAdditionalMails.begin();
            it != KOPrefs::instance()->mAdditionalMails.end(); ++it ) {
    TQListViewItem *item = new TQListViewItem(mAMails);
    item->setText(0,*it);
    mAMails->insertItem(item);
  }
}

void KOPrefsDialogGroupScheduling::usrWriteConfig()
{
  KOPrefs::instance()->mAdditionalMails.clear();
  TQListViewItem *item;
  item = mAMails->firstChild();
  while (item)
  {
    KOPrefs::instance()->mAdditionalMails.append( item->text(0) );
    item = item->nextSibling();
  }
}

void KOPrefsDialogGroupScheduling::addItem()
{
  aEmailsEdit->setEnabled(true);
  TQListViewItem *item = new TQListViewItem(mAMails);
  mAMails->insertItem(item);
  mAMails->setSelected(item,true);
  aEmailsEdit->setText(i18n("(EmptyEmail)"));
  slotWidChanged();
}

void KOPrefsDialogGroupScheduling::removeItem()
{
  TQListViewItem *item;
  item = mAMails->selectedItem();
  if (!item) return;
  mAMails->takeItem(item);
  item = mAMails->selectedItem();
  if (!item) {
    aEmailsEdit->setText("");
    aEmailsEdit->setEnabled(false);
  }
  if (mAMails->childCount() == 0) {
    aEmailsEdit->setEnabled(false);
  }
  slotWidChanged();
}

void KOPrefsDialogGroupScheduling::updateItem()
{
  TQListViewItem *item;
  item = mAMails->selectedItem();
  if (!item) return;
  item->setText(0,aEmailsEdit->text());
  slotWidChanged();
}

void KOPrefsDialogGroupScheduling::updateInput()
{
  TQListViewItem *item;
  item = mAMails->selectedItem();
  if (!item) return;
  aEmailsEdit->setEnabled(true);
  aEmailsEdit->setText(item->text(0));
}

extern "C"
{
  KCModule *create_korganizerconfiggroupscheduling( TQWidget *parent,
                                                     const char * )
  {
    return new KOPrefsDialogGroupScheduling( parent,
                                             "kcmkorganizergroupscheduling" );
  }
}


KOPrefsDialogGroupwareScheduling::KOPrefsDialogGroupwareScheduling( TQWidget *parent, const char *name )
  : KPrefsModule( KOPrefs::instance(), parent, name )
{
  mGroupwarePage = new KOGroupwarePrefsPage( this );
  connect( mGroupwarePage, TQT_SIGNAL( changed() ), TQT_SLOT( slotWidChanged() ) );
  ( new TQVBoxLayout( this ) )->addWidget( mGroupwarePage );

  load();
}

void KOPrefsDialogGroupwareScheduling::usrReadConfig()
{
  mGroupwarePage->publishEnable->setChecked( KOPrefs::instance()->mFreeBusyPublishAuto );
  mGroupwarePage->publishDelay->setValue( KOPrefs::instance()->mFreeBusyPublishDelay );
  mGroupwarePage->publishDays->setValue( KOPrefs::instance()->mFreeBusyPublishDays );

  mGroupwarePage->publishUrl->setText( KOPrefs::instance()->mFreeBusyPublishUrl );
  mGroupwarePage->publishUser->setText( KOPrefs::instance()->mFreeBusyPublishUser );
  mGroupwarePage->publishPassword->setText( KOPrefs::instance()->mFreeBusyPublishPassword );
  mGroupwarePage->publishSavePassword->setChecked( KOPrefs::instance()->mFreeBusyPublishSavePassword );

  mGroupwarePage->retrieveEnable->setChecked( KOPrefs::instance()->mFreeBusyRetrieveAuto );
  mGroupwarePage->fullDomainRetrieval->setChecked( KOPrefs::instance()->mFreeBusyFullDomainRetrieval );
  mGroupwarePage->retrieveUrl->setText( KOPrefs::instance()->mFreeBusyRetrieveUrl );
  mGroupwarePage->retrieveUser->setText( KOPrefs::instance()->mFreeBusyRetrieveUser );
  mGroupwarePage->retrievePassword->setText( KOPrefs::instance()->mFreeBusyRetrievePassword );
  mGroupwarePage->retrieveSavePassword->setChecked( KOPrefs::instance()->mFreeBusyRetrieveSavePassword );
}

void KOPrefsDialogGroupwareScheduling::usrWriteConfig()
{
  KOPrefs::instance()->mFreeBusyPublishAuto = mGroupwarePage->publishEnable->isChecked();
  KOPrefs::instance()->mFreeBusyPublishDelay = mGroupwarePage->publishDelay->value();
  KOPrefs::instance()->mFreeBusyPublishDays = mGroupwarePage->publishDays->value();

  KOPrefs::instance()->mFreeBusyPublishUrl = mGroupwarePage->publishUrl->text();
  KOPrefs::instance()->mFreeBusyPublishUser = mGroupwarePage->publishUser->text();
  KOPrefs::instance()->mFreeBusyPublishPassword = mGroupwarePage->publishPassword->text();
  KOPrefs::instance()->mFreeBusyPublishSavePassword = mGroupwarePage->publishSavePassword->isChecked();

  KOPrefs::instance()->mFreeBusyRetrieveAuto = mGroupwarePage->retrieveEnable->isChecked();
  KOPrefs::instance()->mFreeBusyFullDomainRetrieval = mGroupwarePage->fullDomainRetrieval->isChecked();
  KOPrefs::instance()->mFreeBusyRetrieveUrl = mGroupwarePage->retrieveUrl->text();
  KOPrefs::instance()->mFreeBusyRetrieveUser = mGroupwarePage->retrieveUser->text();
  KOPrefs::instance()->mFreeBusyRetrievePassword = mGroupwarePage->retrievePassword->text();
  KOPrefs::instance()->mFreeBusyRetrieveSavePassword = mGroupwarePage->retrieveSavePassword->isChecked();

  // clear the url cache for our user
  TQString configFile = locateLocal( "data", "korganizer/freebusyurls" );
  KConfig cfg( configFile );
  cfg.deleteGroup( KOPrefs::instance()->email() );
}

extern "C"
{
  KCModule *create_korganizerconfigfreebusy( TQWidget *parent, const char * )
  {
    return new KOPrefsDialogGroupwareScheduling( parent,
                                                 "kcmkorganizerfreebusy" );
  }
}



class PluginItem : public TQCheckListItem {
  public:
    PluginItem( TQListView *parent, KService::Ptr service ) :
      TQCheckListItem( parent, service->name(), TQCheckListItem::CheckBox ), mService( service )
    {}
    KService::Ptr service() { return mService; }
  private:
    KService::Ptr mService;
};


/**
  Dialog for selecting and configuring KOrganizer plugins
*/
KOPrefsDialogPlugins::KOPrefsDialogPlugins( TQWidget *parent, const char* name )
  : KPrefsModule( KOPrefs::instance(), parent, name )
{
  TQBoxLayout *topTopLayout = new TQVBoxLayout( this );

  TQWidget *topFrame = new TQWidget( this );
  topTopLayout->addWidget( topFrame );
  TQBoxLayout *topLayout = new TQVBoxLayout( topFrame );
  topLayout->setSpacing( KDialog::spacingHint() );

  mListView = new TQListView( topFrame );
  mListView->addColumn( i18n("Name") );
  mListView->setResizeMode( TQListView::LastColumn );
  topLayout->addWidget( mListView );

  mDescription = new TQLabel( topFrame );
  mDescription->setAlignment( TQLabel::NoAccel | TQLabel::WordBreak | TQLabel::AlignVCenter );
  mDescription->setFrameShape( TQLabel::Panel );
  mDescription->setFrameShadow( TQLabel::Sunken );
  mDescription->setMinimumSize( TQSize( 0, 55 ) );
  mDescription->setSizePolicy(
         TQSizePolicy( (TQSizePolicy::SizeType)5, (TQSizePolicy::SizeType)0,
                      0, 0, mDescription->sizePolicy().hasHeightForWidth() ) );
  topLayout->addWidget( mDescription );


  TQWidget *buttonRow = new TQWidget( topFrame );
  TQBoxLayout *buttonRowLayout = new TQHBoxLayout( buttonRow );
  mConfigureButton = new KPushButton( KGuiItem( i18n("Configure &Plugin..."),
      "configure", TQString::null, i18n("This button allows you to configure"
      " the plugin that you have selected in the list above") ), buttonRow );

  buttonRowLayout->addWidget( mConfigureButton );
  buttonRowLayout->addItem( new TQSpacerItem(1, 1,  TQSizePolicy::Expanding) );
  topLayout->addWidget( buttonRow );
  connect( mConfigureButton, TQT_SIGNAL( clicked() ), TQT_SLOT( configure() ) );

  connect( mListView, TQT_SIGNAL( selectionChanged( TQListViewItem* ) ),
           TQT_SLOT( selectionChanged( TQListViewItem* ) ) );
  connect( mListView, TQT_SIGNAL( clicked( TQListViewItem* ) ),
           TQT_SLOT( slotWidChanged() ) );

  load();
//  usrReadConfig();
  selectionChanged( 0 );
}

void KOPrefsDialogPlugins::usrReadConfig()
{
  mListView->clear();
  KTrader::OfferList plugins = KOCore::self()->availablePlugins();
  plugins += KOCore::self()->availableParts();

  TQStringList selectedPlugins = KOPrefs::instance()->mSelectedPlugins;

  KTrader::OfferList::ConstIterator it;
  for( it = plugins.begin(); it != plugins.end(); ++it ) {
    TQCheckListItem *item = new PluginItem( mListView, *it );
    if ( selectedPlugins.find( (*it)->desktopEntryName() ) !=
                               selectedPlugins.end() ) {
      item->setOn( true );
    }
  }
}

void KOPrefsDialogPlugins::usrWriteConfig()
{
  TQStringList selectedPlugins;

  PluginItem *item = static_cast<PluginItem *>( mListView->firstChild() );
  while( item ) {
    if( item->isOn() ) {
      selectedPlugins.append( item->service()->desktopEntryName() );
    }
    item = static_cast<PluginItem *>( item->nextSibling() );
  }
  KOPrefs::instance()->mSelectedPlugins = selectedPlugins;
}

void KOPrefsDialogPlugins::configure()
{
  PluginItem *item = static_cast<PluginItem *>( mListView->selectedItem() );
  if ( !item ) return;

  KOrg::Plugin *plugin = KOCore::self()->loadPlugin( item->service() );

  if ( plugin ) {
    plugin->configure( this );
    delete plugin;
  } else {
    KMessageBox::sorry( this, i18n( "Unable to configure this plugin" ),
                        "PluginConfigUnable" );
  }
}

void KOPrefsDialogPlugins::selectionChanged( TQListViewItem *i )
{
  PluginItem *item = dynamic_cast<PluginItem*>( i );
  if ( !item ) {
    mConfigureButton->setEnabled( false );
    mDescription->setText( TQString::null );
    return;
  }

  TQVariant variant = item->service()->property( "X-KDE-KOrganizer-HasSettings" );

  bool hasSettings = true;
  if ( variant.isValid() )
    hasSettings = variant.toBool();

  mDescription->setText( item->service()->comment() );
  mConfigureButton->setEnabled( hasSettings );

  slotWidChanged();
}

extern "C"
{
  KCModule *create_korganizerconfigplugins( TQWidget *parent, const char * )
  {
    return new KOPrefsDialogPlugins( parent,
                                     "kcmkorganizerplugins" );
  }
}


extern "C"
{
  KCModule *create_korgdesignerfields( TQWidget *parent, const char * ) {
    return new KOPrefsDesignerFields( parent, "kcmkorgdesignerfields" );
  }
}

KOPrefsDesignerFields::KOPrefsDesignerFields( TQWidget *parent, const char *name )
  : KCMDesignerFields( parent, name )
{
}

TQString KOPrefsDesignerFields::localUiDir()
{
  TQString dir = locateLocal( "data", "korganizer/designer/event/");
  kdDebug() << "KOPrefsDesignerFields::localUiDir(): " << dir << endl;
  return dir;
}

TQString KOPrefsDesignerFields::uiPath()
{
  return "korganizer/designer/event/";
}

void KOPrefsDesignerFields::writeActivePages( const TQStringList &activePages )
{
  KOPrefs::instance()->setActiveDesignerFields( activePages );
  KOPrefs::instance()->writeConfig();
}

TQStringList KOPrefsDesignerFields::readActivePages()
{
  return KOPrefs::instance()->activeDesignerFields();
}

TQString KOPrefsDesignerFields::applicationName()
{
  return "KORGANIZER";
}

#include "koprefsdialog.moc"
