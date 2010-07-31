/*
 *  prefdlg.cpp  -  program preferences dialog
 *  Program:  kalarm
 *  Copyright © 2001-2008 by David Jarvie <djarvie@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kalarm.h"

#include <tqobjectlist.h>
#include <tqlayout.h>
#include <tqbuttongroup.h>
#include <tqvbox.h>
#include <tqlineedit.h>
#include <tqcheckbox.h>
#include <tqradiobutton.h>
#include <tqpushbutton.h>
#include <tqcombobox.h>
#include <tqwhatsthis.h>
#include <tqtooltip.h>
#include <tqstyle.h>

#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kshell.h>
#include <kmessagebox.h>
#include <kaboutdata.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kcolorcombo.h>
#include <kstdguiitem.h>
#ifdef Q_WS_X11
#include <kwin.h>
#endif
#include <kdebug.h>

#include <kalarmd/kalarmd.h>

#include "alarmcalendar.h"
#include "alarmtimewidget.h"
#include "daemon.h"
#include "editdlg.h"
#include "fontcolour.h"
#include "functions.h"
#include "kalarmapp.h"
#include "kamail.h"
#include "label.h"
#include "latecancel.h"
#include "mainwindow.h"
#include "preferences.h"
#include "radiobutton.h"
#include "recurrenceedit.h"
#ifndef WITHOUT_ARTS
#include "sounddlg.h"
#endif
#include "soundpicker.h"
#include "specialactions.h"
#include "timeedit.h"
#include "timespinbox.h"
#include "traywindow.h"
#include "prefdlg.moc"

// Command strings for executing commands in different types of terminal windows.
// %t = window title parameter
// %c = command to execute in terminal
// %w = command to execute in terminal, with 'sleep 86400' appended
// %C = temporary command file to execute in terminal
// %W = temporary command file to execute in terminal, with 'sleep 86400' appended
static TQString xtermCommands[] = {
	TQString::fromLatin1("xterm -sb -hold -title %t -e %c"),
	TQString::fromLatin1("konsole --noclose -T %t -e ${SHELL:-sh} -c %c"),
	TQString::fromLatin1("gnome-terminal -t %t -e %W"),
	TQString::fromLatin1("eterm --pause -T %t -e %C"),    // some systems use eterm...
	TQString::fromLatin1("Eterm --pause -T %t -e %C"),    // while some use Eterm
	TQString::fromLatin1("rxvt -title %t -e ${SHELL:-sh} -c %w"),
	TQString::null       // end of list indicator - don't change!
};


/*=============================================================================
= Class KAlarmPrefDlg
=============================================================================*/

KAlarmPrefDlg* KAlarmPrefDlg::mInstance = 0;

void KAlarmPrefDlg::display()
{
	if (!mInstance)
	{
		mInstance = new KAlarmPrefDlg;
		mInstance->show();
	}
	else
	{
#ifdef Q_WS_X11
		KWin::WindowInfo info = KWin::windowInfo(mInstance->winId(), static_cast<unsigned long>(NET::WMGeometry | NET::WMDesktop));
		KWin::setCurrentDesktop(info.desktop());
#endif
		mInstance->showNormal();   // un-minimise it if necessary
		mInstance->raise();
		mInstance->setActiveWindow();
	}
}

KAlarmPrefDlg::KAlarmPrefDlg()
	: KDialogBase(IconList, i18n("Preferences"), Help | Default | Ok | Apply | Cancel, Ok, 0, "PrefDlg", false, true)
{
	setWFlags(Qt::WDestructiveClose);
	setIconListAllVisible(true);

	TQVBox* frame = addVBoxPage(i18n("General"), i18n("General"), DesktopIcon("misc"));
	mMiscPage = new MiscPrefTab(frame);

	frame = addVBoxPage(i18n("Email"), i18n("Email Alarm Settings"), DesktopIcon("mail_generic"));
	mEmailPage = new EmailPrefTab(frame);

	frame = addVBoxPage(i18n("View"), i18n("View Settings"), DesktopIcon("view_choose"));
	mViewPage = new ViewPrefTab(frame);

	frame = addVBoxPage(i18n("Font & Color"), i18n("Default Font and Color"), DesktopIcon("colorize"));
	mFontColourPage = new FontColourPrefTab(frame);

	frame = addVBoxPage(i18n("Edit"), i18n("Default Alarm Edit Settings"), DesktopIcon("edit"));
	mEditPage = new EditPrefTab(frame);

	restore();
	adjustSize();
}

KAlarmPrefDlg::~KAlarmPrefDlg()
{
	mInstance = 0;
}

// Restore all defaults in the options...
void KAlarmPrefDlg::slotDefault()
{
	kdDebug(5950) << "KAlarmPrefDlg::slotDefault()" << endl;
	mFontColourPage->setDefaults();
	mEmailPage->setDefaults();
	mViewPage->setDefaults();
	mEditPage->setDefaults();
	mMiscPage->setDefaults();
}

void KAlarmPrefDlg::slotHelp()
{
	kapp->invokeHelp("preferences");
}

// Apply the preferences that are currently selected
void KAlarmPrefDlg::slotApply()
{
	kdDebug(5950) << "KAlarmPrefDlg::slotApply()" << endl;
	TQString errmsg = mEmailPage->validate();
	if (!errmsg.isEmpty())
	{
		showPage(pageIndex(mEmailPage->parentWidget()));
		if (KMessageBox::warningYesNo(this, errmsg) != KMessageBox::Yes)
		{
			mValid = false;
			return;
		}
	}
	errmsg = mEditPage->validate();
	if (!errmsg.isEmpty())
	{
		showPage(pageIndex(mEditPage->parentWidget()));
		KMessageBox::sorry(this, errmsg);
		mValid = false;
		return;
	}
	mValid = true;
	mFontColourPage->apply(false);
	mEmailPage->apply(false);
	mViewPage->apply(false);
	mEditPage->apply(false);
	mMiscPage->apply(false);
	Preferences::syncToDisc();
}

// Apply the preferences that are currently selected
void KAlarmPrefDlg::slotOk()
{
	kdDebug(5950) << "KAlarmPrefDlg::slotOk()" << endl;
	mValid = true;
	slotApply();
	if (mValid)
		KDialogBase::slotOk();
}

// Discard the current preferences and close the dialogue
void KAlarmPrefDlg::slotCancel()
{
	kdDebug(5950) << "KAlarmPrefDlg::slotCancel()" << endl;
	restore();
	KDialogBase::slotCancel();
}

// Discard the current preferences and use the present ones
void KAlarmPrefDlg::restore()
{
	kdDebug(5950) << "KAlarmPrefDlg::restore()" << endl;
	mFontColourPage->restore();
	mEmailPage->restore();
	mViewPage->restore();
	mEditPage->restore();
	mMiscPage->restore();
}


/*=============================================================================
= Class PrefsTabBase
=============================================================================*/
int PrefsTabBase::mIndentWidth = 0;

PrefsTabBase::PrefsTabBase(TQVBox* frame)
	: TQWidget(frame),
	  mPage(frame)
{
	if (!mIndentWidth)
		mIndentWidth = style().subRect(TQStyle::SR_RadioButtonIndicator, this).width();
}

void PrefsTabBase::apply(bool syncToDisc)
{
	Preferences::save(syncToDisc);
}



/*=============================================================================
= Class MiscPrefTab
=============================================================================*/

