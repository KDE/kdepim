#ifndef KSYNC_OVERVIEWPART_H
#define KSYNC_OVERVIEWPART_H

#include <klocale.h>
#include <qpixmap.h>

#include <manipulatorpart.h>

namespace KSync {

class OverviewWidget;

class OverviewPart : public ManipulatorPart
{
    Q_OBJECT
  public:
    OverviewPart( QWidget *parent, const char *name,
	          QObject *object=0, const char *name2 = 0, // make GenericFactory loading possible
	          const QStringList & = QStringList() );
    virtual ~OverviewPart();

    static KAboutData *createAboutData();

    QString type() const;
    QString name() const;
    QString description() const;
    bool hasGui() const;
    QPixmap *pixmap();
    QString iconName() const;
    QWidget *widget();

  private slots:
    void slotPartChanged( ManipulatorPart * );
    void slotPartProgress( ManipulatorPart *part, const Progress & );
    void slotPartError( ManipulatorPart *, const Error & );
    void slotSyncProgress( ManipulatorPart *, int, int );
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
