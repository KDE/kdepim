/*
    Empath - Mailer for KDE
    
    Copyright 1999, 2000
        Rik Hemsley <rik@kde.org>
        Wilco Greven <j.w.greven@student.utwente.nl>
    
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


#include <stream.h>

#include <qfile.h>
#include <qtextstream.h>

// KDE includes
#include <kglobal.h>
#include <kconfig.h>

// Local includes
#include "EmpathDefines.h"
#include "EmpathViewFactory.h"
#include "Empath.h"
#include <rmm/BodyPart.h>
    
EmpathViewFactory::EmpathViewFactory()
{
    // Empty.
}

EmpathViewFactory::~EmpathViewFactory()
{
    // Empty.
}

    void
EmpathViewFactory::init()
{
}

    const QMimeSource *
EmpathViewFactory::data(const QString & abs_name) const
{
    if (abs_name.left(9) == "empath://") {

        RMM::Message m = empath->message(EmpathURL(abs_name));

        if (!m) {
            empathDebug("Oh shit, can't find the message.");
            return 0;
        }

        return new EmpathXMLMessage(m);

    } else {

        return QMimeSourceFactory::data(abs_name);
    }
}

EmpathXMLMessage::EmpathXMLMessage(RMM::BodyPart & part)
    :   QMimeSource(),
        encoded_(false)
{
    messageData_ = part.asString();
    _encode();
}

    const char *
EmpathXMLMessage::format(int i) const
{
    empathDebug(QString::number(i));
    return i == 0 ? "text/xml" : 0;
}

    bool
EmpathXMLMessage::provides(const char * f) const
{
    empathDebug(f);
    return true;
    return !qstricmp(f, format(0));
}

    QByteArray
EmpathXMLMessage::encodedData(const char * f) const
{
    empathDebug(f);
//    if (provides(f))
        return data_;
//    return QByteArray();
}

    void
EmpathXMLMessage::_encode()
{
#define toOutput(a, b) memcpy(outputCursor, (a), (b)); outputCursor += (b)

#define inputToOutput() *(outputCursor++) = *inputCursor

#define nulTerminate() *outputCursor = '\0'

#define safeToLookBack() (inputCursor != inputStart)

#define matchInput(a, b) \
    (inputEnd - inputCursor > b) && \
    (strncmp(inputCursor, (a), (b)) == 0)

#define createURL(a, b) \
                                                                        \
    inputCursor += (b) + 3;                                             \
    len = 0;                                                            \
                                                                        \
    while (                                                             \
        (len < 128) &&                                                  \
        (len < inputEnd - inputCursor) &&                               \
        (*(inputCursor + len) != ' ') &&                                \
        (*(inputCursor + len) != '\n')                                  \
          )                                                             \
        ++len;                                                          \
                                                                        \
    memcpy(outputCursor, "<a href=\"", 9);      outputCursor += 9;      \
    memcpy(outputCursor, (a), (b));             outputCursor += (b);    \
    memcpy(outputCursor, "://", 3);             outputCursor += 3;      \
    memcpy(outputCursor, inputCursor, len);     outputCursor += len;    \
    memcpy(outputCursor, "\">", 2);             outputCursor += 2;      \
    memcpy(outputCursor, (a), (b));             outputCursor += (b);    \
    memcpy(outputCursor, "://", 3);             outputCursor += 3;      \
    memcpy(outputCursor, inputCursor, len);     outputCursor += len;    \
    memcpy(outputCursor, "</a>", 4);            outputCursor += 4;      \
                                                                        \
    inputCursor += len - 1

#define lengthOfAddressStart() (inputCursor - addrStart)

#define writeAddress() \
    { memcpy(outputCursor, addrStart, addrEnd - addrStart); \
        outputCursor += addrEnd - addrStart; }

    QCString encodedData;
    
    // Check size isn't 0.
    unsigned int size = messageData_.size();

    if (size == 0)
        return;

    // Sort out some colours for marking up quoted lines.

    KConfig * config(KGlobal::config());

    config->setGroup("Display");

    QColor quote1(config->readColorEntry("QuoteColourOne"));
    QColor quote2(config->readColorEntry("QuoteColourTwo"));

    QString col;

    col.sprintf("%02X%02X%02X", quote1.red(), quote1.green(), quote1.blue());
    QCString quoteOne(col.utf8());

    col.sprintf("%02X%02X%02X", quote2.red(), quote2.green(), quote2.blue());
    QCString quoteTwo(col.utf8());

    const char * input(messageData_.data());
    
    const char * inputStart = input;
    const char * inputEnd = input + size;

    char * inputCursor = const_cast<char *>(input);
    
    // Allocate an output buffer. When full, it will be dumped into
    // encodedData and reused.
    register char * outputBuf = new char[32768]; // 32k buffer. Will be reused.

    const char * outputEnd = outputBuf + 32768;
    register char * outputCursor = outputBuf;
    
    // Temporary counter + pointers.
    register int len = 0;
    register char * addrStart = 0;
    register char * addrEnd = 0;

    // Current quoting depth.
    int quoteDepth = 0;

    // Are we going to have to mark down on this iteration ?
    bool markDownQuotedLine = false;

    while (inputCursor <= inputEnd) {

        if ((outputEnd - outputCursor) < 256) {

            cerr << "overflow" << endl;
            nulTerminate();
            encodedData += outputBuf;
            outputCursor = outputBuf;
        }
        
        switch (*inputCursor) {

            case '<': toOutput("&lt;", 4); break;
            case '&': toOutput("&amp;", 5); break;

            case ' ':
                if ((inputCursor != inputEnd) && (*(inputCursor + 1) == ' '))
                {
                    toOutput("&nbsp;", 6);
                }
                else
                    inputToOutput();
                break;

            case '\t': toOutput("&nbsp;&nbsp;", 12); break;

            case 'g':
                if (matchInput("gopher://", 9)) { createURL("gopher", 6); }
                else inputToOutput();
                break;

            case 'f':
                if (matchInput("ftp://", 6)) { createURL("ftp", 3); }
                else inputToOutput();
                break;

            case 'h':
                if      (matchInput("http://", 7))  { createURL("http", 4); }
                else if (matchInput("https://", 8)) { createURL("https", 5);}
                else inputToOutput();
                break;

            case '\n':

                if (markDownQuotedLine) {
                    toOutput("</font><br/>\n", 13);
                    markDownQuotedLine = false;
                } else {
                    toOutput("<br/>\n", 6);
                }

                break;

             case '-':

                // Mark up signature separator.
                // Handle '\n--[ ]\n'.
 
                if  (
                        // We must be sure we can look back 1 and forward 3.
                        safeToLookBack() && (inputCursor <  inputEnd - 2) &&

                        // See if we're at the start of the line.
                        (*(inputCursor - 1) == '\n')  &&

                        // The next char must be a '-'
                        (*(inputCursor + 1) == '-')   &&

                        // 2 chars ahead must be either a newline or a space
                        // then a newline.
                        (
                            (*(inputCursor + 2) == '\n') ||
                  ((*(inputCursor + 2) == ' ') && (*(inputCursor + 3) == '\n'))
                        )
                   )
                {
                    toOutput("<br/><hr/><br/>\n", 16);

                    inputCursor += 2;

                    // If there's a trailing space, ignore it.
                    if (*(inputCursor + 2) == ' ')
                        ++inputCursor;
                }
                else
                    inputToOutput();
        
                break;
                  
            case '>':

                cerr << ">" << endl;
                // Mark up quoted line if that's what we have.
         
                // First char on line?
                if (safeToLookBack() && *(inputCursor - 1) == '\n') {
                    
                    cerr << "quoting" << endl;
                    
                    quoteDepth = 0;

                    // While we have '>' or ' ' then we're still in
                    // quoted-line territory ...

                    while (
                        (inputCursor < inputEnd) &&
                        (*inputCursor == '>' || *inputCursor == ' ')
                          )
                    {
                        // .. and if we see '>', then we increment the
                        // quoting depth.
                        if (*inputCursor++ == '>')
                            ++quoteDepth;
                    cerr << "quotedepth == " << quoteDepth << endl;
                    }

                    toOutput("<font color=\"#", 14);
                    
                    // Set the colour according to whether there's an odd
                    // or even quote depth.
                    //
                    // N.B. Leave braces in here to let macros work.

                    if (quoteDepth % 2 == 0)
                    {
                        toOutput(quoteOne, 6);
                    }
                    else
                    {
                        toOutput(quoteTwo, 6);
                    }
                    
                    toOutput("\">", 2);
                    
                    // Write some funky HTML-style versions of '>'.
                    for (len = 0 ; len < quoteDepth; len++)
                        toOutput("&gt; ", 5);

                    cerr << "set markdown" << endl;
                    // Remember that we need to mark down next time we
                    // see '\n'.
                    markDownQuotedLine = true;
                   
                    inputCursor--;
                    
                } else {
                    toOutput("&gt;", 4);
                }
                
                break;

            case '@': // Address matching.
            
                // First check to see if this is an address of the form
                // "Rik Hemsley"@dev.null.
                if (safeToLookBack() && *(inputCursor - 1) == '"') {
                    
                    while (

                        // Don't go back further than the start of the input.
                        (addrStart >= inputStart) &&

                        // Don't go back more than 96 chars. Decent limit IMO.
                        (inputCursor - addrStart  < 96) &&

                        // Stop when we see another double quote.
                        (*addrStart != '\"')
                          )
                    {
                        --addrStart;
                    }

                    ++addrStart;
                
                }
                else
                {
                    // It's a 'normal' address ( if it is an address ).

                    // Work backwards from one before '@' until we're sure
                    // that we've found the start of the address.
  
                    addrStart = inputCursor - 1;

                    while (

                        // Don't go back further than the start of the input.
                        (addrStart >= inputStart) &&

                        // Don't go back more than 96 chars.
                        (inputCursor - addrStart < 96) &&

                        // Stop if we see any of these chars.
                        (*addrStart != '\0')  && (*addrStart != ' ' )   &&
                        (*addrStart != '\n')  && (*addrStart != '\r')   &&
                        (*addrStart != '>' )  && (*addrStart != '<' )   &&
                        (*addrStart != '@' )  && (*addrStart != '"' )   &&
                        (*addrStart != ',' )  && (*addrStart != '\t')
                    )
                    {
                        --addrStart;
                    }

                    ++addrStart;
                }

                // Now work forwards from one after '@' until we're sure
                // that we're clear of the inputEnd of the address.
                
                addrEnd = inputCursor + 1;

                while (

                    // Don't look past the end of the input.
                    (addrEnd - inputCursor < (int)inputEnd) &&

                    // Don't look forward more than 96 chars.
                    (addrEnd - inputCursor < 96) &&

                    // Stop when we see any of these.
                    (*addrEnd != '\0')  && (*addrEnd != ' ' )   &&
                    (*addrEnd != '\n')  && (*addrEnd != '\r')   &&
                    (*addrEnd != '>' )  && (*addrEnd != '<' )   &&
                    (*addrEnd != '@' )  && (*addrEnd != '"' )   &&
                    (*addrEnd != ',' )  && (*addrEnd != '\t')
                )
                {
                    ++addrEnd;
                }

                // If the address is only one character long in either
                // direction then assume it's not an address at all.

                if (
                    (addrStart == inputCursor - 1) ||
                    (addrEnd == inputCursor + 1)
                   )
                {
                    inputToOutput();
                    break;
                }

                // Now we're going to write the address to the output.

                // We've already written all the chars leading up to the '@'
                // sign. We need to write over them now that we know that
                // they are part of this address.

                // outputCursor moves back.
                outputCursor -= lengthOfAddressStart();

                // We're going to write:
                // <a href="mailto:[address]">[address]</a>

                // Start the markup.
                toOutput("<a href=\"mailto:", 16);
                
                writeAddress();

                // End of this part of the markup.
                toOutput("\">", 2);

                writeAddress();

                // End of the markup.
                toOutput("</a>", 4);

                // Shift the input cursor to the end of the address now
                // we've finished with it.
                inputCursor += addrEnd - inputCursor - 1;
                break;

            default:
                inputToOutput();
        }

        ++inputCursor;
    }
    
    nulTerminate();
    encodedData += outputBuf;
    delete [] outputBuf;
    outputBuf = 0;
    messageData_ = encodedData;
    encoded_ = true;
    empathDebug("done");

    QFile f("message.html");
    f.open(IO_WriteOnly);
    QTextStream s(&f);
    s << messageData_;
    f.close();
}

// vim:ts=4:sw=4:tw=78
