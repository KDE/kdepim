// $Id$

#include <klocale.h>

#include "exceptions.h"

using namespace KCal;

Exception::Exception(const QString &message)
{
  mMessage = message;
}

Exception::~Exception()
{
}

QString Exception::message()
{
  if (mMessage.isEmpty()) return i18n("KOrganizer Error");
  else return mMessage;
}
    

ErrorFormat::ErrorFormat(ErrorCodeFormat code,const QString &message) :
  Exception(message)
{
  mCode = code;
}
    
QString ErrorFormat::message()
{
  QString message = "";

  switch (mCode) {
    case LoadError:
      message = i18n("Load Error");
      break;
    case ParseError:
      message = i18n("Parse Error");
      break;
    case CalVersion1:
      message = i18n("vCalendar Version 1.0 detected");
      break;
    case CalVersion2:
      message = i18n("iCalendar Version 2.0 detected");
      break;
    case Restriction:
      message = i18n("Restriction violation");
    default:
      break;
  }
  
  if (!mMessage.isEmpty()) message += ": " + mMessage;
  
  return message;
}

ErrorFormat::ErrorCodeFormat ErrorFormat::errorCode()
{
  return mCode;
}
