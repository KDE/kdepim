#include "objectspage.h"

using namespace Kleo;

class ObjectsPage::Private {
    friend class ::ObjectsPage;
    ObjectsPage * const q;
public:
    explicit Private( ObjectsPage * qq );
    ~Private();
    
private:

};


ObjectsPage::Private::Private( ObjectsPage * qq )
  : q( qq )
{
    
}

ObjectsPage::Private::~Private() {}



ObjectsPage::ObjectsPage( QWidget * parent, Qt::WFlags f )
  : WizardPage( parent, f ), d( new Private( this ) )
{
    
}

ObjectsPage::~ObjectsPage() {}


bool ObjectsPage::isComplete() const
{
    return true;
    // TODO
}

