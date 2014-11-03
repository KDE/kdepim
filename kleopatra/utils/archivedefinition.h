/* -*- mode: c++; c-basic-offset:4 -*-
    utils/archivedefinition.h

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2009 Klarälvdalens Datakonsult AB

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

#ifndef __KLEOPATRA_UTILS_ARCHIVEDEFINITION_H__
#define __KLEOPATRA_UTILS_ARCHIVEDEFINITION_H__

#include <QString>
#include <QStringList>
#include <QMetaType>

#include <gpgme++/global.h> // GpgME::Protocol

#include <vector>

class QDir;

namespace boost
{
template <typename T> class shared_ptr;
}

namespace Kleo
{
class Input;
class Output;
}

namespace Kleo
{

class ArchiveDefinition
{
protected:
    ArchiveDefinition(const QString &id, const QString &label);
public:
    virtual ~ArchiveDefinition();

    enum ArgumentPassingMethod {
        CommandLine,
        NewlineSeparatedInputFile,
        NullSeparatedInputFile,

        NumArgumentPassingMethods
    };

    QString id() const
    {
        return m_id;
    }
    QString label() const
    {
        return m_label;
    }

    const QStringList &extensions(GpgME::Protocol p) const
    {
        checkProtocol(p);
        return m_extensions[p];
    }

    boost::shared_ptr<Input> createInputFromPackCommand(GpgME::Protocol p, const QStringList &files) const;
    ArgumentPassingMethod packCommandArgumentPassingMethod(GpgME::Protocol p) const
    {
        checkProtocol(p);
        return m_packCommandMethod[p];
    }

    boost::shared_ptr<Output> createOutputFromUnpackCommand(GpgME::Protocol p, const QString &file, const QDir &wd) const;
    // unpack-command must use CommandLine ArgumentPassingMethod

    static QString installPath();
    static void setInstallPath(const QString &ip);

    static std::vector< boost::shared_ptr<ArchiveDefinition> > getArchiveDefinitions();
    static std::vector< boost::shared_ptr<ArchiveDefinition> > getArchiveDefinitions(QStringList &errors);

protected:
    void setPackCommandArgumentPassingMethod(GpgME::Protocol p, ArgumentPassingMethod method)
    {
        checkProtocol(p);
        m_packCommandMethod[p] = method;
    }
    void setExtensions(GpgME::Protocol p, const QStringList &extensions)
    {
        checkProtocol(p);
        m_extensions[p] = extensions;
    }

    void checkProtocol(GpgME::Protocol p) const;

private:
    virtual QString doGetPackCommand(GpgME::Protocol p) const = 0;
    virtual QString doGetUnpackCommand(GpgME::Protocol p) const = 0;
    virtual QStringList doGetPackArguments(GpgME::Protocol p, const QStringList &files) const = 0;
    virtual QStringList doGetUnpackArguments(GpgME::Protocol p, const QString &file) const = 0;
private:
    const QString m_id;
    const QString m_label;
    /*const*/ QStringList m_extensions[2];
    ArgumentPassingMethod m_packCommandMethod[2];
    // m_unpackCommandMethod[2] <- must always be CommandLine
};

}

Q_DECLARE_METATYPE(boost::shared_ptr<Kleo::ArchiveDefinition>)

#endif /* __KLEOPATRA_UTILS_ARCHIVEDEFINITION_H__ */

