/*
 *  sounddlg.h  -  sound file selection and configuration dialog
 *  Program:  kalarm
 *  Copyright © 2005,2007,2008 by David Jarvie <djarvie@kde.org>
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

#ifndef SOUNDDLG_H
#define SOUNDDLG_H

#include <tqframe.h>
#include <tqstring.h>
#include <kdialogbase.h>

class QHBox;
class QPushButton;
class KArtsDispatcher;
namespace KDE { class PlayObject; }
class LineEdit;
class PushButton;
class CheckBox;
class SpinBox;
class Slider;


class SoundDlg : public KDialogBase
{
		Q_OBJECT
	public:
		SoundDlg(const TQString& file, float volume, float fadeVolume, int fadeSeconds, bool repeat,
		         const TQString& caption, TQWidget* parent, const char* name = 0);
		~SoundDlg();
		void           setReadOnly(bool);
		bool           isReadOnly() const    { return mReadOnly; }
		TQString        getFile() const       { return mFileName; }
		bool           getSettings(float& volume, float& fadeVolume, int& fadeSeconds) const;
		TQString        defaultDir() const    { return mDefaultDir; }

		static TQString i18n_SetVolume();   // plain text of Set volume checkbox
		static TQString i18n_v_SetVolume(); // text of Set volume checkbox, with 'V' shortcut
		static TQString i18n_Repeat();      // plain text of Repeat checkbox
		static TQString i18n_p_Repeat();    // text of Repeat checkbox, with 'P' shortcut

	protected:
		virtual void   showEvent(TQShowEvent*);
		virtual void   resizeEvent(TQResizeEvent*);

	protected slots:
		virtual void   slotOk();

	private slots:
		void           slotPickFile();
		void           slotVolumeToggled(bool on);
		void           slotFadeToggled(bool on);
		void           playSound();
		void           checkAudioPlay();

	private:
		void           stopPlay();
		bool           checkFile();

		TQPushButton*   mFilePlay;
		LineEdit*      mFileEdit;
		PushButton*    mFileBrowseButton;
		CheckBox*      mRepeatCheckbox;
		CheckBox*      mVolumeCheckbox;
		Slider*        mVolumeSlider;
		CheckBox*      mFadeCheckbox;
		TQHBox*         mFadeBox;
		SpinBox*       mFadeTime;
		TQHBox*         mFadeVolumeBox;
		Slider*        mFadeSlider;
		TQString        mDefaultDir;     // current default directory for mFileEdit
		TQString        mFileName;
		bool           mReadOnly;
		// Sound file playing
		KArtsDispatcher* mArtsDispatcher;
		KDE::PlayObject* mPlayObject;
		TQTimer*          mPlayTimer;       // timer for playing the sound file
		TQString          mLocalAudioFile;  // local copy of audio file
		bool             mPlayStarted;      // the sound file has started playing
};

#endif
