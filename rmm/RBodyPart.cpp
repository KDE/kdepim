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
# pragma implementation "RMM_BodyPart.h"
#endif

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
    :    REntity()
{
    rmmDebug("ctor");
    body_.setAutoDelete(true);
}

RBodyPart::RBodyPart(const RBodyPart & part)
    :    REntity(part),
        envelope_            (part.envelope_),
        data_                (part.data_),
        body_                (part.body_),
        encoding_            (part.encoding_),
        mimeType_            (part.mimeType_),
        mimeSubType_        (part.mimeSubType_),
        contentDescription_    (part.contentDescription_),
        disposition_        (part.disposition_),
        boundary_            (part.boundary_),
        type_                (part.type_),
        preamble_            (part.preamble_),
        epilogue_            (part.epilogue_)
{
    rmmDebug("ctor");
    body_.setAutoDelete(true);
    body_ = part.body_;
}

RBodyPart::RBodyPart(const QCString & s)
    :    REntity(s)
{
    rmmDebug("ctor");
    body_.setAutoDelete(true);
    parsed_ = false;
}

RBodyPart::~RBodyPart()
{
    rmmDebug("dtor");
}

    RBodyPart &
RBodyPart::operator = (const RBodyPart & part)
{
    rmmDebug("operator =");

    if (this == &part) return *this;    // Avoid a = a.
    REntity::operator = (part);
    
    envelope_            = part.envelope_;
    data_                = part.data_;
    body_                = part.body_;
    encoding_            = part.encoding_;
    mimeType_            = part.mimeType_;
    mimeSubType_        = part.mimeSubType_;
    contentDescription_    = part.contentDescription_;
    disposition_        = part.disposition_;
    boundary_            = part.boundary_;
    type_                = part.type_;
    preamble_            = part.preamble_;
    epilogue_            = part.epilogue_;

    return *this;
}
    
    RBodyPart &
RBodyPart::operator = (const QCString & part)
{
    REntity::operator = (part);
    return *this;
}

    bool
RBodyPart::operator == (RBodyPart & part)
{
    parse();
    
    part.parse();
    
    bool equal = (
        envelope_            == part.envelope_            &&
        data_                == part.data_                &&
        encoding_            == part.encoding_            &&
        mimeType_            == part.mimeType_            &&
        mimeSubType_        == part.mimeSubType_        &&
        contentDescription_    == part.contentDescription_    &&
        disposition_        == part.disposition_        &&
        boundary_            == part.boundary_            &&
        type_                == part.type_                &&
        preamble_            == part.preamble_            &&
        epilogue_            == part.epilogue_);
    
    return (equal && false);
    //body_                == part.body_                &&
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
    mimeType_ = t;
    assembled_ = false;
}

    void
RBodyPart::setMimeSubType(MimeSubType st)
{
    mimeSubType_ = st;
    assembled_ = false;
}

    void
RBodyPart::setMimeType(const QCString & s)
{
    mimeType_ = mimeTypeStr2Enum(s);
    assembled_ = false;
}

    void
RBodyPart::setMimeSubType(const QCString & s)
{
    mimeSubType_ = mimeSubTypeStr2Enum(s);
    assembled_ = false;
}

    void
