
#ifndef _DOCKWINDOW_H
#define _DOCKWINDOW_H

#include <ksystemtray.h>
#include <kpopupmenu.h>

#include "alarmdaemon.h"

using namespace KCal;


class AlarmDockWindow : public KSystemTray
{
    Q_OBJECT
  public:
    AlarmDockWindow(AlarmDaemon&, const QString& defaultClient,
                    QWidget *parent = 0, const char *name = 0);
    virtual ~AlarmDockWindow();

    bool alarmsOn()  { return contextMenu()->isItemChecked(itemId); }

    void updateMenuClients();
    void updateMenuCalendars(bool recreate);
    void addToolTip(const QString&);

  protected:
    void mousePressEvent(QMouseEvent*);
    void closeEvent(QCloseEvent*);

  public slots:
    void toggleAlarmsEnabled()
    {
      contextMenu()->setItemChecked(itemId,
              !contextMenu()->isItemChecked(itemId));
      setPixmap(contextMenu()->isItemChecked(itemId) ? dPixmap1 : dPixmap2);
    }
    void select(int menuIndex);
    void selectCal(int menuIndex);

  protected:
    QPixmap       dPixmap1, dPixmap2;
    int           itemId;

  private:
    AlarmDaemon&  alarmDaemon;
    QString       defaultClient;  // default application name
    int           clientIndex;    // menu index to client names separator
    int           nClientIds;     // number of client names + 1 in menu
    int           nCalendarIds;   // number of calendar URLs + 1 in menu
};

#endif
