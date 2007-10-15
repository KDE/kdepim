/* -*- mode: c++; c-basic-offset:4 -*-
    models/keycache.h

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

#ifndef __KLEOPATRA_MODELS_KEYCACHE_H__
#define __KLEOPATRA_MODELS_KEYCACHE_H__

#include <QObject>

#include <utils/pimpl_ptr.h>

#include <boost/shared_ptr.hpp>

#include <string>
#include <vector>

namespace GpgME {
    class Key;
    class DecryptionResult;
    class VerificationResult;
}

namespace Kleo {

    class KeyCache : public QObject {
        Q_OBJECT
    private:
        explicit KeyCache( QObject * parent=0 );
    public:
        ~KeyCache();

        static boost::shared_ptr<KeyCache> make();

        static boost::shared_ptr<const KeyCache> instance();
        static boost::shared_ptr<KeyCache> mutableInstance();

        void insert( const GpgME::Key & key );
        void insert( const std::vector<GpgME::Key> & key );

        void remove( const GpgME::Key & key );

        const GpgME::Key & findByFingerprint( const char * fpr ) const;
        const GpgME::Key & findByFingerprint( const std::string & fpr ) const {
            return findByFingerprint( fpr.c_str() );
        }
        std::vector<GpgME::Key> findByFingerprint( const std::vector<std::string> & fprs ) const;

        const GpgME::Key & findByEMailAddress( const char * email ) const;
        const GpgME::Key & findByEMailAddress( const std::string & email ) const {
            return findByEMailAddress( email.c_str() );
        }

        const GpgME::Key & findByShortKeyID( const char * id ) const;
        const GpgME::Key & findByShortKeyID( const std::string & id ) const {
            return findByShortKeyID( id.c_str() );
        }

        const GpgME::Key & findByKeyIDOrFingerprint( const char * id ) const;
        const GpgME::Key & findByKeyIDOrFingerprint( const std::string & id ) const {
            return findByKeyIDOrFingerprint( id.c_str() );
        }
        std::vector<GpgME::Key> findByKeyIDOrFingerprint( const std::vector<std::string> & ids ) const;

        std::vector<GpgME::Key> findRecipients( const GpgME::DecryptionResult & result ) const;
        std::vector<GpgME::Key> findSigners( const GpgME::VerificationResult & result ) const;

    public Q_SLOTS:
        void clear();

#if 0
    Q_SIGNALS:
        void changed( const GpgME::Key & key );
        void aboutToRemove( const GpgME::Key & key );
        void added( const GpgME::Key & key );
#endif

    private:
        class Private;
        kdtools::pimpl_ptr<Private> d;
    };

}

#endif /* __KLEOPATRA_MODELS_KEYCACHE_H__ */
