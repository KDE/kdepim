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

#include <iostream.h>
#include <ctype.h>

// Qt includes
#include <qfile.h>
#include <qtextstream.h>
#include <qstring.h>

// KDE includes
#include <klocale.h>
#include <kcursor.h>
#include <kapp.h>

// Local includes
#include "RMM_Message.h"
#include "EmpathMessageHTMLView.h"
#include "EmpathConfig.h"
#include "Empath.h"
#include "EmpathUIUtils.h"

static char * replaceTagsHeader[] = {
	// Header fields
	"Approved", "Bcc", "Cc", "Comments", "Content-Description",
	"Content-Disposition", "Content-Id", "Content-Transfer-Encoding",
	"Content-Type", "Control", "Cte", "Date", "Distribution", "Encrypted",
	"Expires", "Followup-To", "From", "In-Reply-To", "Keywords", "Lines",
	"Message-Id", "Mime-Version", "Newsgroups", "Organization", "Path",
	"Received", "References", "Reply-To", "Resent-Bcc", "Resent-Cc",
	"Resent-Date", "Resent-From", "Resent-MessageId", "Resent-ReplyTo",
	"Resent-Sender", "Resent-To", "Return-Path", "Sender", "Subject",
	"Summary", "To", "Xref"
};

static char * replaceTagsBody[] = {
	"@_TT_",
	"@_TTEND_",
	"@_MESSAGE_BODY_",
	"@_SIGNATURE_"
};

const int fsizes[7] = { 14, 14, 14, 14, 14, 14, 14 };

EmpathMessageHTMLWidget::EmpathMessageHTMLWidget(
		const EmpathURL &	url,
		QWidget			*	_parent,
		const char		*	_name)
	:	KHTMLWidget(_parent, _name),
		url_(url)
{
	empathDebug("ctor");
	
	setFontSizes(fsizes);
	KConfig * c = kapp->getConfig();
	c->setGroup(EmpathConfig::GROUP_DISPLAY);
	QString iconSet = c->readEntry(EmpathConfig::KEY_ICON_SET);
	
	begin();
	// Begin welcome message
	write("<HTML>");
	write("<BODY BGCOLOR=\"#");
	write(QColorToHTML(empathWindowColour()));
	write("\">");
	write(
		"<IMG SRC=\"file:" +
		empathDir() +
		"/pics/" +
		iconSet +
		"/empath.png\">");
	write("<TT><FONT COLOR=\"#");
	write(QColorToHTML(empathTextColour()));
	write("\">");
	write (i18n("Welcome to Empath"));
	write ("</FONT></TT></HTML>");
	// End welcome message
	parse();
	end();
	
	empathDebug("ctor finished");
}

EmpathMessageHTMLWidget::~EmpathMessageHTMLWidget()
{
	empathDebug("dtor");
}

	void
EmpathMessageHTMLWidget::setMessage(const EmpathURL & url)
{
	url_ = url;
}

	void
EmpathMessageHTMLWidget::go()
{
	empathDebug("go() called .. message we're working on is \"" +
			url_.asString() + "\"");

	setCursor(waitCursor);

	setStandardFont(empathGeneralFont().family());
	setFixedFont(empathFixedFont().family());
	setURLCursor(KCursor::handCursor());
	setFocusPolicy(QWidget::StrongFocus);
	
	KConfig * c = kapp->getConfig();
	c->setGroup(EmpathConfig::GROUP_DISPLAY);
	
	setDefaultTextColors(
		c->readColorEntry(EmpathConfig::KEY_TEXT_COLOUR),
		c->readColorEntry(EmpathConfig::KEY_LINK_COLOUR),
		c->readColorEntry(EmpathConfig::KEY_VISITED_LINK_COLOUR));
	
	setUnderlineLinks(c->readBoolEntry(EmpathConfig::KEY_UNDERLINE_LINKS));
	
	// Implementation Note:
	// Get template to htmlTemplate
	// Get message headers and body
	// 
	// Use message headers and body to fill in blanks in htmlTemplate.
	// Use htmlTemplate (now filled in) to fill this widget.
	
	// FIXME: Read this from the config.
	QString templateFilename = empathDir() + "/message.template.html";
	
	// Load the template.
	if (!loadTemplate(templateFilename)) {
		empathDebug("Couldn't open HTML message template");
	}

	// Markup the background colour in the template.
	markupBackgroundColour(htmlTemplate);
	markupTextColour(htmlTemplate);
	
	// Markup <TT> and </TT> in the template.
	htmlTemplate.replace(QRegExp("@_TT_"), "<PRE>");
	htmlTemplate.replace(QRegExp("@_TTEND_"), "</PRE>");
	
	RMessage * message(empath->message(url_));
	
	if (message == 0) {
		empathDebug("Can't load message from \"" + url_.asString() + "\"");
		// Don't forget to reset the cursor.
		setCursor(arrowCursor);
		return;
	}
	
	message->parse(); message->assemble();
	
	QCString messageHeaders = message->envelope().asString();
	QCString messageBody = message->body().asString();
	
	// Markup and place headers of the message in the template.
	replaceHeaderTagsByData(messageHeaders, htmlTemplate);

	int bodyTagPos = htmlTemplate.find("@_MESSAGE_BODY_");
	
	begin();
	parse();
	write(htmlTemplate.left(bodyTagPos - 1)); // catch the last char
			
	// Markup and place the body of the message in the template.
	replaceBodyTagsByData(messageBody, htmlTemplate);
	
	write(htmlTemplate.right(htmlTemplate.length() - bodyTagPos + 1));
	
	///////////////////////////////////////////////////////////// Testing ///
	// Write the HTML to a file. Useful considering HTML widget crashes.  ///
	/////////////////////////////////////////////////////////////////////////
#ifndef NDEBUG
	empathDebug("Writing message as html to message.html");
	QFile f("message.html");
	if (f.open(IO_WriteOnly)) {
		QTextStream t(&f);
		t << htmlTemplate;
		f.close();
	}
#endif
	/////////////////////////////////////////////////////////////////////////

	setCursor(arrowCursor);
	parse();
}

	bool
