#ifndef KSYNC_OVERVIEWPART_H
#define KSYNC_OVERVIEWPART_H

#include <klocale.h>
#include <qpixmap.h>

#include <actionpart.h>

namespace KSync {

class OverviewWidget;

class OverviewPart : public ActionPart
{
  Q_OBJECT

  public:
    OverviewPart( QWidget *parent, const char *name,
	                QObject *object = 0, const char *name2 = 0,
                  const QStringList & = QStringList() );
    virtual ~OverviewPart();

    static KAboutData *createAboutData();

    QString type() const;
    QString title() const;
    QString description() const;
    bool hasGui() const;
    QPixmap *pixmap();
    QString iconName() const;
    QWidget *widget();

    void executeAction();

  private slots:
    void slotPartChanged( ActionPart * );
    void slotPartProgress( ActionPart *part, const Progress & );
    void slotPartError( ActionPart *, const Error & );
    void slotSyncProgress( ActionPart *, int, int );
    void slotKonnectorProgress( Konnector *, const Progress & );
    void slotKonnectorError( Konnector *, const Error & );
    void slotProfileChanged( const Profile & );
    void slotStartSync();
    void slotDoneSync();

  private:
    QPixmap m_pixmap;
    OverView::Widget *m_widget;
};

}

#endif