MiscPrefTab::MiscPrefTab(TQVBox* frame)
	: PrefsTabBase(frame)
{
	// Get alignment to use in TQGridLayout (AlignAuto doesn't work correctly there)
	int alignment = TQApplication::reverseLayout() ? Qt::AlignRight : Qt::AlignLeft;

	TQGroupBox* group = new TQButtonGroup(i18n("Run Mode"), mPage, "modeGroup");
	TQGridLayout* grid = new TQGridLayout(group, 6, 2, KDialog::marginHint(), KDialog::spacingHint());
	grid->setColStretch(2, 1);
	grid->addColSpacing(0, indentWidth());
	grid->addColSpacing(1, indentWidth());
	grid->addRowSpacing(0, fontMetrics().lineSpacing()/2);

	// Run-on-demand radio button
	mRunOnDemand = new TQRadioButton(i18n("&Run only on demand"), group, "runDemand");
	mRunOnDemand->setFixedSize(mRunOnDemand->sizeHint());
	connect(mRunOnDemand, TQT_SIGNAL(toggled(bool)), TQT_SLOT(slotRunModeToggled(bool)));
	TQWhatsThis::add(mRunOnDemand,
	      i18n("Check to run KAlarm only when required.\n\n"
	           "Notes:\n"
	           "1. Alarms are displayed even when KAlarm is not running, since alarm monitoring is done by the alarm daemon.\n"
	           "2. With this option selected, the system tray icon can be displayed or hidden independently of KAlarm."));
	grid->addMultiCellWidget(mRunOnDemand, 1, 1, 0, 2, alignment);

	// Run-in-system-tray radio button
	mRunInSystemTray = new TQRadioButton(i18n("Run continuously in system &tray"), group, "runTray");
	mRunInSystemTray->setFixedSize(mRunInSystemTray->sizeHint());
	connect(mRunInSystemTray, TQT_SIGNAL(toggled(bool)), TQT_SLOT(slotRunModeToggled(bool)));
	TQWhatsThis::add(mRunInSystemTray,
	      i18n("Check to run KAlarm continuously in the KDE system tray.\n\n"
	           "Notes:\n"
	           "1. With this option selected, closing the system tray icon will quit KAlarm.\n"
	           "2. You do not need to select this option in order for alarms to be displayed, since alarm monitoring is done by the alarm daemon."
	           " Running in the system tray simply provides easy access and a status indication."));
	grid->addMultiCellWidget(mRunInSystemTray, 2, 2, 0, 2, alignment);

	// Run continuously options
	mDisableAlarmsIfStopped = new TQCheckBox(i18n("Disa&ble alarms while not running"), group, "disableAl");
	mDisableAlarmsIfStopped->setFixedSize(mDisableAlarmsIfStopped->sizeHint());
	connect(mDisableAlarmsIfStopped, TQT_SIGNAL(toggled(bool)), TQT_SLOT(slotDisableIfStoppedToggled(bool)));
	TQWhatsThis::add(mDisableAlarmsIfStopped,
	      i18n("Check to disable alarms whenever KAlarm is not running. Alarms will only appear while the system tray icon is visible."));
	grid->addMultiCellWidget(mDisableAlarmsIfStopped, 3, 3, 1, 2, alignment);

	mQuitWarn = new TQCheckBox(i18n("Warn before &quitting"), group, "disableAl");
	mQuitWarn->setFixedSize(mQuitWarn->sizeHint());
	TQWhatsThis::add(mQuitWarn,
	      i18n("Check to display a warning prompt before quitting KAlarm."));
	grid->addWidget(mQuitWarn, 4, 2, alignment);

	mAutostartTrayIcon = new TQCheckBox(i18n("Autostart at &login"), group, "autoTray");
#ifdef AUTOSTART_BY_KALARMD
	connect(mAutostartTrayIcon, TQT_SIGNAL(toggled(bool)), TQT_SLOT(slotAutostartToggled(bool)));
#endif
	grid->addMultiCellWidget(mAutostartTrayIcon, 5, 5, 0, 2, alignment);

	// Autostart alarm daemon
	mAutostartDaemon = new TQCheckBox(i18n("Start alarm monitoring at lo&gin"), group, "startDaemon");
	mAutostartDaemon->setFixedSize(mAutostartDaemon->sizeHint());
	connect(mAutostartDaemon, TQT_SIGNAL(clicked()), TQT_SLOT(slotAutostartDaemonClicked()));
	TQWhatsThis::add(mAutostartDaemon,
	      i18n("Automatically start alarm monitoring whenever you start KDE, by running the alarm daemon (%1).\n\n"
	           "This option should always be checked unless you intend to discontinue use of KAlarm.")
	          .arg(TQString::fromLatin1(DAEMON_APP_NAME)));
	grid->addMultiCellWidget(mAutostartDaemon, 6, 6, 0, 2, alignment);

	group->setFixedHeight(group->sizeHint().height());

	// Start-of-day time
	TQHBox* itemBox = new TQHBox(mPage);
	TQHBox* box = new TQHBox(itemBox);   // this is to control the TQWhatsThis text display area
	box->setSpacing(KDialog::spacingHint());
	TQLabel* label = new TQLabel(i18n("&Start of day for date-only alarms:"), box);
	mStartOfDay = new TimeEdit(box);
	mStartOfDay->setFixedSize(mStartOfDay->sizeHint());
	label->setBuddy(mStartOfDay);
	static const TQString startOfDayText = i18n("The earliest time of day at which a date-only alarm (i.e. "
	                                           "an alarm with \"any time\" specified) will be triggered.");
	TQWhatsThis::add(box, TQString("%1\n\n%2").arg(startOfDayText).arg(TimeSpinBox::shiftWhatsThis()));
	itemBox->setStretchFactor(new TQWidget(itemBox), 1);    // left adjust the controls
	itemBox->setFixedHeight(box->sizeHint().height());

	// Confirm alarm deletion?
	itemBox = new TQHBox(mPage);   // this is to allow left adjustment
	mConfirmAlarmDeletion = new TQCheckBox(i18n("Con&firm alarm deletions"), itemBox, "confirmDeletion");
	mConfirmAlarmDeletion->setMinimumSize(mConfirmAlarmDeletion->sizeHint());
	TQWhatsThis::add(mConfirmAlarmDeletion,
	      i18n("Check to be prompted for confirmation each time you delete an alarm."));
	itemBox->setStretchFactor(new TQWidget(itemBox), 1);    // left adjust the controls
	itemBox->setFixedHeight(itemBox->sizeHint().height());

	// Expired alarms
	group = new TQGroupBox(i18n("Expired Alarms"), mPage);
	grid = new TQGridLayout(group, 2, 2, KDialog::marginHint(), KDialog::spacingHint());
	grid->setColStretch(1, 1);
	grid->addColSpacing(0, indentWidth());
	grid->addRowSpacing(0, fontMetrics().lineSpacing()/2);
	mKeepExpired = new TQCheckBox(i18n("Keep alarms after e&xpiry"), group, "keepExpired");
	mKeepExpired->setFixedSize(mKeepExpired->sizeHint());
	connect(mKeepExpired, TQT_SIGNAL(toggled(bool)), TQT_SLOT(slotExpiredToggled(bool)));
	TQWhatsThis::add(mKeepExpired,
	      i18n("Check to store alarms after expiry or deletion (except deleted alarms which were never triggered)."));
	grid->addMultiCellWidget(mKeepExpired, 1, 1, 0, 1, alignment);

	box = new TQHBox(group);
	box->setSpacing(KDialog::spacingHint());
	mPurgeExpired = new TQCheckBox(i18n("Discard ex&pired alarms after:"), box, "purgeExpired");
	mPurgeExpired->setMinimumSize(mPurgeExpired->sizeHint());
	connect(mPurgeExpired, TQT_SIGNAL(toggled(bool)), TQT_SLOT(slotExpiredToggled(bool)));
	mPurgeAfter = new SpinBox(box);
	mPurgeAfter->setMinValue(1);
	mPurgeAfter->setLineShiftStep(10);
	mPurgeAfter->setMinimumSize(mPurgeAfter->sizeHint());
	mPurgeAfterLabel = new TQLabel(i18n("da&ys"), box);
	mPurgeAfterLabel->setMinimumSize(mPurgeAfterLabel->sizeHint());
	mPurgeAfterLabel->setBuddy(mPurgeAfter);
	TQWhatsThis::add(box,
	      i18n("Uncheck to store expired alarms indefinitely. Check to enter how long expired alarms should be stored."));
	grid->addWidget(box, 2, 1, alignment);

	mClearExpired = new TQPushButton(i18n("Clear Expired Alar&ms"), group);
	mClearExpired->setFixedSize(mClearExpired->sizeHint());
	connect(mClearExpired, TQT_SIGNAL(clicked()), TQT_SLOT(slotClearExpired()));
	TQWhatsThis::add(mClearExpired,
	      i18n("Delete all existing expired alarms."));
	grid->addWidget(mClearExpired, 3, 1, alignment);
	group->setFixedHeight(group->sizeHint().height());

	// Terminal window to use for command alarms
	group = new TQGroupBox(i18n("Terminal for Command Alarms"), mPage);
	TQWhatsThis::add(group,
	      i18n("Choose which application to use when a command alarm is executed in a terminal window"));
	grid = new TQGridLayout(group, 1, 3, KDialog::marginHint(), KDialog::spacingHint());
	grid->addRowSpacing(0, fontMetrics().lineSpacing()/2);
	int row = 0;

	mXtermType = new TQButtonGroup(group);
	mXtermType->hide();
	TQString whatsThis = i18n("The parameter is a command line, e.g. 'xterm -e'", "Check to execute command alarms in a terminal window by '%1'");
	int index = 0;
	mXtermFirst = -1;
	for (mXtermCount = 0;  !xtermCommands[mXtermCount].isNull();  ++mXtermCount)
	{
		TQString cmd = xtermCommands[mXtermCount];
		TQStringList args = KShell::splitArgs(cmd);
		if (args.isEmpty()  ||  KStandardDirs::findExe(args[0]).isEmpty())
			continue;
		TQRadioButton* radio = new TQRadioButton(args[0], group);
		radio->setMinimumSize(radio->sizeHint());
		mXtermType->insert(radio, mXtermCount);
		if (mXtermFirst < 0)
			mXtermFirst = mXtermCount;   // note the id of the first button
		cmd.replace("%t", kapp->aboutData()->programName());
		cmd.replace("%c", "<command>");
		cmd.replace("%w", "<command; sleep>");
		cmd.replace("%C", "[command]");
		cmd.replace("%W", "[command; sleep]");
		TQWhatsThis::add(radio, whatsThis.arg(cmd));
		grid->addWidget(radio, (row = index/3 + 1), index % 3, Qt::AlignAuto);
		++index;
	}

	box = new TQHBox(group);
	grid->addMultiCellWidget(box, row + 1, row + 1, 0, 2, Qt::AlignAuto);
	TQRadioButton* radio = new TQRadioButton(i18n("Other:"), box);
	radio->setFixedSize(radio->sizeHint());
	connect(radio, TQT_SIGNAL(toggled(bool)), TQT_SLOT(slotOtherTerminalToggled(bool)));
	mXtermType->insert(radio, mXtermCount);
	if (mXtermFirst < 0)
		mXtermFirst = mXtermCount;   // note the id of the first button
	mXtermCommand = new TQLineEdit(box);
	TQWhatsThis::add(box,
	      i18n("Enter the full command line needed to execute a command in your chosen terminal window. "
	           "By default the alarm's command string will be appended to what you enter here. "
	           "See the KAlarm Handbook for details of special codes to tailor the command line."));

	mPage->setStretchFactor(new TQWidget(mPage), 1);    // top adjust the widgets
}

