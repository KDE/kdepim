#ifndef KARM_TOP_H
#define KARM_TOP_H

#include <kmainwindow.h>

class QListViewItem;
class QPoint;

class KDialogBase;

class Karm;
class KAccel;
class KAccelMenuWatch;
class Preferences;
class KarmTray;

class KarmWindow : public KMainWindow
{
  Q_OBJECT

private:
  KAccel		*_accel;
  KAccelMenuWatch	*_watcher;
  Karm		*_karm;
  long		_totalSum;
  long _sessionSum;
  Preferences *_preferences;
  bool _hideOnClose;
  KarmTray *_tray;

  public:
    KarmWindow();
    virtual ~KarmWindow();

  protected slots:
    void keyBindings();
    void resetSessionTime(); 
    void updateTime();
    void updateStatusBar();
    void save();
    void quit();
    void print();
    void slotSelectionChanged();
    void hideOnClose( bool hide );
    void contextMenuRequest( QListViewItem*, const QPoint&, int );
    void enableStopAll();
    void disableStopAll();
    
  protected:
    void startStatusBar();
    virtual void saveProperties( KConfig* );
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
    KAction* actionPreferences;

    friend class KarmTray;
};

#endif


