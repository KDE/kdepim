#include "resultdialog.h"

#include <QStackedWidget>
#include <QProgressBar>
#include <QDialog>
#include <QBoxLayout>
#include <QLabel>
#include <QFrame>

#include <vector>
#include <cassert>

class ProgressWidget : public QFrame {
    Q_OBJECT
public:
    explicit ProgressWidget( const QString& label, QWidget * p=0 )
        : QFrame( p )
    {
        QVBoxLayout *vbox = new QVBoxLayout( this );
        vbox->addWidget( new QLabel( label ) );
        QProgressBar *pb = new QProgressBar( this );
        vbox->addWidget( pb );
        pb->setRange( 0, 0 ); // knight rider mode
    }
};


ResultDialog::ResultDialog( const QStringList & inputs, QWidget * p )
    : QDialog( p ), m_inputs( inputs )
{

}

ResultDialog::~ResultDialog() {}
    
void ResultDialog::init() {
    QVBoxLayout *box = new QVBoxLayout( this );
    box->setMargin( 0 );

    m_stacks.reserve( m_inputs.size() );
    m_payloads.reserve( m_inputs.size() );
    Q_FOREACH( QString i, m_inputs ) {
        QStackedWidget *stack = new QStackedWidget( this );
        box->addWidget( stack );
        stack->setContentsMargins( 0, 0, 0, 0 );
        ProgressWidget * p = new ProgressWidget( i, stack );
        stack->addWidget( p );
        QWidget * payload = doCreatePayload( stack );
        stack->addWidget( payload );
        stack->setCurrentIndex( 0 );
        m_stacks.push_back( stack );
        m_payloads.push_back( payload );
    }
}
    
void ResultDialog::showResultWidget( unsigned int idx ) {
    if ( m_stacks.size() <= idx || m_payloads.size() <= idx ) return;
    QStackedWidget * stack = m_stacks[idx];
    assert(stack); assert( m_payloads[idx] );
    stack->setCurrentWidget( m_payloads[idx] );
}
    
void ResultDialog::showErrorWidget( unsigned int idx, QWidget * errorWidget, const QString& errorString ) {
    if ( m_stacks.size() <= idx ) return;
    QStackedWidget * stack = m_stacks[idx];
    assert(stack);
    if ( !errorWidget ) {
        errorWidget = new QLabel( errorString, this );
        errorWidget->setObjectName( "ErrorWidget" );
        errorWidget->setStyleSheet( QString::fromLatin1("QLabel#ErrorWidget { border:4px solid red; border-radius:2px; }") );
    }
    stack->addWidget( errorWidget );
    stack->setCurrentWidget( errorWidget );
}

#include "moc_resultdialog.cpp"
#include "resultdialog.moc"
