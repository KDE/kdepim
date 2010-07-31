/*
 *  fontcolour.cpp  -  font and colour chooser widget
 *  Program:  kalarm
 *  Copyright © 2001-2003,2005,2008 by David Jarvie <software@astrojar.org.uk>
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

#include <tqobjectlist.h>
#include <tqwidget.h>
#include <tqgroupbox.h>
#include <tqpushbutton.h>
#include <tqhbox.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqwhatsthis.h>

#include <kglobal.h>
#include <klocale.h>
#include <kcolordialog.h>

#include "kalarmapp.h"
#include "preferences.h"
#include "colourcombo.h"
#include "checkbox.h"
#include "fontcolour.moc"


FontColourChooser::FontColourChooser(TQWidget *parent, const char *name,
          bool onlyFixed, const TQStringList &fontList,
          const TQString& frameLabel, bool editColours, bool fg, bool defaultFont,
          int visibleListSize)
	: TQWidget(parent, name),
	  mFgColourButton(0),
	  mRemoveColourButton(0),
	  mColourList(Preferences::messageColours()),
	  mReadOnly(false)
{
	TQVBoxLayout* topLayout = new TQVBoxLayout(this, 0, KDialog::spacingHint());
	TQWidget* page = this;
	if (!frameLabel.isNull())
	{
		page = new TQGroupBox(frameLabel, this);
		topLayout->addWidget(page);
		topLayout = new TQVBoxLayout(page, KDialog::marginHint(), KDialog::spacingHint());
		topLayout->addSpacing(fontMetrics().height() - KDialog::marginHint() + KDialog::spacingHint());
	}
	TQHBoxLayout* hlayout = new TQHBoxLayout(topLayout);
	TQVBoxLayout* colourLayout = new TQVBoxLayout(hlayout);
	if (fg)
	{
		TQHBox* box = new TQHBox(page);    // to group widgets for TQWhatsThis text
		box->setSpacing(KDialog::spacingHint()/2);
		colourLayout->addWidget(box);

		TQLabel* label = new TQLabel(i18n("&Foreground color:"), box);
		box->setStretchFactor(new TQWidget(box), 0);
		mFgColourButton = new ColourCombo(box);
		connect(mFgColourButton, TQT_SIGNAL(activated(const TQString&)), TQT_SLOT(setSampleColour()));
		label->setBuddy(mFgColourButton);
		TQWhatsThis::add(box, i18n("Select the alarm message foreground color"));
	}

	TQHBox* box = new TQHBox(page);    // to group widgets for TQWhatsThis text
	box->setSpacing(KDialog::spacingHint()/2);
	colourLayout->addWidget(box);

	TQLabel* label = new TQLabel(i18n("&Background color:"), box);
	box->setStretchFactor(new TQWidget(box), 0);
	mBgColourButton = new ColourCombo(box);
	connect(mBgColourButton, TQT_SIGNAL(activated(const TQString&)), TQT_SLOT(setSampleColour()));
	label->setBuddy(mBgColourButton);
	TQWhatsThis::add(box, i18n("Select the alarm message background color"));
	hlayout->addStretch();

	if (editColours)
	{
		TQHBoxLayout* layout = new TQHBoxLayout(topLayout);
		TQPushButton* button = new TQPushButton(i18n("Add Co&lor..."), page);
		button->setFixedSize(button->sizeHint());
		connect(button, TQT_SIGNAL(clicked()), TQT_SLOT(slotAddColour()));
		TQWhatsThis::add(button, i18n("Choose a new color to add to the color selection list."));
		layout->addWidget(button);

		mRemoveColourButton = new TQPushButton(i18n("&Remove Color"), page);
		mRemoveColourButton->setFixedSize(mRemoveColourButton->sizeHint());
		connect(mRemoveColourButton, TQT_SIGNAL(clicked()), TQT_SLOT(slotRemoveColour()));
		TQWhatsThis::add(mRemoveColourButton,
		      i18n("Remove the color currently shown in the background color chooser, from the color selection list."));
		layout->addWidget(mRemoveColourButton);
	}

	if (defaultFont)
	{
		TQHBoxLayout* layout = new TQHBoxLayout(topLayout);
		mDefaultFont = new CheckBox(i18n("Use &default font"), page);
		mDefaultFont->setMinimumSize(mDefaultFont->sizeHint());
		connect(mDefaultFont, TQT_SIGNAL(toggled(bool)), TQT_SLOT(slotDefaultFontToggled(bool)));
		TQWhatsThis::add(mDefaultFont,
		      i18n("Check to use the default font current at the time the alarm is displayed."));
		layout->addWidget(mDefaultFont);
		layout->addWidget(new TQWidget(page));    // left adjust the widget
	}
	else
		mDefaultFont = 0;

	mFontChooser = new KFontChooser(page, name, onlyFixed, fontList, false, visibleListSize);
	mFontChooser->installEventFilter(this);   // for read-only mode
	const TQObjectList* kids = mFontChooser->queryList();
	for (TQObjectList::ConstIterator it = kids->constBegin();  it != kids->constEnd();  ++it)
		(*it)->installEventFilter(this);
	topLayout->addWidget(mFontChooser);

	slotDefaultFontToggled(false);
}

void FontColourChooser::setDefaultFont()
{
	if (mDefaultFont)
		mDefaultFont->setChecked(true);
}

void FontColourChooser::setFont(const TQFont& font, bool onlyFixed)
{
	if (mDefaultFont)
		mDefaultFont->setChecked(false);
	mFontChooser->setFont(font, onlyFixed);
}

bool FontColourChooser::defaultFont() const
{
	return mDefaultFont ? mDefaultFont->isChecked() : false;
}

TQFont FontColourChooser::font() const
{
	return (mDefaultFont && mDefaultFont->isChecked()) ? TQFont() : mFontChooser->font();
}

void FontColourChooser::setBgColour(const TQColor& colour)
{
	mBgColourButton->setColor(colour);
	mFontChooser->setBackgroundColor(colour);
}

void FontColourChooser::setSampleColour()
{
	TQColor bg = mBgColourButton->color();
	mFontChooser->setBackgroundColor(bg);
	TQColor fg = fgColour();
	mFontChooser->setColor(fg);
	if (mRemoveColourButton)
		mRemoveColourButton->setEnabled(!mBgColourButton->isCustomColour());   // no deletion of custom colour
}

TQColor FontColourChooser::bgColour() const
{
	return mBgColourButton->color();
}

TQColor FontColourChooser::fgColour() const
{
	if (mFgColourButton)
		return mFgColourButton->color();
	else
	{
		TQColor bg = mBgColourButton->color();
		TQPalette pal(bg, bg);
		return pal.color(TQPalette::Active, TQColorGroup::Text);
	}
}

TQString FontColourChooser::sampleText() const
{
	return mFontChooser->sampleText();
}

void FontColourChooser::setSampleText(const TQString& text)
{
	mFontChooser->setSampleText(text);
}

void FontColourChooser::setFgColour(const TQColor& colour)
{
	if (mFgColourButton)
	{
		mFgColourButton->setColor(colour);
		mFontChooser->setColor(colour);
	}
}

void FontColourChooser::setReadOnly(bool ro)
{
	if (ro != mReadOnly)
	{
		mReadOnly = ro;
		if (mFgColourButton)
			mFgColourButton->setReadOnly(ro);
		mBgColourButton->setReadOnly(ro);
		mDefaultFont->setReadOnly(ro);
	}
}

bool FontColourChooser::eventFilter(TQObject*, TQEvent* e)
{
	if (mReadOnly)
	{
		switch (e->type())
		{
			case TQEvent::MouseButtonPress:
			case TQEvent::MouseButtonRelease:
			case TQEvent::MouseButtonDblClick:
			case TQEvent::KeyPress:
			case TQEvent::KeyRelease:
				return true;   // prevent the event being handled
			default:
				break;
		}
	}
	return false;
}

void FontColourChooser::slotDefaultFontToggled(bool on)
{
	mFontChooser->setEnabled(!on);
}

void FontColourChooser::setColours(const ColourList& colours)
{
	mColourList = colours;
	mBgColourButton->setColours(mColourList);
	mFontChooser->setBackgroundColor(mBgColourButton->color());
}

void FontColourChooser::slotAddColour()
{
	TQColor colour;
	if (KColorDialog::getColor(colour, this) == TQDialog::Accepted)
	{
		mColourList.insert(colour);
		mBgColourButton->setColours(mColourList);
	}
}

void FontColourChooser::slotRemoveColour()
{
	if (!mBgColourButton->isCustomColour())
	{
		mColourList.remove(mBgColourButton->color());
		mBgColourButton->setColours(mColourList);
	}
}

