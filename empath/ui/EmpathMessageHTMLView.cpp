/*
    Empath - Mailer for KDE
    
    Copyright (C) 1998, 1999 Rik Hemsley rik@kde.org
    
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

#ifdef __GNUG__
# pragma implementation "EmpathMessageHTMLView.h"
#endif

#include <ctype.h>

// Qt includes
#include <qfile.h>
#include <qtextstream.h>
#include <qstring.h>

// KDE includes
#include <klocale.h>
#include <kcursor.h>
#include <kapp.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <khtml.h>

// Local includes
#include "EmpathMessageHTMLView.h"
#include "EmpathConfig.h"
#include "Empath.h"
#include "EmpathUIUtils.h"
#include "EmpathUtilities.h"
#include <RMM_Message.h>
#include <RMM_Enum.h>


EmpathMessageHTMLWidget::EmpathMessageHTMLWidget(
        QWidget     *   _parent,
        const char  *   _name)
    :   KHTMLWidget(_parent, _name),
        busy_(false)
{
    setFrameStyle(QFrame::NoFrame);
    begin();
    QString baseColour = "ffffff";
//        QColorToHTML(kapp->palette().color(QPalette::Normal, QColorGroup::Base));
    QString textColour = "000000";
//        QColorToHTML(kapp->palette().color(QPalette::Normal, QColorGroup::Text));

    QString welcomeText = i18n("Welcome to Empath");

    QString imgPath = KGlobal::dirs()->findResource("appdata", "empath_logo.png");

    empathDebug("Logo is at `" + imgPath + "'");
    
    // Welcome message
    write("<HTML><BODY BGCOLOR=\"#" + baseColour + "\"><CENTER><IMG SRC=\""
        + imgPath + "\"><BR><TT><FONT SIZE=\"+2\" COLOR=\"#" + textColour
        + "\">" + welcomeText + "</FONT></TT></CENTER></BODY></HTML>");
    end();
    
    QObject::connect(
        this, SIGNAL(popupMenu(QString, const QPoint &)),
        this, SLOT(s_popupMenu(QString, const QPoint &)));
}

EmpathMessageHTMLWidget::~EmpathMessageHTMLWidget()
{
}

    bool
EmpathMessageHTMLWidget::showText(const QString & s, bool markup)
{
    if (busy_) return false;
    busy_ = true;

    setCursor(waitCursor);
    
    KConfig * c(KGlobal::config());

    using namespace EmpathConfig;

    c->setGroup(GROUP_DISPLAY);
    
    QFont defaultFixed(KGlobal::fixedFont());
    
    QFont f = c->readFontEntry(UI_FIXED_FONT, &defaultFixed);

    int fs = f.pointSize();
    int fsizes[7] = { fs, fs, fs, fs, fs, fs, fs };

    setFixedFont(f.family());
    setFontSizes(fsizes);
    setStandardFont(KGlobal::generalFont().family());
    setURLCursor(KCursor::handCursor());
    setFocusPolicy(QWidget::StrongFocus);
    setDefaultBGColor(
        kapp->palette().color(QPalette::Normal, QColorGroup::Base));
    setDefaultTextColors(
        kapp->palette().color(QPalette::Normal, QColorGroup::Text),
        c->readColorEntry(UI_LINK, &DFLT_LINK),
        c->readColorEntry(UI_VLINK, &DFLT_VLINK));
    setUnderlineLinks(c->readBoolEntry(UI_UNDERLINE_LINKS, DFLT_UNDER_LINKS));

    if (s.isEmpty()) {
        write(
            "<HTML><BODY BGCOLOR=" +
            QColorToHTML(
                kapp->palette().color(QPalette::Normal, QColorGroup::Base)) +
            "><PRE>" +
            i18n("This part is empty") +
            "</PRE></BODY></HTML>");
        setCursor(arrowCursor);
        busy_ = false;
        return true;
    }
        
    begin();

    if (markup) {
        
        QColor bgcol =
            kapp->palette().color(QPalette::Normal, QColorGroup::Base);

        QString bg = QColorToHTML(bgcol);

        QString html(s);
        
        toHTML(html);
        
        write("<HTML><BODY BGCOLOR="+bg+"><PRE>"+html+"</PRE></BODY></HTML>");

        QFile f("message.html");

        f.open(IO_WriteOnly);

        QTextStream t(&f);
        t << html;
        f.close();
        
    } else {
        
        write("<HTML><BODY><PRE>" + s + "</PRE></BODY></HTML>");
    }
    
    setCursor(arrowCursor);
    busy_ = false;
    end();
    return true;
}

    void
EmpathMessageHTMLWidget::toHTML(QString & _str) // This is black magic.
{
    QCString str(_str.latin1());

    KConfig * config(KGlobal::config());

    using namespace EmpathConfig;

    config->setGroup(GROUP_DISPLAY);

    QColor quote1(config->readColorEntry(UI_QUOTE_ONE, &DFLT_Q_1));
    QColor quote2(config->readColorEntry(UI_QUOTE_TWO, &DFLT_Q_2));

    QCString quoteOne = QColorToHTML(quote1).latin1();
    QCString quoteTwo = QColorToHTML(quote2).latin1();
    
    register char * buf = new char[32768]; // 32k buffer. Will be reused.
    QCString outStr;
    
    if (!buf) { 
        empathDebug("Couldn't allocate buffer");
        return;
    }

    register char * pos = (char *)str.data();   // Index into source string.
    char * start = pos;                         // Start of source string.
    register char * end = start + str.length(); // End of source string.
    
    if (start == end) {
        delete [] buf;
        buf = 0;
        return;
    }

    register char * bufpos = buf;
    register int x = 0;
    register char * startAddress = 0;
    register char * endAddress = 0;
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
                
                    memcpy(bufpos, "</FONT>", 7);
                    bufpos += 7;
                    markDownQuotedLine = false;
                
                }
                
                memcpy(bufpos, "<BR>\n", 5);
                bufpos += 5;
                
                break;

            case '<':
                
                memcpy(bufpos, "&lt;", 4);
                bufpos += 4;
                
                break;
            
            case '-':
                // Markup sig.
                // Ensure first char on line, and following char is '-'
                // Handle '\n--[ ]'
                if (
                    (pos != start)        &&
                    (pos <  end - 2)      &&
                    (*(pos - 1) == '\n')  &&
                    (*(pos + 1) == '-')   &&
                        (
                            (*(pos + 2) == '\n')  ||
                            ((*(pos + 2) == ' ') && (*(pos + 3) == '\n'))
                        )
                    )
                {
                    memcpy(bufpos, "<BR><HR><BR>\n", 13);
                    bufpos += 13;
                    
                    pos += 2;
                    
                    if (*(pos + 2) == ' ')
                        ++pos;
                }
                else
                    *(bufpos++) = *pos;
        
                break;
                    
            case '>':
                // Markup quoted line if that's what we have.
                if (pos != start && *(pos - 1) == '\n') { // First char on line?
                    
                    quoteDepth = 0;
                    
                    while (pos < end && (*pos == '>' || *pos == ' '))
                        if (*pos++ == '>') ++quoteDepth;

                    memcpy(bufpos, "<FONT COLOR=\"#", 14);
                    bufpos += 14;
                    
                    if (quoteDepth % 2 == 0)
                        memcpy(bufpos, quoteOne, 6);
                    else
                        memcpy(bufpos, quoteTwo, 6);
                    
                    bufpos += 6;
                    
                    memcpy(bufpos, "\">", 2);
                    
                    bufpos += 2;
                    
                    for (x = 0 ; x < quoteDepth; x++) {
                        memcpy(bufpos, "&gt; ", 5);
                        bufpos += 5;
                    }

                    markDownQuotedLine = true;

                    pos--; // Need to catch \n if next char for mark down.
                    
                } else {
                    
                    memcpy(bufpos, "&gt;", 4);
                    bufpos += 4;
                }
                
                break;

            case '&':
                
                memcpy(bufpos, "&amp;", 5);
                bufpos += 5;
                
                break;

            case 'g': // Match gopher URLs.

                if (strncmp(pos, "gopher://", 9) == 0) {
                    
                    pos += 9;
                    
                    x = 0;

                    while (x < 128 && *(pos + x) != ' ' && *(pos + x) != '\n')
                        ++x;
        
                    memcpy(bufpos, "<A HREF=\"gopher://", 18);
                    bufpos += 18;
                
                    memcpy(bufpos, pos, x);
                    bufpos += x;
                
                    memcpy(bufpos, "\">gopher://", 11);
                    bufpos += 11;
                
                    memcpy(bufpos, pos, x); 
                    bufpos += x;
                
                    memcpy(bufpos, "</A>", 4);
                    bufpos += 4;

                    pos += x - 1; // -1 so that the last char is processed.
                    
                }
                else
                    *(bufpos++) = *pos;
                
                break;

            case 'f': // Match ftp URLs.

                if (strncmp(pos, "ftp://", 6) == 0) {
                    
                    pos += 6;
                    
                    x = 0;

                    while (x < 128 && *(pos + x) != ' ' && *(pos + x) != '\n')
                        ++x;
        
                    memcpy(bufpos, "<A HREF=\"ftp://", 15);
                    bufpos += 15;
                    
                    memcpy(bufpos, pos, x);
                    bufpos += x;
                    
                    memcpy(bufpos, "\">ftp://", 8);
                    bufpos += 8;
                    
                    memcpy(bufpos, pos, x); 
                    bufpos += x;
                    
                    memcpy(bufpos, "</A>", 4);
                    bufpos += 4;

                    pos += x - 1; // -1 so that the last char is processed.
                    
                }
                else
                    *(bufpos++) = *pos;
                
                break;

            case 'h': // Match http URLs.

                if (strncmp(pos, "http://", 7) == 0) {
                    
                    pos += 7;
                    
                    x = 0;

                    while (x < 128 && *(pos + x) != ' ' && *(pos + x) != '\n')
                        ++x;
        
                    memcpy(bufpos, "<A HREF=\"http://", 16);
                    bufpos += 16;
                    
                    memcpy(bufpos, pos, x);
                    bufpos += x;
                    
                    memcpy(bufpos, "\">http://", 9);
                    bufpos += 9;
                    
                    memcpy(bufpos, pos, x); 
                    bufpos += x;
                    
                    memcpy(bufpos, "</A>", 4);
                    bufpos += 4;

                    pos += x - 1; // -1 so that the last char is processed.
                
                } else if (strncmp(pos, "https://", 8) == 0) {
                    
                    pos += 7;
                    
                    x = 0;

                    while (x < 128 && *(pos + x) != ' ' && *(pos + x) != '\n')
                        ++x;
        
                    memcpy(bufpos, "<A HREF=\"https://", 17);
                    bufpos += 17;
                    
                    memcpy(bufpos, pos, x);
                    bufpos += x;
                    
                    memcpy(bufpos, "\">https://", 18);
                    bufpos += 10;
                    
                    memcpy(bufpos, pos, x); 
                    bufpos += x;
                    
                    memcpy(bufpos, "</A>", 4);
                    bufpos += 4;

                    pos += x - 1; // -1 so that the last char is processed.
                    
                } else *(bufpos++) = *pos;
                
                break;

            case '@': // Address matching.
            
    
                // First check to see if this is an address of the form
                // "Rik Hemsley"@dev.null.
                if (pos != start && *(pos - 1) == '"') {
                    
                    while (
                        startAddress        >= start    &&
                        pos - startAddress  < 128       &&
                        *startAddress       != '\"')
                        --startAddress;

                    ++startAddress;
                
                } else { // It's a normal address ( if it is an address ).

                    // Work backwards from one before '@' until we're sure
                    // that we're before the address start.
                    startAddress = pos - 1;

                    while (
                        startAddress        >=  start   &&
                        pos - startAddress  <   128     &&
                        *startAddress       !=  '<'     &&
                        *startAddress       !=  '>'     &&
                        *startAddress       !=  '"'     &&
                        *startAddress       !=  ','     &&
                        *startAddress       !=  ' '     &&
                        *startAddress       !=  '\t'    &&
                        *startAddress       !=  '\n'    &&
                        (isalnum(*startAddress)    || ispunct(*startAddress)))
                    {
                        --startAddress;
                    }

                    ++startAddress;
                }
                
                // Now work forwards from one after '@' until we're sure
                // that we're clear of the end of the address.
                
                endAddress = pos + 1;

                while (
                    endAddress - pos    < (int)end  &&
                    endAddress - pos    < 128       &&
                    *endAddress         != '\0'     &&
                    *endAddress         != ' '      &&
                    *endAddress         != '\n'     &&
                    *endAddress         != '\r'     &&
                    *endAddress         != '>'      &&
                    *endAddress         != '<'      &&
                    *endAddress         != '@'      &&
                    *endAddress         != '"'      &&
                    *endAddress         != ','      &&
                    *endAddress         != '\t')
                    ++endAddress;
                
                if (startAddress == pos - 1 || endAddress == pos + 1) {
                    *(bufpos++) = *pos;
                    break;
                }

                // bufpos moves back by length of startaddress.
                bufpos -= (pos - startAddress);

                // Now replace from the cursor with <A HREF...
                memcpy(bufpos, "<A HREF=\"empath://mailto:", 25);
                bufpos += 25;

                // Now add the start address after the markup
                memcpy(bufpos, startAddress, pos - startAddress + 1);
                bufpos += pos - startAddress;

                // Add the end address.
                memcpy(bufpos, pos , endAddress - pos);
                bufpos += endAddress - pos;

                // Add the end of this part of the markup
                memcpy(bufpos, "\">", 2);
                bufpos += 2;

                // Add the startaddress bit again
                memcpy(bufpos, startAddress, pos - startAddress + 1);
                bufpos += pos - startAddress;

                // Add the end of the address again
                memcpy(bufpos, pos, endAddress - pos);
                bufpos += endAddress - pos;

                // Add the end of the markup.
                memcpy(bufpos, "</A>", 4);
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
    _str = outStr;

}

    void
EmpathMessageHTMLWidget::s_popupMenu(QString s, const QPoint &)
{
    if (s.isEmpty())
        return;
    
    popup_.clear();
    
    empathDebug("URL clicked was: \"" + s + "\"");
    
    if (s.left(16) == "empath://mailto:") {
        popup_.insertItem(empathIcon("menu-compose"),
            i18n("New message to"), empath, SLOT(s_compose()));
    }
    
    if (s.left(7) == "http://"      ||
        s.left(6) == "ftp://"       ||
        s.left(8) == "https://"     ||
        s.left(9) == "gopher://")
    {
        
        popup_.insertItem(empathIcon("menu-view"), i18n("Browse"),
            parent(), SLOT(s_URLSelected()));
        
        popup_.insertItem(empathIcon("menu-view"), i18n("Bookmark"),
            parent(), SLOT(s_URLSelected()));
    }
    
    popup_.exec(QCursor::pos());
}

    QString
EmpathMessageHTMLWidget::QColorToHTML(const QColor & c)
{
    QString s;
    s.sprintf("%02X%02X%02X", c.red(), c.green(), c.blue());
    return s;
}
        
    QSize
EmpathMessageHTMLWidget::sizeHint() const
{
    return QSize(width(), 400);
}

    QSize
EmpathMessageHTMLWidget::minimumSizeHint() const
{
    return QSize(0, 0);
}

// vim:ts=4:sw=4:tw=78
