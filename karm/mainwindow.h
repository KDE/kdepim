#ifndef KARM_MAIN_WINDOW_H
#define KARM_MAIN_WINDOW_H

#include <kmainwindow.h>
#include <karmdcopiface.h>

class QListViewItem;
class QPoint;
class QString;

class KAccel;
class KDialogBase;

class KAccelMenuWatch;
class KarmTray;
class Preferences;
class TaskView;
class PrintDialog;

/**
 * Main window to tie the application together.
 */

class MainWindow : public KMainWindow, virtual public KarmDCOPIface
{
  Q_OBJECT

  private:
    KAccel          *_accel;
    KAccelMenuWatch *_watcher;
    TaskView        *_taskView;
    long            _totalSum;
    long            _sessionSum;
    Preferences     *_preferences;
    KarmTray        *_tray;

  public:
    MainWindow( const QString &icsfile = "" );
    virtual ~MainWindow();

    // DCOP
    QString version() const;
    QString setStorage( const QString & storage);


  protected slots:
    void keyBindings();
    void startNewSession();
    void resetAllTimes();
    void updateTime( long, long );
    void updateStatusBar();
    void save();
    void exportcsvHistory();
    void quit();
    void print();
    void slotSelectionChanged();
    void contextMenuRequest( QListViewItem*, const QPoint&, int );
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

  private:
    void makeMenus();

    KDialogBase *dialog;
    KAction* actionStart;
    KAction* actionStop;
    KAction* actionStopAll;
    KAction* actionDelete;
    KAction* actionEdit;
//    KAction* actionAddComment;
    KAction* actionMarkAsComplete;
    KAction* actionPreferences;
    KAction* actionClipTotals;
    KAction* actionClipHistory;

    friend class KarmTray;
};

#endif // KARM_MAIN_WINDOW_H
