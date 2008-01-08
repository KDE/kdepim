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

#include <QDialog>

#include <QStringList>

#include <vector>

namespace Kleo {

class ResultDisplayWidget;

class ResultDialog : public QDialog {
    Q_OBJECT
public:
    explicit ResultDialog( QWidget * parent=0 );
    ~ResultDialog();

    void setLabels( const QStringList & inputs );

    virtual ResultDisplayWidget * widget( unsigned int idx ) { return m_payloads.at( idx ); }

    void showResultWidget( unsigned int idx );
    void showError( unsigned int idx, const QString & errorString );

private:
    virtual ResultDisplayWidget * doCreatePayload( QWidget * parent ) const = 0;

private:
    std::vector<ResultDisplayWidget*> m_payloads;
};

template <typename T>
class ResultDialogImpl : public ResultDialog {
public:
    explicit ResultDialogImpl( QWidget * p=0 )
        : ResultDialog( p ) {}

    /* reimp */ T * widget( unsigned int idx ) { return static_cast<T*>( ResultDialog::widget( idx ) ); }

private:
    /* reimp */ T * doCreatePayload( QWidget * p ) const { return new T( p ); }
};

}

#endif /*__RESULTDIALOG_H__*/
