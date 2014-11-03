/* -*- mode: c++; c-basic-offset:4 -*-
    commands/importcertificatescommand_p.h

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2007, 2008 Klarälvdalens Datakonsult AB

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

#ifndef __KLEOPATRA_IMPORTCERTIFICATESCOMMAND_P_H__
#define __KLEOPATRA_IMPORTCERTIFICATESCOMMAND_P_H__

#include "command_p.h"
#include "importcertificatescommand.h"

#include <gpgme++/global.h>

#include <vector>
#include <map>

namespace GpgME
{
class ImportResult;
class Error;
}

namespace Kleo
{
class AbstractImportJob;
}

class QByteArray;

class Kleo::ImportCertificatesCommand::Private : public Command::Private
{
    friend class ::Kleo::ImportCertificatesCommand;
    Kleo::ImportCertificatesCommand *q_func() const
    {
        return static_cast<ImportCertificatesCommand *>(q);
    }
public:
    explicit Private(ImportCertificatesCommand *qq, KeyListController *c);
    ~Private();

    void setWaitForMoreJobs(bool waiting);

    void startImport(GpgME::Protocol proto, const QByteArray &data, const QString &id = QString());
    void startImport(GpgME::Protocol proto, const std::vector<GpgME::Key> &keys, const QString &id = QString());
    void importResult(const GpgME::ImportResult &);
    void importResult(const GpgME::ImportResult &, const QString &);

    void showError(QWidget *parent, const GpgME::Error &error, const QString &id = QString());
    void showError(const GpgME::Error &error, const QString &id = QString());

    void showDetails(QWidget *parent, const std::vector<GpgME::ImportResult> &results, const QStringList &ids);
    void showDetails(const std::vector<GpgME::ImportResult> &results, const QStringList &ids);

    void setImportResultProxyModel(const std::vector<GpgME::ImportResult> &results, const QStringList &ids);

private:
    void tryToFinish();

private:
    bool waitForMoreJobs;
    std::vector<GpgME::Protocol> nonWorkingProtocols;
    std::map<QObject *, QString> idsByJob;
    std::vector<Kleo::AbstractImportJob *> jobs;
    std::vector<GpgME::ImportResult> results;
    QStringList ids;
};

inline Kleo::ImportCertificatesCommand::Private *Kleo::ImportCertificatesCommand::d_func()
{
    return static_cast<Private *>(d.get());
}
inline const Kleo::ImportCertificatesCommand::Private *Kleo::ImportCertificatesCommand::d_func() const
{
    return static_cast<const Private *>(d.get());
}

inline Kleo::ImportCertificatesCommand::ImportCertificatesCommand(Private *pp) : Command(pp) {}
inline Kleo::ImportCertificatesCommand::ImportCertificatesCommand(QAbstractItemView *v, Private *pp) : Command(v, pp) {}

#endif /* __KLEOPATRA_IMPORTCERTIFICATESCOMMAND_P_H__ */

