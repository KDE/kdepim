#ifndef KARM_TRAY_H
#define KARM_TRAY_H

#include <tqptrvector.h>
#include <tqpixmap.h>
#include <tqptrlist.h>
// experiement
// #include <kpopupmenu.h>
#include <ksystemtray.h>

#include "task.h"
#include "karm_part.h"

class KarmPart;

class TQPopupMenu;
class TQTimer;

class KSystemTray;
class MainWindow;
// experiment
// class KPopupMenu;

class KarmTray : public KSystemTray
{
  Q_OBJECT

  public:
    KarmTray(MainWindow * parent);
    KarmTray(karmPart * parent);
    ~KarmTray();

  private:
    int _activeIcon;
    static TQPtrVector<TQPixmap> *icons;
    TQTimer *_taskActiveTimer;

  public slots:
    void startClock();
    void stopClock();
    void resetClock();
    void updateToolTip( TQPtrList<Task> activeTasks);
    void initToolTip();

  protected slots:
    void advanceClock();
    
  // experiment
  /*
    void insertTitle(TQString title);

  private:
    KPopupMenu *trayPopupMenu;
    TQPopupMenu *trayPopupMenu2;
    */
};

#endif // KARM_TRAY_H
