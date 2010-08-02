#ifndef KARM_MAIN_WINDOW_H
#define KARM_MAIN_WINDOW_H

#include <kparts/mainwindow.h>

#include "karmerrors.h"
#include <karmdcopiface.h>
#include "reportcriteria.h"

class KAccel;
class KAccelMenuWatch;
class KarmTray;
class TQListViewItem;
class TQPoint;
class TQString;

class Preferences;
class PrintDialog;
class Task;
class TaskView;

/**
 * Main window to tie the application together.
 */

class MainWindow : public KParts::MainWindow, virtual public KarmDCOPIface
{
  Q_OBJECT

  private:
    void             makeMenus();
    TQString          _hasTask( Task* task, const TQString &taskname ) const;
    Task*            _hasUid( Task* task, const TQString &uid ) const;

    KAccel*          _accel;
    KAccelMenuWatch* _watcher;
    TaskView*        _taskView;
    long             _totalSum;
    long             _sessionSum;
    Preferences*     _preferences;
    KarmTray*        _tray;
    KAction*         actionStart;
    KAction*         actionStop;
    KAction*         actionStopAll;
    KAction*         actionDelete;
    KAction*         actionEdit;
    KAction*         actionMarkAsComplete;
    KAction*         actionMarkAsIncomplete;
    KAction*         actionPreferences;
    KAction*         actionClipTotals;
    KAction*         actionClipHistory;
    TQString          m_error[ KARM_MAX_ERROR_NO + 1 ];

    friend class KarmTray;

  //private:

    //KDialogBase *dialog;



  public:
    MainWindow( const TQString &icsfile = "" );
    virtual ~MainWindow();

    // DCOP
    TQString version() const;
    TQString taskIdFromName( const TQString &taskName ) const;
    /** @reimp from KarmDCOPIface::addTask */
    int addTask( const TQString &storage );
    /** @reimp from KarmDCOPIface::setPerCentComplete */
    TQString setPerCentComplete( const TQString& taskName, int PerCent );
    /** @reimp from KarmDCOPIface::bookTime */
    int bookTime( const TQString& taskId, const TQString& iso8601StartDateTime, long durationInMinutes );
    /** @reimp from KarmDCOPIface::getError */
    TQString getError( int karmErrorNumber ) const;
    int totalMinutesForTaskId( const TQString& taskId );
    /** start the timer for taskname */
    TQString starttimerfor( const TQString &taskname );
    /** stop the timer for taskname */
    TQString stoptimerfor( const TQString &taskname );
    TQString deletetodo();
    /** shall there be a "really delete" question */
    bool    getpromptdelete();
    /** set if there will be a "really delete" question */
    TQString setpromptdelete( bool prompt );
    TQString exportcsvfile( TQString filename, TQString from, TQString to, int type, bool decimalMinutes, bool allTasks, TQString delimiter, TQString quote );
    TQString importplannerfile( TQString filename );

  public slots:
    void setStatusBar( TQString );
    void quit();

  protected slots:
    void keyBindings();
    void startNewSession();
    void resetAllTimes();
    void updateTime( long, long );
    void updateStatusBar();
    bool save();
    void exportcsvHistory();
    void print();
    void slotSelectionChanged();
    void contextMenuRequest( TQListViewItem*, const TQPoint&, int );
    void enableStopAll();
    void disableStopAll();
//    void timeLoggingChanged( bool on );

  protected:
    void startStatusBar();
    virtual void saveProperties( KConfig* );
    virtual void readProperties( KConfig* );
    void saveGeometry();
    void loadGeometry();
    bool queryClose();

};

#endif // KARM_MAIN_WINDOW_H
