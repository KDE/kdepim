#ifndef RMM_BODY_PART_H
#define RMM_BODY_PART_H

#include <qcolor.h>
#include <qcstring.h>
#include <qlist.h>

#include <rmm/MessageComponent.h>
#include <rmm/Enum.h>
#include <rmm/Defines.h>
#include <rmm/Envelope.h>
#include <rmm/Entity.h>

namespace RMM {

class BodyPart : public Entity {
    
#include "rmm/BodyPart_generated.h"

    public:
        
        enum PartType {
            Basic,
            Mime
        };
    
        virtual QCString data();
        
        RMM::MimeGroup  mimeGroup();
        RMM::MimeValue  mimeValue();
        
        Envelope &      envelope();
        void            setEnvelope(Envelope);
        QList<BodyPart> body();
        BodyPart        part(unsigned int);
        unsigned int    partCount();
        
        void            setBody(QList<BodyPart> &);
        void            setData(const QCString &);
        
        void            addPart(BodyPart *);
        void            removePart(BodyPart *);
        
        void            setMimeGroup(RMM::MimeGroup);
        void            setMimeGroup(const QCString &);
        void            setMimeValue(RMM::MimeValue);
        void            setMimeValue(const QCString &);
        
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
        void                _replacePartList(const QList<BodyPart> &);

        Envelope            envelope_;
        QCString            data_;
        RMM::CteType        encoding_;
        RMM::MimeGroup      mimeGroup_;
        RMM::MimeValue      mimeValue_;
        QCString            contentDescription_;
        RMM::DispType       disposition_;
        QCString            boundary_;
        PartType            type_;
        QCString            preamble_;
        QCString            epilogue_;
        QCString            charset_;
        
        QList<BodyPart>     body_;
        
        RMM::MessageStatus    status_;
};

typedef BodyPart Message;

}

#endif

// vim:ts=4:sw=4:tw=78
