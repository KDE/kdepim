/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Holger Freyther <zecke@handhelds.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

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
void KonnectorState::mousePressEvent( QMouseEvent* ) {
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
    return m_state->state();
}

#include "konnectorbar.moc"
