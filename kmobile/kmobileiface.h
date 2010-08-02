/*
 * Copyright (C) 2003 Helge Deller <deller@kde.org>
 */

#ifndef _KMOBILEIFACE_H_
#define _KMOBILEIFACE_H_

#include <dcopobject.h>
#include <tqstringlist.h>

class TQStringList;

class kmobileIface : virtual public DCOPObject
{
  K_DCOP
public:

k_dcop:
    virtual TQStringList deviceNames() = 0;

    virtual void removeDevice( TQString deviceName ) = 0;
    virtual void configDevice( TQString deviceName ) = 0;

    virtual bool connectDevice( TQString deviceName ) = 0;
    virtual bool disconnectDevice( TQString deviceName ) = 0;
    virtual bool connected( TQString deviceName ) = 0;

    virtual TQString deviceClassName( TQString deviceName ) = 0;
    virtual TQString deviceName( TQString deviceName ) = 0;
    virtual TQString revision( TQString deviceName ) = 0;
    virtual int classType( TQString deviceName ) = 0;

    virtual int capabilities( TQString deviceName ) = 0;
    virtual TQString nameForCap( TQString deviceName, int cap ) = 0;

    virtual TQString iconFileName( TQString deviceName ) = 0;

    virtual int     numAddresses( TQString deviceName ) = 0;
    virtual TQString readAddress( TQString deviceName, int index ) = 0;
    virtual bool    storeAddress( TQString deviceName, int index, TQString vcard, bool append ) = 0;

    virtual int numCalendarEntries( TQString deviceName ) = 0;

    virtual int numNotes( TQString deviceName ) = 0;
    virtual TQString readNote( TQString deviceName, int index ) = 0;
    virtual bool storeNote( TQString deviceName, int index, TQString note ) = 0;

    /*
     * DCOP functions for the devices:/ kioslave
     */
k_dcop:
    virtual TQStringList kio_devices_deviceInfo(TQString deviceName) = 0;
};

#endif // _KMOBILEIFACE_H_
