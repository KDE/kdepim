/*
 *  slider.cpp  -  slider control with read-only option
 *  Program:  kalarm
 *  Copyright (c) 2004 by David Jarvie <software@astrojar.org.uk>
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

#include "slider.moc"


Slider::Slider(TQWidget* parent, const char* name)
	: TQSlider(parent, name),
	  mReadOnly(false)
{ }

Slider::Slider(Orientation o, TQWidget* parent, const char* name)
	: TQSlider(o, parent, name),
	  mReadOnly(false)
{ }

Slider::Slider(int minval, int maxval, int pageStep, int value, Orientation o, TQWidget* parent, const char* name)
	: TQSlider(minval, maxval, pageStep, value, o, parent, name),
	  mReadOnly(false)
{ }

/******************************************************************************
*  Set the read-only status. If read-only, the slider can be moved by the
*  application, but not by the user.
*/
void Slider::setReadOnly(bool ro)
{
	mReadOnly = ro;
}

/******************************************************************************
*  Event handlers to intercept events if in read-only mode.
*  Any events which could change the slider value are discarded.
*/
void Slider::mousePressEvent(TQMouseEvent* e)
{
	if (mReadOnly)
	{
		// Swallow up the event if it's the left button
		if (e->button() == Qt::LeftButton)
			return;
	}
	TQSlider::mousePressEvent(e);
}

void Slider::mouseReleaseEvent(TQMouseEvent* e)
{
	if (!mReadOnly)
		TQSlider::mouseReleaseEvent(e);
}

void Slider::mouseMoveEvent(TQMouseEvent* e)
{
	if (!mReadOnly)
		TQSlider::mouseMoveEvent(e);
}

void Slider::keyPressEvent(TQKeyEvent* e)
{
	if (!mReadOnly  ||  e->key() == Qt::Key_Escape)
		TQSlider::keyPressEvent(e);
}

void Slider::keyReleaseEvent(TQKeyEvent* e)
{
	if (!mReadOnly)
		TQSlider::keyReleaseEvent(e);
}
