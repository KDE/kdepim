/* -*- mode: c++; c-basic-offset:4 -*-
    ./resultdialog.h

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

#ifndef __RESULTDIALOG_H__
#define __RESULTDIALOG_H__

#include <QStackedWidget>
#include <QProgressBar>
#include <QDialog>
#include <QBoxLayout>
#include <QLabel>

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
        box->setContentsMargins( 0, 0, 0, 0 );

        m_stacks.reserve( m_count );
        m_payloads.reserve( m_count );
        for ( int i=0; i< m_count; i++ ) {
            QStackedWidget *w = new QStackedWidget( this );
            w->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
            QProgressBar * p = new QProgressBar( w );
            box->addWidget( w );
            p->setRange( 0, 0 ); // knight rider mode
            w->addWidget( p );
            T* payload = new T( w );
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
    
    void showResultWidget( int idx )
    {
        if ( m_stacks.size() <= idx || m_payloads.size() <= idx ) return;
        QStackedWidget * stack = m_stacks[idx];
        assert(stack); assert( m_payloads[idx] );
        stack->setCurrentWidget( m_payloads[idx] );
    }
    
    void showError( int idx, const QString& errorString )
    {
        showErrorWidget( idx, 0, errorString );
    }
    
    void showErrorWidget( int idx, QWidget* _errorWidget, const QString& errorString = QString() )
    {
        if ( m_stacks.size() <= idx ) return;
        QStackedWidget * stack = m_stacks[idx];
        assert(stack);
        QWidget *errorWidget = _errorWidget;
        if ( !errorWidget ) {
            errorWidget = new QLabel( this, errorString );
            errorWidget->setObjectName( "ErrorWidget" );
            errorWidget->setStyleSheet( QString::fromLatin1("QLabel#ErrorWidget { border:4px solid red; border-radius:2px; }") );
        }
        stack->addWidget( errorWidget );
        stack->setCurrentWidget( errorWidget );
    }

private:
    int m_count;
    std::vector<QStackedWidget*> m_stacks;
    std::vector<T*> m_payloads;
    
};


#endif /*__RESULTDIALOG_H__*/
