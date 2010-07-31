/*
 *  checkbox.cpp  -  check box with read-only option
 *  Program:  kalarm
 *  Copyright (c) 2002, 2003 by David Jarvie <software@astrojar.org.uk>
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

#include "checkbox.moc"


CheckBox::CheckBox(TQWidget* parent, const char* name)
	: TQCheckBox(parent, name),
	  mFocusPolicy(focusPolicy()),
	  mFocusWidget(0),
	  mReadOnly(false)
{ }

CheckBox::CheckBox(const TQString& text, TQWidget* parent, const char* name)
	: TQCheckBox(text, parent, name),
	  mFocusPolicy(focusPolicy()),
	  mFocusWidget(0),
	  mReadOnly(false)
{ }

/******************************************************************************
*  Set the read-only status. If read-only, the checkbox can be toggled by the
*  application, but not by the user.
*/
void CheckBox::setReadOnly(bool ro)
{
	if ((int)ro != (int)mReadOnly)
	{
		mReadOnly = ro;
		setFocusPolicy(ro ? TQWidget::NoFocus : mFocusPolicy);
		if (ro)
			clearFocus();
	}
}

/******************************************************************************
*  Specify a widget to receive focus when the checkbox is clicked on.
*/
void CheckBox::setFocusWidget(TQWidget* w, bool enable)
{
	mFocusWidget = w;
	mFocusWidgetEnable = enable;
	if (w)
		connect(this, TQT_SIGNAL(clicked()), TQT_SLOT(slotClicked()));
	else
		disconnect(this, TQT_SIGNAL(clicked()), this, TQT_SLOT(slotClicked()));
}

/******************************************************************************
*  Called when the checkbox is clicked.
*  If it is now checked, focus is transferred to any specified focus widget.
*/
void CheckBox::slotClicked()
{
	if (mFocusWidget  &&  isChecked())
	{
		if (mFocusWidgetEnable)
			mFocusWidget->setEnabled(true);
		mFocusWidget->setFocus();
	}
}

/******************************************************************************
*  Event handlers to intercept events if in read-only mode.
*  Any events which could change the checkbox state are discarded.
*/
void CheckBox::mousePressEvent(TQMouseEvent* e)
{
	if (mReadOnly)
	{
		// Swallow up the event if it's the left button
		if (e->button() == Qt::LeftButton)
			return;
	}
	TQCheckBox::mousePressEvent(e);
}

void CheckBox::mouseReleaseEvent(TQMouseEvent* e)
{
	if (mReadOnly)
	{
		// Swallow up the event if it's the left button
		if (e->button() == Qt::LeftButton)
			return;
	}
	TQCheckBox::mouseReleaseEvent(e);
}

void CheckBox::mouseMoveEvent(TQMouseEvent* e)
{
	if (!mReadOnly)
		TQCheckBox::mouseMoveEvent(e);
}

void CheckBox::keyPressEvent(TQKeyEvent* e)
{
	if (mReadOnly)
		switch (e->key())
		{
			case Qt::Key_Up:
			case Qt::Key_Left:
			case Qt::Key_Right:
			case Qt::Key_Down:
				// Process keys which shift the focus
				break;
			default:
				return;
		}
	TQCheckBox::keyPressEvent(e);
}

void CheckBox::keyReleaseEvent(TQKeyEvent* e)
{
	if (!mReadOnly)
		TQCheckBox::keyReleaseEvent(e);
}
