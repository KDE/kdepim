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
