#include <kmime_util.h>
#include <kmime_header_parsing.h>
#include <kdebug.h>
#include <kinstance.h>
using namespace KMime;


int
main()
{
  KInstance app("# ");
  DateFormatter t;

  time_t ntime = time(0);
  kdDebug()<<"Time now:"<<endl;
  kdDebug()<<"\tFancy : \t"<<t.dateString(ntime)<<endl;
  t.setFormat(DateFormatter::Localized);
  kdDebug()<<"\tLocalized : \t"<<t.dateString(ntime)<<endl;
  t.setFormat(DateFormatter::CTime);
  kdDebug()<<"\tCTime : \t"<<t.dateString(ntime)<<endl;
  t.setFormat(DateFormatter::Iso);
  kdDebug()<<"\tIso   : \t"<<t.dateString(ntime)<<endl;
  kdDebug()<<"\trfc2822 : \t"<<t.rfc2822(ntime)<<endl;
  QString rfcd = t.rfc2822(ntime);
  Types::DateTime dt;
  QDateTime qdt;
  const char *str = rfcd.latin1();
  if ( HeaderParsing::parseDateTime( str, str + rfcd.length(), dt ) ) {
      kdDebug()<<"@@@ ntime = "<<(ntime)<<", dt = "<<(dt.time)<<endl;
      qdt.setTime_t( dt.time );
      kdDebug()<<"@@@ qq = "<< qdt.toString("ddd, dd MMM yyyy hh:mm:ss") <<endl;
      kdDebug()<<"@@@ rfc2822 : "<<t.rfc2822(dt.time)<<endl;
  }
  QString ddd = "Mon, 05 Aug 2002 01:57:51 -0700";
  str = ddd.latin1();
  if ( HeaderParsing::parseDateTime( str, str + ddd.length(), dt ) ) {
      kdDebug()<<"dt = "<<(dt.time)<<endl;
      kdDebug()<<"@@@ rfc2822 : "<<t.rfc2822(dt.time)<<endl;
  }

  t.setCustomFormat("MMMM dddd yyyy Z");
  kdDebug()<<"\tCustom : \t"<<t.dateString(ntime)<<endl;

  ntime -= (24 * 3600 + 1);
  kdDebug()<<"Time 24 hours and 1 second ago:"<<endl;
  t.setFormat( DateFormatter::Fancy );
  kdDebug()<<"\tFancy : \t"<<t.dateString(ntime)<<endl;
  t.setFormat(DateFormatter::Localized);
  kdDebug()<<"\tLocalized : \t"<<t.dateString(ntime)<<endl;
  t.setFormat(DateFormatter::CTime);
  kdDebug()<<"\tCTime : \t"<<t.dateString(ntime)<<endl;
  t.setFormat(DateFormatter::Iso);
  kdDebug()<<"\tIso   : \t"<<t.dateString(ntime)<<endl;
  kdDebug()<<"\trfc2822 : \t"<<t.rfc2822(ntime)<<endl;
  t.setCustomFormat("MMMM dddd Z yyyy");
  kdDebug()<<"\tCustom : \t"<<t.dateString(ntime)<<endl;

  t.setFormat(DateFormatter::Fancy);
  ntime -= (24*3600 *30 + 59);
  kdDebug()<<"Time 31 days and 1 minute ago:"<<endl;
  kdDebug()<<"\tFancy : \t"<<t.dateString(ntime)<<endl;
  t.setFormat(DateFormatter::Localized);
  kdDebug()<<"\tLocalized : \t"<<t.dateString(ntime)<<endl;
  t.setFormat(DateFormatter::CTime);
  kdDebug()<<"\tCTime : \t"<<t.dateString(ntime)<<endl;
  t.setFormat(DateFormatter::Iso);
  kdDebug()<<"\tIso   : \t"<<t.dateString(ntime)<<endl;
  kdDebug()<<"\trfc2822 : \t"<<t.rfc2822(ntime)<<endl;
  t.setCustomFormat("MMMM Z dddd yyyy");
  kdDebug()<<"\tCustom : \t"<<t.dateString(ntime)<<endl;


  kdDebug()<<"Static functions (dates like in the last test):"<<endl;
  kdDebug()<<"\tFancy : \t"<< DateFormatter::formatDate( DateFormatter::Fancy, ntime) <<endl;
  kdDebug()<<"\tLocalized : \t"<< DateFormatter::formatDate( DateFormatter::Localized, ntime) <<endl;
  kdDebug()<<"\tCTime : \t"<< DateFormatter::formatDate( DateFormatter::CTime, ntime ) <<endl;
  kdDebug()<<"\tIso   : \t"<< DateFormatter::formatDate( DateFormatter::Iso, ntime ) <<endl;
  kdDebug()<<"\trfc2822 : \t"<< DateFormatter::rfc2822FormatDate( ntime ) <<endl;
  kdDebug()<<"\tCustom : \t"<< DateFormatter::formatDate( DateFormatter::Custom, ntime,
							  "Z MMMM dddd yyyy") <<endl;
  t.setFormat(DateFormatter::Fancy);
  kdDebug()<<"QDateTime taking: (dates as in first test)"<<endl;
  kdDebug()<<"\tFancy : \t"<<t.dateString((QDateTime::currentDateTime()))<<endl;
  t.setFormat(DateFormatter::Localized);
  kdDebug()<<"\tLocalized : \t"<<t.dateString(QDateTime::currentDateTime())<<endl;
  t.setFormat(DateFormatter::CTime);
  kdDebug()<<"\tCTime : \t"<<t.dateString(QDateTime::currentDateTime())<<endl;
  t.setFormat(DateFormatter::Iso);
  kdDebug()<<"\tIso   : \t"<<t.dateString(QDateTime::currentDateTime())<<endl;
  t.setCustomFormat("MMMM d dddd yyyy Z");
  kdDebug()<<"\tCustom : \t"<<t.dateString(QDateTime::currentDateTime())<<endl;

}
