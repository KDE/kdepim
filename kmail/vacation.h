/*  -*- c++ -*-
    vacation.cpp

    KMail, the KDE mail client.
    Copyright (c) 2002 Marc Mutz <mutz@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License,
    version 2.0, as published by the Free Software Foundation.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifndef __KMAIL_VACATION_H__
#define __KMAIL_VACATION_H__

#include <tqobject.h>

#include <kurl.h>

class TQString;
class TQStringList;
template <typename T> class TQValueList;
namespace KMail {
  class SieveJob;
  class VacationDialog;
}
namespace KMime {
  namespace Types {
    struct AddrSpec;
    typedef TQValueList<AddrSpec> AddrSpecList;
  }
}

namespace KMail {

  class Vacation : public TQObject {
    Q_OBJECT
  public:
    Vacation( TQObject * parent=0, bool checkOnly = false, const char * name=0 );
    virtual ~Vacation();

    bool isUsable() const { return !mUrl.isEmpty(); }

    static TQString defaultMessageText();
    static int defaultNotificationInterval();
    static TQStringList defaultMailAliases();
    static bool defaultSendForSpam();
    static TQString defaultDomainName();

  protected:
    static TQString composeScript( const TQString & messageText,
				  int notificationInterval,
				  const KMime::Types::AddrSpecList & aliases,
                                  bool sendForSpam, const TQString & excludeDomain );
    static bool parseScript( const TQString & script, TQString & messageText,
			     int & notificationInterval, TQStringList & aliases,
                             bool & sendForSpam, TQString & domainName );
    KURL findURL() const;
    void handlePutResult( KMail::SieveJob * job, bool success, bool );


  signals:
    void result( bool success );
    // indicates if the vaction script is active or not
    void scriptActive( bool active );

  protected slots:
    void slotDialogDefaults();
    void slotGetResult( KMail::SieveJob * job, bool success,
			const TQString & script, bool active );
    void slotDialogOk();
    void slotDialogCancel();
    void slotPutActiveResult( KMail::SieveJob *, bool );
    void slotPutInactiveResult( KMail::SieveJob *, bool );
  protected:
    // IO:
    KMail::SieveJob * mSieveJob;
    KURL mUrl;
    // GUI:
    KMail::VacationDialog * mDialog;
    bool mWasActive;
    bool mCheckOnly;
  };

} // namespace KMail

#endif // __KMAIL_VACATION_H__
