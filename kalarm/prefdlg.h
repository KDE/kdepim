/*
 *  prefdlg.h  -  program preferences dialog
 *  Program:  kalarm
 *  Copyright © 2001-2007 by David Jarvie <djarvie@kde.org>
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

#ifndef PREFDLG_H
#define PREFDLG_H

#include <tqsize.h>
#include <tqdatetime.h>
#include <ktabctl.h>
#include <kdialogbase.h>

#include "preferences.h"
#include "recurrenceedit.h"
#include "soundpicker.h"

class TQButtonGroup;
class TQCheckBox;
class TQRadioButton;
class TQPushButton;
class TQComboBox;
class TQLineEdit;
class KColorCombo;
class FontColourChooser;
class ButtonGroup;
class TimeEdit;
class SpinBox;
class SpecialActionsButton;

class FontColourPrefTab;
class EditPrefTab;
class EmailPrefTab;
class ViewPrefTab;
class MiscPrefTab;


// The Preferences dialog
class KAlarmPrefDlg : public KDialogBase
{
		Q_OBJECT
	public:
		static void display();
		~KAlarmPrefDlg();

		FontColourPrefTab* mFontColourPage;
		EditPrefTab*       mEditPage;
		EmailPrefTab*      mEmailPage;
		ViewPrefTab*       mViewPage;
		MiscPrefTab*       mMiscPage;

	protected slots:
		virtual void slotOk();
		virtual void slotApply();
		virtual void slotHelp();
		virtual void slotDefault();
		virtual void slotCancel();

	private:
		KAlarmPrefDlg();
		void            restore();

		static KAlarmPrefDlg* mInstance;
		bool            mValid;
};

// Base class for each tab in the Preferences dialog
class PrefsTabBase : public QWidget
{
		Q_OBJECT
	public:
		PrefsTabBase(TQVBox*);

		void         setPreferences();
		virtual void restore() = 0;
		virtual void apply(bool syncToDisc) = 0;
		virtual void setDefaults() = 0;
		static int   indentWidth()    { return mIndentWidth; }

	protected:
		TQVBox*       mPage;

	private:
		static int   mIndentWidth;       // indent width for checkboxes etc.
};


// Miscellaneous tab of the Preferences dialog
class MiscPrefTab : public PrefsTabBase
{
		Q_OBJECT
	public:
		MiscPrefTab(TQVBox*);

		virtual void restore();
		virtual void apply(bool syncToDisc);
		virtual void setDefaults();

	private slots:
		void         slotAutostartDaemonClicked();
		void         slotRunModeToggled(bool);
		void         slotDisableIfStoppedToggled(bool);
		void         slotExpiredToggled(bool);
		void         slotClearExpired();
		void         slotOtherTerminalToggled(bool);
//#ifdef AUTOSTART_BY_KALARMD
		void         slotAutostartToggled(bool);
//#endif

	private:
		void         setExpiredControls(int purgeDays);

		TQCheckBox*     mAutostartDaemon;
		TQRadioButton*  mRunInSystemTray;
		TQRadioButton*  mRunOnDemand;
		TQCheckBox*     mDisableAlarmsIfStopped;
		TQCheckBox*     mQuitWarn;
		TQCheckBox*     mAutostartTrayIcon;
		TQCheckBox*     mConfirmAlarmDeletion;
		TQCheckBox*     mKeepExpired;
		TQCheckBox*     mPurgeExpired;
		SpinBox*       mPurgeAfter;
		TQLabel*        mPurgeAfterLabel;
		TQPushButton*   mClearExpired;
		TimeEdit*      mStartOfDay;
		TQButtonGroup*  mXtermType;
		TQLineEdit*     mXtermCommand;
		int            mXtermFirst;              // id of first terminal window radio button
		int            mXtermCount;              // number of terminal window types
};


// Email tab of the Preferences dialog
class EmailPrefTab : public PrefsTabBase
{
		Q_OBJECT
	public:
		EmailPrefTab(TQVBox*);

		TQString      validate();
		virtual void restore();
		virtual void apply(bool syncToDisc);
		virtual void setDefaults();

	private slots:
		void         slotEmailClientChanged(int);
		void         slotFromAddrChanged(int);
		void         slotBccAddrChanged(int);
		void         slotAddressChanged()    { mAddressChanged = true; }

	private:
		void         setEmailAddress(Preferences::MailFrom, const TQString& address);
		void         setEmailBccAddress(bool useControlCentre, const TQString& address);
		TQString      validateAddr(ButtonGroup*, TQLineEdit* addr, const TQString& msg);

		ButtonGroup*   mEmailClient;
		ButtonGroup*   mFromAddressGroup;
		TQLineEdit*     mEmailAddress;
		ButtonGroup*   mBccAddressGroup;
		TQLineEdit*     mEmailBccAddress;
		TQCheckBox*     mEmailQueuedNotify;
		TQCheckBox*     mEmailCopyToKMail;
		bool           mAddressChanged;
		bool           mBccAddressChanged;
};


// Edit defaults tab of the Preferences dialog
class EditPrefTab : public PrefsTabBase
{
		Q_OBJECT
	public:
		EditPrefTab(TQVBox*);

		TQString      validate();
		virtual void restore();
		virtual void apply(bool syncToDisc);
		virtual void setDefaults();

	private slots:
		void         slotBrowseSoundFile();

	private:
		TQCheckBox*      mAutoClose;
		TQCheckBox*      mConfirmAck;
		TQComboBox*      mReminderUnits;
		SpecialActionsButton* mSpecialActionsButton;
		TQCheckBox*      mCmdScript;
		TQCheckBox*      mCmdXterm;
		TQCheckBox*      mEmailBcc;
		TQComboBox*      mSound;
		TQLabel*         mSoundFileLabel;
		TQLineEdit*      mSoundFile;
		TQPushButton*    mSoundFileBrowse;
		TQCheckBox*      mSoundRepeat;
		TQCheckBox*      mCopyToKOrganizer;
		TQCheckBox*      mLateCancel;
		TQComboBox*      mRecurPeriod;
		TQButtonGroup*   mFeb29;

		static int soundIndex(SoundPicker::Type);
		static int recurIndex(RecurrenceEdit::RepeatType);
};


// View tab of the Preferences dialog
class ViewPrefTab : public PrefsTabBase
{
		Q_OBJECT
	public:
		ViewPrefTab(TQVBox*);

		virtual void restore();
		virtual void apply(bool syncToDisc);
		virtual void setDefaults();

	private slots:
		void         slotTooltipAlarmsToggled(bool);
		void         slotTooltipMaxToggled(bool);
		void         slotTooltipTimeToggled(bool);
		void         slotTooltipTimeToToggled(bool);

	private:
		void         setTooltip(int maxAlarms, bool time, bool timeTo, const TQString& prefix);

		TQCheckBox*     mTooltipShowAlarms;
		TQCheckBox*     mTooltipMaxAlarms;
		SpinBox*       mTooltipMaxAlarmCount;
		TQCheckBox*     mTooltipShowTime;
		TQCheckBox*     mTooltipShowTimeTo;
		TQLineEdit*     mTooltipTimeToPrefix;
		TQLabel*        mTooltipTimeToPrefixLabel;
		TQCheckBox*     mModalMessages;
		SpinBox*       mDaemonTrayCheckInterval;
};


// Font & Colour tab of the Preferences dialog
class FontColourPrefTab : public PrefsTabBase
{
		Q_OBJECT
	public:
		FontColourPrefTab(TQVBox*);

		virtual void restore();
		virtual void apply(bool syncToDisc);
		virtual void setDefaults();

	private:
		FontColourChooser*  mFontChooser;
		KColorCombo*        mDisabledColour;
		KColorCombo*        mExpiredColour;
};

#endif // PREFDLG_H