void MiscPrefTab::restore()
{
	mAutostartDaemon->setChecked(Daemon::autoStart());
	bool systray = Preferences::mRunInSystemTray;
	mRunInSystemTray->setChecked(systray);
	mRunOnDemand->setChecked(!systray);
	mDisableAlarmsIfStopped->setChecked(Preferences::mDisableAlarmsIfStopped);
	mQuitWarn->setChecked(Preferences::quitWarn());
	mAutostartTrayIcon->setChecked(Preferences::mAutostartTrayIcon);
	mConfirmAlarmDeletion->setChecked(Preferences::confirmAlarmDeletion());
	mStartOfDay->setValue(Preferences::mStartOfDay);
	setExpiredControls(Preferences::mExpiredKeepDays);
	TQString xtermCmd = Preferences::cmdXTermCommand();
	int id = mXtermFirst;
	if (!xtermCmd.isEmpty())
	{
		for ( ;  id < mXtermCount;  ++id)
		{
			if (mXtermType->find(id)  &&  xtermCmd == xtermCommands[id])
				break;
		}
	}
	mXtermType->setButton(id);
	mXtermCommand->setEnabled(id == mXtermCount);
	mXtermCommand->setText(id == mXtermCount ? xtermCmd : "");
	slotDisableIfStoppedToggled(true);
}

void MiscPrefTab::apply(bool syncToDisc)
{
	// First validate anything entered in Other X-terminal command
	int xtermID = mXtermType->selectedId();
	if (xtermID >= mXtermCount)
	{
		TQString cmd = mXtermCommand->text();
		if (cmd.isEmpty())
			xtermID = -1;       // 'Other' is only acceptable if it's non-blank
		else
		{
			TQStringList args = KShell::splitArgs(cmd);
			cmd = args.isEmpty() ? TQString::null : args[0];
			if (KStandardDirs::findExe(cmd).isEmpty())
			{
				mXtermCommand->setFocus();
				if (KMessageBox::warningContinueCancel(this, i18n("Command to invoke terminal window not found:\n%1").arg(cmd))
				                != KMessageBox::Continue)
					return;
			}
		}
	}
	if (xtermID < 0)
	{
		xtermID = mXtermFirst;
		mXtermType->setButton(mXtermFirst);
	}

	bool systray = mRunInSystemTray->isChecked();
	Preferences::mRunInSystemTray        = systray;
	Preferences::mDisableAlarmsIfStopped = mDisableAlarmsIfStopped->isChecked();
	if (mQuitWarn->isEnabled())
		Preferences::setQuitWarn(mQuitWarn->isChecked());
	Preferences::mAutostartTrayIcon = mAutostartTrayIcon->isChecked();
#ifdef AUTOSTART_BY_KALARMD
	bool newAutostartDaemon = mAutostartDaemon->isChecked() || Preferences::mAutostartTrayIcon;
#else
	bool newAutostartDaemon = mAutostartDaemon->isChecked();
#endif
	if (newAutostartDaemon != Daemon::autoStart())
		Daemon::enableAutoStart(newAutostartDaemon);
	Preferences::setConfirmAlarmDeletion(mConfirmAlarmDeletion->isChecked());
	int sod = mStartOfDay->value();
	Preferences::mStartOfDay.setHMS(sod/60, sod%60, 0);
	Preferences::mExpiredKeepDays = !mKeepExpired->isChecked() ? 0
	                              : mPurgeExpired->isChecked() ? mPurgeAfter->value() : -1;
	Preferences::mCmdXTermCommand = (xtermID < mXtermCount) ? xtermCommands[xtermID] : mXtermCommand->text();
	PrefsTabBase::apply(syncToDisc);
}

void MiscPrefTab::setDefaults()
{
	mAutostartDaemon->setChecked(true);
	bool systray = Preferences::default_runInSystemTray;
	mRunInSystemTray->setChecked(systray);
	mRunOnDemand->setChecked(!systray);
	mDisableAlarmsIfStopped->setChecked(Preferences::default_disableAlarmsIfStopped);
	mQuitWarn->setChecked(Preferences::default_quitWarn);
	mAutostartTrayIcon->setChecked(Preferences::default_autostartTrayIcon);
	mConfirmAlarmDeletion->setChecked(Preferences::default_confirmAlarmDeletion);
	mStartOfDay->setValue(Preferences::default_startOfDay);
	setExpiredControls(Preferences::default_expiredKeepDays);
	mXtermType->setButton(mXtermFirst);
	mXtermCommand->setEnabled(false);
	slotDisableIfStoppedToggled(true);
}

void MiscPrefTab::slotAutostartDaemonClicked()
{
	if (!mAutostartDaemon->isChecked()
	&&  KMessageBox::warningYesNo(this,
		                      i18n("You should not uncheck this option unless you intend to discontinue use of KAlarm"),
		                      TQString::null, KStdGuiItem::cont(), KStdGuiItem::cancel()
		                     ) != KMessageBox::Yes)
		mAutostartDaemon->setChecked(true);	
}

void MiscPrefTab::slotRunModeToggled(bool)
{
	bool systray = mRunInSystemTray->isOn();
	mAutostartTrayIcon->setText(systray ? i18n("Autostart at &login") : i18n("Autostart system tray &icon at login"));
	TQWhatsThis::add(mAutostartTrayIcon, (systray ? i18n("Check to run KAlarm whenever you start KDE.")
	                                             : i18n("Check to display the system tray icon whenever you start KDE.")));
	mDisableAlarmsIfStopped->setEnabled(systray);
	slotDisableIfStoppedToggled(true);
}

/******************************************************************************
* If autostart at login is selected, the daemon must be autostarted so that it
* can autostart KAlarm, in which case disable the daemon autostart option.
*/
void MiscPrefTab::slotAutostartToggled(bool)
{
#ifdef AUTOSTART_BY_KALARMD
	mAutostartDaemon->setEnabled(!mAutostartTrayIcon->isChecked());
#endif
}

void MiscPrefTab::slotDisableIfStoppedToggled(bool)
{
	bool enable = mDisableAlarmsIfStopped->isEnabled()  &&  mDisableAlarmsIfStopped->isChecked();
	mQuitWarn->setEnabled(enable);
}

void MiscPrefTab::setExpiredControls(int purgeDays)
{
	mKeepExpired->setChecked(purgeDays);
	mPurgeExpired->setChecked(purgeDays > 0);
	mPurgeAfter->setValue(purgeDays > 0 ? purgeDays : 0);
	slotExpiredToggled(true);
}

void MiscPrefTab::slotExpiredToggled(bool)
{
	bool keep = mKeepExpired->isChecked();
	bool after = keep && mPurgeExpired->isChecked();
	mPurgeExpired->setEnabled(keep);
	mPurgeAfter->setEnabled(after);
	mPurgeAfterLabel->setEnabled(keep);
	mClearExpired->setEnabled(keep);
}

void MiscPrefTab::slotClearExpired()
{
	AlarmCalendar* cal = AlarmCalendar::expiredCalendarOpen();
	if (cal)
		cal->purgeAll();
}

void MiscPrefTab::slotOtherTerminalToggled(bool on)
{
	mXtermCommand->setEnabled(on);
}


/*=============================================================================
= Class EmailPrefTab
=============================================================================*/

