#include <KabField.h>
#include <KabSubValue.h>

  void
Field::save(QDataStream & str)
{
  cerr << "Field::save" << endl;
  str << name_;
  str << subValueList_.count();
  SubValueList::Iterator it(subValueList_.begin());
  for (; it != subValueList_.end(); ++it)
    (*it).save(str);
}

  void
Field::load(QDataStream & str)
{
  cerr << "Field::load" << endl;
  str >> name_;

  int subValCount;
  str >> subValCount;
  for (int i = 0; i < subValCount; i++) {
    SubValue x;
    x.load(str);
    subValueList_.append(x);
  }
}

