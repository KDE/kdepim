#include "searchbarstatehandler.h"
#include "searchbar.h"
#include "tabwidget.h"

#include <QAbstractItemView>
#include <QMap>
#include <QPointer>

class SearchBarStateHandler::Private {
    friend class ::SearchBarStateHandler;
    SearchBarStateHandler * const q;
public:
    explicit Private( TabWidget* tabWidget, SearchBar* bar, SearchBarStateHandler * qq );
    ~Private();

    void currentViewChanged( QAbstractItemView* view );
    void viewDestroyed( QObject* obj )
;
private:
    TabWidget* m_tabWidget;
    SearchBar* m_searchBar;
    QMap<QObject*, boost::shared_ptr<SearchBar::State> > m_viewToState;
    QPointer<QAbstractItemView> m_currentView;
};


SearchBarStateHandler::Private::Private( TabWidget* tab, SearchBar* bar, SearchBarStateHandler * qq )
    : q( qq ), m_tabWidget( tab ), m_searchBar( bar )
{
    assert( m_tabWidget );
    assert( m_searchBar );
    connect( m_tabWidget, SIGNAL( currentViewChanged( QAbstractItemView* ) ), 
             q, SLOT( currentViewChanged( QAbstractItemView* ) ) );  
}

SearchBarStateHandler::Private::~Private() {}

void SearchBarStateHandler::Private::currentViewChanged( QAbstractItemView* view )
{
    assert( view ); // correct?

    if ( view == m_currentView )
        return;

    if ( m_currentView )
        m_viewToState[m_currentView] = m_searchBar->state();
    m_currentView = view;

    //connect exactly once
    disconnect( m_currentView, SIGNAL( destroyed( QObject* ) ),
                q, SLOT( viewDestroyed( QObject* ) ) );
    connect( m_currentView, SIGNAL( destroyed( QObject* ) ),
                q, SLOT( viewDestroyed( QObject* ) ) );

    if ( m_viewToState.contains( m_currentView ) )
        m_searchBar->setState( m_viewToState[m_currentView] );
    else
        m_searchBar->resetState();
}

void SearchBarStateHandler::Private::viewDestroyed( QObject* obj )
{
    assert( obj );
    m_viewToState.remove( obj );
    if ( m_currentView == obj )
        m_currentView = 0;
}

SearchBarStateHandler::SearchBarStateHandler( TabWidget * tabWidget, SearchBar * searchBar, QObject* parent )
    : QObject( parent ), d( new Private( tabWidget, searchBar, this ) )
{
    
}

SearchBarStateHandler::~SearchBarStateHandler() {}

#include "searchbarstatehandler.moc"
