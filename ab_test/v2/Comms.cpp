#include <KabComms.h>
#include <KabEmailAddress.h>
#include <KabTelephone.h>
#include <KabTalkAddress.h>
#include <KabSubValue.h>

  void
Comms::save(QDataStream & str)
{
  email_  .save(str);
  tel_    .save(str);
  fax_    .save(str);
  talk_   .save(str);
  str << xValues_.count();
  XValueList::Iterator it(xValues_.begin());
  for (; it != xValues_.end(); ++it)
    (*it).save(str);
}
  
  void
Comms::load(QDataStream & str)
{
  email_  .load(str);
  tel_    .load(str);
  fax_    .load(str);
  talk_   .load(str);
  int xValCount;
  str >> xValCount;
  for (int i = 0; i < xValCount; i++) {
    XValue x;
    x.load(str);
    xValues_.append(x);
  }
}


