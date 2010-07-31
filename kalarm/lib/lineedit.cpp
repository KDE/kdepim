/*
 *  lineedit.cpp  -  Line edit widget with extra drag and drop options
 *  Program:  kalarm
 *  Copyright (C) 2003 - 2005 by David Jarvie <software@astrojar.org.uk>
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

#include <tqregexp.h>
#include <tqdragobject.h>

#include <kurldrag.h>
#include <kurlcompletion.h>

#include <libkdepim/maillistdrag.h>
#include <libkdepim/kvcarddrag.h>
#include <libkcal/icaldrag.h>

#include "lineedit.moc"


/*=============================================================================
= Class LineEdit
= Line edit which accepts drag and drop of text, URLs and/or email addresses.
* It has an option to prevent its contents being selected when it receives
= focus.
=============================================================================*/
LineEdit::LineEdit(Type type, TQWidget* parent, const char* name)
	: KLineEdit(parent, name),
	  mType(type),
	  mNoSelect(false),
	  mSetCursorAtEnd(false)
{
	init();
}

LineEdit::LineEdit(TQWidget* parent, const char* name)
	: KLineEdit(parent, name),
	  mType(Text),
	  mNoSelect(false),
	  mSetCursorAtEnd(false)
{
	init();
}

void LineEdit::init()
{
	if (mType == Url)
	{
		setCompletionMode(KGlobalSettings::CompletionShell);
		KURLCompletion* comp = new KURLCompletion(KURLCompletion::FileCompletion);
		comp->setReplaceHome(true);
		setCompletionObject(comp);
		setAutoDeleteCompletionObject(true);
	}
	else
		setCompletionMode(KGlobalSettings::CompletionNone);
}

/******************************************************************************
*  Called when the line edit receives focus.
*  If 'noSelect' is true, prevent the contents being selected.
*/
void LineEdit::focusInEvent(TQFocusEvent* e)
{
	if (mNoSelect)
		TQFocusEvent::setReason(TQFocusEvent::Other);
	KLineEdit::focusInEvent(e);
	if (mNoSelect)
	{
		TQFocusEvent::resetReason();
		mNoSelect = false;
	}
}

void LineEdit::setText(const TQString& text)
{
	KLineEdit::setText(text);
	setCursorPosition(mSetCursorAtEnd ? text.length() : 0);
}

void LineEdit::dragEnterEvent(TQDragEnterEvent* e)
{
	if (KCal::ICalDrag::canDecode(e))
		e->accept(false);   // don't accept "text/calendar" objects
	e->accept(TQTextDrag::canDecode(e)
	       || KURLDrag::canDecode(e)
	       || mType != Url && KPIM::MailListDrag::canDecode(e)
	       || mType == Emails && KVCardDrag::canDecode(e));
}

void LineEdit::dropEvent(TQDropEvent* e)
{
	TQString               newText;
	TQStringList           newEmails;
	TQString               txt;
	KPIM::MailList        mailList;
	KURL::List            files;
	KABC::Addressee::List addrList;

	if (mType != Url
	&&  e->provides(KPIM::MailListDrag::format())
	&&  KPIM::MailListDrag::decode(e, mailList))
	{
		// KMail message(s) - ignore all but the first
		if (mailList.count())
		{
			if (mType == Emails)
				newText = mailList.first().from();
			else
				setText(mailList.first().subject());    // replace any existing text
		}
	}
	// This must come before KURLDrag
	else if (mType == Emails
	&&  KVCardDrag::canDecode(e)  &&  KVCardDrag::decode(e, addrList))
	{
		// KAddressBook entries
		for (KABC::Addressee::List::Iterator it = addrList.begin();  it != addrList.end();  ++it)
		{
			TQString em((*it).fullEmail());
			if (!em.isEmpty())
				newEmails.append(em);
		}
	}
	else if (KURLDrag::decode(e, files)  &&  files.count())
	{
		// URL(s)
		switch (mType)
		{
			case Url:
				// URL entry field - ignore all but the first dropped URL
				setText(files.first().prettyURL());    // replace any existing text
				break;
			case Emails:
			{
				// Email entry field - ignore all but mailto: URLs
				TQString mailto = TQString::fromLatin1("mailto");
				for (KURL::List::Iterator it = files.begin();  it != files.end();  ++it)
				{
					if ((*it).protocol() == mailto)
						newEmails.append((*it).path());
				}
				break;
			}
			case Text:
				newText = files.first().prettyURL();
				break;
		}
	}
	else if (TQTextDrag::decode(e, txt))
	{
		// Plain text
		if (mType == Emails)
		{
			// Remove newlines from a list of email addresses, and allow an eventual mailto: protocol
			TQString mailto = TQString::fromLatin1("mailto:");
			newEmails = TQStringList::split(TQRegExp("[\r\n]+"), txt);
			for (TQStringList::Iterator it = newEmails.begin();  it != newEmails.end();  ++it)
			{
				if ((*it).startsWith(mailto))
				{
					KURL url(*it);
					*it = url.path();
				}
			}
		}
		else
		{
			int newline = txt.find('\n');
			newText = (newline >= 0) ? txt.left(newline) : txt;
		}
	}

	if (newEmails.count())
	{
		newText = newEmails.join(",");
		int c = cursorPosition();
		if (c > 0)
			newText.prepend(",");
		if (c < static_cast<int>(text().length()))
			newText.append(",");
	}
	if (!newText.isEmpty())
		insert(newText);
}
