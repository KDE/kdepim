#ifndef RMM_MESSAGE_COMPONENT_H
#define RMM_MESSAGE_COMPONENT_H

#include <qcstring.h>

namespace RMM {

/**
 * @short Base class of all message components.
 * An RMessageComponent is the base class of all parts of a message.
 * It provides some abstract methods, which need to be implemented by all
 * derived classes.
 * It encapsulates a string representation, which all derived components have.
 * This representation is parsed to create the subcomponents of a component,
 * and assembled from the subcomponents.
 */
class RMessageComponent {

    public:

        virtual ~RMessageComponent();

        RMessageComponent & operator = (const RMessageComponent & m);
        RMessageComponent & operator = (const QCString & s);
        
        bool operator == (RMessageComponent &);
        virtual bool operator == (const QCString &);

        virtual void parse() 
            { if (!parsed_) _parse(); parsed_ = true; assembled_ = false; }
        virtual void assemble() 
            { parse(); if (!assembled_) _assemble(); assembled_ = true; }

        virtual void createDefault()    = 0L;

        QCString asString() { assemble(); return strRep_; }

        virtual const char * className() const { return "RMessageComponent"; }
        
    protected:

        RMessageComponent();
        RMessageComponent(const RMessageComponent & component);
        RMessageComponent(const QCString &);

        virtual void _parse()       = 0L;
        virtual void _assemble()    = 0L;

        QCString            strRep_;
        bool                parsed_;
        bool                assembled_;
};

};

#endif
// vim:ts=4:sw=4:tw=78
