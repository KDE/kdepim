/* -*- mode: c++; c-basic-offset:4 -*-
    utils/multivalidator.h

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2008 Klarälvdalens Datakonsult AB

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

#ifndef __KLEOPATRA_UTILS_MULTIVALIDATOR_H__
#define __KLEOPATRA_UTILS_MULTIVALIDATOR_H__

#include <QValidator>
#include <QList>

namespace Kleo {

    class MultiValidator : public QValidator {
        Q_OBJECT
    public:
        explicit MultiValidator( QObject * parent=0 )
            : QValidator( parent ) {}
        explicit MultiValidator( QValidator * validator, QObject * parent=0 )
            : QValidator( parent ) { addValidator( validator ); }
        explicit MultiValidator( const QList<QValidator*> & validators, QObject * parent=0 )
            : QValidator( parent ) { addValidators( validators ); }
        ~MultiValidator();

        void addValidator( QValidator * validator );
        void addValidators( const QList<QValidator*> & validators );

        void removeValidator( QValidator * validator );
        void removeValidators( const QList<QValidator*> & validators );

        /* reimp */ void fixup( QString & str ) const;
        /* reimp */ State validate( QString & str, int & pos ) const;

    private Q_SLOTS:
        void _kdmv_slotDestroyed( QObject * );

    private:
        QList<QValidator*> m_validators;
    };

}

#endif /* __KLEOPATRA_UTILS_MULTIVALIDATOR_H__ */