EmailPrefTab::EmailPrefTab(TQVBox* frame)
	: PrefsTabBase(frame),
	  mAddressChanged(false),
	  mBccAddressChanged(false)
{
	TQHBox* box = new TQHBox(mPage);
	box->setSpacing(2*KDialog::spacingHint());
	TQLabel* label = new TQLabel(i18n("Email client:"), box);
	mEmailClient = new ButtonGroup(box);
	mEmailClient->hide();
	RadioButton* radio = new RadioButton(i18n("&KMail"), box, "kmail");
	radio->setMinimumSize(radio->sizeHint());
	mEmailClient->insert(radio, Preferences::KMAIL);
	radio = new RadioButton(i18n("&Sendmail"), box, "sendmail");
	radio->setMinimumSize(radio->sizeHint());
	mEmailClient->insert(radio, Preferences::SENDMAIL);
	connect(mEmailClient, TQT_SIGNAL(buttonSet(int)), TQT_SLOT(slotEmailClientChanged(int)));
	box->setFixedHeight(box->sizeHint().height());
	TQWhatsThis::add(box,
	      i18n("Choose how to send email when an email alarm is triggered.\n"
	           "KMail: The email is sent automatically via KMail. KMail is started first if necessary.\n"
	           "Sendmail: The email is sent automatically. This option will only work if "
	           "your system is configured to use sendmail or a sendmail compatible mail transport agent."));

	box = new TQHBox(mPage);   // this is to allow left adjustment
	mEmailCopyToKMail = new TQCheckBox(i18n("Co&py sent emails into KMail's %1 folder").arg(KAMail::i18n_sent_mail()), box);
	mEmailCopyToKMail->setFixedSize(mEmailCopyToKMail->sizeHint());
	TQWhatsThis::add(mEmailCopyToKMail,
	      i18n("After sending an email, store a copy in KMail's %1 folder").arg(KAMail::i18n_sent_mail()));
	box->setStretchFactor(new TQWidget(box), 1);    // left adjust the controls
	box->setFixedHeight(box->sizeHint().height());

	// Your Email Address group box
	TQGroupBox* group = new TQGroupBox(i18n("Your Email Address"), mPage);
	TQGridLayout* grid = new TQGridLayout(group, 6, 3, KDialog::marginHint(), KDialog::spacingHint());
	grid->addRowSpacing(0, fontMetrics().lineSpacing()/2);
	grid->setColStretch(1, 1);

	// 'From' email address controls ...
	label = new Label(EditAlarmDlg::i18n_f_EmailFrom(), group);
	label->setFixedSize(label->sizeHint());
	grid->addWidget(label, 1, 0);
	mFromAddressGroup = new ButtonGroup(group);
	mFromAddressGroup->hide();
	connect(mFromAddressGroup, TQT_SIGNAL(buttonSet(int)), TQT_SLOT(slotFromAddrChanged(int)));

	// Line edit to enter a 'From' email address
	radio = new RadioButton(group);
	mFromAddressGroup->insert(radio, Preferences::MAIL_FROM_ADDR);
	radio->setFixedSize(radio->sizeHint());
	label->setBuddy(radio);
	grid->addWidget(radio, 1, 1);
	mEmailAddress = new TQLineEdit(group);
	connect(mEmailAddress, TQT_SIGNAL(textChanged(const TQString&)), TQT_SLOT(slotAddressChanged()));
	TQString whatsThis = i18n("Your email address, used to identify you as the sender when sending email alarms.");
	TQWhatsThis::add(radio, whatsThis);
	TQWhatsThis::add(mEmailAddress, whatsThis);
	radio->setFocusWidget(mEmailAddress);
	grid->addWidget(mEmailAddress, 1, 2);

	// 'From' email address to be taken from Control Centre
	radio = new RadioButton(i18n("&Use address from Control Center"), group);
	radio->setFixedSize(radio->sizeHint());
	mFromAddressGroup->insert(radio, Preferences::MAIL_FROM_CONTROL_CENTRE);
	TQWhatsThis::add(radio,
	      i18n("Check to use the email address set in the KDE Control Center, to identify you as the sender when sending email alarms."));
	grid->addMultiCellWidget(radio, 2, 2, 1, 2, Qt::AlignAuto);

	// 'From' email address to be picked from KMail's identities when the email alarm is configured
	radio = new RadioButton(i18n("Use KMail &identities"), group);
	radio->setFixedSize(radio->sizeHint());
	mFromAddressGroup->insert(radio, Preferences::MAIL_FROM_KMAIL);
	TQWhatsThis::add(radio,
	      i18n("Check to use KMail's email identities to identify you as the sender when sending email alarms. "
	           "For existing email alarms, KMail's default identity will be used. "
	           "For new email alarms, you will be able to pick which of KMail's identities to use."));
	grid->addMultiCellWidget(radio, 3, 3, 1, 2, Qt::AlignAuto);

	// 'Bcc' email address controls ...
	grid->addRowSpacing(4, KDialog::spacingHint());
	label = new Label(i18n("'Bcc' email address", "&Bcc:"), group);
	label->setFixedSize(label->sizeHint());
	grid->addWidget(label, 5, 0);
	mBccAddressGroup = new ButtonGroup(group);
	mBccAddressGroup->hide();
	connect(mBccAddressGroup, TQT_SIGNAL(buttonSet(int)), TQT_SLOT(slotBccAddrChanged(int)));

	// Line edit to enter a 'Bcc' email address
	radio = new RadioButton(group);
	radio->setFixedSize(radio->sizeHint());
	mBccAddressGroup->insert(radio, Preferences::MAIL_FROM_ADDR);
	label->setBuddy(radio);
	grid->addWidget(radio, 5, 1);
	mEmailBccAddress = new TQLineEdit(group);
	whatsThis = i18n("Your email address, used for blind copying email alarms to yourself. "
	                 "If you want blind copies to be sent to your account on the computer which KAlarm runs on, you can simply enter your user login name.");
	TQWhatsThis::add(radio, whatsThis);
	TQWhatsThis::add(mEmailBccAddress, whatsThis);
	radio->setFocusWidget(mEmailBccAddress);
	grid->addWidget(mEmailBccAddress, 5, 2);

	// 'Bcc' email address to be taken from Control Centre
	radio = new RadioButton(i18n("Us&e address from Control Center"), group);
	radio->setFixedSize(radio->sizeHint());
	mBccAddressGroup->insert(radio, Preferences::MAIL_FROM_CONTROL_CENTRE);
	TQWhatsThis::add(radio,
	      i18n("Check to use the email address set in the KDE Control Center, for blind copying email alarms to yourself."));
	grid->addMultiCellWidget(radio, 6, 6, 1, 2, Qt::AlignAuto);

	group->setFixedHeight(group->sizeHint().height());

	box = new TQHBox(mPage);   // this is to allow left adjustment
	mEmailQueuedNotify = new TQCheckBox(i18n("&Notify when remote emails are queued"), box);
	mEmailQueuedNotify->setFixedSize(mEmailQueuedNotify->sizeHint());
	TQWhatsThis::add(mEmailQueuedNotify,
	      i18n("Display a notification message whenever an email alarm has queued an email for sending to a remote system. "
	           "This could be useful if, for example, you have a dial-up connection, so that you can then ensure that the email is actually transmitted."));
	box->setStretchFactor(new TQWidget(box), 1);    // left adjust the controls
	box->setFixedHeight(box->sizeHint().height());

	mPage->setStretchFactor(new TQWidget(mPage), 1);    // top adjust the widgets
}

void EmailPrefTab::restore()
{
	mEmailClient->setButton(Preferences::mEmailClient);
	mEmailCopyToKMail->setChecked(Preferences::emailCopyToKMail());
	setEmailAddress(Preferences::mEmailFrom, Preferences::mEmailAddress);
	setEmailBccAddress((Preferences::mEmailBccFrom == Preferences::MAIL_FROM_CONTROL_CENTRE), Preferences::mEmailBccAddress);
	mEmailQueuedNotify->setChecked(Preferences::emailQueuedNotify());
	mAddressChanged = mBccAddressChanged = false;
}

void EmailPrefTab::apply(bool syncToDisc)
{
	int client = mEmailClient->id(mEmailClient->selected());
	Preferences::mEmailClient = (client >= 0) ? Preferences::MailClient(client) : Preferences::default_emailClient;
	Preferences::mEmailCopyToKMail = mEmailCopyToKMail->isChecked();
	Preferences::setEmailAddress(static_cast<Preferences::MailFrom>(mFromAddressGroup->selectedId()), mEmailAddress->text().stripWhiteSpace());
	Preferences::setEmailBccAddress((mBccAddressGroup->selectedId() == Preferences::MAIL_FROM_CONTROL_CENTRE), mEmailBccAddress->text().stripWhiteSpace());
	Preferences::setEmailQueuedNotify(mEmailQueuedNotify->isChecked());
	PrefsTabBase::apply(syncToDisc);
}

