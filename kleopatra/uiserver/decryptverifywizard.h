#ifndef __KLEOPATRA_UISERVER_DECRYPTVERIFYWIZARD_H__
#define __KLEOPATRA_UISERVER_DECRYPTVERIFYWIZARD_H__

#include <QWizard>

#include <utils/pimpl_ptr.h>

namespace Kleo {

    class DecryptVerifyOperationWidget;
    class DecryptVerifyResultWidget;

    class DecryptVerifyWizard : public QWizard {
        Q_OBJECT
    public:
        explicit DecryptVerifyWizard( QWidget * parent=0, Qt::WindowFlags f=0 );
        ~DecryptVerifyWizard();

        void setOutputDirectory( const QString & dir );
        QString outputDirectory() const;

        DecryptVerifyOperationWidget * operationWidget( unsigned int idx );
        DecryptVerifyResultWidget * resultWidget( unsigned int idx );

    private:
        class Private;
        kdtools::pimpl_ptr<Private> d;
    };

}

#endif /* __KLEOPATRA_UISERVER_DECRYPTVERIFYWIZARD_H__ */
