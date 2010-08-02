#ifndef _KARMPART_H_
#define _KARMPART_H_

#include <kparts/part.h>
#include "karmerrors.h"
#include <kparts/factory.h>
#include <karmdcopiface.h>
#include "reportcriteria.h"
#include <tqlistview.h>

class KAccel;
class KAccelMenuWatch;
class KarmTray;
class TQWidget;
class TQPainter;
class KURL;

class Preferences;
class Task;
class TaskView;

/**
 * This is a "Part".  It that does all the real work in a KPart
 * application.
 *
 * @short Main Part
 * @author Thorsten Staerk <kde at staerk dot de>
 * @version 0.1
 */
class karmPart : public KParts::ReadWritePart, virtual public KarmDCOPIface
{
  Q_OBJECT

  private:
    void             makeMenus();
    TQString          _hastodo( Task* task, const TQString &taskname ) const;
    TQString          _hasTask( Task* task, const TQString &taskname ) const;
    Task*            _hasUid( Task* task, const TQString &uid ) const;

    KAccel*          _accel;
    KAccelMenuWatch* _watcher;
    TaskView*        _taskView;
    Preferences*     _preferences;
    KarmTray*        _tray;
    KAction*         actionStart;
    KAction*         actionStop;
    KAction*         actionStopAll;
    KAction*         actionDelete;
    KAction*         actionEdit;
//    KAction* actionAddComment;
    KAction*         actionMarkAsComplete;
    KAction*         actionMarkAsIncomplete;
    KAction*         actionPreferences;
    KAction*         actionClipTotals;
    KAction*         actionClipHistory;
    TQString          m_error[ KARM_MAX_ERROR_NO + 1 ];

    friend class KarmTray;

public:
    karmPart(TQWidget *parentWidget, const char *widgetName,
             TQObject *parent, const char *name);
    // DCOP
    void quit();
    virtual bool save();
    TQString version() const;
    TQString taskIdFromName( const TQString &taskName ) const;
    /** @reimp from KarmDCOPIface */
    int addTask( const TQString &taskName );
    /** @reimp from KarmDCOPIface */
    TQString setPerCentComplete( const TQString& taskName, int PerCent );
    /** @reimp from KarmDCOPIface */
    int bookTime( const TQString& taskId, const TQString& iso8601StartDateTime, long durationInMinutes );
    /** @reimp from KarmDCOPIface */
    TQString getError( int karmErrorNumber ) const;
    int totalMinutesForTaskId( const TQString& taskId );
    TQString starttimerfor( const TQString &taskname );
    TQString stoptimerfor( const TQString &taskname );
    TQString deletetodo();
    bool    getpromptdelete();
    TQString setpromptdelete( bool prompt );
    TQString exportcsvfile( TQString filename, TQString from, TQString to, int type = 0, bool decimalMinutes=true, bool allTasks=true, TQString delimiter="r", TQString quote="q" );
    TQString importplannerfile( TQString filename );

    virtual ~karmPart();

    /**
     * This is a virtual function inherited from KParts::ReadWritePart.  
     * A shell will use this to inform this Part if it should act
     * read-only
     */
    virtual void setReadWrite(bool rw);

    /**
     * Reimplemented to disable and enable Save action
     */
    virtual void setModified(bool modified);

protected:
    /**
     * This must be implemented by each part
     */
    virtual bool openFile();

    /**
     * This must be implemented by each read-write part
     */
    virtual bool saveFile();

protected slots:
    void contextMenuRequest( TQListViewItem*, const TQPoint& point, int );
    void fileOpen();
    void fileSaveAs();
    void slotSelectionChanged();
    void startNewSession(); 
};

class KInstance;
class KAboutData;

class karmPartFactory : public KParts::Factory
{
    Q_OBJECT
public:
    karmPartFactory();
    virtual ~karmPartFactory();
    virtual KParts::Part* createPartObject( TQWidget *parentWidget, const char *widgetName,
                                            TQObject *parent, const char *name,
                                            const char *classname, const TQStringList &args );
    static KInstance* instance();
 
private:
    static KInstance* s_instance;
    static KAboutData* s_about;
};

#endif // _KARMPART_H_