void EmailPrefTab::setDefaults()
{
	mEmailClient->setButton(Preferences::default_emailClient);
	setEmailAddress(Preferences::default_emailFrom(), Preferences::default_emailAddress);
	setEmailBccAddress((Preferences::default_emailBccFrom == Preferences::MAIL_FROM_CONTROL_CENTRE), Preferences::default_emailBccAddress);
	mEmailQueuedNotify->setChecked(Preferences::default_emailQueuedNotify);
}

void EmailPrefTab::setEmailAddress(Preferences::MailFrom from, const TQString& address)
{
	mFromAddressGroup->setButton(from);
	mEmailAddress->setText(from == Preferences::MAIL_FROM_ADDR ? address.stripWhiteSpace() : TQString());
}

void EmailPrefTab::setEmailBccAddress(bool useControlCentre, const TQString& address)
{
	mBccAddressGroup->setButton(useControlCentre ? Preferences::MAIL_FROM_CONTROL_CENTRE : Preferences::MAIL_FROM_ADDR);
	mEmailBccAddress->setText(useControlCentre ? TQString() : address.stripWhiteSpace());
}

void EmailPrefTab::slotEmailClientChanged(int id)
{
	mEmailCopyToKMail->setEnabled(id == Preferences::SENDMAIL);
}

void EmailPrefTab::slotFromAddrChanged(int id)
{
	mEmailAddress->setEnabled(id == Preferences::MAIL_FROM_ADDR);
	mAddressChanged = true;
}

void EmailPrefTab::slotBccAddrChanged(int id)
{
	mEmailBccAddress->setEnabled(id == Preferences::MAIL_FROM_ADDR);
	mBccAddressChanged = true;
}

TQString EmailPrefTab::validate()
{
	if (mAddressChanged)
	{
		mAddressChanged = false;
		TQString errmsg = validateAddr(mFromAddressGroup, mEmailAddress, KAMail::i18n_NeedFromEmailAddress());
		if (!errmsg.isEmpty())
			return errmsg;
	}
	if (mBccAddressChanged)
	{
		mBccAddressChanged = false;
		return validateAddr(mBccAddressGroup, mEmailBccAddress, i18n("No valid 'Bcc' email address is specified."));
	}
	return TQString::null;
}

TQString EmailPrefTab::validateAddr(ButtonGroup* group, TQLineEdit* addr, const TQString& msg)
{
	TQString errmsg = i18n("%1\nAre you sure you want to save your changes?").arg(msg);
	switch (group->selectedId())
	{
		case Preferences::MAIL_FROM_CONTROL_CENTRE:
			if (!KAMail::controlCentreAddress().isEmpty())
				return TQString::null;
			errmsg = i18n("No email address is currently set in the KDE Control Center. %1").arg(errmsg);
			break;
		case Preferences::MAIL_FROM_KMAIL:
			if (KAMail::identitiesExist())
				return TQString::null;
			errmsg = i18n("No KMail identities currently exist. %1").arg(errmsg);
			break;
		case Preferences::MAIL_FROM_ADDR:
			if (!addr->text().stripWhiteSpace().isEmpty())
				return TQString::null;
			break;
	}
	return errmsg;
}


/*=============================================================================
= Class FontColourPrefTab
=============================================================================*/

FontColourPrefTab::FontColourPrefTab(TQVBox* frame)
	: PrefsTabBase(frame)
{
	mFontChooser = new FontColourChooser(mPage, 0, false, TQStringList(), i18n("Message Font && Color"), true, false);
	mPage->setStretchFactor(mFontChooser, 1);

	TQFrame* layoutBox = new TQFrame(mPage);
	TQHBoxLayout* hlayout = new TQHBoxLayout(layoutBox);
	TQVBoxLayout* colourLayout = new TQVBoxLayout(hlayout, KDialog::spacingHint());
	hlayout->addStretch();

	TQHBox* box = new TQHBox(layoutBox);    // to group widgets for TQWhatsThis text
	box->setSpacing(KDialog::spacingHint()/2);
	colourLayout->addWidget(box);
	TQLabel* label1 = new TQLabel(i18n("Di&sabled alarm color:"), box);
	box->setStretchFactor(new TQWidget(box), 1);
	mDisabledColour = new KColorCombo(box);
	label1->setBuddy(mDisabledColour);
	TQWhatsThis::add(box,
	      i18n("Choose the text color in the alarm list for disabled alarms."));

	box = new TQHBox(layoutBox);    // to group widgets for TQWhatsThis text
	box->setSpacing(KDialog::spacingHint()/2);
	colourLayout->addWidget(box);
	TQLabel* label2 = new TQLabel(i18n("E&xpired alarm color:"), box);
	box->setStretchFactor(new TQWidget(box), 1);
	mExpiredColour = new KColorCombo(box);
	label2->setBuddy(mExpiredColour);
	TQWhatsThis::add(box,
	      i18n("Choose the text color in the alarm list for expired alarms."));
}

void FontColourPrefTab::restore()
{
	mFontChooser->setBgColour(Preferences::mDefaultBgColour);
	mFontChooser->setColours(Preferences::mMessageColours);
	mFontChooser->setFont(Preferences::mMessageFont);
	mDisabledColour->setColor(Preferences::mDisabledColour);
	mExpiredColour->setColor(Preferences::mExpiredColour);
}

void FontColourPrefTab::apply(bool syncToDisc)
{
	Preferences::mDefaultBgColour = mFontChooser->bgColour();
	Preferences::mMessageColours  = mFontChooser->colours();
	Preferences::mMessageFont     = mFontChooser->font();
	Preferences::mDisabledColour  = mDisabledColour->color();
	Preferences::mExpiredColour   = mExpiredColour->color();
	PrefsTabBase::apply(syncToDisc);
}

void FontColourPrefTab::setDefaults()
{
	mFontChooser->setBgColour(Preferences::default_defaultBgColour);
	mFontChooser->setColours(Preferences::default_messageColours);
	mFontChooser->setFont(Preferences::default_messageFont());
	mDisabledColour->setColor(Preferences::default_disabledColour);
	mExpiredColour->setColor(Preferences::default_expiredColour);
}


/*=============================================================================
= Class EditPrefTab
=============================================================================*/

