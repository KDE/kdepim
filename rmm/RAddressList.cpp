#include <iostream>

#include <RMM_Address.h>
#include <RMM_AddressList.h>
#include <RMM_HeaderBody.h>
#include <RMM_Token.h>

using namespace RMM;

RAddressList::RAddressList()
    :    RHeaderBody()
{
    // Empty.
}


RAddressList::RAddressList(const RAddressList & list)
    :   RHeaderBody(list),
        list_(list.list_)
{
    // Empty.
}

RAddressList::RAddressList(const QCString & s)
    :    RHeaderBody(s)
{
    // Empty.
}

RAddressList::~RAddressList()
{
    // Empty.
}
        
    RAddressList &
RAddressList::operator = (const RAddressList & al)
{
    if (this == &al) return *this; // Don't do a = a.
    
    list_ = al.list_;
    RHeaderBody::operator = (al);

    assembled_    = false;
    return *this;
}
    
    RAddressList &
RAddressList::operator = (const QCString & s)
{
    RHeaderBody::operator = (s);
    return *this;
}

    bool
RAddressList::operator == (RAddressList & al)
{
    parse();
    if (al.list_.count() != list_.count()) return false;
    return true; // FIXME: Duh ? This isn't right.
}
        
    RAddress
RAddressList::at(unsigned int i)
{
    parse();

    cerr << "list count == " << list_.count() << endl;
    if (!list_.isEmpty())
        return *(list_.at(i));

    return RAddress();
}

    unsigned int
RAddressList::count()
{
    parse();
    return list_.count();
}

    void
RAddressList::_parse()
{
    list_.clear();

    QStrList l;
    RTokenise(strRep_, ",\n\r", l, true, false);

    if (l.count() == 0 && !strRep_.isEmpty()) { // Lets try what we have then.

        list_.append(RAddress(strRep_));
        
    } else {

        QStrListIterator it(l);

        for (; it.current(); ++it)
            list_.append(RAddress(it.current()));
    }
}

    void
RAddressList::_assemble()
{
    bool firstTime = true;

    QValueList<RAddress>::Iterator it;

    strRep_ = "";
    
    for (it = list_.begin(); it != list_.end(); ++it) {
        if (!firstTime) 
            strRep_ += QCString(",\n    ");
        firstTime = false;
        strRep_ += (*it).asString();
    }
}

    void
RAddressList::createDefault()
{
    if (count() == 0) {
        RAddress a;
        a.createDefault();
        list_.append(a);
    }
}

// vim:ts=4:sw=4:tw=78
