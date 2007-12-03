#ifndef __KLEOPATRA_OBJECTSPAGE_H__
#define __KLEOPATRA_OBJECTSPAGE_H__

#include "wizardpage.h"
    
#include <utils/pimpl_ptr.h>

namespace Kleo {
 
    class ObjectsPage : public WizardPage {
        Q_OBJECT
            public:
        explicit ObjectsPage( QWidget * parent=0, Qt::WFlags f=0 );
        ~ObjectsPage();
        
        bool isComplete() const;

    private:
        class Private;
        kdtools::pimpl_ptr<Private> d;
    };
}

#endif // __KLEOPATRA_OBJECTSPAGE_H__
 