EditPrefTab::EditPrefTab(TQVBox* frame)
	: PrefsTabBase(frame)
{
	// Get alignment to use in TQLabel::setAlignment(alignment | Qt::WordBreak)
	// (AlignAuto doesn't work correctly there)
	int alignment = TQApplication::reverseLayout() ? Qt::AlignRight : Qt::AlignLeft;

	int groupTopMargin = fontMetrics().lineSpacing()/2;
	TQString defsetting   = i18n("The default setting for \"%1\" in the alarm edit dialog.");
	TQString soundSetting = i18n("Check to select %1 as the default setting for \"%2\" in the alarm edit dialog.");

	// DISPLAY ALARMS
	TQGroupBox* group = new TQGroupBox(i18n("Display Alarms"), mPage);
	TQBoxLayout* layout = new TQVBoxLayout(group, KDialog::marginHint(), KDialog::spacingHint());
	layout->addSpacing(groupTopMargin);

	mConfirmAck = new TQCheckBox(EditAlarmDlg::i18n_k_ConfirmAck(), group, "defConfAck");
	mConfirmAck->setMinimumSize(mConfirmAck->sizeHint());
	TQWhatsThis::add(mConfirmAck, defsetting.arg(EditAlarmDlg::i18n_ConfirmAck()));
	layout->addWidget(mConfirmAck, 0, Qt::AlignAuto);

	mAutoClose = new TQCheckBox(LateCancelSelector::i18n_i_AutoCloseWinLC(), group, "defAutoClose");
	mAutoClose->setMinimumSize(mAutoClose->sizeHint());
	TQWhatsThis::add(mAutoClose, defsetting.arg(LateCancelSelector::i18n_AutoCloseWin()));
	layout->addWidget(mAutoClose, 0, Qt::AlignAuto);

	TQHBox* box = new TQHBox(group);
	box->setSpacing(KDialog::spacingHint());
	layout->addWidget(box);
	TQLabel* label = new TQLabel(i18n("Reminder &units:"), box);
	label->setFixedSize(label->sizeHint());
	mReminderUnits = new TQComboBox(box, "defWarnUnits");
	mReminderUnits->insertItem(TimePeriod::i18n_Minutes(), TimePeriod::MINUTES);
	mReminderUnits->insertItem(TimePeriod::i18n_Hours_Mins(), TimePeriod::HOURS_MINUTES);
	mReminderUnits->insertItem(TimePeriod::i18n_Days(), TimePeriod::DAYS);
	mReminderUnits->insertItem(TimePeriod::i18n_Weeks(), TimePeriod::WEEKS);
	mReminderUnits->setFixedSize(mReminderUnits->sizeHint());
	label->setBuddy(mReminderUnits);
	TQWhatsThis::add(box,
	      i18n("The default units for the reminder in the alarm edit dialog."));
	box->setStretchFactor(new TQWidget(box), 1);    // left adjust the control

	mSpecialActionsButton = new SpecialActionsButton(EditAlarmDlg::i18n_SpecialActions(), box);
	mSpecialActionsButton->setFixedSize(mSpecialActionsButton->sizeHint());

	// SOUND
	TQButtonGroup* bgroup = new TQButtonGroup(SoundPicker::i18n_Sound(), mPage, "soundGroup");
	layout = new TQVBoxLayout(bgroup, KDialog::marginHint(), KDialog::spacingHint());
	layout->addSpacing(groupTopMargin);

	TQBoxLayout* hlayout = new TQHBoxLayout(layout, KDialog::spacingHint());
	mSound = new TQComboBox(false, bgroup, "defSound");
	mSound->insertItem(SoundPicker::i18n_None());         // index 0
	mSound->insertItem(SoundPicker::i18n_Beep());         // index 1
	mSound->insertItem(SoundPicker::i18n_File());         // index 2
	if (theApp()->speechEnabled())
		mSound->insertItem(SoundPicker::i18n_Speak());  // index 3
	mSound->setMinimumSize(mSound->sizeHint());
	TQWhatsThis::add(mSound, defsetting.arg(SoundPicker::i18n_Sound()));
	hlayout->addWidget(mSound);
	hlayout->addStretch(1);

#ifndef WITHOUT_ARTS
	mSoundRepeat = new TQCheckBox(i18n("Repea&t sound file"), bgroup, "defRepeatSound");
	mSoundRepeat->setMinimumSize(mSoundRepeat->sizeHint());
	TQWhatsThis::add(mSoundRepeat, i18n("sound file \"Repeat\" checkbox", "The default setting for sound file \"%1\" in the alarm edit dialog.").arg(SoundDlg::i18n_Repeat()));
	hlayout->addWidget(mSoundRepeat);
#endif

	box = new TQHBox(bgroup);   // this is to control the TQWhatsThis text display area
	box->setSpacing(KDialog::spacingHint());
	mSoundFileLabel = new TQLabel(i18n("Sound &file:"), box);
	mSoundFileLabel->setFixedSize(mSoundFileLabel->sizeHint());
	mSoundFile = new TQLineEdit(box);
	mSoundFileLabel->setBuddy(mSoundFile);
	mSoundFileBrowse = new TQPushButton(box);
	mSoundFileBrowse->setPixmap(SmallIcon("fileopen"));
	mSoundFileBrowse->setFixedSize(mSoundFileBrowse->sizeHint());
	connect(mSoundFileBrowse, TQT_SIGNAL(clicked()), TQT_SLOT(slotBrowseSoundFile()));
	TQToolTip::add(mSoundFileBrowse, i18n("Choose a sound file"));
	TQWhatsThis::add(box,
	      i18n("Enter the default sound file to use in the alarm edit dialog."));
	box->setFixedHeight(box->sizeHint().height());
	layout->addWidget(box);
	bgroup->setFixedHeight(bgroup->sizeHint().height());

	// COMMAND ALARMS
	group = new TQGroupBox(i18n("Command Alarms"), mPage);
	layout = new TQVBoxLayout(group, KDialog::marginHint(), KDialog::spacingHint());
	layout->addSpacing(groupTopMargin);
	layout = new TQHBoxLayout(layout, KDialog::spacingHint());

	mCmdScript = new TQCheckBox(EditAlarmDlg::i18n_p_EnterScript(), group, "defCmdScript");
	mCmdScript->setMinimumSize(mCmdScript->sizeHint());
	TQWhatsThis::add(mCmdScript, defsetting.arg(EditAlarmDlg::i18n_EnterScript()));
	layout->addWidget(mCmdScript);
	layout->addStretch();

	mCmdXterm = new TQCheckBox(EditAlarmDlg::i18n_w_ExecInTermWindow(), group, "defCmdXterm");
	mCmdXterm->setMinimumSize(mCmdXterm->sizeHint());
	TQWhatsThis::add(mCmdXterm, defsetting.arg(EditAlarmDlg::i18n_ExecInTermWindow()));
	layout->addWidget(mCmdXterm);

	// EMAIL ALARMS
	group = new TQGroupBox(i18n("Email Alarms"), mPage);
	layout = new TQVBoxLayout(group, KDialog::marginHint(), KDialog::spacingHint());
	layout->addSpacing(groupTopMargin);

	// BCC email to sender
	mEmailBcc = new TQCheckBox(EditAlarmDlg::i18n_e_CopyEmailToSelf(), group, "defEmailBcc");
	mEmailBcc->setMinimumSize(mEmailBcc->sizeHint());
	TQWhatsThis::add(mEmailBcc, defsetting.arg(EditAlarmDlg::i18n_CopyEmailToSelf()));
	layout->addWidget(mEmailBcc, 0, Qt::AlignAuto);

	// MISCELLANEOUS
	// Show in KOrganizer
	mCopyToKOrganizer = new TQCheckBox(EditAlarmDlg::i18n_g_ShowInKOrganizer(), mPage, "defShowKorg");
	mCopyToKOrganizer->setMinimumSize(mCopyToKOrganizer->sizeHint());
	TQWhatsThis::add(mCopyToKOrganizer, defsetting.arg(EditAlarmDlg::i18n_ShowInKOrganizer()));

	// Late cancellation
	box = new TQHBox(mPage);
	box->setSpacing(KDialog::spacingHint());
	mLateCancel = new TQCheckBox(LateCancelSelector::i18n_n_CancelIfLate(), box, "defCancelLate");
	mLateCancel->setMinimumSize(mLateCancel->sizeHint());
	TQWhatsThis::add(mLateCancel, defsetting.arg(LateCancelSelector::i18n_CancelIfLate()));
	box->setStretchFactor(new TQWidget(box), 1);    // left adjust the control

	// Recurrence
	TQHBox* itemBox = new TQHBox(box);   // this is to control the TQWhatsThis text display area
	itemBox->setSpacing(KDialog::spacingHint());
	label = new TQLabel(i18n("&Recurrence:"), itemBox);
	label->setFixedSize(label->sizeHint());
	mRecurPeriod = new TQComboBox(itemBox, "defRecur");
	mRecurPeriod->insertItem(RecurrenceEdit::i18n_NoRecur());
	mRecurPeriod->insertItem(RecurrenceEdit::i18n_AtLogin());
	mRecurPeriod->insertItem(RecurrenceEdit::i18n_HourlyMinutely());
	mRecurPeriod->insertItem(RecurrenceEdit::i18n_Daily());
	mRecurPeriod->insertItem(RecurrenceEdit::i18n_Weekly());
	mRecurPeriod->insertItem(RecurrenceEdit::i18n_Monthly());
	mRecurPeriod->insertItem(RecurrenceEdit::i18n_Yearly());
	mRecurPeriod->setFixedSize(mRecurPeriod->sizeHint());
	label->setBuddy(mRecurPeriod);
	TQWhatsThis::add(itemBox,
	      i18n("The default setting for the recurrence rule in the alarm edit dialog."));
	box->setFixedHeight(itemBox->sizeHint().height());

	// How to handle February 29th in yearly recurrences
	TQVBox* vbox = new TQVBox(mPage);   // this is to control the TQWhatsThis text display area
	vbox->setSpacing(KDialog::spacingHint());
	label = new TQLabel(i18n("In non-leap years, repeat yearly February 29th alarms on:"), vbox);
	label->setAlignment(alignment | Qt::WordBreak);
	itemBox = new TQHBox(vbox);
	itemBox->setSpacing(2*KDialog::spacingHint());
	mFeb29 = new TQButtonGroup(itemBox);
	mFeb29->hide();
	TQWidget* widget = new TQWidget(itemBox);
	widget->setFixedWidth(3*KDialog::spacingHint());
	TQRadioButton* radio = new TQRadioButton(i18n("February 2&8th"), itemBox);
	radio->setMinimumSize(radio->sizeHint());
	mFeb29->insert(radio, KARecurrence::FEB29_FEB28);
	radio = new TQRadioButton(i18n("March &1st"), itemBox);
	radio->setMinimumSize(radio->sizeHint());
	mFeb29->insert(radio, KARecurrence::FEB29_MAR1);
	radio = new TQRadioButton(i18n("Do &not repeat"), itemBox);
	radio->setMinimumSize(radio->sizeHint());
	mFeb29->insert(radio, KARecurrence::FEB29_FEB29);
	itemBox->setFixedHeight(itemBox->sizeHint().height());
	TQWhatsThis::add(vbox,
	      i18n("For yearly recurrences, choose what date, if any, alarms due on February 29th should occur in non-leap years.\n"
	           "Note that the next scheduled occurrence of existing alarms is not re-evaluated when you change this setting."));

	mPage->setStretchFactor(new TQWidget(mPage), 1);    // top adjust the widgets
}

