#ifndef __KLEOPATRA_CONF_SMIMEVALIDATIONCONFIGURATIONWIDGET_H__
#define __KLEOPATRA_CONF_SMIMEVALIDATIONCONFIGURATIONWIDGET_H__

#include <QWidget>

namespace Kleo {
namespace Config {

    class SMimeValidationConfigurationWidget : public QWidget {
        Q_OBJECT
    public:
        explicit SMimeValidationConfigurationWidget( QWidget * parent=0, Qt::WindowFlags f=0 );
        ~SMimeValidationConfigurationWidget();

        void load();
        void save() const;
        void defaults();

    private:
        class Private;
        Private * d;
    };

}    
}

#endif /* __KLEOPATRA_CONF_SMIMEVALIDATIONCONFIGURATIONWIDGET_H__ */
