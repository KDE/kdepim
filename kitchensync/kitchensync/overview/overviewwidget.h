#ifndef KSYNC_OVERVIEW_WIDGET_H
#define KSYNC_OVERVIEW_WIDGET_H

#include <qwidget.h>
#include <qlayout.h>
#include <qlabel.h>

class QTextEdit;
namespace KSync {
    class KonnectorProfile;
    class Profile;
    class Error;
    class Progress;
    typedef QString UDI;

namespace OverView {

    /**
     * This is the MainWidget of the OverView and the only interface to the part...
     *
     */
    class Widget : public QWidget {
        Q_OBJECT
    public:
        Widget( QWidget* parent, const char* name );
        ~Widget();

        void setProfile( const Profile& );
        void setProfile( const QString&,const QPixmap& pix );
        void addProgress( const UDI&, const Progress& );
        void addProgress( ManipulatorPart*, const Progress& );
        void addError( const UDI&, const Error& );
        void addError( ManipulatorPart*, const Error& );
        void startSync();
    private:
        QVBoxLayout* m_lay;
        QLabel* m_device;
        QLabel* m_profile;
        QLabel* m_logo;
        QTextEdit* m_edit;

    };
}
}
#endif
