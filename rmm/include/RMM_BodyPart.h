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
# pragma interface "RMM_BodyPart.h"
#endif

#ifndef RMM_BODY_PART_H
#define RMM_BODY_PART_H

#include <qstring.h>
#include <qlist.h>

#include <RMM_Entity.h>
#include <RMM_Enum.h>
#include <RMM_Defines.h>
#include <RMM_Envelope.h>

namespace RMM {

class RBodyPart : public REntity {
    
#include "generated/RBodyPart_generated.h"

    public:
        
        enum PartType {
            Basic,
            Mime
        };
    
        virtual QCString data();
        
        RMM::MimeType       mimeType();
        RMM::MimeSubType    mimeSubType();
        
        REnvelope &         envelope();
        void                setEnvelope(REnvelope);
        QList<RBodyPart> &  body();
        
        void            setBody(QList<RBodyPart> &);
        void            setData(const QCString &);
        
        void            addPart(RBodyPart *);
        void            removePart(RBodyPart *);
        
        void            setMimeType(RMM::MimeType);
        void            setMimeType(const QCString &);
        void            setMimeSubType(RMM::MimeSubType);
        void            setMimeSubType(const QCString &);

        QCString        description();
        RMM::DispType   disposition();
        
        void            setDescription(const QCString &);
        void            setDisposition(RMM::DispType);
        RMM::CteType    encoding();
        void            setEncoding(RMM::CteType);
        Q_UINT32        size();
        PartType        type();
        RBodyPart       decode();

    protected:
        
        void                _update();

        REnvelope           envelope_;
        QCString            data_;
        QList<RBodyPart>    body_;
        RMM::CteType        encoding_;
        RMM::MimeType       mimeType_;
        RMM::MimeSubType    mimeSubType_;
        QCString            contentDescription_;
        RMM::DispType       disposition_;
        QCString            boundary_;
        PartType            type_;
        QCString            preamble_;
        QCString            epilogue_;
};

typedef QList<RBodyPart>            RBodyPartList;
typedef QListIterator<RBodyPart>    RBodyPartListIterator;

}

#endif

// vim:ts=4:sw=4:tw=78