EmpathMessageHTMLWidget::loadTemplate(const QString & templateFilename)
{
	empathDebug("loadTemplate");
	QFile f(templateFilename);
	
	htmlTemplate = QString::null;
	
	if (!f.open(IO_ReadOnly)) {
		
		empathDebug("Couldn't open file");
		htmlTemplate =
			"<HTML>\n<H1>" +
			QString(i18n("Warning: Couldn't load the template for displaying this message")) +
			"</H1>";
		return false;
	
	} else {
	
		QTextStream t(&f);
		
		while (!t.eof())
		   htmlTemplate	+= t.readLine();
		
		f.close();
	
		return true;
	}
}

	void
EmpathMessageHTMLWidget::markupBackgroundColour(QCString & html)
{
	empathDebug("markupBackgroundColour");
	
	QCString bgcol;
	
	KConfig * c = kapp->getConfig();
	c->setGroup(EmpathConfig::GROUP_DISPLAY);
	
	bgcol = "BGCOLOR=\"#" +
		QColorToHTML(c->readColorEntry(EmpathConfig::KEY_BACKGROUND_COLOUR)) +
		"\"";
	
	html.replace(QRegExp("@_BODY_BACKGROUND_"), bgcol);
	empathDebug("end markupBk");
}

	void
EmpathMessageHTMLWidget::markupTextColour(QCString & html)
{
	empathDebug("markupTextColour");
	
	KConfig * c = kapp->getConfig();
	c->setGroup(EmpathConfig::GROUP_DISPLAY);
	
	QCString textcol;

	textcol = "\"#" +
		QColorToHTML(c->readColorEntry(EmpathConfig::KEY_TEXT_COLOUR)) +
		"\"";
	
	html.replace(QRegExp("@_NORMAL_TEXT_"), textcol);
	empathDebug("end markupTcol");
}

	void
EmpathMessageHTMLWidget::replaceHeaderTagsByData(
	const QCString & body, QCString & html)
{
	empathDebug("replaceHeaderTagsByData");
	markupHeaderNames(html);
	markupHeaderBodies(body, html);
}

	void
EmpathMessageHTMLWidget::replaceBodyTagsByData(
	const QCString & body, QCString & html)
{
	empathDebug("replaceBodyTagsByData");

	QRegExp sigSep("\n-- ?\n");
	int i = sigSep.match(body);
	
	QCString bodyOnly;
	QCString sigOnly;

	// Handle condition where there's no sig.	
	if (i != -1) {
		
		sigOnly		= body.right(body.length() - i - 4);
		bodyOnly	= body.left(i);

	} else {

		sigOnly		= QString::null;
		bodyOnly	= body;
	}
	toHTML(bodyOnly);
	toHTML(sigOnly);
	
	// Replace the body stuff with relevant data
	html.replace(QRegExp("@_MESSAGE_BODY_"), bodyOnly);
	
	html.replace(QRegExp("@_SIGNATURE_"), sigOnly);
}

	void
EmpathMessageHTMLWidget::markupHeaderNames(QCString & html)
{
	empathDebug("markupHeaderNames");
	QCString replacementField;
	QRegExp fieldToReplace;
	fieldToReplace.setCaseSensitive(false);

	// Replace all @_HEADER_* with appropriate header names
	for (int i = 0 ; i < 42 ; i++) {
		
		replacementField = replaceTagsHeader[i];
		fieldToReplace =
			"@_HEADER_" +
			replacementField +
			"_";
		
		if (html.find(fieldToReplace, 0) == -1)
			continue;
		
		empathDebug("Replacing all " + QString(fieldToReplace.pattern()) +
				QString(" with ") + QString(replacementField) + QString(":"));

		html.replace(fieldToReplace, replacementField + ":");
	}
}

	void
