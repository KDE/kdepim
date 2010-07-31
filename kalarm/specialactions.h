/*
 *  specialactions.h  -  widget to specify special alarm actions
 *  Program:  kalarm
 *  Copyright © 2004,2005 by David Jarvie <software@astrojar.org.uk>
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

#ifndef SPECIALACTIONS_H
#define SPECIALACTIONS_H

#include <kdialogbase.h>
#include <tqwidget.h>
#include <tqpushbutton.h>

class KLineEdit;


class SpecialActionsButton : public QPushButton
{
		Q_OBJECT
	public:
		SpecialActionsButton(const TQString& caption, TQWidget* parent = 0, const char* name = 0);
		void           setActions(const TQString& pre, const TQString& post);
		const TQString& preAction() const      { return mPreAction; }
		const TQString& postAction() const     { return mPostAction; }
		virtual void   setReadOnly(bool ro)   { mReadOnly = ro; }
		virtual bool   isReadOnly() const     { return mReadOnly; }

	signals:
		void           selected();

	protected slots:
		void           slotButtonPressed();

	private:
		TQString  mPreAction;
		TQString  mPostAction;
		bool     mReadOnly;
};


// Pre- and post-alarm actions widget
class SpecialActions : public QWidget
{
		Q_OBJECT
	public:
		SpecialActions(TQWidget* parent = 0, const char* name = 0);
		void         setActions(const TQString& pre, const TQString& post);
		TQString      preAction() const;
		TQString      postAction() const;
		void         setReadOnly(bool);
		bool         isReadOnly() const    { return mReadOnly; }

	private:
		KLineEdit*   mPreAction;
		KLineEdit*   mPostAction;
		bool         mReadOnly;
};


// Pre- and post-alarm actions dialogue displayed by the push button
class SpecialActionsDlg : public KDialogBase
{
		Q_OBJECT
	public:
		SpecialActionsDlg(const TQString& preAction, const TQString& postAction,
		                  const TQString& caption, TQWidget* parent = 0, const char* name = 0);
		TQString      preAction() const     { return mActions->preAction(); }
		TQString      postAction() const    { return mActions->postAction(); }
		void         setReadOnly(bool ro)  { mActions->setReadOnly(ro); }
		bool         isReadOnly() const    { return mActions->isReadOnly(); }

	protected:
		virtual void resizeEvent(TQResizeEvent*);

	protected slots:
		virtual void slotOk();

	private:
		SpecialActions* mActions;
};

#endif // SPECIALACTIONS_H
