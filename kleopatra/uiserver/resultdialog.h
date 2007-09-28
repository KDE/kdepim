#ifndef __RESULTDIALOG_H__
#define __RESULTDIALOG_H__

#include <QStackedWidget>
#include <QProgressBar>
#include <QDialog>
#include <QBoxLayout>

#include <vector>


template <typename T>
class ResultDialog : public QDialog
{
public:
    ResultDialog( QWidget* parent, int count )
    :QDialog( parent ), m_count(count)
    {
        init();
    }
    virtual ~ResultDialog() {}
    
    void init()
    {
        QVBoxLayout *box = new QVBoxLayout( this );
        m_stacks.reserve( m_count );
        m_payloads.reserve( m_count );
        for ( int i=0; i< m_count; i++ ) {
            QStackedWidget *w = new QStackedWidget( this );
            box->addWidget( w );
            QProgressBar * p = new QProgressBar( this );
            w->addWidget( p );
            T* payload = new T( this );
            w->addWidget( payload );
            w->setCurrentIndex( 0 );
            m_stacks.push_back( w );
            m_payloads.push_back( payload );
        }
    }
    
    T* widget( int idx )
    {
        if ( m_payloads.size() <= idx ) return 0;
        return m_payloads[ idx ];
    }
    
    void toggle( int idx )
    {
        if ( m_stacks.size() <= idx ) return;
        QStackedWidget * stack = m_stacks[idx];
        assert(stack);
        stack->setCurrentIndex( stack->currentIndex() + 1 % stack->count() );
    }

private:
    int m_count;
    std::vector<QStackedWidget*> m_stacks;
    std::vector<T*> m_payloads;
    
};


#endif /*__RESULTDIALOG_H__*/
