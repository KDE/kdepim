/*  -*- c++ -*-
    vacation.cpp

    Copyright (c) 2002 Marc Mutz <mutz@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License,
    version 2.0, as published by the Free Software Foundation.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifndef KSIEVE_KSIEVEUI_VACATION_H
#define KSIEVE_KSIEVEUI_VACATION_H

#include "ksieveui_export.h"

#include <kurl.h>

#include <QtCore/QObject>

class QString;
class QStringList;
template <typename T> class QList;

namespace KMime {
namespace Types {
struct AddrSpec;
typedef QList<AddrSpec> AddrSpecList;
}
}

namespace KManageSieve {
class SieveJob;
}

namespace KSieveUi {

class VacationDialog;

class KSIEVEUI_EXPORT Vacation : public QObject
{
    Q_OBJECT

public:
    explicit Vacation( QObject * parent=0, bool checkonly = false, const KUrl &url = KUrl() );
    virtual ~Vacation();

    bool isUsable() const { return !mUrl.isEmpty(); }
    QString serverName() const { return mServerName; }

    void showVacationDialog();

    static QString defaultMessageText();
    static int defaultNotificationInterval();
    static QStringList defaultMailAliases();
    static bool defaultSendForSpam();
    static QString defaultDomainName();

protected:
    static QString composeScript( const QString & messageText,
                                  int notificationInterval,
                                  const KMime::Types::AddrSpecList & aliases,
                                  bool sendForSpam, const QString & excludeDomain );
    static bool parseScript( const QString & script, QString & messageText,
                             int & notificationInterval, QStringList & aliases,
                             bool & sendForSpam, QString & domainName );
    KUrl findURL(QString &serverName) const;
    void handlePutResult( KManageSieve::SieveJob * job, bool success, bool );


signals:
    void result( bool success );
    // indicates if the vacation script is active or not
    void scriptActive( bool active, const QString &serverName );
    void requestEditVacation();

protected slots:
    void slotDialogDefaults();
    void slotGetResult( KManageSieve::SieveJob * job, bool success,
                        const QString & script, bool active );
    void slotDialogOk();
    void slotDialogCancel();
    void slotPutActiveResult( KManageSieve::SieveJob *, bool );
    void slotPutInactiveResult( KManageSieve::SieveJob *, bool );
protected:
    // IO:
    KManageSieve::SieveJob * mSieveJob;
    KUrl mUrl;
    QString mServerName;
    // GUI:
    VacationDialog * mDialog;
    bool mWasActive;
    bool mCheckOnly;
};

}

#endif
