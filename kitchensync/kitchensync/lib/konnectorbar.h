#ifndef KSYNC_KONNECTOR_STATUS_BAR_H
#define KSYNC_KONNECTOR_STATUS_BAR_H

#include <qhbox.h>
#include <qlabel.h>
#include <qpixmap.h>

namespace KSync {
    /**
     * The konnector bar is meant to represent
     * the connection state of a Konnector and
     * allow toggling connection/disconnection of a konnector
     */
    typedef QLabel KonnectorLabel;
    class KonnectorState : public QLabel {
        Q_OBJECT
    public:
        KonnectorState( QWidget* wid );
        ~KonnectorState();
        void setState( bool );
        bool state()const;

    signals:
        void clicked( bool );
    protected:
        void mousePressEvent( QMouseEvent* );

    private:
        int m_state;
        QPixmap m_pix[2];

    };
    class KonnectorBar : public QHBox {
        Q_OBJECT
    public:
        enum State { Connected, Disconnected };
        KonnectorBar(QWidget* parent );
        ~KonnectorBar();

        void setName( const QString& name );
        QString name()const;

        void setState( bool );
        bool state()const;
        bool isOn()const;
    signals:
        void toggled(bool );

    private:
        KonnectorLabel* m_lbl;
        KonnectorState* m_state;
    };
}


#endif
