/*
 *  colourcombo.cpp  -  colour selection combo box
 *  Program:  kalarm
 *  Copyright (c) 2001 - 2003, 2005 by David Jarvie <software@astrojar.org.uk>
 *
 *  Some code taken from kdelibs/kdeui/kcolorcombo.cpp in the KDE libraries:
 *  Copyright (C) 1997 Martin Jones (mjones@kde.org)
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

#include <tqpainter.h>

#include <klocale.h>
#include <kcolordialog.h>

#include "preferences.h"
#include "colourcombo.moc"


ColourCombo::ColourCombo(TQWidget* parent, const char* name, const TQColor& defaultColour)
	: TQComboBox(parent, name),
	  mColourList(Preferences::messageColours()),
	  mSelectedColour(defaultColour),
	  mCustomColour(255, 255, 255),
	  mReadOnly(false),
	  mDisabled(false)
{
	addColours();
	connect(this, TQT_SIGNAL(activated(int)), TQT_SLOT(slotActivated(int)));
	connect(this, TQT_SIGNAL(highlighted(int)), TQT_SLOT(slotHighlighted(int)));
	Preferences::connect(TQT_SIGNAL(preferencesChanged()), this, TQT_SLOT(slotPreferencesChanged()));
}

void ColourCombo::setColour(const TQColor& colour)
{
	mSelectedColour = colour;
	addColours();
}

/******************************************************************************
*  Set a new colour selection.
*/
void ColourCombo::setColours(const ColourList& colours)
{
	mColourList = colours;
	if (mSelectedColour != mCustomColour
	&&  !mColourList.contains(mSelectedColour))
	{
		// The current colour has been deleted
		mSelectedColour = mColourList.count() ? mColourList.first() : mCustomColour;
	}
	addColours();
}

/******************************************************************************
*  Called when the user changes the preference settings.
*  If the colour list has changed, update the colours displayed.
*/
void ColourCombo::slotPreferencesChanged()
{
	const ColourList& prefColours = Preferences::messageColours();
	if (prefColours != mColourList)
		setColours(prefColours);      // update the display with the new colours
}

/******************************************************************************
*  Enable or disable the control.
*  If it is disabled, its colour is set to the dialog background colour.
*/
void ColourCombo::setEnabled(bool enable)
{
	if (enable  &&  mDisabled)
	{
		mDisabled = false;
		setColour(mSelectedColour);
	}
	else if (!enable  &&  !mDisabled)
	{
		mSelectedColour = color();
		int end = count();
		if (end > 1)
		{
			// Add a dialog background colour item
			TQPixmap pm = *pixmap(1);
			pm.fill(paletteBackgroundColor());
			insertItem(pm);
			setCurrentItem(end);
		}
		mDisabled = true;
	}
	TQComboBox::setEnabled(enable);
}

void ColourCombo::slotActivated(int index)
{
	if (index)
		mSelectedColour = mColourList[index - 1];
	else
	{
		if (KColorDialog::getColor(mCustomColour, this) == TQDialog::Accepted)
		{
			TQRect rect;
			drawCustomItem(rect, false);
		}
		mSelectedColour = mCustomColour;
	}
	emit activated(mSelectedColour);
}

void ColourCombo::slotHighlighted(int index)
{
	mSelectedColour = index ? mColourList[index - 1] : mCustomColour;
	emit highlighted(mSelectedColour);
}

/******************************************************************************
*  Initialise the items in the combo box to one for each colour in the list.
*/
void ColourCombo::addColours()
{
	clear();

	for (ColourList::const_iterator it = mColourList.begin();  ;  ++it)
	{
		if (it == mColourList.end())
		{
			mCustomColour = mSelectedColour;
			break;
		}
		if (mSelectedColour == *it)
			break;
	}

	TQRect rect;
	drawCustomItem(rect, true);

	TQPainter painter;
	TQPixmap pixmap(rect.width(), rect.height());
	int i = 1;
	for (ColourList::const_iterator it = mColourList.begin();  it != mColourList.end();  ++i, ++it)
	{
		painter.begin(&pixmap);
		TQBrush brush(*it);
		painter.fillRect(rect, brush);
		painter.end();

		insertItem(pixmap);
		pixmap.detach();

		if (*it == mSelectedColour.rgb())
			setCurrentItem(i);
	}
}

void ColourCombo::drawCustomItem(TQRect& rect, bool insert)
{
	TQPen pen;
	if (qGray(mCustomColour.rgb()) < 128)
		pen.setColor(Qt::white);
	else
		pen.setColor(Qt::black);

	TQPainter painter;
	TQFontMetrics fm = TQFontMetrics(painter.font());
	rect.setRect(0, 0, width(), fm.height() + 4);
	TQPixmap pixmap(rect.width(), rect.height());

	painter.begin(&pixmap);
	TQBrush brush(mCustomColour);
	painter.fillRect(rect, brush);
	painter.setPen(pen);
	painter.drawText(2, fm.ascent() + 2, i18n("Custom..."));
	painter.end();

	if (insert)
		insertItem(pixmap);
	else
		changeItem(pixmap, 0);
	pixmap.detach();
}

void ColourCombo::setReadOnly(bool ro)
{
	mReadOnly = ro;
}

void ColourCombo::resizeEvent(TQResizeEvent* re)
{
	TQComboBox::resizeEvent(re);
	addColours();
}

void ColourCombo::mousePressEvent(TQMouseEvent* e)
{
	if (mReadOnly)
	{
		// Swallow up the event if it's the left button
		if (e->button() == Qt::LeftButton)
			return;
	}
	TQComboBox::mousePressEvent(e);
}

void ColourCombo::mouseReleaseEvent(TQMouseEvent* e)
{
	if (!mReadOnly)
		TQComboBox::mouseReleaseEvent(e);
}

void ColourCombo::mouseMoveEvent(TQMouseEvent* e)
{
	if (!mReadOnly)
		TQComboBox::mouseMoveEvent(e);
}

void ColourCombo::keyPressEvent(TQKeyEvent* e)
{
	if (!mReadOnly  ||  e->key() == Qt::Key_Escape)
		TQComboBox::keyPressEvent(e);
}

void ColourCombo::keyReleaseEvent(TQKeyEvent* e)
{
	if (!mReadOnly)
		TQComboBox::keyReleaseEvent(e);
}
