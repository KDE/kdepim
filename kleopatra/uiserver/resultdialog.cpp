/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/resultdialog.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2007 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include "resultdialog.h"

#include "resultdisplaywidget.h"

#include <QLayout>

#include <cassert>

using namespace Kleo;

ResultDialog::ResultDialog( QWidget * p )
    : QDialog( p )
{
    QVBoxLayout * box = new QVBoxLayout( this );
    box->setMargin( 0 );
}

ResultDialog::~ResultDialog() {}
    
void ResultDialog::setLabels( const QStringList & inputs ) {

    qDeleteAll( m_payloads );
    m_payloads.clear();
    m_payloads.reserve( inputs.size() );

    assert( qobject_cast<QVBoxLayout*>( layout() ) );
    QVBoxLayout & vlay = *static_cast<QVBoxLayout*>( layout() );

    Q_FOREACH( QString i, inputs )
        if ( ResultDisplayWidget * pl = doCreatePayload( this ) ) {
            vlay.addWidget( pl );
            m_payloads.push_back( pl );
        }

    if ( isVisible() )
        Q_FOREACH( QWidget * w, m_payloads )
            w->show();
}
    
void ResultDialog::showResultWidget( unsigned int idx ) {
    m_payloads.at( idx )->showResultWidget();
}
    
void ResultDialog::showError( unsigned int idx, const QString & errorString ) {
    m_payloads.at( idx )->setError( errorString );
}

#include "moc_resultdialog.cpp"