EmpathMessageHTMLWidget::markupHeaderBodies(
		const QCString & body, QCString & html)
{
	empathDebug("markupHeaderBodies");
	QCString replacementField;
	QRegExp fieldToReplace;
	fieldToReplace.setCaseSensitive(false);
	
	// Replace all header body elements with appropriate header bodies, if
	// they exist. If not, use null string.
	
	for (int i = 0 ; i < 42 ; i++) {
		
		replacementField = QCString(replaceTagsHeader[i]).upper();
		replacementField.replace(QRegExp("-"), "_");
		fieldToReplace =
			"@_BODY_" +
			replacementField +
			"_";
		
		// If we can't find the field, loop.
		if (html.find(fieldToReplace, 0) == -1) continue;

		empathDebug("fieldToReplace: " + QCString(fieldToReplace.pattern()));
		
		// e.g. replacementRegExp = "Subject" + ":".
	
		QRegExp replacementRegExp(replaceTagsHeader[i] +
				QCString(":"));
		
		empathDebug("replacementRegExp: " + QCString(replacementRegExp.pattern()));
		
		// Try to find the header in the message header, not case sensitive.
		int headerPos = body.find(replacementRegExp.pattern(), 0, false);
		
		// Didn't find the field in the message header ? Replace the tag in the
		// html with nothing.
		if (headerPos == -1) {
			html.replace(fieldToReplace, QString::null);
			continue;
		}

		empathDebug("Found replacementRegExp");

		// headerPosToEnd = everything from where we found it in the headers to
		// the end of the headers.		
		QCString headerPosToEnd =
			body.right(body.length() - headerPos);
		
		// Now find the end of the header
		QRegExp endOfHeaderBody("\n[^ ]");
		
		// Get all the header into a string.
		QCString header =
			headerPosToEnd.left(endOfHeaderBody.match(headerPosToEnd));
		
		// Remove the header of the header :) (leaves us with header body).
		QCString headerBody =
			header.right(
				header.length() - header.find(":") - 1).stripWhiteSpace();
		
		empathDebug("header: " + headerBody); 
		
		toHTML(headerBody);
		
		empathDebug("header: " + headerBody); 
		
		empathDebug("Replacing all " + QString(fieldToReplace.pattern()) +
				QString(" with ") + QString(headerBody));
		html.replace(QRegExp(fieldToReplace.pattern()), headerBody);
	}
}
	

	void
EmpathMessageHTMLWidget::toHTML(QCString & str) // This is black magic.
{
	
	// Will this work with Qt-2.0's QString ?
	register char * buf = new char[32768]; // 32k buffer. Will be reused.
	QCString outStr;
	
	if (!buf) { 
		cerr << "Couldn't allocate buffer" << endl;
		return;
	}

	register char * pos = (char *)str.data();	// Index into source string.
	char * start = pos;							// Start of source string.
	register char * end = start + str.length();	// End of source string.
	if (start == end) return;

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
				
				} else {
				
					*(bufpos++) = *pos;
				}
				
				break;

			case '<':
				strcpy(bufpos, "&lt;");
				bufpos += 4;
				break;
			
			case '>':
				// Markup quoted line if that's what we have.
				if (pos != start && *(pos - 1) == '\n') { // First char on line?
					
					quoteDepth = 0;
					
					while (pos < end && (*pos == '>' || *pos == ' '))
						if (*pos++ == '>') ++quoteDepth;

					if (quoteDepth % 2 == 0)
						strcpy(bufpos, "<FONT COLOR=\"#009900\">");
					else
						strcpy(bufpos, "<FONT COLOR=\"#000099\">");
						
					bufpos += 22;
					
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
				if (pos > start && *(pos - 1) == '"') {
					
					while (
						startAddress		>= start	&&
						pos - startAddress	< 128		&&
						*startAddress		!= '\"')
						--startAddress;

					++startAddress;
				
				} else { // It's a normal address ( if it is an address ).

					for (
						startAddress = pos - 1;
						startAddress 		>=	start	&&
						pos - startAddress	<	128		&&
						*startAddress		!=	'<'		&&
						*startAddress		!=	'>'		&&
						*startAddress		!=	'"'		&&
						*startAddress		!=	','		&&
						(
							isalnum(*startAddress)	||
							ispunct(*startAddress)
						);
						--startAddress);

					++startAddress;
				}
				
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
				strcpy(bufpos, "<A HREF=\"mailto:");
				bufpos += 16;

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
	cerr << buf << endl;
	outStr += buf;
	delete [] buf;
	str = outStr.data();
}

