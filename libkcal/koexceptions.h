#ifndef KOEXCEPTIONS_H
#define KOEXCEPTIONS_H
// $Id$
//
// Exception classes of KOrganizer.
//
// We don't use actual C++ exceptions right now. These classes are currently
// returned by an error function, but we can build upon them, when we start
// to use C++ exceptions.

#include <qstring.h>

namespace KCal {

/**
  KOrganizer exceptions base class. This is currently used as a fancy kind of error code not as an
  C++ exception.
*/
class KOException {
  public:
    /** Construct exception with descriptive message \a message. */
    KOException(const QString &message=QString::null);
    virtual ~KOException();

    /** Return descriptive message of exception. */    
    virtual QString message();
    
  protected:
    QString mMessage;
};

/** Calendar format related error class */
class KOErrorFormat : public KOException {
  public:
    enum ErrorCodeFormat { LoadError,ParseError,CalVersion1,CalVersion2,
                           CalVersionUnknown,
                           Restriction };
  
    /** Create format error exception. */
    KOErrorFormat(ErrorCodeFormat code,const QString &message = QString::null);
    
    /** Return format error message. */
    QString message();
    /** Return format error code. */
    ErrorCodeFormat errorCode();
    
  private:
    ErrorCodeFormat mCode;
};

}

#endif
