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
  cerr << "Arse" << endl;
  str >> type_;
  cerr << "Arse" << endl;
  str >> subType_;
  cerr << "Arse" << endl;
  str >> data_;
  cerr << "Field::load done" << endl;
}

