#ifndef KSYNC_FILEVIEWPART
#define KSYNC_FILEVIEWPART

#include <klocale.h>
#include <qpixmap.h>

#include <manipulatorpart.h>


namespace KitchenSync {

    class FileviewPart : public ManipulatorPart {
        Q_OBJECT
    public:
        FileviewPart(QWidget *parent, const char *name,
                     QObject *obj = 0, const char *na=0,
                     const QStringList & = QStringList() );
        virtual ~FileviewPart();

        QString type()const { return QString::fromLatin1("Fileview"); };
        int progress()const { return 0; };
        QString name()const { return i18n("Fileview" ); };
        QString description()const { return i18n("This part is responsible for syncing your\n Files."); };
        QPixmap *pixmap();
        bool partIsVisible()const { return true; };
        bool configIsVisible()const { return true; };
        QWidget* widget();
        QWidget* configWidget();
  private:
        QPixmap m_pixmap;
        QWidget *m_widget;
        QWidget *m_config;
    };
};

#endif
