#ifndef RMM_BODY_PART_H
#define RMM_BODY_PART_H

#include <qcolor.h>
#include <qcstring.h>
#include <qlist.h>

#include <RMM_Entity.h>
#include <RMM_Enum.h>
#include <RMM_Defines.h>
#include <RMM_Envelope.h>

namespace RMM {

class RBodyPart : public REntity {
    
#include "RMM_BodyPart_generated.h"

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
        QList<RBodyPart> body();
        RBodyPart       part(unsigned int);
        unsigned int    partCount();
        
        void            setBody(QList<RBodyPart> &);
        void            setData(const QCString &);
        
        void            addPart(RBodyPart *);
        void            removePart(RBodyPart *);
        
        void            setMimeType(RMM::MimeType);
        void            setMimeType(const QCString &);
        void            setMimeSubType(RMM::MimeSubType);
        void            setMimeSubType(const QCString &);
        
        QCString        charset();
        void            setCharset(const QCString &);

        QCString        description();
        RMM::DispType   disposition();
        
        void            setDescription(const QCString &);
        void            setDisposition(RMM::DispType);
        RMM::CteType    encoding();
        void            setEncoding(RMM::CteType);
        Q_UINT32        size();
        PartType        type();
        QByteArray      decode();

        QCString        asXML(QColor q1, QColor q2);

        QCString        preamble();
        QCString        epilogue();
        
        void setStatus(RMM::MessageStatus s) { status_ = s; }
        RMM::MessageStatus status() const { return status_; }

    protected:
        
        void                _update();
        void                _init();
        void                _replacePartList(QList<RBodyPart> &);

        REnvelope           envelope_;
        QCString            data_;
        RMM::CteType        encoding_;
        RMM::MimeType       mimeType_;
        RMM::MimeSubType    mimeSubType_;
        QCString            contentDescription_;
        RMM::DispType       disposition_;
        QCString            boundary_;
        PartType            type_;
        QCString            preamble_;
        QCString            epilogue_;
        QCString            charset_;
        
        QList<RBodyPart>    body_;
        
        RMM::MessageStatus    status_;
};

}

#endif

// vim:ts=4:sw=4:tw=78
