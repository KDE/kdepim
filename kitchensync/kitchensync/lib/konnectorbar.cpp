#include <kiconloader.h>

#include "konnectorbar.h"

using namespace KSync;

KonnectorState::KonnectorState( QWidget* parent )
    : QLabel( parent ) {
    m_state = 1; // off;
    m_pix[0] = ::SmallIcon( QString::fromLatin1("connect_established") );
    m_pix[1] = ::SmallIcon( QString::fromLatin1("connect_no") );
    setPixmap( m_pix[1] );
}
KonnectorState::~KonnectorState() {

}
void KonnectorState::setState( bool b) {
    /* on */
    if (b )
        m_state = 0;
    else
        m_state = 1;

    setPixmap( m_pix[m_state] );
}
bool KonnectorState::state()const {
    return ( m_state != 0 );
}
void KonnectorState::mousePressEvent( QMouseEvent* e ) {
    emit clicked( state() );
}

KonnectorBar::KonnectorBar( QWidget* parent )
    : QHBox( parent ) {
    m_lbl = new KonnectorLabel(this);
    m_state = new KonnectorState(this);
    connect(m_state, SIGNAL(clicked(bool) ),
            this, SIGNAL(toggled(bool) ) );
}
KonnectorBar::~KonnectorBar() {
}
void KonnectorBar::setName( const QString& name ) {
    m_lbl->setText( name );
}
QString KonnectorBar::name()const{
    return m_lbl->text();
}
void KonnectorBar::setState( bool b ) {
    m_state->setState( b );
}
bool KonnectorBar::state()const {
    return isOn();
}
bool KonnectorBar::isOn()const {
    m_state->state();
}

#include "konnectorbar.moc"
