#include <qstring.h>
#include <qstringlist.h>

#include <KabSubValue.h>

#ifndef KAB_PERSONALNAME_H
#define KAB_PERSONALNAME_H

namespace KAB
{

class PersonalName
{    
  public:

    PersonalName()
    {
      // Empty.
    }
    
    PersonalName(const PersonalName & n)
      : displayName_  (n.displayName_ ),
        firstName_    (n.firstName_   ),
        lastName_     (n.lastName_    ),
        otherNames_   (n.otherNames_  ),
        nickName_     (n.nickName_    ),
        prefixes_     (n.prefixes_    ),
        suffixes_     (n.suffixes_    ),
        xValues_      (n.xValues_     )
    {
      // Empty.
    }
 
    ~PersonalName()
    {
      // Empty.
    }
    
    PersonalName & operator = (const PersonalName & n)
    {
      if (this == &n) return *this;
      
      displayName_  = n.displayName_;
      firstName_    = n.firstName_;
      lastName_     = n.lastName_;
      otherNames_   = n.otherNames_;
      nickName_     = n.nickName_;
      prefixes_     = n.prefixes_;
      suffixes_     = n.suffixes_;
      xValues_      = n.xValues_;
      
      return *this; 
    }
    
    bool operator == (const PersonalName & n) const
    {
      return (
        (displayName_ == n.displayName_ ) &&
        (firstName_   == n.firstName_   ) &&
        (lastName_    == n.lastName_    ) &&
        (otherNames_  == n.otherNames_  ) &&
        (nickName_    == n.nickName_    ) &&
        (prefixes_    == n.prefixes_    ) &&
        (suffixes_    == n.suffixes_    ) &&
        (xValues_     == n.xValues_     ));
    }
 
    QString     displayName  () { return displayName_;  }
    QString     firstName    () { return firstName_;    }
    QString     lastName     () { return lastName_;     }
    QStringList otherNames   () { return otherNames_;   }
    QString     nickName     () { return nickName_;  }
    QStringList prefixes     () { return prefixes_;     }
    QStringList suffixes     () { return suffixes_;     }
    XValueList  xValues      () { return xValues_;      }

   
    void  setDisplayName  (const QString     & s) { displayName_  = s; }
    void  setFirstName    (const QString     & s) { firstName_    = s; }
    void  setLastName     (const QString     & s) { lastName_     = s; }
    void  setOtherNames   (const QStringList & l) { otherNames_   = l; }
    void  setNickName     (const QString     & s) { nickName_     = s; }
    void  setPrefixes     (const QStringList & l) { prefixes_     = l; }
    void  setSuffixes     (const QStringList & l) { suffixes_     = l; }
    void  setXValues      (const XValueList  & l) { xValues_      = l; }
    
    virtual void save(QDataStream & str);
    virtual void load(QDataStream & str);

  private:
    
    QString       displayName_;
    QString       firstName_;
    QString       lastName_;
    QStringList   otherNames_;
    QString       nickName_;
    QStringList   prefixes_;
    QStringList   suffixes_;
    XValueList    xValues_;
};

} // End namespace KAB

#endif