void EditPrefTab::restore()
{
	mAutoClose->setChecked(Preferences::mDefaultAutoClose);
	mConfirmAck->setChecked(Preferences::mDefaultConfirmAck);
	mReminderUnits->setCurrentItem(Preferences::mDefaultReminderUnits);
	mSpecialActionsButton->setActions(Preferences::mDefaultPreAction, Preferences::mDefaultPostAction);
	mSound->setCurrentItem(soundIndex(Preferences::mDefaultSoundType));
	mSoundFile->setText(Preferences::mDefaultSoundFile);
#ifndef WITHOUT_ARTS
	mSoundRepeat->setChecked(Preferences::mDefaultSoundRepeat);
#endif
	mCmdScript->setChecked(Preferences::mDefaultCmdScript);
	mCmdXterm->setChecked(Preferences::mDefaultCmdLogType == EditAlarmDlg::EXEC_IN_TERMINAL);
	mEmailBcc->setChecked(Preferences::mDefaultEmailBcc);
	mCopyToKOrganizer->setChecked(Preferences::mDefaultCopyToKOrganizer);
	mLateCancel->setChecked(Preferences::mDefaultLateCancel);
	mRecurPeriod->setCurrentItem(recurIndex(Preferences::mDefaultRecurPeriod));
	mFeb29->setButton(Preferences::mDefaultFeb29Type);
}

void EditPrefTab::apply(bool syncToDisc)
{
	Preferences::mDefaultAutoClose        = mAutoClose->isChecked();
	Preferences::mDefaultConfirmAck       = mConfirmAck->isChecked();
	Preferences::mDefaultReminderUnits    = static_cast<TimePeriod::Units>(mReminderUnits->currentItem());
	Preferences::mDefaultPreAction        = mSpecialActionsButton->preAction();
	Preferences::mDefaultPostAction       = mSpecialActionsButton->postAction();
	switch (mSound->currentItem())
	{
		case 3:  Preferences::mDefaultSoundType = SoundPicker::SPEAK;      break;
		case 2:  Preferences::mDefaultSoundType = SoundPicker::PLAY_FILE;  break;
		case 1:  Preferences::mDefaultSoundType = SoundPicker::BEEP;       break;
		case 0:
		default: Preferences::mDefaultSoundType = SoundPicker::NONE;       break;
	}
	Preferences::mDefaultSoundFile        = mSoundFile->text();
#ifndef WITHOUT_ARTS
	Preferences::mDefaultSoundRepeat      = mSoundRepeat->isChecked();
#endif
	Preferences::mDefaultCmdScript        = mCmdScript->isChecked();
	Preferences::mDefaultCmdLogType       = (mCmdXterm->isChecked() ? EditAlarmDlg::EXEC_IN_TERMINAL : EditAlarmDlg::DISCARD_OUTPUT);
	Preferences::mDefaultEmailBcc         = mEmailBcc->isChecked();
	Preferences::mDefaultCopyToKOrganizer = mCopyToKOrganizer->isChecked();
	Preferences::mDefaultLateCancel       = mLateCancel->isChecked() ? 1 : 0;
	switch (mRecurPeriod->currentItem())
	{
		case 6:  Preferences::mDefaultRecurPeriod = RecurrenceEdit::ANNUAL;    break;
		case 5:  Preferences::mDefaultRecurPeriod = RecurrenceEdit::MONTHLY;   break;
		case 4:  Preferences::mDefaultRecurPeriod = RecurrenceEdit::WEEKLY;    break;
		case 3:  Preferences::mDefaultRecurPeriod = RecurrenceEdit::DAILY;     break;
		case 2:  Preferences::mDefaultRecurPeriod = RecurrenceEdit::SUBDAILY;  break;
		case 1:  Preferences::mDefaultRecurPeriod = RecurrenceEdit::AT_LOGIN;  break;
		case 0:
		default: Preferences::mDefaultRecurPeriod = RecurrenceEdit::NO_RECUR;  break;
	}
	int feb29 = mFeb29->selectedId();
	Preferences::mDefaultFeb29Type  = (feb29 >= 0) ? static_cast<KARecurrence::Feb29Type>(feb29) : Preferences::default_defaultFeb29Type;
	PrefsTabBase::apply(syncToDisc);
}

void EditPrefTab::setDefaults()
{
	mAutoClose->setChecked(Preferences::default_defaultAutoClose);
	mConfirmAck->setChecked(Preferences::default_defaultConfirmAck);
	mReminderUnits->setCurrentItem(Preferences::default_defaultReminderUnits);
	mSpecialActionsButton->setActions(Preferences::default_defaultPreAction, Preferences::default_defaultPostAction);
	mSound->setCurrentItem(soundIndex(Preferences::default_defaultSoundType));
	mSoundFile->setText(Preferences::default_defaultSoundFile);
#ifndef WITHOUT_ARTS
	mSoundRepeat->setChecked(Preferences::default_defaultSoundRepeat);
#endif
	mCmdScript->setChecked(Preferences::default_defaultCmdScript);
	mCmdXterm->setChecked(Preferences::default_defaultCmdLogType == EditAlarmDlg::EXEC_IN_TERMINAL);
	mEmailBcc->setChecked(Preferences::default_defaultEmailBcc);
	mCopyToKOrganizer->setChecked(Preferences::default_defaultCopyToKOrganizer);
	mLateCancel->setChecked(Preferences::default_defaultLateCancel);
	mRecurPeriod->setCurrentItem(recurIndex(Preferences::default_defaultRecurPeriod));
	mFeb29->setButton(Preferences::default_defaultFeb29Type);
}

void EditPrefTab::slotBrowseSoundFile()
{
	TQString defaultDir;
	TQString url = SoundPicker::browseFile(defaultDir, mSoundFile->text());
	if (!url.isEmpty())
		mSoundFile->setText(url);
}

int EditPrefTab::soundIndex(SoundPicker::Type type)
{
	switch (type)
	{
		case SoundPicker::SPEAK:      return 3;
		case SoundPicker::PLAY_FILE:  return 2;
		case SoundPicker::BEEP:       return 1;
		case SoundPicker::NONE:
		default:                      return 0;
	}
}

int EditPrefTab::recurIndex(RecurrenceEdit::RepeatType type)
{
	switch (type)
	{
		case RecurrenceEdit::ANNUAL:   return 6;
		case RecurrenceEdit::MONTHLY:  return 5;
		case RecurrenceEdit::WEEKLY:   return 4;
		case RecurrenceEdit::DAILY:    return 3;
		case RecurrenceEdit::SUBDAILY: return 2;
		case RecurrenceEdit::AT_LOGIN: return 1;
		case RecurrenceEdit::NO_RECUR:
		default:                       return 0;
	}
}

TQString EditPrefTab::validate()
{
	if (mSound->currentItem() == SoundPicker::PLAY_FILE  &&  mSoundFile->text().isEmpty())
	{
		mSoundFile->setFocus();
		return i18n("You must enter a sound file when %1 is selected as the default sound type").arg(SoundPicker::i18n_File());;
	}
	return TQString::null;
}


/*=============================================================================
= Class ViewPrefTab
=============================================================================*/

