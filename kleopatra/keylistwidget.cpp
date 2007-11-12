#include "keylistwidget.h"
#include "models/keylistmodel.h"

#include <QGridLayout>
#include <QTreeView>

class KeyListWidget::Private {
    friend class ::KeyListWidget;
    KeyListWidget * const q;
public:
    explicit Private( KeyListWidget * qq );
    ~Private();
    
private:
    QTreeView * m_view;
};


KeyListWidget::Private::Private( KeyListWidget * qq )
  : q( qq )
{
    QGridLayout * const layout = new QGridLayout( q );
    layout->setMargin( 0 );
    m_view = new QTreeView;
    m_view->setRootIsDecorated( false );
    m_view->setSortingEnabled( true );
    m_view->sortByColumn( Kleo::AbstractKeyListModel::Fingerprint, Qt::AscendingOrder );
    layout->addWidget( m_view, 0, 0 );
}

KeyListWidget::Private::~Private() {}



KeyListWidget::KeyListWidget( QWidget * parent, Qt::WFlags f )
  : QWidget( parent, f ), d( new Private( this ) )
{
    
}

KeyListWidget::~KeyListWidget() {}
