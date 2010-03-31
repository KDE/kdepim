/* -*- mode: c++; c-basic-offset:4 -*-
    utils/checksumdefinition.h

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2010 Klarälvdalens Datakonsult AB

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

#ifndef __KLEOPATRA_UTILS_CHECKSUMDEFINITION_H__
#define __KLEOPATRA_UTILS_CHECKSUMDEFINITION_H__

#include <QString>
#include <QStringList>

#include <vector>

namespace boost {
    template <typename T> class shared_ptr;
}

namespace Kleo {

    class ChecksumDefinition {
    protected:
        ChecksumDefinition( const QString & id, const QString & label, const QString & outputFileName, const QStringList & extensions );
    public:
        virtual ~ChecksumDefinition();

        QString id() const { return m_id; }
        QString label() const { return m_label; }

        const QStringList & patterns() const { return m_patterns; }
        QString outputFileName() const { return m_outputFileName; }

        QString createCommand() const;
        QStringList createCommandArguments( const QStringList & files ) const;

        QString verifyCommand() const;
        QStringList verifyCommandArguments( const QStringList & files ) const;

        static std::vector< boost::shared_ptr<ChecksumDefinition> > getChecksumDefinitions();
        static std::vector< boost::shared_ptr<ChecksumDefinition> > getChecksumDefinitions( QStringList & errors );

    private:
        virtual QString doGetCreateCommand() const = 0;
        virtual QString doGetVerifyCommand() const = 0;
        virtual QStringList doGetCreateArguments( const QStringList & files ) const = 0;
        virtual QStringList doGetVerifyArguments( const QStringList & files ) const = 0;
    private:
        const QString m_id;
        const QString m_label;
        const QString m_outputFileName;
        const QStringList m_patterns;
    };
        
}

#endif /* __KLEOPATRA_UTILS_CHECKSUMDEFINITION_H__ */

