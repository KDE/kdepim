/*
	Empath - Mailer for KDE
	
	Copyright (C) 1998 Rik Hemsley rik@kde.org
	
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <ctype.h>

// Qt includes
#include <qfile.h>
#include <qtextstream.h>
#include <qstring.h>
#include <qpngio.h>

// KDE includes
#include <klocale.h>
#include <kcursor.h>
#include <kapp.h>
#include <kconfig.h>
#include <kglobal.h>

// Local includes
#include "RMM_Message.h"
#include "RMM_Enum.h"
#include "EmpathMessageHTMLView.h"
#include "EmpathConfig.h"
#include "Empath.h"
#include "EmpathUIUtils.h"
#include "EmpathUtilities.h"


EmpathMessageHTMLWidget::EmpathMessageHTMLWidget(
		QWidget			*	_parent,
		const char		*	_name)
	:	KHTMLWidget(_parent, _name),
		busy_(false)
{
	empathDebug("ctor");
	qInitPngIO();
	
	KConfig * c = KGlobal::config();
	c->setGroup(EmpathConfig::GROUP_DISPLAY);
	QString iconSet = c->readEntry(EmpathConfig::KEY_ICON_SET);
	
	begin();
	// Begin welcome message
	write("<HTML>");
	write("<BODY BGCOLOR=\"#");
	write(QColorToHTML(
			kapp->palette().color(QPalette::Normal, QColorGroup::Base)));
	write("\">");
	write("<TT><FONT COLOR=\"#");
	write(QColorToHTML(
			kapp->palette().color(QPalette::Normal, QColorGroup::Text)));
	write("\">");
	write (i18n("Welcome to Empath").ascii());
	write ("</FONT></TT></HTML>");
	// End welcome message
	parse();
	end();

	QObject::connect(
		this, SIGNAL(popupMenu(const char *, const QPoint &)),
		this, SLOT(s_popupMenu(const char *, const QPoint &)));
	
	empathDebug("ctor finished");
}

EmpathMessageHTMLWidget::~EmpathMessageHTMLWidget()
{
	empathDebug("dtor");
}

	bool
EmpathMessageHTMLWidget::show(const QCString & s, bool markup)
{
	empathDebug("show() called");
	
	if (busy_) return false;
	busy_ = true;

	setCursor(waitCursor);
	
	KConfig * config(KGlobal::config());
	config->setGroup(EmpathConfig::GROUP_DISPLAY);
	
	QFont defaultFixed(kapp->fixedFont());
	
	QFont f =
		config->readFontEntry(
			EmpathConfig::KEY_FIXED_FONT, &defaultFixed);

	setFixedFont(f.family().ascii());
	
	int fs = f.pointSize();
	
	int fsizes[7] = { fs, fs, fs, fs, fs, fs, fs };
	setFontSizes(fsizes);
	
	setStandardFont(kapp->generalFont().family().ascii());

	setURLCursor(KCursor::handCursor());
	setFocusPolicy(QWidget::StrongFocus);
	
	KConfig * c = KGlobal::config();
	c->setGroup(EmpathConfig::GROUP_DISPLAY);
	
	setDefaultBGColor(
		kapp->palette().color(QPalette::Normal, QColorGroup::Base));
	setDefaultTextColors(
		kapp->palette().color(QPalette::Normal, QColorGroup::Text),
		c->readColorEntry(EmpathConfig::KEY_LINK_COLOUR),
		c->readColorEntry(EmpathConfig::KEY_VISITED_LINK_COLOUR));
	
	setUnderlineLinks(c->readBoolEntry(EmpathConfig::KEY_UNDERLINE_LINKS));

	if (s.isEmpty()) {
		write(
			"<HTML><BODY BGCOLOR=" +
			QColorToHTML(
				kapp->palette().color(QPalette::Normal, QColorGroup::Base)) +
			"><PRE>" +
			i18n("This part is empty").ascii() +
			"</PRE></BODY></HTML>");
		setCursor(arrowCursor);
		parse();
		busy_ = false;
		return true;
	}
		
	begin();

	if (markup) {
		
		QCString html(s);
		toHTML(html);
		write(
			"<HTML><BODY BGCOLOR=" +
			QColorToHTML(
				kapp->palette().color(QPalette::Normal, QColorGroup::Base)) +
			"><PRE>" +
			html +
			"</PRE></BODY></HTML>");
	
	} else {
		
		empathDebug("No markup required");

		write("<HTML><BODY><PRE>" + s + "</PRE></BODY></HTML>");
	}
	
	setCursor(arrowCursor);
	parse();
	busy_ = false;
	return true;
}

	void
EmpathMessageHTMLWidget::toHTML(QCString & str) // This is black magic.
{
	KConfig * config(KGlobal::config());
	config->setGroup(EmpathConfig::GROUP_DISPLAY);

	QColor quote1, quote2;
	QColor color1(Qt::darkBlue);
	QColor color2(Qt::darkGreen);

	quote1 = config->readColorEntry(
		EmpathConfig::KEY_QUOTE_COLOUR_ONE, &color1);

	quote2 = config->readColorEntry(
		EmpathConfig::KEY_QUOTE_COLOUR_TWO, &color2);

	QCString quoteOne = QColorToHTML(quote1);
	QCString quoteTwo = QColorToHTML(quote2);
	
	empathDebug("Quote colours: \"" + quoteOne + "\", \"" + quoteTwo + "\"");
	
	// Will this work with Qt-2.0's QString ?
	register char * buf = new char[32768]; // 32k buffer. Will be reused.
	QCString outStr;
	
	if (!buf) { 
		empathDebug("Couldn't allocate buffer");
		return;
	}

	register char * pos = (char *)str.data();	// Index into source string.
	char * start = pos;							// Start of source string.
	register char * end = start + str.length();	// End of source string.
	
	if (start == end) {
		delete [] buf;
		buf = 0;
		return;
	}

	register char * bufpos = buf;
	int numBufs = 1;
	int tabstop = 4; // FIXME: Hard coded !
	int totalLength = 0;
	register int x = 0;
	register char * startAddress = 0;
	register char * endAddress = 0;
	char * c = 0;
	int quoteDepth = 0;
	bool markDownQuotedLine = false;

	for (char * i = pos; i <= end ; i++, pos++) {
		
		// Check to see if we're approaching the end of the buffer.
		// If so, copy what we've done so far into outStr and start at
		// the beginning of the buffer again.
		if ((bufpos - buf) > 32256) {
			*bufpos = '\0';
			outStr += buf;
			bufpos = buf;
		}
		
		// Look at char under 'cursor' and act appropriately.
		switch (*pos) {
			
			case '\n':
				
				if (markDownQuotedLine) { // If this line was quoted.
				
					strcpy(bufpos, "</FONT>\n");
					bufpos += 8;
					markDownQuotedLine = false;
				
				} else *(bufpos++) = *pos;
				
				break;

			case '<':
				strcpy(bufpos, "&lt;");
				bufpos += 4;
				break;
			
			case '-':
				// Markup sig.
				// Ensure first char on line, and following char is '-'
				if (pos != start && *(pos - 1) == '\n' &&
					*(pos + 1) == '-' &&
					*(pos + 2) == '\n')
				{			
					strcpy(bufpos, "<BR><HR><BR>");
					bufpos += 12;
					++pos;

				} else *(bufpos++) = *pos;
		
				break;
					
			case '>':
				// Markup quoted line if that's what we have.
				if (pos != start && *(pos - 1) == '\n') { // First char on line?
					
					quoteDepth = 0;
					
					while (pos < end && (*pos == '>' || *pos == ' '))
						if (*pos++ == '>') ++quoteDepth;

					strcpy(bufpos, "<FONT COLOR=\"#");
					bufpos += 14;
					
					if (quoteDepth % 2 == 0)
						strcpy(bufpos, quoteOne);
					else
						strcpy(bufpos, quoteTwo);
					
					bufpos += 6;
					
					strcpy(bufpos, "\">");
					
					bufpos += 2;
					
					for (x = 0 ; x < quoteDepth; x++) {
						strcpy(bufpos, "&gt; ");
						bufpos += 5;
					}

					markDownQuotedLine = true;

					pos--; // Need to catch \n if next char for mark down.
					
				} else {
					
					strcpy(bufpos, "&gt;");
					bufpos += 4;
				}
				break;

			case '&':
				strcpy(bufpos, "&amp;");
				bufpos += 5;
				break;

			case 'g': // Match gopher URLs.

				if (strncmp(pos, "gopher://", 9) == 0) {
					
					pos += 9;
					
					x = 0;

					while (x < 128 && *(pos + x) != ' ' && *(pos + x) != '\n')
						++x;
		
					strcpy(bufpos, "<A HREF=\"gopher://");
					bufpos += 18;
					strncpy(bufpos, pos, x);
					bufpos += x;
					strcpy(bufpos, "\">gopher://");
					bufpos += 11;
					strncpy(bufpos, pos, x); 
					bufpos += x;
					strcpy(bufpos, "</A>");
					bufpos += 4;

					pos += x - 1; // -1 so that the last char is processed.
					
				} else *(bufpos++) = *pos;
				
				break;

			case 'f': // Match ftp URLs.

				if (strncmp(pos, "ftp://", 6) == 0) {
					
					pos += 6;
					
					x = 0;

					while (x < 128 && *(pos + x) != ' ' && *(pos + x) != '\n')
						++x;
		
					strcpy(bufpos, "<A HREF=\"ftp://");
					bufpos += 15;
					strncpy(bufpos, pos, x);
					bufpos += x;
					strcpy(bufpos, "\">ftp://");
					bufpos += 8;
					strncpy(bufpos, pos, x); 
					bufpos += x;
					strcpy(bufpos, "</A>");
					bufpos += 4;

					pos += x - 1; // -1 so that the last char is processed.
					
				} else *(bufpos++) = *pos;
				
				break;

			case 'h': // Match http URLs.

				if (strncmp(pos, "http://", 7) == 0) {
					
					pos += 7;
					
					x = 0;

					while (x < 128 && *(pos + x) != ' ' && *(pos + x) != '\n')
						++x;
		
					strcpy(bufpos, "<A HREF=\"http://");
					bufpos += 16;
					strncpy(bufpos, pos, x);
					bufpos += x;
					strcpy(bufpos, "\">http://");
					bufpos += 9;
					strncpy(bufpos, pos, x); 
					bufpos += x;
					strcpy(bufpos, "</A>");
					bufpos += 4;

					pos += x - 1; // -1 so that the last char is processed.
				
				} else if (strncmp(pos, "https://", 8) == 0) {
					
					pos += 7;
					
					x = 0;

					while (x < 128 && *(pos + x) != ' ' && *(pos + x) != '\n')
						++x;
		
					strcpy(bufpos, "<A HREF=\"https://");
					bufpos += 17;
					strncpy(bufpos, pos, x);
					bufpos += x;
					strcpy(bufpos, "\">https://");
					bufpos += 10;
					strncpy(bufpos, pos, x); 
					bufpos += x;
					strcpy(bufpos, "</A>");
					bufpos += 4;

					pos += x - 1; // -1 so that the last char is processed.
					
				} else *(bufpos++) = *pos;
				
				break;

			case '@': // Address matching.
			
	
				// First check to see if this is an address of the form
				// "Rik Hemsley"@dev.null.
				if (pos != start && *(pos - 1) == '"') {
					
					while (
						startAddress		>= start	&&
						pos - startAddress	< 128		&&
						*startAddress		!= '\"')
						--startAddress;

					++startAddress;
				
				} else { // It's a normal address ( if it is an address ).

					// Work backwards from one before '@' until we're sure
					// that we're before the address start.
					startAddress = pos - 1;

					while (
						startAddress 		>=	start	&&
						pos - startAddress	<	128		&&
						*startAddress		!=	'<'		&&
						*startAddress		!=	'>'		&&
						*startAddress		!=	'"'		&&
						*startAddress		!=	','		&&
						*startAddress		!=	' '		&&
						*startAddress		!=	'\t'	&&
						*startAddress		!=	'\n'	&&
						(isalnum(*startAddress)	|| ispunct(*startAddress)))
					{
						--startAddress;
					}

					++startAddress;
				}
				
				// Now work forwards from one after '@' until we're sure
				// that we're clear of the end of the address.
				
				endAddress = pos + 1;

				while (
					endAddress - pos	< (int)end 		&&
					endAddress - pos	< 128			&&
					*endAddress			!= '\0'			&&
					*endAddress			!= ' '			&&
					*endAddress			!= '\n'			&&
					*endAddress			!= '\r'			&&
					*endAddress			!= '>'			&&
					*endAddress			!= '<'			&&
					*endAddress			!= '@'			&&
					*endAddress			!= '"'			&&
					*endAddress			!= ','			&&
					*endAddress			!= '\t')
					++endAddress;
				
				if (startAddress == pos - 1 || endAddress == pos + 1) {
					*(bufpos++) = *pos;
					break;
				}

				// bufpos moves back by length of startaddress.
				bufpos -= (pos - startAddress);

				// Now replace from the cursor with <A HREF...
				strcpy(bufpos, "<A HREF=\"empath://mailto:");
				bufpos += 25;

				// Now add the start address after the markup
				strncpy(bufpos, startAddress, pos - startAddress + 1);
				bufpos += pos - startAddress;

				// Add the end address.
				strncpy(bufpos, pos , endAddress - pos);
				bufpos += endAddress - pos;

				// Add the end of this part of the markup
				strcpy(bufpos, "\">");
				bufpos += 2;

				// Add the startaddress bit again
				strncpy(bufpos, startAddress, pos - startAddress + 1);
				bufpos += pos - startAddress;

				// Add the end of the address again
				strncpy(bufpos, pos, endAddress - pos);
				bufpos += endAddress - pos;

				// Add the end of the markup.
				strcpy(bufpos, "</A>");
				bufpos += 4;

				// Change the cursor in the source string to avoid the address.
				pos += endAddress - pos - 1;
				break;

			default:
				*(bufpos++) = *pos;
		}
	}
	
	*bufpos = '\0';
	outStr += buf;
	ASSERT(buf != 0);
	delete [] buf;
	buf = 0;
	str = outStr.data();
}

	void
EmpathMessageHTMLWidget::s_popupMenu(const char * c, const QPoint & p)
{
	if (c == 0)
		return;
	
	popup_.clear();
	
	QString s(c);
	
	empathDebug("URL clicked was: \"" + s + "\"");
	
	if (s.left(16) == "empath://mailto:") {
		popup_.insertItem(empathIcon("mini-compose.png"),
			i18n("New message to"), empath, SLOT(s_compose()));
	}
	
	if (s.left(7) == "http://"	||
		s.left(6) == "ftp://"	||
		s.left(8) == "https://"	||
		s.left(9) == "gopher://") {
		
		popup_.insertItem(empathIcon("mini-view.png"), i18n("Browse"),
			parent(), SLOT(s_URLSelected()));
		
		popup_.insertItem(empathIcon("mini-view.png"), i18n("Bookmark"),
			parent(), SLOT(s_URLSelected()));
	}
	
	popup_.exec(QCursor::pos());
}

