#ifndef ssk_karm_h
#define ssk_karm_h

#include <qstring.h>
#include <qptrlist.h>
#include <qobject.h>
#include <qlistview.h>
#include <qptrvector.h>
#include <qpixmap.h>
#include "taskview.h"
#include "logging.h"
#include "todo.h"
#include "event.h"

class QFile;
class QTimer;
class Logging;

/**
 * Store information about each task.
 */

class Task : public QObject, public QListViewItem
{
  Q_OBJECT

  public:
    /** constructor */
    Task( const QString& taskame, long minutes, long sessionTime, 
          DesktopListType desktops, QListView *parent = 0);
    Task( const QString& taskame, long minutes, long sessionTime, 
          DesktopListType desktops, QListViewItem *parent = 0);
    Task( KCal::Incidence* event, QListView* parent );

    void init( const QString& taskame, long minutes, long sessionTime, 
               DesktopListType desktops);

    /** increments the total task time
     *  @param minutes to increment by
     */
    void incrementTime( long minutes );

    /** decrements the total task time
     *  @param minutes to decrement by
     */
    void decrementTime( long minutes );

    /** sets the total time accumulated by the task
     *  @param minutes time in minutes
     */
    void setTotalTime ( long minutes );
    void setSessionTime ( long minutes );
    void setDesktopList ( DesktopListType dl );

    /** returns the total time accumulated by the task
     * @return total time in minutes
     */
    long totalTime() const
           { return _totalTime; };

    long sessionTime() const
           { return _sessionTime; };

    DesktopListType getDesktops()
           { return _desktops;}

    QString getDesktopStr() const;

    /** sets the name of the task
     *  @param name    a pointer to the name. A deep copy will be made.
     */
    void setName( const QString& name );

    /** returns the name of this task.
     * @return a pointer to the name.
     */
    QString name() const
           { return _name; };

    /** Updates the content of the QListViewItem with respect to _name and
     *  _totalTime */
    inline void update()
           { setText(0, _name);
             setText(1, TaskView::formatTime(_sessionTime));
             setText(2, TaskView::formatTime(_totalTime));
           }

    void resetSessionTime();

    void setRunning(bool on);
    bool isRunning() const;

    KCal::Event* asEvent( int level );

    static bool parseIncidence( KCal::Incidence*, long&, QString&, int&,
                                DesktopListType& );

  protected slots:
    void updateActiveIcon();

  private:
    long totalTimeInSeconds() const
           { return totalTime() * 60; }

    void noNegativeTimes();

    QString _name;
    long _totalTime;
    long _sessionTime;
    DesktopListType _desktops;
    QTimer *_timer;
    int _i;
    static QPtrVector<QPixmap> *icons;
    Logging *_logging;
};

#endif