ViewPrefTab::ViewPrefTab(TQVBox* frame)
	: PrefsTabBase(frame)
{
	TQGroupBox* group = new TQGroupBox(i18n("System Tray Tooltip"), mPage);
	TQGridLayout* grid = new TQGridLayout(group, 5, 3, KDialog::marginHint(), KDialog::spacingHint());
	grid->setColStretch(2, 1);
	grid->addColSpacing(0, indentWidth());
	grid->addColSpacing(1, indentWidth());
	grid->addRowSpacing(0, fontMetrics().lineSpacing()/2);

	mTooltipShowAlarms = new TQCheckBox(i18n("Show next &24 hours' alarms"), group, "tooltipShow");
	mTooltipShowAlarms->setMinimumSize(mTooltipShowAlarms->sizeHint());
	connect(mTooltipShowAlarms, TQT_SIGNAL(toggled(bool)), TQT_SLOT(slotTooltipAlarmsToggled(bool)));
	TQWhatsThis::add(mTooltipShowAlarms,
	      i18n("Specify whether to include in the system tray tooltip, a summary of alarms due in the next 24 hours"));
	grid->addMultiCellWidget(mTooltipShowAlarms, 1, 1, 0, 2, Qt::AlignAuto);

	TQHBox* box = new TQHBox(group);
	box->setSpacing(KDialog::spacingHint());
	mTooltipMaxAlarms = new TQCheckBox(i18n("Ma&ximum number of alarms to show:"), box, "tooltipMax");
	mTooltipMaxAlarms->setMinimumSize(mTooltipMaxAlarms->sizeHint());
	connect(mTooltipMaxAlarms, TQT_SIGNAL(toggled(bool)), TQT_SLOT(slotTooltipMaxToggled(bool)));
	mTooltipMaxAlarmCount = new SpinBox(1, 99, 1, box);
	mTooltipMaxAlarmCount->setLineShiftStep(5);
	mTooltipMaxAlarmCount->setMinimumSize(mTooltipMaxAlarmCount->sizeHint());
	TQWhatsThis::add(box,
	      i18n("Uncheck to display all of the next 24 hours' alarms in the system tray tooltip. "
	           "Check to enter an upper limit on the number to be displayed."));
	grid->addMultiCellWidget(box, 2, 2, 1, 2, Qt::AlignAuto);

	mTooltipShowTime = new TQCheckBox(MainWindow::i18n_m_ShowAlarmTime(), group, "tooltipTime");
	mTooltipShowTime->setMinimumSize(mTooltipShowTime->sizeHint());
	connect(mTooltipShowTime, TQT_SIGNAL(toggled(bool)), TQT_SLOT(slotTooltipTimeToggled(bool)));
	TQWhatsThis::add(mTooltipShowTime,
	      i18n("Specify whether to show in the system tray tooltip, the time at which each alarm is due"));
	grid->addMultiCellWidget(mTooltipShowTime, 3, 3, 1, 2, Qt::AlignAuto);

	mTooltipShowTimeTo = new TQCheckBox(MainWindow::i18n_l_ShowTimeToAlarm(), group, "tooltipTimeTo");
	mTooltipShowTimeTo->setMinimumSize(mTooltipShowTimeTo->sizeHint());
	connect(mTooltipShowTimeTo, TQT_SIGNAL(toggled(bool)), TQT_SLOT(slotTooltipTimeToToggled(bool)));
	TQWhatsThis::add(mTooltipShowTimeTo,
	      i18n("Specify whether to show in the system tray tooltip, how long until each alarm is due"));
	grid->addMultiCellWidget(mTooltipShowTimeTo, 4, 4, 1, 2, Qt::AlignAuto);

	box = new TQHBox(group);   // this is to control the TQWhatsThis text display area
	box->setSpacing(KDialog::spacingHint());
	mTooltipTimeToPrefixLabel = new TQLabel(i18n("&Prefix:"), box);
	mTooltipTimeToPrefixLabel->setFixedSize(mTooltipTimeToPrefixLabel->sizeHint());
	mTooltipTimeToPrefix = new TQLineEdit(box);
	mTooltipTimeToPrefixLabel->setBuddy(mTooltipTimeToPrefix);
	TQWhatsThis::add(box,
	      i18n("Enter the text to be displayed in front of the time until the alarm, in the system tray tooltip"));
	box->setFixedHeight(box->sizeHint().height());
	grid->addWidget(box, 5, 2, Qt::AlignAuto);
	group->setMaximumHeight(group->sizeHint().height());

	mModalMessages = new TQCheckBox(i18n("Message &windows have a title bar and take keyboard focus"), mPage, "modalMsg");
	mModalMessages->setMinimumSize(mModalMessages->sizeHint());
	TQWhatsThis::add(mModalMessages,
	      i18n("Specify the characteristics of alarm message windows:\n"
	           "- If checked, the window is a normal window with a title bar, which grabs keyboard input when it is displayed.\n"
	           "- If unchecked, the window does not interfere with your typing when "
	           "it is displayed, but it has no title bar and cannot be moved or resized."));

	TQHBox* itemBox = new TQHBox(mPage);   // this is to control the TQWhatsThis text display area
	box = new TQHBox(itemBox);
	box->setSpacing(KDialog::spacingHint());
	TQLabel* label = new TQLabel(i18n("System tray icon &update interval:"), box);
	mDaemonTrayCheckInterval = new SpinBox(1, 9999, 1, box, "daemonCheck");
	mDaemonTrayCheckInterval->setLineShiftStep(10);
	mDaemonTrayCheckInterval->setMinimumSize(mDaemonTrayCheckInterval->sizeHint());
	label->setBuddy(mDaemonTrayCheckInterval);
	label = new TQLabel(i18n("seconds"), box);
	TQWhatsThis::add(box,
	      i18n("How often to update the system tray icon to indicate whether or not the Alarm Daemon is monitoring alarms."));
	itemBox->setStretchFactor(new TQWidget(itemBox), 1);    // left adjust the controls
	itemBox->setFixedHeight(box->sizeHint().height());

	mPage->setStretchFactor(new TQWidget(mPage), 1);    // top adjust the widgets
}

void ViewPrefTab::restore()
{
	setTooltip(Preferences::mTooltipAlarmCount,
	           Preferences::mShowTooltipAlarmTime,
	           Preferences::mShowTooltipTimeToAlarm,
	           Preferences::mTooltipTimeToPrefix);
	mModalMessages->setChecked(Preferences::mModalMessages);
	mDaemonTrayCheckInterval->setValue(Preferences::mDaemonTrayCheckInterval);
}

void ViewPrefTab::apply(bool syncToDisc)
{
	int n = mTooltipShowAlarms->isChecked() ? -1 : 0;
	if (n  &&  mTooltipMaxAlarms->isChecked())
		n = mTooltipMaxAlarmCount->value();
	Preferences::mTooltipAlarmCount       = n;
	Preferences::mShowTooltipAlarmTime    = mTooltipShowTime->isChecked();
	Preferences::mShowTooltipTimeToAlarm  = mTooltipShowTimeTo->isChecked();
	Preferences::mTooltipTimeToPrefix     = mTooltipTimeToPrefix->text();
	Preferences::mModalMessages           = mModalMessages->isChecked();
	Preferences::mDaemonTrayCheckInterval = mDaemonTrayCheckInterval->value();
	PrefsTabBase::apply(syncToDisc);
}

void ViewPrefTab::setDefaults()
{
	setTooltip(Preferences::default_tooltipAlarmCount,
	           Preferences::default_showTooltipAlarmTime,
	           Preferences::default_showTooltipTimeToAlarm,
	           Preferences::default_tooltipTimeToPrefix);
	mModalMessages->setChecked(Preferences::default_modalMessages);
	mDaemonTrayCheckInterval->setValue(Preferences::default_daemonTrayCheckInterval);
}

void ViewPrefTab::setTooltip(int maxAlarms, bool time, bool timeTo, const TQString& prefix)
{
	if (!timeTo)
		time = true;    // ensure that at least one time option is ticked

	// Set the states of the controls without calling signal
	// handlers, since these could change the checkboxes' states.
	mTooltipShowAlarms->blockSignals(true);
	mTooltipShowTime->blockSignals(true);
	mTooltipShowTimeTo->blockSignals(true);

	mTooltipShowAlarms->setChecked(maxAlarms);
	mTooltipMaxAlarms->setChecked(maxAlarms > 0);
	mTooltipMaxAlarmCount->setValue(maxAlarms > 0 ? maxAlarms : 1);
	mTooltipShowTime->setChecked(time);
	mTooltipShowTimeTo->setChecked(timeTo);
	mTooltipTimeToPrefix->setText(prefix);

	mTooltipShowAlarms->blockSignals(false);
	mTooltipShowTime->blockSignals(false);
	mTooltipShowTimeTo->blockSignals(false);

	// Enable/disable controls according to their states
	slotTooltipTimeToToggled(timeTo);
	slotTooltipAlarmsToggled(maxAlarms);
}

void ViewPrefTab::slotTooltipAlarmsToggled(bool on)
{
	mTooltipMaxAlarms->setEnabled(on);
	mTooltipMaxAlarmCount->setEnabled(on && mTooltipMaxAlarms->isChecked());
	mTooltipShowTime->setEnabled(on);
	mTooltipShowTimeTo->setEnabled(on);
	on = on && mTooltipShowTimeTo->isChecked();
	mTooltipTimeToPrefix->setEnabled(on);
	mTooltipTimeToPrefixLabel->setEnabled(on);
}

void ViewPrefTab::slotTooltipMaxToggled(bool on)
{
	mTooltipMaxAlarmCount->setEnabled(on && mTooltipMaxAlarms->isEnabled());
}

void ViewPrefTab::slotTooltipTimeToggled(bool on)
{
	if (!on  &&  !mTooltipShowTimeTo->isChecked())
		mTooltipShowTimeTo->setChecked(true);
}

void ViewPrefTab::slotTooltipTimeToToggled(bool on)
{
	if (!on  &&  !mTooltipShowTime->isChecked())
		mTooltipShowTime->setChecked(true);
	on = on && mTooltipShowTimeTo->isEnabled();
	mTooltipTimeToPrefix->setEnabled(on);
	mTooltipTimeToPrefixLabel->setEnabled(on);
}
