#include <iostream>

#include <KabField.h>

using namespace KAB;

  void
Field::save(QDataStream & str)
{
  str << name_;
  str << type_;
  str << subType_;
  str << data_;
}

  void
Field::load(QDataStream & str)
{
  cerr << "Field::load" << endl;
  str >> name_;
  cerr << "- My name is \"" << name_ << "\"" << endl;
  str >> type_;
  cerr << "- My type is \"" << type_ << "\"" << endl;
  str >> subType_;
  cerr << "- My subtype is \"" << subType_ << "\"" << endl;
  str >> data_;
  cerr << "Field: Done reading myself" << endl;
}

