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

    TQCString appId() const { return m_clientAppId; };

    bool isKMobileAvailable();
    bool startKMobileApplication();

    /**
     * DCOP implementation
     */
    TQStringList deviceNames();

    void removeDevice( TQString deviceName );
    void configDevice( TQString deviceName );

    bool connectDevice( TQString deviceName );
    bool disconnectDevice( TQString deviceName );
    bool connected( TQString deviceName );

    TQString deviceClassName( TQString deviceName );
    TQString deviceName( TQString deviceName );
    TQString revision( TQString deviceName );
    int classType( TQString deviceName );

    int capabilities( TQString deviceName );
    TQString nameForCap( TQString deviceName, int cap );

    TQString iconFileName( TQString deviceName );

    int     numAddresses( TQString deviceName );
    TQString readAddress( TQString deviceName, int index );
    bool    storeAddress( TQString deviceName, int index, TQString vcard, bool append );

    int numCalendarEntries( TQString deviceName );

    int numNotes( TQString deviceName );
    TQString readNote( TQString deviceName, int index );
    bool storeNote( TQString deviceName, int index, TQString note );

private:
    TQCString m_clientAppId;
    TQCString m_kmobileApp;
    TQCString m_kmobileObj;

};

#endif // _KMOBILECLIENT_H_
