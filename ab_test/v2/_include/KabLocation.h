#include <qstring.h>

#ifndef KAB_LOCATION_H
#define KAB_LOCATION_H

namespace KAB
{

class Location : public Entity
{
  public:
    
    Location()
      : Entity(EntityTypeLocation)
    {
    }

    Location(AddressBook & pab, const QString & name)
      : Entity(EntityTypeLocation, pab, name)
    {
      // Empty.
    }
    
    Location(const Location & l)
      : Entity          (l),
        type_           (l.type_          ),
        typeName_       (l.typeName_      ),
        streetAddress_  (l.streetAddress_ ),
        area_           (l.area_          ),
        country_        (l.country_       ),
        postCode_       (l.postCode_      )
    {
      // Empty.
    }
    
    ~Location()
    {
      // Empty.
    }
  
    Location & operator = (const Location & l)
    {
      if (this == &l) return *this;
      
      type_             = l.type_;
      typeName_         = l.typeName_;
      streetAddress_    = l.streetAddress_;
      area_             = l.area_;
      country_          = l.country_;
      postCode_         = l.postCode_;
      
      Entity::operator = (l);
      
      return *this;
    }
    
    bool operator == (const Location & l) const
    {
      return (
        (type_          == l.type_          ) &&
        (typeName_      == l.typeName_      ) &&
        (streetAddress_ == l.streetAddress_ ) &&
        (area_          == l.area_          ) &&
        (country_       == l.country_       ) &&
        (postCode_      == l.postCode_      ) &&
        (xValues_       == l.xValues_       ));
 
    } 
        
    LocationType  type()          const { return type_;           }
    QString       typeName()      const { return typeName_;       }
    QString       streetAddress() const { return streetAddress_;  }
    DivisionList  area()          const { return area_;           }
    QString       country()       const { return country_;        }
    QString       postCode()      const { return postCode_;       }
 
    void  setType             (const LocationType  & t)
    { touch(); type_ = t;     }
    
    void  setTypeName         (const QString       & s)
    { touch(); typeName_ = s; }
    
    void  setStreetAddress    (const QString       & s)
    { touch(); streetAddress_ = s; }
    
    void  setArea             (const DivisionList  & l)
    { touch(); area_ = l; }
    
    void  setCountry          (const QString       & s)
    { touch(); country_ = s; }
    
    void  setPostCode         (const QString       & s)
    { touch(); postCode_ = s; }
    
    friend QDataStream & operator << (QDataStream &, const Location &);
    friend QDataStream & operator >> (QDataStream &, Location &);
    
  private:
    
    LocationType  type_;
    QString       typeName_;
    QString       streetAddress_;
    DivisionList  area_;
    QString       country_;
    QString       postCode_;
    PersonRefList connectedPersons_;
};

  QDataStream &
operator << (QDataStream & str, const Location & l)
{  
  str << (int)l.type_ << l.typeName_ << l.streetAddress_ << l.area_ 
    << l.country_ << l.postCode_ << l.connectedPersons_;
  
  operator << (str, *((Entity *)&l));
  
  return str;
}

  QDataStream &
operator >> (QDataStream & str, Location & l)
{
  int i;
  str >> i;
  l.type_ = (LocationType)i;

  str >> l.typeName_ >> l.streetAddress_ >> l.area_ >> l.country_
      >> l.postCode_ >> l.connectedPersons_;
 
  operator >> (str, *((Entity *)&l));
  
  return str;
}

} // End namespace KAB

#endif
