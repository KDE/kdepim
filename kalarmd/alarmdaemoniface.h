// KDE Alarm Daemon DCOP interface.
// (c) 2001 David Jarvie
// Based on the original, (c) 1998, 1999 Preston Brown

#ifndef ALARMDAEMONIFACE_H
#define ALARMDAEMONIFACE_H

#include <dcopobject.h>

class AlarmDaemonIface : virtual public DCOPObject
{
    K_DCOP
  k_dcop:
    virtual void addCal(const QString& appname, const QString& urlString) = 0;
    virtual void addMsgCal(const QString& appname, const QString& urlString) = 0;
    virtual void reloadCal(const QString& appname, const QString& urlString) = 0;
    virtual void reloadMsgCal(const QString& appname, const QString& urlString) = 0;
    virtual void removeCal(const QString& urlString) = 0;
    virtual void resetMsgCal(const QString& appname, const QString& urlString) = 0;
    virtual void registerApp(const QString& appName, const QString& appTitle,
                             const QString& dcopObject, bool commandLineNotify,
                             bool displayCalendarName) = 0;
    virtual void quit() = 0;
};

#endif
