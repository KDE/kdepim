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
#include "RMM_DispositionType.h"
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

main(int argc, char ** argv)
{
  if (argc != 2) {
    qDebug("Usage: " + QString(argv[0]) + " <maildir path>");
    exit(1);
  }
      
  char * buf = new char[1024];

  QString path = argv[1] + QString("/cur");

  QDir d(path);
  d.setFilter(QDir::Files);

  QStringList l(d.entryList());

  QTime t(QTime::currentTime());

  QStringList::ConstIterator it(l.begin());

  for (; it != l.end(); ++it) {

    QFile f(path + "/" + *it);

    if (!f.open(IO_ReadOnly)) {
      qDebug("Cannot open " + path + "/" + *it);
      exit(1);
    }

    QCString msgBuf;

    while (!f.atEnd()) {

      f.readLine(buf, 1000);

      msgBuf += buf;
    }

    RMM::RMessage m(msgBuf);
    RMM::RText subject = m.envelope().subject();
    RMM::RAddress sender = m.envelope().firstSender();
    RMM::RDateTime date = m.envelope().date();
  }

  QTime t2(QTime::currentTime());

  qDebug("Parse time: " + QString::number(t.secsTo(t2)) + "." + QString::number(t.msecsTo(t2) - t.secsTo(t2) * 1000));
}

