#include <KabPersonalName.h>
#include <KabSubValue.h>

  void
PersonalName::save(QDataStream & str)
{
  str << displayName_ << firstName_ << lastName_  << otherNames_
      << nickName_    << prefixes_  << suffixes_;

  str << xValues_.count();
  XValueList::Iterator it(xValues_.begin());
  for (; it != xValues_.end(); ++it)
    (*it).save(str);
}

  void
PersonalName::load(QDataStream & str)
{
  str >> displayName_ >> firstName_ >> lastName_  >> otherNames_
      >> nickName_    >> prefixes_  >> suffixes_ ;

  int xValCount;
  str >> xValCount;
  for (int i = 0; i < xValCount; i++) {
    XValue x;
    x.load(str);
    xValues_.append(x);
  }
}


