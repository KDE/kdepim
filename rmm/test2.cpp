#include <iostream>
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
  if (argc != 3) {
    cerr << "Usage: " << argv[0] << " decode [base64 encoded file]" << endl;
    cerr << "Usage: " << argv[0] << " encode [binary file]" << endl;
    cerr << " -- Will result in filename.out" << endl;
    exit(1);
  }
      
  QTime t(QTime::currentTime());

  QCString filename(argv[2]);
  filename+=".out";
  QFile outf(filename);
  outf.open(IO_WriteOnly);

  QFile f(argv[2]);
  f.open(IO_ReadOnly);
      
  if (QString(argv[1]) == "decode")
  {
    QByteArray array=f.readAll();
    QDataStream ds(&outf);
    ds << RMM::decodeBase64(QCString(array));
  }
  else if (QString(argv[1]) == "encode")
  {

    QByteArray array=f.readAll();
    QDataStream ds(&outf);
    ds << RMM::encodeBase64(array);
  }
  else
    cerr << "Improper command line" << endl;


  
  
  QTime t2(QTime::currentTime());

  cerr  << "Decode time: "
        << t.secsTo(t2)
        << "."
        << t.msecsTo(t2) - t.secsTo(t2) * 1000
        << endl;
}

