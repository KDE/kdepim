#ifndef KSYNC_QTOPIA_CONFIG_H
#define KSYNC_QTOPIA_CONFIG_H

#include <qlayout.h>
#include <qlineedit.h>

#include <configwidget.h>

class QComboBox;
class QLabel;

namespace OpieHelper {

    class QtopiaConfig : public KSync::ConfigWidget {
        Q_OBJECT
    public:
        QtopiaConfig( const KSync::Kapabilities& cap, QWidget* parent, const char* name );
        QtopiaConfig( QWidget* parent, const char* name );
        ~QtopiaConfig();

        KSync::Kapabilities capabilities()const;
        void setCapabilities( const KSync::Kapabilities& caps );
    private:
        void initUI();
        QString name()const;
        QGridLayout* m_layout;
        QLabel *m_lblUser, *m_lblPass, *m_lblName, *m_lblIP;
        QLabel* m_lblDev;
        QComboBox* m_cmbUser, *m_cmbPass, *m_cmbIP, *m_cmbDev;
        QLineEdit *m_name;
    private slots:
        void slotTextChanged( const QString& );

    };
};


#endif
