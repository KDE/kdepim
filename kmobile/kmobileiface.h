/*
 * Copyright (C) 2003 Helge Deller <deller@kde.org>
 */

#ifndef _KMOBILEIFACE_H_
#define _KMOBILEIFACE_H_

#include <dcopobject.h>
#include <qstringlist.h>

class QStringList;

class kmobileIface : virtual public DCOPObject
{
  K_DCOP
public:

k_dcop:
    virtual QStringList deviceNames() = 0;

    virtual void removeDevice( QString deviceName ) = 0;
    virtual void configDevice( QString deviceName ) = 0;

    virtual bool connectDevice( QString deviceName ) = 0;
    virtual bool disconnectDevice( QString deviceName ) = 0;
    virtual bool connected( QString deviceName ) = 0;

    virtual QString deviceClassName( QString deviceName ) = 0;
    virtual QString deviceName( QString deviceName ) = 0;
    virtual QString revision( QString deviceName ) = 0;
    virtual int classType( QString deviceName ) = 0;

    virtual int capabilities( QString deviceName ) = 0;
    virtual QString nameForCap( QString deviceName, int cap ) = 0;

    virtual QString iconFileName( QString deviceName ) = 0;

    virtual int     numAddresses( QString deviceName ) = 0;
    virtual QString readAddress( QString deviceName, int index ) = 0;
    virtual bool    storeAddress( QString deviceName, int index, QString vcard, bool append ) = 0;

    virtual int numCalendarEntries( QString deviceName ) = 0;

    virtual int numNotes( QString deviceName ) = 0;
    virtual QString readNote( QString deviceName, int index ) = 0;
    virtual bool storeNote( QString deviceName, int index, QString note ) = 0;
};

#endif // _KMOBILEIFACE_H_
