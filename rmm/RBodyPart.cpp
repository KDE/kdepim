/* This file is part of the KDE project

   Copyright (C) 1999, 2000 Rik Hemsley <rik@kde.org>
             (C) 1999, 2000 Wilco Greven <j.w.greven@student.utwente.nl>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

// Qt includes
#include <qregexp.h>

// Local includes
#include <RMM_Utility.h>
#include <RMM_BodyPart.h>
#include <RMM_Envelope.h>
#include <RMM_Message.h>
#include <RMM_Enum.h>

using namespace RMM;


RBodyPart::RBodyPart()
    :   REntity(),
        status_(static_cast<RMM::MessageStatus>(0))
{
    _init();
}

RBodyPart::RBodyPart(const RBodyPart & part)
    :   REntity(part),
        envelope_           (part.envelope_),
        data_               (part.data_.copy()),
        encoding_           (part.encoding_),
        mimeType_           (part.mimeType_),
        mimeSubType_        (part.mimeSubType_),
        contentDescription_ (part.contentDescription_.copy()),
        disposition_        (part.disposition_),
        boundary_           (part.boundary_.copy()),
        type_               (part.type_),
        preamble_           (part.preamble_.copy()),
        epilogue_           (part.epilogue_.copy()),
        charset_            (part.charset_.copy()),
        status_             (static_cast<RMM::MessageStatus>(0))
{
    _init();
    _replacePartList(part.body_);
}

RBodyPart::RBodyPart(const QCString & s)
    :   REntity(s),
        status_(static_cast<RMM::MessageStatus>(0))
{
    _init();
}

    RBodyPart &
RBodyPart::operator = (const RBodyPart & part)
{
    if (this == &part) return *this;    // Avoid a = a.
    
    envelope_           = part.envelope_;
    data_               = part.data_.copy();
    encoding_           = part.encoding_;
    mimeType_           = part.mimeType_;
    mimeSubType_        = part.mimeSubType_;
    contentDescription_ = part.contentDescription_.copy();
    disposition_        = part.disposition_;
    boundary_           = part.boundary_.copy();
    type_               = part.type_;
    preamble_           = part.preamble_.copy();
    epilogue_           = part.epilogue_.copy();
    charset_            = part.charset_.copy();

    _replacePartList(part.body_);

    status_ = part.status_;

    REntity::operator = (part);

    return *this;
}

RBodyPart::~RBodyPart()
{
    // Empty.
}
    
    RBodyPart &
RBodyPart::operator = (const QCString & part)
{
    REntity::operator = (part);

    body_.clear();

    return *this;
}

    bool
RBodyPart::operator == (RBodyPart & part)
{
    parse();
    
    part.parse();
    
    bool equal = (
        envelope_           == part.envelope_           &&
        data_               == part.data_               &&
        encoding_           == part.encoding_           &&
        mimeType_           == part.mimeType_           &&
        mimeSubType_        == part.mimeSubType_        &&
        contentDescription_ == part.contentDescription_ &&
        disposition_        == part.disposition_        &&
        boundary_           == part.boundary_           &&
        type_               == part.type_               &&
        preamble_           == part.preamble_           &&
        epilogue_           == part.epilogue_           &&
        body_               == part.body_               &&
        charset_            == part.charset_ );
    
    return equal;
}

    MimeType
RBodyPart::mimeType()
{
    parse();
    return mimeType_;
}

    MimeSubType
RBodyPart::mimeSubType()
{
    parse();
    return mimeSubType_;
}

    void
RBodyPart::setMimeType(MimeType t)
{
    parse();
    mimeType_ = t;
}

    void
RBodyPart::setMimeSubType(MimeSubType st)
{
    parse();
    mimeSubType_ = st;
}

    void
RBodyPart::setMimeType(const QCString & s)
{
    parse();
    mimeType_ = mimeTypeStr2Enum(s);
}

    void
RBodyPart::setMimeSubType(const QCString & s)
{
    parse();
    mimeSubType_ = mimeSubTypeStr2Enum(s);
}

    void
RBodyPart::_parse()
{
    //rmmDebug("=== RBodyPart parse start =====================================");
    
    body_.clear();
    mimeType_       = MimeTypeUnknown;
    mimeSubType_    = MimeSubTypeUnknown;
    
    // A body part consists of an envelope and a body.
    // The body may, again, consist of multiple body parts.
    
    int endOfHeaders = strRep_.find(QRegExp("\n\n"));
    
    if (endOfHeaders == -1) {
        
        // The body is blank. We'll treat what there is as the envelope.
        //rmmDebug("empty body");
        envelope_    = strRep_.copy();
        data_        = "";
        
    } else {
        
        // Add 1 to include eol of last header.
        envelope_ = strRep_.left(endOfHeaders + 1);
        
        // Add 2 to ignore eol on last header plus empty line.
        data_ = strRep_.mid(endOfHeaders + 2);
    }
    

    //rmmDebug("Looking to see if there's a Content-Type header");
    // Now see if there's a Content-Type header in the envelope.
    // If there is, we might be looking at a multipart message.
    if (!envelope_.has(HeaderContentType)) {
        
        //rmmDebug("done parse(1)");
        //rmmDebug("=== RBodyPart parse end   =================================");
        return;
    }

    //rmmDebug("There's a Content-Type header");
    
    RContentType contentType(envelope_.contentType());
    
    //rmmDebug("contentType.type() == " + contentType.type());
    
    // If this isn't multipart, we've finished parsing.
    if (stricmp(contentType.type(), "multipart") != 0) {
        mimeType_       = mimeTypeStr2Enum(contentType.type());
        mimeSubType_    = mimeSubTypeStr2Enum(contentType.subType());

        //rmmDebug("=== RBodyPart parse end   =================================");
        return;
    }
 
    //rmmDebug(" ==== This part is multipart ========================");

    QValueList<RParameter> parameterList(contentType.parameterList().list());
    QValueList<RParameter>::Iterator it;
    
    //rmmDebug("Looking for boundary");

    for (it = parameterList.begin(); it != parameterList.end(); ++it)
        if (0 == stricmp((*it).attribute(), "boundary"))
            boundary_ = (*it).value();
    
    //rmmDebug("boundary == \"" + boundary_ + "\"");
    
    if (boundary_.isEmpty()) {
        //rmmDebug("Boundary not found in ContentType header. Giving up.");
        return;
    }
    
    if (boundary_.at(0) == '\"') {
        //rmmDebug("Boundary is quoted. Removing quotes.");
        boundary_.remove(boundary_.length() - 1, 1);
        boundary_.remove(0, 1);

        if (boundary_.isEmpty()) {
            //rmmDebug("The (quoted) boundary is empty ! Giving up.");
            //rmmDebug("done parse(2)");
            //rmmDebug("=== RBodyPart parse end   =============================");
            return;
        }
    }

    QCString bound("--" + boundary_);

    int boundaryStart(data_.find(bound));

    if (boundaryStart == -1) {
        // Let's just call it a plain text message.
        //rmmDebug("No boundary found in message. Assume plain ?");
        //rmmDebug("done parse (3)");
        //rmmDebug("=== RBodyPart parse end   =============================");
        return;
    }

    // We can now take whatever's before the first boundary as the preamble.

    preamble_ = data_.left(boundaryStart).stripWhiteSpace();

    //rmmDebug("preamble:\n" + preamble_);

    int previousBoundaryEnd = boundaryStart + bound.length();

    // Now find the rest of the parts.
    
    // We keep track of the end of the last boundary and the start of the next.

    boundaryStart = data_.find(bound, previousBoundaryEnd);

    while (boundaryStart != -1) {

        RBodyPart * newPart = new RBodyPart(
            data_.mid(previousBoundaryEnd, boundaryStart - previousBoundaryEnd)
        );

        body_.append(newPart);
        
        previousBoundaryEnd = boundaryStart + bound.length();
        
        boundaryStart = data_.find(bound, previousBoundaryEnd);
    }

    // No more body parts. Anything that's left is the epilogue.

    epilogue_ = data_.right(data_.length() - previousBoundaryEnd);

    //rmmDebug("epilogue == \"" + epilogue_ + "\"");
    
    mimeType_       = mimeTypeStr2Enum(contentType.type());
    mimeSubType_    = mimeSubTypeStr2Enum(contentType.subType());

    //rmmDebug("=== RBodyPart parse end   =====================================");
}

    void
RBodyPart::_assemble()
{
    strRep_ = envelope_.asString();
    strRep_ += "\n";
    strRep_ += preamble_;
    strRep_ += data_;
    strRep_ += epilogue_;
}

    void
RBodyPart::createDefault()
{
    envelope_.createDefault();
}

    REnvelope &
RBodyPart::envelope()
{
    parse();
    return envelope_;
}

    QList<RBodyPart>
RBodyPart::body()
{
    parse();
    return body_;
}

    Q_UINT32
RBodyPart::size()
{
    parse();
    return data_.length();
}

    void
RBodyPart::_update()
{
    // STUB
//    type_ = (0 == 1) ? Basic : Mime;
}

    void
RBodyPart::addPart(RBodyPart *)
{
    // STUB
    parse();
    _update();
}

    void
RBodyPart::removePart(RBodyPart *)
{
    // STUB
    parse();
    _update();
}

    RBodyPart::PartType
RBodyPart::type()
{
    parse();
    return type_;
}

    QCString
RBodyPart::data()
{
    parse();
    return data_;
}

    DispType
RBodyPart::disposition()
{
    parse();
    return disposition_;
}

    void
RBodyPart::setEnvelope(REnvelope e)
{
    parse();
    envelope_ = e;
}    

    void
RBodyPart::setData(const QCString & s)
{
    parse();
    data_ = s.copy();
}

    void
RBodyPart::setBody(QList<RBodyPart> & b)
{
    parse();
    body_ = b;
}

    QByteArray
RBodyPart::decode()
{
    rmmDebug("decode()");
    parse();
    REnvelope e;
    QByteArray output;
    
    if (envelope_.has(Cte)) {
        
        rmmDebug("This part has cte header");
    
        switch (envelope_.contentTransferEncoding().mechanism()) {
            
            case CteTypeBase64:
                output = decodeBase64(data_);
                break;

            case CteTypeQuotedPrintable:
                output = decodeQuotedPrintable(data_);
                break;
                
            case CteType7bit:
            case CteType8bit:
            case CteTypeBinary:
            case CteTypeXtension:
            default:
                output = data_.copy();
                break;
        }

    } else {

        rmmDebug("This part is not encoded");
    }

    return output;
}

    void
RBodyPart::setDescription(const QCString & s)
{
    parse();
    contentDescription_ = s.copy();
}

    void
RBodyPart::setEncoding(CteType t)
{
    parse();
    encoding_ = t;
}

    void
RBodyPart::setCharset(const QCString & s)
{
    parse();
    charset_ = s.copy();
}

    QCString
RBodyPart::charset()
{
    parse();
    return charset_;
}


    RBodyPart
RBodyPart::part(unsigned int idx)
{
    parse();
    if (body_.count() < idx)
        return RBodyPart();
    return *(body_.at(idx));
}

    unsigned int
RBodyPart::partCount()
{
    parse();
    return body_.count();
}

    QCString
RBodyPart::preamble()
{
    parse();
    return preamble_;
}

    QCString
RBodyPart::epilogue()
{
    parse();
    return epilogue_;
}

    void
RBodyPart::_init()
{
    body_.setAutoDelete(true);
}

    void
RBodyPart::_replacePartList(const QList<RBodyPart> & l)
{
    body_.clear();

    QListIterator<RBodyPart> it(l);

    for (; it.current(); ++it)
        body_.append(new RBodyPart(*it.current()));
}

    QCString
RBodyPart::asXML(QColor quote1, QColor quote2)
{
    parse();

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
    unsigned int length = data_.length();

    if (length == 0)
        return encodedData;

    QString col;

    col.sprintf("%02X%02X%02X", quote1.red(), quote1.green(), quote1.blue());
    QCString quoteOne(col.utf8());

    col.sprintf("%02X%02X%02X", quote2.red(), quote2.green(), quote2.blue());
    QCString quoteTwo(col.utf8());

    const char * input(data_.data());
    
    const char * inputStart = input;
    const char * inputEnd = input + length;

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

                // Mark up quoted line if that's what we have.
         
                // First char on line?
                if (safeToLookBack() && *(inputCursor - 1) == '\n') {
                    
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
    return encodedData;
}

// vim:ts=4:sw=4:tw=78
