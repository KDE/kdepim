#include <rmm/Address.h>
#include <rmm/AddressList.h>
#include <rmm/Token.h>

using namespace RMM;

AddressList::AddressList()
    :    HeaderBody()
{
    // Empty.
}


AddressList::AddressList(const AddressList & list)
    :   HeaderBody(list),
        list_(list.list_)
{
    // Empty.
}

AddressList::AddressList(const QCString & s)
    :    HeaderBody(s)
{
    // Empty.
}

AddressList::~AddressList()
{
    // Empty.
}
        
    AddressList &
AddressList::operator = (const AddressList & al)
{
    if (this == &al) return *this; // Don't do a = a.
    
    list_ = al.list_;
    HeaderBody::operator = (al);

    return *this;
}
    
    AddressList &
AddressList::operator = (const QCString & s)
{
    HeaderBody::operator = (s);
    return *this;
}

    bool
AddressList::operator == (AddressList &)
{
    parse();
    rmmDebug("STUB");
    return false;
}
        
    Address
AddressList::at(unsigned int i)
{
    parse();

    if (!list_.isEmpty())
        return *(list_.at(i));

    return Address();
}

    unsigned int
AddressList::count()
{
    parse();
    return list_.count();
}

    void
AddressList::_parse()
{
    list_.clear();

    QStrList l;
    tokenise(strRep_, ",\n\r", l, true, false);
    
    if (l.count() == 0 && !strRep_.isEmpty()) { // Lets try what we have then.

        list_.append(Address(strRep_));
        
    } else {

        QStrListIterator it(l);

        for (; it.current(); ++it)
            list_.append(Address(it.current()));
    }
}

    void
AddressList::_assemble()
{
    bool firstTime = true;

    QValueList<Address>::Iterator it;

    strRep_ = "";
    
    for (it = list_.begin(); it != list_.end(); ++it) {
        if (!firstTime) 
            strRep_ += QCString(",\n    ");
        firstTime = false;
        strRep_ += (*it).asString();
    }
}

    void
AddressList::createDefault()
{
    if (count() == 0) {
        Address a;
        a.createDefault();
        list_.append(a);
    }
}

// vim:ts=4:sw=4:tw=78
