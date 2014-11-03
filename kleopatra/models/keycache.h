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

#include <gpgme++/global.h>

#include <boost/shared_ptr.hpp>

#include <string>
#include <vector>

namespace GpgME
{
class Key;
class DecryptionResult;
class VerificationResult;
class KeyListResult;
class Subkey;
}

namespace KMime
{
namespace Types
{
class Mailbox;
}
}

namespace Kleo
{

class FileSystemWatcher;

class KeyCache : public QObject
{
    Q_OBJECT
protected:
    explicit KeyCache();
public:
    static boost::shared_ptr<const KeyCache> instance();
    static boost::shared_ptr<KeyCache> mutableInstance();

    ~KeyCache();

    void insert(const GpgME::Key &key);
    void insert(const std::vector<GpgME::Key> &keys);

    void refresh(const std::vector<GpgME::Key> &keys);

    void remove(const GpgME::Key &key);
    void remove(const std::vector<GpgME::Key> &keys);

    void addFileSystemWatcher(const boost::shared_ptr<FileSystemWatcher> &watcher);

    void enableFileSystemWatcher(bool enable);

    const std::vector<GpgME::Key> &keys() const;
    std::vector<GpgME::Key> secretKeys() const;

    const GpgME::Key &findByFingerprint(const char *fpr) const;
    const GpgME::Key &findByFingerprint(const std::string &fpr) const
    {
        return findByFingerprint(fpr.c_str());
    }

    std::vector<GpgME::Key> findByFingerprint(const std::vector<std::string> &fprs) const;

    std::vector<GpgME::Key> findByEMailAddress(const char *email) const;
    std::vector<GpgME::Key> findByEMailAddress(const std::string &email) const;

    const GpgME::Key &findByShortKeyID(const char *id) const;
    const GpgME::Key &findByShortKeyID(const std::string &id) const
    {
        return findByShortKeyID(id.c_str());
    }

    const GpgME::Key &findByKeyIDOrFingerprint(const char *id) const;
    const GpgME::Key &findByKeyIDOrFingerprint(const std::string &id) const
    {
        return findByKeyIDOrFingerprint(id.c_str());
    }
    std::vector<GpgME::Key> findByKeyIDOrFingerprint(const std::vector<std::string> &ids) const;

    std::vector<GpgME::Subkey> findSubkeysByKeyID(const std::vector<std::string> &ids) const;

    std::vector<GpgME::Key> findRecipients(const GpgME::DecryptionResult &result) const;
    std::vector<GpgME::Key> findSigners(const GpgME::VerificationResult &result) const;

    std::vector<GpgME::Key> findSigningKeysByMailbox(const KMime::Types::Mailbox &mb) const;
    std::vector<GpgME::Key> findEncryptionKeysByMailbox(const KMime::Types::Mailbox &mb) const;

    enum Option {
        NoOption = 0,
        RecursiveSearch = 1,
        IncludeSubject = 2
    };
    Q_DECLARE_FLAGS(Options, Option)

    std::vector<GpgME::Key> findSubjects(const GpgME::Key &key, Options option = RecursiveSearch) const;
    std::vector<GpgME::Key> findSubjects(const std::vector<GpgME::Key> &keys, Options options = RecursiveSearch) const;
    std::vector<GpgME::Key> findSubjects(std::vector<GpgME::Key>::const_iterator first, std::vector<GpgME::Key>::const_iterator last, Options options = RecursiveSearch) const;

    std::vector<GpgME::Key> findIssuers(const GpgME::Key &key, Options options = RecursiveSearch) const;
    std::vector<GpgME::Key> findIssuers(const std::vector<GpgME::Key> &keys, Options options = RecursiveSearch) const;
    std::vector<GpgME::Key> findIssuers(std::vector<GpgME::Key>::const_iterator first, std::vector<GpgME::Key>::const_iterator last, Options options = RecursiveSearch) const;

public Q_SLOTS:
    void clear();
    void startKeyListing(GpgME::Protocol proto = GpgME::UnknownProtocol)
    {
        reload(proto);
    }
    void reload(GpgME::Protocol proto = GpgME::UnknownProtocol);
    void cancelKeyListing();

Q_SIGNALS:
    //void changed( const GpgME::Key & key );
    void aboutToRemove(const GpgME::Key &key);
    void added(const GpgME::Key &key);
    void keyListingDone(const GpgME::KeyListResult &result);
    void keysMayHaveChanged();

private:
    class RefreshKeysJob;

    class Private;
    kdtools::pimpl_ptr<Private> d;
    Q_PRIVATE_SLOT(d, void refreshJobDone(GpgME::KeyListResult))
};

}

Q_DECLARE_OPERATORS_FOR_FLAGS(Kleo::KeyCache::Options)

#endif /* __KLEOPATRA_MODELS_KEYCACHE_H__ */
