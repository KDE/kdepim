/*
 *  editdlg.h  -  dialogue to create or modify an alarm or alarm template
 *  Program:  kalarm
 *  Copyright © 2001-2006,2008 by David Jarvie <djarvie@kde.org>
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

#ifndef EDITDLG_H
#define EDITDLG_H

#include <tqdatetime.h>
#include <tqlineedit.h>

#include <kdialogbase.h>

#include "alarmevent.h"
#include "alarmtext.h"
#include "datetime.h"
#include "soundpicker.h"

class QButton;
class QGroupBox;
class QComboBox;
class QTabWidget;
class QVBox;
class QHBox;
class EmailIdCombo;
class FontColourButton;
class ColourCombo;
class ButtonGroup;
class TimeEdit;
class RadioButton;
class CheckBox;
class LateCancelSelector;
class AlarmTimeWidget;
class RecurrenceEdit;
class Reminder;
class SpecialActionsButton;
class TimeSpinBox;
class LineEdit;
class TextEdit;
class PickAlarmFileRadio;


class EditAlarmDlg : public KDialogBase
{
		Q_OBJECT
	public:
		enum MessageType { MESSAGE, FILE };
		enum CmdLogType { DISCARD_OUTPUT, LOG_TO_FILE, EXEC_IN_TERMINAL };

		EditAlarmDlg(bool Template, const TQString& caption, TQWidget* parent = 0, const char* name = 0,
                     const KAEvent* = 0, bool readOnly = false);
		virtual ~EditAlarmDlg();
		bool            getEvent(KAEvent&);
		void            setAction(KAEvent::Action, const AlarmText& = AlarmText());

		static CheckBox* createConfirmAckCheckbox(TQWidget* parent, const char* name = 0);

		static TQString  i18n_ConfirmAck();         // plain text of 'Confirm acknowledgement' checkbox
		static TQString  i18n_k_ConfirmAck();       // text of 'Confirm acknowledgement' checkbox, with 'k' shortcut
		static TQString  i18n_SpecialActions();     // plain text of 'Special Actions...' button
		static TQString  i18n_ShowInKOrganizer();   // plain text of 'Show in KOrganizer' checkbox
		static TQString  i18n_g_ShowInKOrganizer(); // text of 'Show in KOrganizer' checkbox, with 'G' shortcut
		static TQString  i18n_EnterScript();        // plain text of 'Enter a script' checkbox
		static TQString  i18n_p_EnterScript();      // text of 'Enter a script' checkbox, with 'P' shortcut
		static TQString  i18n_ExecInTermWindow();   // plain text of 'Execute in terminal window' checkbox
		static TQString  i18n_w_ExecInTermWindow(); // text of 'Execute in terminal window' radio button, with 'W' shortcut
		static TQString  i18n_u_ExecInTermWindow(); // text of 'Execute in terminal window' radio button, with 'U' shortcut
		static TQString  i18n_g_LogToFile();        // text of 'Log to file' radio button, with 'G' shortcut
		static TQString  i18n_CopyEmailToSelf();    // plain text of 'Copy email to self' checkbox
		static TQString  i18n_e_CopyEmailToSelf();  // text of 'Copy email to self' checkbox, with 'E' shortcut
		static TQString  i18n_s_CopyEmailToSelf();  // text of 'Copy email to self' checkbox, with 'S' shortcut
		static TQString  i18n_EmailFrom();          // plain text of 'From:' (email address)
		static TQString  i18n_f_EmailFrom();        // text of 'From:' (email address), with 'F' shortcut
		static TQString  i18n_EmailTo();            // plain text of 'To:' (email addressee)
		static TQString  i18n_EmailSubject();       // plain text of 'Subject:' (email)
		static TQString  i18n_j_EmailSubject();     // text of 'Subject:' (email), with 'J' shortcut

	protected:
		virtual void    resizeEvent(TQResizeEvent*);
		virtual void    showEvent(TQShowEvent*);
	protected slots:
		virtual void    slotOk();
		virtual void    slotCancel();
		virtual void    slotTry();
		virtual void    slotDefault();   // Load Template
	private slots:
		void            slotRecurTypeChange(int repeatType);
		void            slotRecurFrequencyChange();
		void            slotAlarmTypeChanged(int id);
		void            slotEditDeferral();
		void            openAddressBook();
		void            slotAddAttachment();
		void            slotRemoveAttachment();
		void            slotShowMainPage();
		void            slotShowRecurrenceEdit();
		void            slotAnyTimeToggled(bool anyTime);
		void            slotTemplateTimeType(int id);
		void            slotSetSubRepetition();
		void            slotCmdScriptToggled(bool);

	private:
		void            initialise(const KAEvent*);
		void            setReadOnly();
		void            setEvent(KAEvent&, const TQString& text, bool trial);
		KAEvent::Action getAlarmType() const;
		int             getAlarmFlags() const;
		bool            checkText(TQString& result, bool showErrorMessage = true) const;
		void            setSoundPicker();
		void            setRecurTabTitle(const KAEvent* = 0);
		bool            checkCommandData();
		bool            checkEmailData();

		void            initDisplayAlarms(TQWidget* parent);
		void            initCommand(TQWidget* parent);
		void            initEmail(TQWidget* parent);
		void            saveState(const KAEvent*);
		bool            stateChanged() const;

		TQTabWidget*         mTabs;                // the tabs in the dialog
		int                 mMainPageIndex;
		int                 mRecurPageIndex;
		bool                mMainPageShown;            // true once the main tab has been displayed
		bool                mRecurPageShown;           // true once the recurrence tab has been displayed
		bool                mRecurSetDefaultEndDate;   // adjust default end date/time when recurrence tab is displayed

		ButtonGroup*        mActionGroup;
		RadioButton*        mMessageRadio;
		RadioButton*        mCommandRadio;
		PickAlarmFileRadio* mFileRadio;
		RadioButton*        mEmailRadio;
		TQWidgetStack*       mAlarmTypeStack;

		// Templates
		TQLineEdit*          mTemplateName;
		ButtonGroup*        mTemplateTimeGroup;
		RadioButton*        mTemplateDefaultTime; // no alarm time is specified
		RadioButton*        mTemplateUseTimeAfter;// alarm time is specified as an offset from current
		RadioButton*        mTemplateAnyTime;     // alarms have date only, no time
		RadioButton*        mTemplateUseTime;     // an alarm time is specified
		TimeSpinBox*        mTemplateTimeAfter;   // the specified offset from the current time
		TimeEdit*           mTemplateTime;        // the alarm time which is specified

		// Display alarm options widgets
		TQFrame*             mDisplayAlarmsFrame;
		TQHBox*              mFileBox;
		TQHBox*              mFilePadding;
		SoundPicker*        mSoundPicker;
		CheckBox*           mConfirmAck;
		FontColourButton*   mFontColourButton;   // for text display option
		ColourCombo*        mBgColourButton;     // for file display option
		TQHBox*              mBgColourBox;
		SpecialActionsButton* mSpecialActionsButton;
		Reminder*           mReminder;
		bool                mReminderDeferral;
		bool                mReminderArchived;
		// Text message alarm widgets
		TextEdit*           mTextMessageEdit;    // text message edit box
		// Text file alarm widgets
		LineEdit*           mFileMessageEdit;    // text file URL edit box
		TQPushButton*        mFileBrowseButton;   // text file browse button
		TQString             mFileDefaultDir;     // default directory for browse button
		// Command alarm widgets
		TQFrame*             mCommandFrame;
		CheckBox*           mCmdTypeScript;      // entering a script
		LineEdit*           mCmdCommandEdit;     // command line edit box
		TextEdit*           mCmdScriptEdit;      // script edit box
		ButtonGroup*        mCmdOutputGroup;     // what to do with command output
		LineEdit*           mCmdLogFileEdit;     // log file URL edit box
		TQWidget*            mCmdPadding;
		// Email alarm widgets
		TQFrame*             mEmailFrame;
		EmailIdCombo*       mEmailFromList;
		LineEdit*           mEmailToEdit;
		TQPushButton*        mEmailAddressButton; // email open address book button
		TQLineEdit*          mEmailSubjectEdit;
		TextEdit*           mEmailMessageEdit;   // email body edit box
		TQComboBox*          mEmailAttachList;
		TQPushButton*        mEmailAddAttachButton;
		TQPushButton*        mEmailRemoveButton;
		CheckBox*           mEmailBcc;
		TQString             mAttachDefaultDir;

		TQGroupBox*          mDeferGroup;
		TQLabel*             mDeferTimeLabel;
		TQPushButton*        mDeferChangeButton;

		AlarmTimeWidget*    mTimeWidget;
		LateCancelSelector* mLateCancel;
		CheckBox*           mShowInKorganizer;

		RecurrenceEdit*     mRecurrenceEdit;

		TQString             mAlarmMessage;       // message text/file name/command/email message
		DateTime            mAlarmDateTime;
		DateTime            mDeferDateTime;
		EmailAddressList    mEmailAddresses;     // list of addresses to send email to
		TQStringList         mEmailAttachments;   // list of email attachment file names
		unsigned long       mKMailSerialNumber;  // if email text, message's KMail serial number, else 0
		int                 mDeferGroupHeight;   // height added by deferred time widget
		int                 mDesktop;            // desktop to display the dialog in
		bool                mTemplate;           // editing an alarm template
		bool                mExpiredRecurrence;  // initially a recurrence which has expired
		mutable bool        mChanged;            // controls other than deferral have changed since dialog was displayed
		mutable bool        mOnlyDeferred;       // the only change made in the dialog was to the existing deferral
		bool                mDesiredReadOnly;    // the specified read-only status of the dialogue
		bool                mReadOnly;           // the actual read-only status of the dialogue

		// Initial state of all controls
		KAEvent*            mSavedEvent;
		TQString             mSavedTemplateName;   // mTemplateName value
		TQButton*            mSavedTemplateTimeType; // selected button in mTemplateTimeGroup
		TQTime               mSavedTemplateTime;   // mTemplateTime value
		int                 mSavedTemplateAfterTime; // mTemplateAfterTime value
		TQButton*            mSavedTypeRadio;      // mMessageRadio, etc
		SoundPicker::Type   mSavedSoundType;      // mSoundPicker sound type
		bool                mSavedRepeatSound;    // mSoundPicker repeat status
		TQString             mSavedSoundFile;      // mSoundPicker sound file
		float               mSavedSoundVolume;    // mSoundPicker volume
		float               mSavedSoundFadeVolume;// mSoundPicker fade volume
		int                 mSavedSoundFadeSeconds;// mSoundPicker fade time
		bool                mSavedConfirmAck;     // mConfirmAck status
		TQFont               mSavedFont;           // mFontColourButton font
		TQColor              mSavedBgColour;       // mFontColourButton background colour
		TQColor              mSavedFgColour;       // mFontColourButton foreground colour
		TQString             mSavedPreAction;      // mSpecialActionsButton pre-alarm action
		TQString             mSavedPostAction;     // mSpecialActionsButton post-alarm action
		int                 mSavedReminder;       // mReminder value
		bool                mSavedOnceOnly;       // mReminder once-only status
		TQString             mSavedTextFileCommandMessage;  // mTextMessageEdit/mFileMessageEdit/mCmdCommandEdit/mEmailMessageEdit value
		TQString             mSavedEmailFrom;      // mEmailFromList current value
		TQString             mSavedEmailTo;        // mEmailToEdit value
		TQString             mSavedEmailSubject;   // mEmailSubjectEdit value
		TQStringList         mSavedEmailAttach;    // mEmailAttachList values
		bool                mSavedEmailBcc;       // mEmailBcc status
		bool                mSavedCmdScript;      // mCmdTypeScript status
		TQButton*            mSavedCmdOutputRadio; // selected button in mCmdOutputGroup
		TQString             mSavedCmdLogFile;     // mCmdLogFileEdit value
		DateTime            mSavedDateTime;       // mTimeWidget value
		int                 mSavedRecurrenceType; // RecurrenceEdit::RepeatType value
		int                 mSavedLateCancel;     // mLateCancel value
		bool                mSavedAutoClose;      // mLateCancel->isAutoClose() value
		bool                mSavedShowInKorganizer; // mShowInKorganizer status
};

#endif // EDITDLG_H
