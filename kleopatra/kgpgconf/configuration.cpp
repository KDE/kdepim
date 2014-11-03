/* -*- mode: c++; c-basic-offset:4 -*-
    configuration.cpp

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

#include "configuration.h"

#include <QDebug>
#include <KLocalizedString>

#include <QStringList>

#include <cassert>

namespace
{

static QString gpgconf_unescape(const QString &str)
{
    // Looks like it's the same rules as QUrl.
    return QUrl::fromPercentEncoding(str.toUtf8());
}

static QString gpgconf_escape(const QString &str)
{
    // Escape special chars (including ':' and '%')
    QString enc = QLatin1String(QUrl::toPercentEncoding(str));   // and convert to utf8 first (to get %12%34 for one special char)
    // Also encode commas, for lists.
    enc.replace(QLatin1Char(','), QLatin1String("%2c"));
    return enc;
}

static QString urlpart_encode(const QString &str)
{
    QString enc(str);
    enc.replace(QLatin1Char('%'), QLatin1String("%25"));   // first!
    enc.replace(QLatin1Char(':'), QLatin1String("%3a"));
    //qDebug() <<"  urlpart_encode:" << str <<" ->" << enc;
    return enc;
}

static QString urlpart_decode(const QString &str)
{
    return QUrl::fromPercentEncoding(str.toLatin1());
}

static QUrl parseUrl(ConfigEntry::ArgType argType, const QString &str)
{
    if (argType == ConfigEntry::LdapUrl) {
        // The format is HOSTNAME:PORT:USERNAME:PASSWORD:BASE_DN
        QStringList items = str.split(QLatin1Char(':'));
        if (items.count() == 5) {
            QStringList::const_iterator it = items.constBegin();
            QUrl url;
            url.setScheme(QLatin1String("ldap"));
            url.setHost(urlpart_decode(*it++));
            url.setPort((*it++).toInt());
            url.setPath(QLatin1String("/"));   // workaround QUrl parsing bug
            url.setUserName(urlpart_decode(*it++));
            url.setPassword(urlpart_decode(*it++));
            url.setQuery(urlpart_decode(*it));
            return url;
        } else {
            qWarning() << "parseURL: malformed LDAP server:" << str;
        }
    }
    // other URLs : assume wellformed URL syntax.
    return QUrl(str);
}

// The opposite of parseURL
static QString splitUrl(ConfigEntry::ArgType argType, const QUrl &url)
{
    if (argType == ConfigEntry::LdapUrl) {   // LDAP server
        // The format is HOSTNAME:PORT:USERNAME:PASSWORD:BASE_DN
        assert(url.scheme() == QLatin1String("ldap"));
        return urlpart_encode(url.host()) + QLatin1Char(':') +
               QString::number(url.port()) + QLatin1Char(':') +
               urlpart_encode(url.userName()) + QLatin1Char(':') +
               urlpart_encode(url.password()) + QLatin1Char(':') +
               // QUrl automatically encoded the query (e.g. for spaces inside it),
               // so decode it before writing it out to gpgconf (issue119)
               urlpart_encode(QUrl::fromPercentEncoding(url.query().mid(1).toLatin1()));
    }
    return url.path();
}

}

Config::Config()
{
}

Config::~Config()
{
    qDeleteAll(m_components);
}

QStringList Config::componentList() const
{
    return m_components.keys();
}

ConfigComponent *Config::component(const QString &name) const
{
    return m_components[name];
}

void Config::addComponent(ConfigComponent *component)
{
    assert(component);
    if (m_components.contains(component->name())) {
        return;
    }
    m_components[component->name()] = component;
}

ConfigComponent::ConfigComponent(const QString &name) : m_name(name)
{
}

ConfigComponent::~ConfigComponent()
{
    qDeleteAll(m_groups);
}

QString ConfigComponent::name() const
{
    return m_name;
}

void ConfigComponent::setName(const QString &name)
{
    m_name = name;
}

QString ConfigComponent::description() const
{
    return m_description;
}

void ConfigComponent::setDescription(const QString &description)
{
    m_description = description;
}

QStringList ConfigComponent::groupList() const
{
    return m_groups.keys();
}

ConfigGroup *ConfigComponent::group(const QString &name) const
{
    return m_groups[name];
}

void ConfigComponent::addGroup(ConfigGroup *group)
{
    assert(group);
    if (m_groups.contains(group->name())) {
        return;
    }
    m_groups[group->name()] = group;
}

ConfigEntry *ConfigComponent::entry(const QString &name) const
{
    if (m_entries.contains(name)) {
        return m_entries[name];
    }
    Q_FOREACH (ConfigGroup *const i, m_groups) {
        if (ConfigEntry *const entry = i->entry(name)) {
            m_entries[name] = entry;
            return entry;
        }
    }
    return 0;
}

ConfigGroup::ConfigGroup(const QString &name) : m_name(name)
{
}

bool ConfigGroup::isEmpty() const
{
    return m_entries.isEmpty();
}

ConfigGroup::~ConfigGroup()
{
    qDeleteAll(m_entries);
}

QString ConfigGroup::name() const
{
    return m_name;
}

void ConfigGroup::setName(const QString &name)
{
    m_name = name;
}

QString ConfigGroup::description() const
{
    return m_description;
}

void ConfigGroup::setDescription(const QString &description)
{
    m_description = description;
}

QStringList ConfigGroup::entryList() const
{
    return m_entries.keys();
}

ConfigEntry *ConfigGroup::entry(const QString &name) const
{
    return m_entries[name];
}

void ConfigGroup::addEntry(ConfigEntry *entry)
{
    assert(entry);
    if (m_entries.contains(entry->name())) {
        return;
    }
    m_entries[entry->name()] = entry;
}

ConfigEntry::ConfigEntry(const QString &name) : m_dirty(false), m_name(name), m_mutability(ConfigEntry::UnspecifiedMutability), m_useDefault(false), m_argType(None), m_isList(false)
{
}

bool ConfigEntry::isDirty() const
{
    return m_dirty;
}

void ConfigEntry::unsetDirty()
{
    m_dirty = false;
}

QString ConfigEntry::name() const
{
    return m_name;
}

void ConfigEntry::setName(const QString &name)
{
    if (m_name == name) {
        return;
    }
    m_name = name;
    m_dirty = true;
}

QString ConfigEntry::description() const
{
    return m_description;
}

void ConfigEntry::setDescription(const QString &desc)
{
    if (m_description == desc) {
        return;
    }
    m_description = desc;
    m_dirty = true;
}

void ConfigEntry::setMutability(Mutability mutability)
{
    if (m_mutability == mutability) {
        return;
    }
    m_mutability = mutability;
    m_dirty = true;
}

ConfigEntry::Mutability ConfigEntry::mutability() const
{
    return m_mutability;
}

bool ConfigEntry::useBuiltInDefault() const
{
    return m_useDefault;
}

void ConfigEntry::setUseBuiltInDefault(bool useDefault)
{
    if (useDefault == m_useDefault) {
        return;
    }
    m_useDefault = useDefault;
    m_dirty = true;
}

void ConfigEntry::setArgType(ArgType type, ListType listType)
{
    m_argType = type;
    m_isList = listType == List;
}

ConfigEntry::ArgType ConfigEntry::argType() const
{
    return m_argType;
}

QString ConfigEntry::typeDescription() const
{
    const bool list = isList();

    switch (m_argType) {
    case None:
        return list ? i18nc("as in \"verbosity level\"", "Level") : i18n("Set/Unset");
    case String:
        return list ? i18n("String List") : i18n("String");
    case Int:
        return list ? i18n("List of Integers") : i18n("Integer");
    case UInt:
        return list ? i18n("List of Unsigned Integers") : i18n("Unsigned Integer ");
    case Path:
        return list ? i18n("Path List") : i18n("Path");
    case Url:
        return list ? i18n("List of URLs") : i18n("URL");
    case LdapUrl:
        return list ? i18n("List of LDAP URLs") : i18n("LDAP URL");
    case DirPath:
        return list ? i18n("Directory Path List") : i18n("Directory Path");
    }
    return QString();
}

bool ConfigEntry::isList() const
{
    return m_isList;
}

bool ConfigEntry::boolValue() const
{
    assert(m_argType == None);
    assert(!isList());
    return m_value.toBool();
}

QString ConfigEntry::stringValue() const
{
    return toString(NoEscape);
}

int ConfigEntry::intValue() const
{
    assert(m_argType == Int);
    assert(!isList());
    return m_value.toInt();
}

unsigned int ConfigEntry::uintValue() const
{
    assert(m_argType == UInt);
    assert(!isList());
    return m_value.toUInt();
}

QUrl ConfigEntry::urlValue() const
{
    assert(m_argType == Path || m_argType == Url || m_argType == LdapUrl);
    assert(!isList());
    QString str = m_value.toString();
    if (m_argType == Path) {
        QUrl url;
        url.setPath(str);
        return url;
    }
    return parseUrl(m_argType, str);
}

bool ConfigEntry::isStringType() const
{
    return (m_argType == String
            || m_argType == Path
            || m_argType == Url
            || m_argType == LdapUrl);
}

QStringList ConfigEntry::stringValueList() const
{
    assert(isStringType());
    assert(isList());
    return m_value.toStringList();
}

QList<int> ConfigEntry::intValueList() const
{
    assert(m_argType == Int);
    assert(isList());
    QList<int> ret;
    QList<QVariant> lst = m_value.toList();
    for (QList<QVariant>::const_iterator it = lst.constBegin(); it != lst.constEnd(); ++it) {
        ret.append((*it).toInt());
    }
    return ret;
}

QList<unsigned int> ConfigEntry::uintValueList() const
{
    assert(m_argType == UInt);
    assert(isList());
    QList<unsigned int> ret;
    QList<QVariant> lst = m_value.toList();
    for (QList<QVariant>::const_iterator it = lst.constBegin(); it != lst.constEnd(); ++it) {
        ret.append((*it).toUInt());
    }
    return ret;
}

QList<QUrl> ConfigEntry::urlValueList() const
{
    assert(m_argType == Path || m_argType == Url || m_argType == LdapUrl);
    assert(isList());
    const QStringList lst = m_value.toStringList();

    QList<QUrl> ret;
    Q_FOREACH (const QString &i, lst) {
        if (m_argType == Path) {
            QUrl url;
            url.setPath(i);
            ret << url;
        } else {
            ret << parseUrl(m_argType, i);
        }
    }
    return ret;
}

void ConfigEntry::setValueFromRawString(const QString &raw)
{
    m_value = stringToValue(raw, Unescape);
}

void ConfigEntry::setValueFromUiString(const QString &raw)
{
    m_value = stringToValue(raw, DoNotUnescape);
}

void ConfigEntry::setBoolValue(bool b)
{
    assert(m_argType == None);
    assert(!isList());
    // A "no arg" option is either set or not set.
    // Being set means mSet==true + m_value==true, being unset means resetToDefault(), i.e. both false
    m_value = b;
    m_dirty = true;
}

void ConfigEntry::setStringValue(const QString &str)
{
    m_value = stringToValue(str, DoNotUnescape);
    // When setting a string to empty (and there's no default), we need to act like resetToDefault
    // Otherwise we try e.g. "ocsp-responder:0:" and gpgconf answers:
    // "gpgconf: argument required for option ocsp-responder"
    m_dirty = true;
}

void ConfigEntry::setIntValue(int i)
{
    assert(m_argType == Int);
    assert(!isList());
    m_value = i;
    m_dirty = true;
}

void ConfigEntry::setUIntValue(unsigned int i)
{
    m_value = i;
    m_dirty = true;
}

void ConfigEntry::setURLValue(const QUrl &url)
{
    QString str = splitUrl(m_argType, url);
    m_value = str;
    m_dirty = true;
}

void ConfigEntry::setNumberOfTimesSet(uint i)
{
    assert(m_argType == None);
    assert(isList());
    setUIntValue(i);
}

unsigned int ConfigEntry::numberOfTimesSet() const
{
    assert(m_argType == None);
    assert(isList());
    return m_value.toUInt();
}

void ConfigEntry::setStringValueList(const QStringList &lst)
{
    m_value = lst;
    m_dirty = true;
}

void ConfigEntry::setIntValueList(const QList<int> &lst)
{
    QList<QVariant> ret;
    for (QList<int>::const_iterator it = lst.begin(); it != lst.end(); ++it) {
        ret << QVariant(*it);
    }
    m_value = ret;
    m_dirty = true;
}

void ConfigEntry::setUIntValueList(const QList<unsigned int> &lst)
{
    QList<QVariant> ret;
    for (QList<unsigned int>::const_iterator it = lst.begin(); it != lst.end(); ++it) {
        ret << QVariant(*it);
    }
    m_value = ret;
    m_dirty = true;
}

void ConfigEntry::setURLValueList(const QList<QUrl> &urls)
{
    QStringList lst;
    Q_FOREACH (const QUrl &i, urls) {
        lst << splitUrl(m_argType, i);
    }
    m_value = lst;
    m_dirty = true;
}

QString ConfigEntry::outputString() const
{
    return toString(Quote);
}

QVariant ConfigEntry::stringToValue(const QString &str, UnescapeMode mode) const
{
    const bool isString = isStringType();
    const bool unescape = mode & Unescape;
    if (isList()) {
        QList<QVariant> lst;
        const QStringList items = str.split(QLatin1Char(','), QString::SkipEmptyParts);
        for (QStringList::const_iterator valit = items.constBegin(); valit != items.constEnd(); ++valit) {
            QString val = *valit;
            if (isString) {
                if (val.isEmpty()) {
                    lst << QVariant(QString());
                    continue;
                } else if (unescape) {
                    if (val.startsWith(QLatin1Char('"'))) {
                        val = val.mid(1);
                    } else { // see README.gpgconf
                        qWarning() << "String value should start with '\"' :" << val;
                    }
                }
            }
            lst << QVariant(unescape ? gpgconf_unescape(val) : val);
        }
        return lst;
    } else { // not a list
        QString val(str);
        if (isString) {
            if (val.isEmpty()) {
                return QVariant(QString());    // not set  [ok with lists too?]
            } else if (unescape) {
                if (val.startsWith(QLatin1Char('"'))) {
                    val = val.mid(1);
                } else { // see README.gpgconf
                    qWarning() << "String value should start with '\"' :" << val;
                }
            }
        }
        return QVariant(unescape ? gpgconf_unescape(val) : val);
    }
}

QString ConfigEntry::toString(ConfigEntry::EscapeMode mode) const
{
    const bool escape = mode & Escape;
    const bool quote = mode & Quote;

    // Basically the opposite of stringToValue
    if (isStringType()) {
        if (m_value.isNull()) {
            return QString();
        } else if (isList()) { // string list
            QStringList lst = m_value.toStringList();
            if (escape || quote) {
                for (QStringList::iterator it = lst.begin(); it != lst.end(); ++it) {
                    if (!(*it).isNull()) {
                        if (escape) {
                            *it = gpgconf_escape(*it);
                        }
                        if (quote) {
                            *it = (*it).prepend(QLatin1Char('\"'));
                        }
                    }
                }
            }
            QString res = lst.join(QLatin1String(","));
            //qDebug() <<"toString:" << res;
            return res;
        } else { // normal string
            QString res = m_value.toString();
            if (escape) {
                res = gpgconf_escape(res);
            }
            if (quote) {
                res = res.prepend(QLatin1Char('\"'));
            }
            return res;
        }
    }
    if (!isList()) { // non-list non-string
        if (m_argType == None) {
            return m_value.toBool() ? QString::fromLatin1("1") : QString();
        } else { // some int
            assert(m_argType == Int || m_argType == UInt);
            return m_value.toString(); // int to string conversion
        }
    }

    // Lists (of other types than strings)
    if (m_argType == None) {
        const int numTimesSet = numberOfTimesSet();
        return numTimesSet > 0 ? QString::number(numTimesSet) : QString();
    }
    QStringList ret;
    QList<QVariant> lst = m_value.toList();
    QList<QVariant>::const_iterator end(lst.constEnd());
    for (QList<QVariant>::const_iterator it = lst.constBegin(); it != end; ++it) {
        ret << (*it).toString(); // QVariant does the conversion
    }
    return ret.join(QLatin1String(","));
}

