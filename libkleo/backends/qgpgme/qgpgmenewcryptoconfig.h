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

    /* reimp */ QString name() const;
    /* reimp */ QString description() const;
    /* reimp */ QString path() const;
    /* reimp */ bool isOptional() const;
    /* reimp */ bool isReadOnly() const;
    /* reimp */ bool isList() const;
    /* reimp */ bool isRuntime() const;
    /* reimp */ Level level() const;
    /* reimp */ ArgType argType() const;
    /* reimp */ bool isSet() const;
    /* reimp */ bool boolValue() const;
    /* reimp */ QString stringValue() const;
    /* reimp */ int intValue() const;
    /* reimp */ unsigned int uintValue() const;
    /* reimp */ KUrl urlValue() const;
    /* reimp */ unsigned int numberOfTimesSet() const;
    /* reimp */ QStringList stringValueList() const;
    /* reimp */ std::vector<int> intValueList() const;
    /* reimp */ std::vector<unsigned int> uintValueList() const;
    /* reimp */ KUrl::List urlValueList() const;
    /* reimp */ void resetToDefault();
    /* reimp */ void setBoolValue(bool);
    /* reimp */ void setStringValue(const QString &);
    /* reimp */ void setIntValue(int);
    /* reimp */ void setUIntValue(unsigned int);
    /* reimp */ void setURLValue(const KUrl &);
    /* reimp */ void setNumberOfTimesSet(unsigned int);
    /* reimp */ void setStringValueList(const QStringList &);
    /* reimp */ void setIntValueList(const std::vector<int> &);
    /* reimp */ void setUIntValueList(const std::vector<unsigned int> &);
    /* reimp */ void setURLValueList(const KUrl::List &);
    /* reimp */ bool isDirty() const;

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

    /* reimp */ QString name() const;
    /* reimp */ QString iconName() const
    {
        return QString();
    }
    /* reimp */ QString description() const;
    /* reimp */ QString path() const;
    /* reimp */ Kleo::CryptoConfigEntry::Level level() const;
    /* reimp */ QStringList entryList() const;
    /* reimp */ QGpgMENewCryptoConfigEntry *entry(const QString &name) const;

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

    /* reimp */ QString name() const;
    /* reimp */ QString iconName() const
    {
        return name();
    }
    /* reimp */ QString description() const;
    /* reimp */ QStringList groupList() const;
    /* reimp */ QGpgMENewCryptoConfigGroup *group(const QString &name) const;

    /* reimp */ void sync(bool runtime);

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

    /* reimp */ QStringList componentList() const;

    /* reimp */ QGpgMENewCryptoConfigComponent *component(const QString &name) const;

    /* reimp */ void clear();
    /* reimp */ void sync(bool runtime);

private:
    /// @param showErrors if true, a messagebox will be shown if e.g. gpgconf wasn't found
    void reloadConfiguration(bool showErrors);

private:
    QHash< QString, boost::shared_ptr<QGpgMENewCryptoConfigComponent> > m_componentsByName;
    bool m_parsed;
};

#endif /* KLEO_QGPGMENEWCRYPTOCONFIG_H */
