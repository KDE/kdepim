// KDE Alarm Daemon.
// (c) 2001 David Jarvie
// Based on the original, (c) 1998, 1999 Preston Brown

#ifndef _ALARMDAEMON_H
#define _ALARMDAEMON_H

#include "qfont.h"
#include "qstrlist.h"

#include "alarmdaemoniface.h"
#include "calendarlocal.h"

class AlarmDialog;
class AlarmDockWindow;

using namespace KCal;


struct ClientInfo
{
    ClientInfo() { }
    ClientInfo(const QString& titl, const QString& dcopObj, bool cmdLine, bool disp, int menuindex = 0);
    QString  title;             // application title for display purposes
    QString  dcopObject;        // object to receive DCOP messages (if applicable)
    int      menuIndex;         // context menu index to this client's entry
    bool     commandLineNotify; // notify events using command line if client app isn't running
    bool     displayCalName;    // true to display calendar name in tooltip
};

typedef QMap<QString, ClientInfo> ClientMap;   // maps client names against client data

// The ClientIteration class gives secure public access to AlarmDaemon::mClients
class ClientIteration
{
  public:
    ClientIteration(ClientMap& c)     : clients(c) { iter = clients.begin(); }
    bool           ok() const         { return iter != clients.end(); }
    bool           next()             { return ++iter != clients.end(); }
    const QString& appName() const    { return iter.key(); }
    const QString& title() const      { return iter.data().title; }
    int            menuIndex() const  { return iter.data().menuIndex; }
    void           menuIndex(int n)   { iter.data().menuIndex = n; }
  private:
    ClientMap&          clients;
    ClientMap::Iterator iter;
};

// Class for Alarm Daemon calendar access
class ADCalendar : public CalendarLocal
{
  public:
    enum Type { KORGANISER = 0, KALARM = 1 };
    ADCalendar(const QString& url, const QString& appname, Type type, bool quiet = false);
    ~ADCalendar()  { }
    const QString& urlString() const   { return urlString_; }
    const QString& appName() const     { return appName_; }
    bool           loaded() const      { return loaded_; }
    bool           enabled() const     { return enabled_ && !unregistered; }
    bool           available() const   { return loaded_ && !unregistered; }
    Type           actionType() const  { return actionType_; }
    static bool    eventHandled(const QString& ID);
    void           setEventHandled(const QString& ID);
    static void    clearEventsHandled(const QString& calendarURL);
    bool           loadFile(bool quiet = false);

    bool             enabled_;       // events are currently manually enabled
    bool             unregistered;   // client has registered since calendar was
                                     // constructed, but has not since added the
                                     // calendar. Monitoring is disabled.
  private:
    ADCalendar(const ADCalendar&);             // prohibit copying
    ADCalendar& operator=(const ADCalendar&);  // prohibit copying

    QString          urlString_;     // calendar file URL
    QString          appName_;       // name of application owning this calendar
    Type             actionType_;    // action to take on event
    bool             loaded_;        // true if calendar file is currently loaded
    typedef QMap<QString, QString>  EventsHandledMap;   // event ID, calendar URL
    static EventsHandledMap  eventsHandled_; // IDs of displayed KALARM type events
};

typedef QList<ADCalendar> CalendarList;

// The CalendarIteration class gives secure public access to AlarmDaemon::mCalendars
class CalendarIteration
{
  public:
    CalendarIteration(CalendarList& c)  : calendars(c) { calendar = calendars.first(); }
    bool           ok() const           { return !!calendar; }
    bool           next()               { return !!(calendar = calendars.next()); }
    bool           loaded() const       { return calendar->loaded(); }
    bool           available() const    { return calendar->available(); }
    bool           enabled() const      { return calendar->enabled(); }
    void           enabled(bool tf)     { calendar->enabled_ = tf; }
    const QString& urlString() const    { return calendar->urlString(); }
  private:
    CalendarList&  calendars;
    ADCalendar*    calendar;
};



class AlarmDaemon : public QObject, virtual public AlarmDaemonIface
{
    Q_OBJECT
  public:
    AlarmDaemon(QObject *parent = 0, const char *name = 0);
    virtual ~AlarmDaemon();

    const ClientInfo* getClientInfo(const QString& appName);
    ClientIteration   getClientIteration()    { return ClientIteration(mClients); }
    CalendarIteration getCalendarIteration()  { return CalendarIteration(mCalendars); }
    int               clientCount() const     { return mClients.count(); }
    void              setDefaultClient(int menuIndex);
    int               calendarCount() const   { return mCalendars.count(); }

  public slots:
    void    suspend(int minutes);
  private slots:
    void    checkAlarmsSlot();
    void    showAlarmDialog();

  private:
    // DCOP interface
    void    reloadCal(const QString& appname, const QString& urlString)
                       { reloadCal_(appname, expandURL(urlString), false); }
    void    reloadMsgCal(const QString& appname, const QString& urlString)
                       { reloadCal_(appname, expandURL(urlString), true); }
    void    addCal(const QString& appname, const QString& urlString)
                       { addCal_(appname, expandURL(urlString), false); }
    void    addMsgCal(const QString& appname, const QString& urlString)
                       { addCal_(appname, expandURL(urlString), true); }
    void    removeCal(const QString& urlString)
                       { removeCal_(expandURL(urlString)); }
    void    resetMsgCal(const QString& appname, const QString& urlString)
                       { resetMsgCal_(appname, /*expandURL*/(urlString)); }
    void    registerApp(const QString& appName, const QString& appTitle,
                        const QString& dcopObject, bool commandLineNotify,
                        bool displayCalendarName);
    void    quit();

  private:
    void        addCal_(const QString& appname, const QString& urlString, bool msgCal);
    void        reloadCal_(const QString& appname, const QString& urlString, bool msgCal);
    void        reloadCal_(ADCalendar*);
    void        resetMsgCal_(const QString& appname, const QString& urlString);
    void        removeCal_(const QString& urlString);
    void        checkAlarms();
    bool        checkAlarms(ADCalendar*, bool showDialog);
    void        checkAlarms(const QString& appName);
    void        notifyEvent(const ADCalendar*, const QString& eventID);
    QString     readConfig();
    void        writeConfigClient(const QString& appName, const ClientInfo&);
    void        writeConfigCalendar(const QString& appName, const ADCalendar*);
    void        deleteConfigCalendar(const ADCalendar*);
    QString     checkDefaultClient();
    ADCalendar* getCalendar(const QString& calendarURL);
    void        setToolTipStartTimer();
    void        removeDialogEvents(const Calendar*);
    static QString expandURL(const QString& urlString);

    AlarmDockWindow*  mDocker;              // the panel icon
    ClientMap         mClients;             // client application names and data
    CalendarList      mCalendars;           // the calendars being monitored
    AlarmDialog*      mAlarmDialog;
    QTimer*           mAlarmTimer;
    QTimer*           mSuspendTimer;
    QString           mClientDataFile;      // path of file containing client data
    bool              mAlarmTimerSyncing;   // true while alarm timer interval < 1 minute
    bool              mRevisingAlarmDialog; // true while mAlarmDialog is being revised
    bool              mDrawAlarmDialog;     // true to show mAlarmDialog after revision is complete
};

#endif
