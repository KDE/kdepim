
#ifndef SSK_KORNBUTT_H
#define SSK_KORNBUTT_H

#include <qtoolbutton.h>

class QString;

class KMailDrop;
class KornShell;
class KornSettings;
class KornBtnStyle;
class KProcess;

/**
 * KornButton instances are the buttons representing one mailbox
 */
class KornButton: public QToolButton
{
  Q_OBJECT

  public:

    /**
     * KornButton Constructor
     * @param parant parent widget
     * @param box mailbox represented by this KornButton instance
     * @param shell back reference to the KornShell instance which created this
     */
    KornButton( QWidget *parent, KMailDrop *box, KornShell *shell);

    /**
     * return the mailbox represented by this KornButton instance
     * @return the mailbox
     */
    KMailDrop * getMailDrop() {return _box;}

  public slots:

    // draws button with the number.
    void setNumber( int );
    // runs the click command associated with this mailbox.
    void runCommand(bool onlyIfUnread);

    /**
     * slot triggered if the right mouse button was clicked on the button
     */
    void popupMenu();

  protected slots:

    void disconnectMonitor();
    void monitorUpdated();
    void commandRun(KProcess*);

  protected:

    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void executeCmd(QString cmd);

  signals:

    void rightClick();
    void dying(KornButton *);

  private:

    KMailDrop *_box;
    KornShell *_shell;
    int _lastNum;
	KornBtnStyle *_style;
};


//FIXME: move this class to a separate file

/** this class sets a head on top the buttons
 * which display the box monitors
 * This is usefull, if the last monitor is erased
 * to provide a clickable widget and a describing relation
 * to the buttons below
 */
class HeadButton: public QToolButton
{
  Q_OBJECT

public:
  HeadButton( QWidget *parent);

protected:

  void mousePressEvent(QMouseEvent *);
  
signals:

  void rightClick();

};

#endif
