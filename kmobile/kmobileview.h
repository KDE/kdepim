/*
 * Copyright (C) 2003 Helge Deller <deller@kde.org>
 */

#ifndef _KMOBILEVIEW_H_
#define _KMOBILEVIEW_H_

#include <tqiconview.h>

#include "kmobileiface.h"

#include <kdepimmacros.h>
class KConfig;
class KMobileItem;

/**
 * This is the main view class for kmobile.
 *
 * @short Main view
 * @author Helge Deller <deller@kde.org>
 * @version 0.1
 */
class KDE_EXPORT KMobileView : public TQIconView, public kmobileIface
{
    Q_OBJECT
public:
    KMobileView(TQWidget *parent, KConfig *_config);
    virtual ~KMobileView();

    bool addNewDevice(KConfig *config, KService::Ptr service);
    bool startKonqueror(const TQString &devName);

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

    /* devices kioslave support: */
    TQStringList kio_devices_deviceInfo(TQString deviceName);

public:
    void saveAll();
    void restoreAll();

protected:
    KMobileItem * findDevice( const TQString &deviceName ) const;

protected slots:
    void slotDoubleClicked( TQIconViewItem * item );

signals:
    /**
     * Use this signal to change the content of the statusbar
     */
    void signalChangeStatusbar(const TQString& text);

private:
    KConfig *m_config;

};

#endif // _KMOBILEVIEW_H_