RBodyPart::_parse()
{
    rmmDebug("=== RBodyPart parse start =====================================");
    
    body_.clear();
    mimeType_       = MimeTypeUnknown;
    mimeSubType_    = MimeSubTypeUnknown;
    
    // A body part consists of an envelope and a body.
    // The body may, again, consist of multiple body parts.
    
    int endOfHeaders = strRep_.find(QRegExp("\n\n"));
    
    if (endOfHeaders == -1) {
        
        // The body is blank. We'll treat what there is as the envelope.
        rmmDebug("empty body");
        envelope_    = strRep_;
        data_        = "";
        
    } else {
        
        // Add 1 to include eol of last header.
        envelope_ = strRep_.left(endOfHeaders + 1);
        
        // Add 2 to ignore eol on last header plus empty line.
        data_ = strRep_.mid(endOfHeaders + 2);
    }
    

    rmmDebug("Looking to see if there's a Content-Type header");
    // Now see if there's a Content-Type header in the envelope.
    // If there is, we might be looking at a multipart message.
    if (!envelope_.has(HeaderContentType)) {
        
        parsed_     = true;
        assembled_  = false;
        rmmDebug("done parse(1)");
        rmmDebug("=== RBodyPart parse end   =================================");
        return;
    }

    rmmDebug("There's a Content-Type header");
    
    RContentType contentType(envelope_.contentType());
    
    rmmDebug("contentType.type() == " + contentType.type());
    
    // If this isn't multipart, we've finished parsing.
    if (stricmp(contentType.type(), "multipart") != 0) {
        mimeType_       = mimeTypeStr2Enum(contentType.type());
        mimeSubType_    = mimeSubTypeStr2Enum(contentType.subType());

        rmmDebug("=== RBodyPart parse end   =================================");
        return;
    }
 
    rmmDebug(" ==== This part is multipart ========================");

    RParameterListIterator it(contentType.parameterList());
    
    rmmDebug("Looking for boundary");
    for (; it.current(); ++it) {
    
        if (!stricmp(it.current()->attribute(), "boundary"))
            boundary_ = it.current()->value();
    }
    
    rmmDebug("boundary == \"" + boundary_ + "\"");
    
    if (boundary_.isEmpty()) {
        rmmDebug("Boundary not found in ContentType header. Giving up.");
        parsed_        = true;
        assembled_    = false;
        return;
    }
    
    if (boundary_.at(0) == '\"') {
        rmmDebug("Boundary is quoted. Removing quotes.");
        boundary_.remove(boundary_.length() - 1, 1);
        boundary_.remove(0, 1);

        if (boundary_.isEmpty()) {
            rmmDebug("The (quoted) boundary is empty ! Giving up.");
            parsed_       = true;
            assembled_    = false;
            rmmDebug("done parse(2)");
            rmmDebug("=== RBodyPart parse end   =============================");
            return;
        }
    }

    int boundaryStart(strRep_.find(boundary_, endOfHeaders));

    if (boundaryStart == -1) {
        // Let's just call it a plain text message.
        rmmDebug("No boundary found in message. Assume plain ?");
        parsed_     = true;
        assembled_  = false;
        rmmDebug("done parse (3)");
        rmmDebug("=== RBodyPart parse end   =============================");
        return;
    }

    // We can now take whatever's before the first boundary as the preamble.

    preamble_ = data_.left(boundaryStart).stripWhiteSpace();

    rmmDebug("preamble == \"" + preamble_ + "\"");

    int previousBoundaryEnd = boundaryStart + boundary_.length();

    // Now find the rest of the parts.
    
    // We keep track of the end of the last boundary and the start of the next.

    boundaryStart = data_.find(boundary_, previousBoundaryEnd);

    while (boundaryStart != -1) {

        RBodyPart * newPart =
            new RBodyPart(strRep_.mid(
                previousBoundaryEnd, boundaryStart - previousBoundaryEnd));

        body_.append(newPart);
        
        newPart->parse();
        
        previousBoundaryEnd = boundaryStart + boundary_.length();
        
        boundaryStart = strRep_.find(boundary_, previousBoundaryEnd);
    }

    // No more body parts. Anything that's left is the epilogue.

    epilogue_ = strRep_.right(strRep_.length() - previousBoundaryEnd);

    rmmDebug("epilogue == \"" + epilogue_ + "\"");
    
    mimeType_       = mimeTypeStr2Enum(contentType.type());
    mimeSubType_    = mimeSubTypeStr2Enum(contentType.subType());

    rmmDebug("=== RBodyPart parse end   =====================================");
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

    RBodyPartList &
RBodyPart::body()
{
    parse();
    return body_;
}

    Q_UINT32
RBodyPart::size()
{
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
    rmmDebug("setEnvelope() called");
    parse();
    envelope_ = e;
    assembled_ = false;
}    

    void
RBodyPart::setData(const QCString & s)
{
    rmmDebug("setData() called");
    parse();
    data_ = s;
    assembled_ = false;
}

    void
RBodyPart::setBody(QList<RBodyPart> & b)
{
    rmmDebug("setBody() called");
    parse();
    body_ = b;
    assembled_ = false;
}

    RBodyPart
RBodyPart::decode()
{
    rmmDebug("decode()");
    REnvelope e;
    RBodyPart x;
    
    if (envelope_.has(Cte)) {
        
        rmmDebug("This part has cte header");
    
        switch (envelope_.contentTransferEncoding().mechanism()) {
            
            case CteTypeBase64:
                rmmDebug("This part is encoded in base 64");
                e = envelope_;
                e.set(HeaderContentTransferEncoding, "");
//                x = e.asString() + ecodeBase64(data_);
                break;

            case CteTypeQuotedPrintable:
                rmmDebug("This part is encoded in quoted-printable");
                e = envelope_;
                e.set(HeaderContentTransferEncoding, "");
//                x = e.asString() + RDecodeQuotedPrintable(data_);
                break;
                
            case CteType7bit:
            case CteType8bit:
            case CteTypeBinary:
            case CteTypeXtension:
            default:
                rmmDebug("This part is not encoded");
                return *this;
                break;
        }

    } else {

        rmmDebug("This part is not encoded");
        return *this;
    }

    return x;
}

    void
RBodyPart::setDescription(const QCString & s)
{
    contentDescription_ = s;
}

    void
RBodyPart::setEncoding(CteType t)
{
    encoding_ = t;
}

// vim:ts=4:sw=4:tw=78
