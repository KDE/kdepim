#include <Enum.h>

using namespace KAB;

  ValueType
KAB::typeNameToEnum(const QString & s)
{
  ValueType t(XValue);
  
  if (s.isEmpty())
    return t;
  
  switch (s[0]) {
    
    case 't':
      if       ( s == typeText    ) t = Text;
      else if  ( s == typeTime    ) t = Time;
      break;
  
    case 'd':
      if       ( s == typeDate    ) t = Date;
      else if  ( s == typeDateTime) t = DateTime;
      else if  ( s == typeDouble  ) t = Double;
      break;
  
    case 'e':
      if      ( s == typeEmail    ) t = Email;
      break;
  
    case 'i':
      if      ( s == typeImage    ) t = Image;
      else if ( s == typeImageRef ) t = ImageRef;
      else if ( s == typeInteger  ) t = Integer;
      break;
  
    case 'u':
      if      ( s == typeURL      ) t = URL;
      else if ( s == typeUTCOffset) t = UTCOffset;
      break;
  
    case 's':
      if      ( s == typeSound    ) t = Sound;
      else if ( s == typeSoundRef ) t = SoundRef;
      break;
      
    default:
      break;
  }
  
  return t;
}

  QString
KAB::valueTypeToString(ValueType t)
{
  QString s(QString::null);
  
  switch (t) {
  
    case Text:      s = typeText;       break;
    case Date:      s = typeDate;       break;
    case Time:      s = typeTime;       break;
    case DateTime:  s = typeDateTime;   break;
    case Email:     s = typeEmail;      break;
    case Image:     s = typeImage;      break;
    case ImageRef:  s = typeImageRef;   break;
    case URL:       s = typeURL;        break;
    case Integer:   s = typeInteger;    break;
    case Double:    s = typeDouble;     break;
    case Sound:     s = typeSound;      break;
    case SoundRef:  s = typeSoundRef;   break;
    case UTCOffset: s = typeUTCOffset;  break;
    default:                            break;
  }
  
  return s;
}

// vim:ts=2:sw=2:tw=78:
