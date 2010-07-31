/*
 *  pushbutton.cpp  -  push button with read-only option
 *  Program:  kalarm
 *  Copyright (c) 2002 by David Jarvie <software@astrojar.org.uk>
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

#include "pushbutton.moc"


PushButton::PushButton(TQWidget* parent, const char* name)
	: TQPushButton(parent, name),
	  mFocusPolicy(focusPolicy()),
	  mReadOnly(false)
{ }

PushButton::PushButton(const TQString& text, TQWidget* parent, const char* name)
	: TQPushButton(text, parent, name),
	  mFocusPolicy(focusPolicy()),
	  mReadOnly(false)
{ }

PushButton::PushButton(const TQIconSet& icon, const TQString& text, TQWidget* parent, const char* name)
	: TQPushButton(icon, text, parent, name),
	  mFocusPolicy(focusPolicy()),
	  mReadOnly(false)
{ }

void PushButton::setReadOnly(bool ro)
{
	if ((int)ro != (int)mReadOnly)
	{
		mReadOnly = ro;
		setFocusPolicy(ro ? TQWidget::NoFocus : mFocusPolicy);
		if (ro)
			clearFocus();
	}
}

void PushButton::mousePressEvent(TQMouseEvent* e)
{
	if (mReadOnly)
	{
		// Swallow up the event if it's the left button
		if (e->button() == Qt::LeftButton)
			return;
	}
	TQPushButton::mousePressEvent(e);
}

void PushButton::mouseReleaseEvent(TQMouseEvent* e)
{
	if (mReadOnly)
	{
		// Swallow up the event if it's the left button
		if (e->button() == Qt::LeftButton)
			return;
	}
	TQPushButton::mouseReleaseEvent(e);
}

void PushButton::mouseMoveEvent(TQMouseEvent* e)
{
	if (!mReadOnly)
		TQPushButton::mouseMoveEvent(e);
}

void PushButton::keyPressEvent(TQKeyEvent* e)
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
	TQPushButton::keyPressEvent(e);
}

void PushButton::keyReleaseEvent(TQKeyEvent* e)
{
	if (!mReadOnly)
		TQPushButton::keyReleaseEvent(e);
}
