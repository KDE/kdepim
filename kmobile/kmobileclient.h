/*
 * Copyright (C) 2003-2005 Helge Deller <deller@kde.org>
 */

#ifndef _KMOBILECLIENT_H_
#define _KMOBILECLIENT_H_

#include <dcopclient.h>
#include <kdepimmacros.h>

class KDE_EXPORT KMobileClient : public DCOPClient
{
    Q_OBJECT
public:
    KMobileClient();
    virtual ~KMobileClient();

    QCString appId() const { return m_clientAppId; };

    bool isKMobileAvailable();
    bool startKMobileApplication();

    /**
     * DCOP implementation
     */
    QStringList deviceNames();

    void removeDevice( QString deviceName );
    void configDevice( QString deviceName );

    bool connectDevice( QString deviceName );
    bool disconnectDevice( QString deviceName );
    bool connected( QString deviceName );

    QString deviceClassName( QString deviceName );
    QString deviceName( QString deviceName );
    QString revision( QString deviceName );
    int classType( QString deviceName );

    int capabilities( QString deviceName );
    QString nameForCap( QString deviceName, int cap );

    QString iconFileName( QString deviceName );

    int     numAddresses( QString deviceName );
    QString readAddress( QString deviceName, int index );
    bool    storeAddress( QString deviceName, int index, QString vcard, bool append );

    int numCalendarEntries( QString deviceName );

    int numNotes( QString deviceName );
    QString readNote( QString deviceName, int index );
    bool storeNote( QString deviceName, int index, QString note );

private:
    QCString m_clientAppId;
    QCString m_kmobileApp;
    QCString m_kmobileObj;

};

#endif // _KMOBILECLIENT_H_
