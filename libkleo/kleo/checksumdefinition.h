/* -*- mode: c++; c-basic-offset:4 -*-
    checksumdefinition.h

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2010 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    Libkleopatra is distributed in the hope that it will be useful,
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

#ifndef __KLEO_CHECKSUMDEFINITION_H__
#define __KLEO_CHECKSUMDEFINITION_H__

#include "kleo_export.h"

#include <QtCore/QString>
#include <QtCore/QStringList>

#include <vector>

class QProcess;

namespace boost
{
template <typename T> class shared_ptr;
}

namespace Kleo
{

class KLEO_EXPORT ChecksumDefinition
{
protected:
    ChecksumDefinition(const QString &id, const QString &label, const QString &outputFileName, const QStringList &extensions);
public:
    virtual ~ChecksumDefinition();

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

    const QStringList &patterns() const
    {
        return m_patterns;
    }
    QString outputFileName() const
    {
        return m_outputFileName;
    }

    QString createCommand() const;
    ArgumentPassingMethod createCommandArgumentPassingMethod() const
    {
        return m_createMethod;
    }

    QString verifyCommand() const;
    ArgumentPassingMethod verifyCommandArgumentPassingMethod() const
    {
        return m_verifyMethod;
    }

    bool startCreateCommand(QProcess *process, const QStringList &files) const;
    bool startVerifyCommand(QProcess *process, const QStringList &files) const;

    static QString installPath();
    static void setInstallPath(const QString &ip);

    static std::vector< boost::shared_ptr<ChecksumDefinition> > getChecksumDefinitions();
    static std::vector< boost::shared_ptr<ChecksumDefinition> > getChecksumDefinitions(QStringList &errors);

    static boost::shared_ptr<ChecksumDefinition> getDefaultChecksumDefinition(const std::vector< boost::shared_ptr<ChecksumDefinition> > &available);
    static void setDefaultChecksumDefinition(const boost::shared_ptr<ChecksumDefinition> &checksumDefinition);

protected:
    void setCreateCommandArgumentPassingMethod(ArgumentPassingMethod method)
    {
        m_createMethod = method;
    }
    void setVerifyCommandArgumentPassingMethod(ArgumentPassingMethod method)
    {
        m_verifyMethod = method;
    }

private:
    virtual QString doGetCreateCommand() const = 0;
    virtual QString doGetVerifyCommand() const = 0;
    virtual QStringList doGetCreateArguments(const QStringList &files) const = 0;
    virtual QStringList doGetVerifyArguments(const QStringList &files) const = 0;
private:
    const QString m_id;
    const QString m_label;
    const QString m_outputFileName;
    const QStringList m_patterns;
    ArgumentPassingMethod m_createMethod, m_verifyMethod;
};

}

#endif /* __KLEO_CHECKSUMDEFINITION_H__ */

