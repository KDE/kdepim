#include <iostream>
#include <iomanip>
#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <qstring.h>
#include <qdir.h>
#include <qstringlist.h>
#include <qfile.h>
#include <qdatetime.h>

#include "RMM_Address.h"
#include "RMM_AddressList.h"
#include "RMM_BodyPart.h"
#include "RMM_ContentType.h"
#include "RMM_Cte.h"
#include "RMM_DateTime.h"
#include "RMM_Defines.h"
#include "RMM_ContentDisposition.h"
#include "RMM_Entity.h"
#include "RMM_Enum.h"
#include "RMM_Envelope.h"
#include "RMM_Header.h"
#include "RMM_HeaderBody.h"
#include "RMM_HeaderList.h"
#include "RMM_Mailbox.h"
#include "RMM_Mechanism.h"
#include "RMM_Message.h"
#include "RMM_MessageComponent.h"
#include "RMM_MessageID.h"
#include "RMM_MimeType.h"
#include "RMM_Parameter.h"
#include "RMM_ParameterList.h"
#include "RMM_Text.h"
#include "RMM_Token.h"
#include "RMM_Utility.h"

struct timeval timeval_sub(struct timeval *tv1, struct timeval *tv2)
{
  struct timeval retval;

  retval.tv_sec = tv1->tv_sec - tv2->tv_sec;

  if (tv1->tv_usec >= tv2->tv_usec)

    retval.tv_usec = tv1->tv_usec - tv2->tv_usec;

  else {

    retval.tv_sec--;
    retval.tv_usec = (tv1->tv_usec + 1000000) - tv2->tv_usec;
  }

  return retval;        
}

struct timeval timeval_add(struct timeval *tv1, struct timeval *tv2)
{
  struct timeval retval;

  retval.tv_sec = tv1->tv_sec + tv2->tv_sec;
  retval.tv_usec = tv1->tv_usec + tv2->tv_usec;
  retval.tv_sec += retval.tv_usec / 1000000;
  retval.tv_usec %= 1000000;
  return retval;
}

ostream & operator << (ostream& o, struct timeval& tv)
{
  o.fill('0');
  return o << tv.tv_sec << "." << setw(3) << tv.tv_usec / 1000;
}

int main(int argc, char ** argv)
{
  if (argc != 2) {
    cerr << "Usage: " << argv[0] << " <maildir path>" << endl;
    exit(1);
  }
      
  char * buf = new char[1024];

  QString path = argv[1] + QString("/cur");

  QDir d(path);
  d.setFilter(QDir::Files);

  QStringList l(d.entryList());

  QStringList::ConstIterator it(l.begin());
  cerr << l.count() << " messages to parse" << endl;
  struct timeval tv_total;
  tv_total.tv_sec = tv_total.tv_usec = 0;

  for (; it != l.end(); ++it) {

    QFile f(path + "/" + *it);

    if (!f.open(IO_ReadOnly)) {
      cerr << "Cannot open " << path << "/" << *it << endl;
      exit(1);
    }

    QCString msgBuf;

    Q_UINT32 buflen = 32768;
    char * buf = new char[buflen];

    while (!f.atEnd()) {
        
        int bytesRead = f.readBlock(buf, buflen);

        if (bytesRead == -1) {
            cerr << "A serious error occurred while reading the file." << endl;
            delete [] buf;
            buf = 0;
            f.close();
            return 1;
        }
        
        msgBuf += QCString(buf).left(bytesRead);
    }

    delete [] buf;
    buf = 0;
    f.close();
 
    struct timeval start_tv;
    struct rusage  start_usage;

    gettimeofday(&start_tv, NULL);
    getrusage(RUSAGE_SELF, &start_usage);

    RMM::RMessage m(msgBuf);

    // Simulate creating an index record from the message.
    // This is what Empath does, unless I change it...
    int i;
    QCString s;
    s = m.envelope().subject().asString();
    s = m.envelope().firstSender().phrase();
    s = m.envelope().firstSender().route();
    s = m.envelope().date().asString();
    i = m.status();
    i = m.size();
    s = m.envelope().messageID().asString();
    s = m.envelope().parentMessageId().asString();

    struct timeval stop_tv;
    struct rusage  stop_usage;

    getrusage(RUSAGE_SELF, &stop_usage);
    gettimeofday(&stop_tv, NULL);

    struct timeval real_time;
    struct timeval user_time, sys_time, cpu_time;

    real_time = timeval_sub(&stop_tv, &start_tv);
    user_time = timeval_sub(&stop_usage.ru_utime, &start_usage.ru_utime);
    sys_time = timeval_sub(&stop_usage.ru_stime, &start_usage.ru_stime);
    cpu_time = timeval_add(&user_time, &sys_time);

    struct timeval tv_temp = tv_total;

    tv_total = timeval_add(&tv_temp, &cpu_time);

    cerr  << "r:" << real_time  << " "
          << "u:" << user_time  << " "
          << "s:" << sys_time   << " "
          << "c:" << cpu_time   << " "
          << *it  << endl;
  }

  unsigned int total_us;
  total_us = tv_total.tv_usec;
  total_us += tv_total.tv_sec * 1000000;
  unsigned int average_us = total_us / l.count();

  cerr << "average cpu ms: " << average_us / 1000
       << '.'
       << setw(3) << average_us % 1000 << endl;

  return 0;
}

