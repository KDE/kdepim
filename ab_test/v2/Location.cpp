#include <KabLocation.h>

  void
Location::save(QDataStream & str)
{  
  Entity::save(str);

  str << (int)type_ << typeName_  << streetAddress_     << area_ 
      << country_   << postCode_  << connectedPersons_;
}

  void
Location::load(QDataStream & str)
{
  Entity::load(str);

  int i;
  str >> i;
  type_ = (LocationType)i;

  str >> typeName_  >> streetAddress_ >> area_
      >> country_   >> postCode_      >> connectedPersons_;
}


