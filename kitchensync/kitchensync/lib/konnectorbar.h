#ifndef KSYNC_KONNECTOR_STATUS_BAR_H
#define KSYNC_KONNECTOR_STATUS_BAR_H

#include <qhbox.h>
#include <qlabel.h>
#include <qpixmap.h>

namespace KSync {
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

    /**
     * The konnector bar is meant to represent
     * the connection state of a Konnector and
     * allow toggling connection/disconnection of a konnector
     * Either it is connected or not
     * @version 0.1
     * @author Holger 'zecke' Freyther <freyther@kde.org>
     */
    class KonnectorBar : public QHBox {
        Q_OBJECT
    public:
        enum State { Connected, Disconnected };

        /**
         * The Constructor to construct a KonnectorBox
         * @param parent The parent widget
         * @short Constructor
         */
        KonnectorBar(QWidget* parent );

        /**
         * d'tor
         */
        ~KonnectorBar();

        /**
         * Set the name of the Konnector. The string
         * will be displayed
         * @param name The name of the Konnector
         */
        void setName( const QString& name );

        /**
         * Return the name of the current Konnector
         * @return the name of the konnector
         */
        QString name()const;

        /**
         * Set the state of the connection
         * @param b The state true if connected, false if not connected
         */
        void setState( bool b);

        /**
         * @return the state
         */
        bool state()const;

        /**
         * @return if is connected
         */
        bool isOn()const;
    signals:
        /**
         * if the KonnectarBar got toggled
         * @param b the state of the connection
         */
        void toggled(bool b);

    private:
        KonnectorLabel* m_lbl;
        KonnectorState* m_state;
    };
}


#endif
