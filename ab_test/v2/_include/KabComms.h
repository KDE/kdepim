#include <KabSubValue.h>
#include <KabTelephone.h>
#include <KabTalkAddress.h>
#include <KabEmailAddress.h>

#ifndef KAB_COMMS_H
#define KAB_COMMS_H

namespace KAB
{

class Comms
{ 
  public:

    Comms()
    {
      // Empty.
    }
    
    Comms(const Comms & c)
      : email_    (c.email_   ),
        tel_      (c.tel_     ),
        fax_      (c.fax_     ),
        talk_     (c.talk_    ),
        xValues_  (c.xValues_ )
    {
      // Empty.
    }
 
    ~Comms()
    {
      // Empty.
    }
    
    Comms & operator = (const Comms & c)
    {
      if (this == &c) return *this;
      
      email_    = c.email_;
      tel_      = c.tel_;
      fax_      = c.fax_;
      talk_     = c.talk_;
      xValues_  = c.xValues_;
      
      return *this; 
    }
    
    bool operator == (const Comms & c) const
    {
      return (
        (email_    == c.email_  ) &&
        (tel_      == c.tel_    ) &&
        (fax_      == c.fax_    ) &&
        (talk_     == c.talk_   ) &&
        (xValues_  == c.xValues_));
    }
    
    EmailAddress  email()   const { return email_;    }
    Telephone     tel()     const { return tel_;      }
    Telephone     fax()     const { return fax_;      }
    TalkAddress   talk()    const { return talk_;     }
    XValueList    xValues() const { return xValues_;  }
    
    void setEmail   (const EmailAddress & a) { email_     = a; }
    void setTel     (const Telephone    & t) { tel_       = t; }
    void setFax     (const Telephone    & t) { fax_       = t; }
    void setTalk    (const TalkAddress  & a) { talk_      = a; }
    void setXValues (const XValueList   & l) { xValues_   = l; }

  private:
    
    EmailAddress  email_;
    Telephone     tel_;
    Telephone     fax_;
    TalkAddress   talk_;
    XValueList    xValues_;
};

} // End namespace KAB

#endif

