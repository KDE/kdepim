#ifndef KARM_TOP_H
#define KARM_TOP_H

#include <kmainwindow.h>
#include <kkeydialog.h>
#include "task.h"
#include "tray.h"

class Karm;
class KAccel;
class KAccelMenuWatch;
class Preferences;
class KarmTray;

class KarmWindow : public KMainWindow
{
  Q_OBJECT

private:
  KAccel          *_accel;
  KAccelMenuWatch *_watcher;
  Karm            *_karm;
  long            _totalTime;
  Preferences     *_preferences;
  KarmTray        *_tray;

  public:
    KarmWindow();
    virtual ~KarmWindow();

  public slots:
    void updateTime( long difference );

  protected slots:
    void keyBindings();
    void resetSessionTime(); 
    void updateTime();
    void updateStatusBar();
    void save();
    void quit();
    void print();
    void slotSelectionChanged();
    
  protected:
    virtual void saveProperties( KConfig* );
    void saveGeometry();
    void loadGeometry();

  private:
    void makeMenus();
    KDialogBase *dialog;
    KAction 
      *actionStart,
      *actionStop,
      *actionDelete,
      *actionEdit;

};

#endif


