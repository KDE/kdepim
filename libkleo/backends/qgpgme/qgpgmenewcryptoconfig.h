/*
    qgpgmenewcryptoconfig.h

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

#ifndef KLEO_QGPGMENEWCRYPTOCONFIG_H
#define KLEO_QGPGMENEWCRYPTOCONFIG_H

#include "kleo_export.h"
#include "kleo/cryptoconfig.h"

#include <QHash>
#include <QStringList>
#include <QVariant>

#include <gpgme++/configuration.h>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <vector>
#include <utility>

class QGpgMENewCryptoConfig;
class QGpgMENewCryptoConfigComponent;
class QGpgMENewCryptoConfigGroup;
class QGpgMENewCryptoConfigEntry;

class QGpgMENewCryptoConfigEntry : public Kleo::CryptoConfigEntry
{
public:
    QGpgMENewCryptoConfigEntry(const boost::shared_ptr<QGpgMENewCryptoConfigGroup> &group, const GpgME::Configuration::Option &option);
    ~QGpgMENewCryptoConfigEntry();

    QString name() const Q_DECL_OVERRIDE;
    QString description() const Q_DECL_OVERRIDE;
    QString path() const Q_DECL_OVERRIDE;
    bool isOptional() const Q_DECL_OVERRIDE;
    bool isReadOnly() const Q_DECL_OVERRIDE;
    bool isList() const Q_DECL_OVERRIDE;
    bool isRuntime() const Q_DECL_OVERRIDE;
    Level level() const Q_DECL_OVERRIDE;
    ArgType argType() const Q_DECL_OVERRIDE;
    bool isSet() const Q_DECL_OVERRIDE;
    bool boolValue() const Q_DECL_OVERRIDE;
    QString stringValue() const Q_DECL_OVERRIDE;
    int intValue() const Q_DECL_OVERRIDE;
    unsigned int uintValue() const Q_DECL_OVERRIDE;
    KUrl urlValue() const Q_DECL_OVERRIDE;
    unsigned int numberOfTimesSet() const Q_DECL_OVERRIDE;
    QStringList stringValueList() const Q_DECL_OVERRIDE;
    std::vector<int> intValueList() const Q_DECL_OVERRIDE;
    std::vector<unsigned int> uintValueList() const Q_DECL_OVERRIDE;
    KUrl::List urlValueList() const Q_DECL_OVERRIDE;
    void resetToDefault() Q_DECL_OVERRIDE;
    void setBoolValue(bool) Q_DECL_OVERRIDE;
    void setStringValue(const QString &) Q_DECL_OVERRIDE;
    void setIntValue(int) Q_DECL_OVERRIDE;
    void setUIntValue(unsigned int) Q_DECL_OVERRIDE;
    void setURLValue(const KUrl &) Q_DECL_OVERRIDE;
    void setNumberOfTimesSet(unsigned int) Q_DECL_OVERRIDE;
    void setStringValueList(const QStringList &) Q_DECL_OVERRIDE;
    void setIntValueList(const std::vector<int> &) Q_DECL_OVERRIDE;
    void setUIntValueList(const std::vector<unsigned int> &) Q_DECL_OVERRIDE;
    void setURLValueList(const KUrl::List &) Q_DECL_OVERRIDE;
    bool isDirty() const Q_DECL_OVERRIDE;

#if 0
    void setDirty(bool b);
    QString outputString() const;

protected:
    bool isStringType() const;
    QVariant stringToValue(const QString &value, bool unescape) const;
    QString toString(bool escape) const;
#endif
private:
    boost::weak_ptr<QGpgMENewCryptoConfigGroup> m_group;
    GpgME::Configuration::Option m_option;
};

class QGpgMENewCryptoConfigGroup : public Kleo::CryptoConfigGroup
{
public:
    QGpgMENewCryptoConfigGroup(const boost::shared_ptr<QGpgMENewCryptoConfigComponent> &parent, const GpgME::Configuration::Option &option);
    ~QGpgMENewCryptoConfigGroup();

    QString name() const Q_DECL_OVERRIDE;
    QString iconName() const Q_DECL_OVERRIDE
    {
        return QString();
    }
    QString description() const Q_DECL_OVERRIDE;
    QString path() const Q_DECL_OVERRIDE;
    Kleo::CryptoConfigEntry::Level level() const Q_DECL_OVERRIDE;
    QStringList entryList() const Q_DECL_OVERRIDE;
    QGpgMENewCryptoConfigEntry *entry(const QString &name) const Q_DECL_OVERRIDE;

private:
    friend class QGpgMENewCryptoConfigComponent; // it adds the entries
    boost::weak_ptr<QGpgMENewCryptoConfigComponent> m_component;
    GpgME::Configuration::Option m_option;
    QStringList m_entryNames;
    QHash< QString, boost::shared_ptr<QGpgMENewCryptoConfigEntry> > m_entriesByName;
};

/// For docu, see kleo/cryptoconfig.h
class QGpgMENewCryptoConfigComponent : public Kleo::CryptoConfigComponent, public boost::enable_shared_from_this<QGpgMENewCryptoConfigComponent>
{
public:
    QGpgMENewCryptoConfigComponent();
    ~QGpgMENewCryptoConfigComponent();

    void setComponent(const GpgME::Configuration::Component &component);

    QString name() const Q_DECL_OVERRIDE;
    QString iconName() const Q_DECL_OVERRIDE
    {
        return name();
    }
    QString description() const Q_DECL_OVERRIDE;
    QStringList groupList() const Q_DECL_OVERRIDE;
    QGpgMENewCryptoConfigGroup *group(const QString &name) const Q_DECL_OVERRIDE;

    void sync(bool runtime);

private:
    GpgME::Configuration::Component m_component;
    QHash< QString, boost::shared_ptr<QGpgMENewCryptoConfigGroup> > m_groupsByName;
};

/**
 * CryptoConfig implementation around the gpgconf command-line tool
 * For method docu, see kleo/cryptoconfig.h
 */
class KLEO_EXPORT QGpgMENewCryptoConfig : public Kleo::CryptoConfig
{
public:
    /**
     * Constructor
     */
    QGpgMENewCryptoConfig();
    ~QGpgMENewCryptoConfig();

    QStringList componentList() const Q_DECL_OVERRIDE;

    QGpgMENewCryptoConfigComponent *component(const QString &name) const Q_DECL_OVERRIDE;

    void clear() Q_DECL_OVERRIDE;
    void sync(bool runtime) Q_DECL_OVERRIDE;

private:
    /// @param showErrors if true, a messagebox will be shown if e.g. gpgconf wasn't found
    void reloadConfiguration(bool showErrors);

private:
    QHash< QString, boost::shared_ptr<QGpgMENewCryptoConfigComponent> > m_componentsByName;
    bool m_parsed;
};

#endif /* KLEO_QGPGMENEWCRYPTOCONFIG_H */
