#ifndef VR3_CONFIG_DIALOG_H
#define VR3_CONFIG_DIALOG_H

#include "../../configwidget.h" // sorry

#include <qlayout.h>

class QLabel;
class QComboBox;
class QLineEdit;

namespace Vr3 {
    class ConfigWidget : public KSync::ConfigWidget {
        Q_OBJECT
    public:
        /** c'tor with a already existing kapability */
        ConfigWidget( const KSync::Kapabilities& cap, QWidget* parent, const char* name );

        /** no configuration before */
        ConfigWidget( QWidget* parent, const char* name );
        ~ConfigWidget();

        /** return the configured dialog */
        KSync::Kapabilities capabilities()const;

        /** set the capabilities object */
        void setCapabilities( const KSync::Kapabilities& );


    private:
        void initUI();
        QString name()const;
        QGridLayout* m_lay;
        QLabel* m_lblIP;
        QComboBox* m_cmbIP;
        QLabel* m_lblName;
        QLineEdit* m_lneName;
    };
}

#endif
