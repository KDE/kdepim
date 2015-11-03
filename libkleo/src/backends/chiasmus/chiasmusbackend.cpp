/*
    chiasmusbackend.cpp

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2005 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    Libkleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

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

#include "chiasmusbackend.h"

#include "config_data.h"
#include "obtainkeysjob.h"
#include "chiasmusjob.h"

#include "libkleo/cryptoconfig.h"

#include <KLocalizedString>
#include <kconfig.h>
#include <kshell.h>
#include <QDebug>
#include <kconfiggroup.h>

#include <QStringList>
#include <QVariant>
#include <QFileInfo>

#include <QList>
#include <QUrl>

#include <map>
#include <memory>
#include <vector>

#include <cassert>

namespace
{

//
// The usual QVariant template helpers:
//

// to<> is a demarshaller. It's a class b/c you can't partially
// specialize function templates yet. However, to<> can be used as if
// it was a function: QString s = to<QString>( myVariant );
template <typename T> class to {};

#define MAKE_TO( type, func ) \
    template <> \
    class to< type > { \
        type m; \
    public: \
        to( const QVariant & v ) : m( v.func() ) {} \
        operator type() const { return m; } \
    }

MAKE_TO(int, toInt);
MAKE_TO(unsigned int, toUInt);

template <>
class to<QUrl>
{
    QUrl m;
public:
    to(const QVariant &v)
    {
        m.setPath(v.toString());
    }
    operator QUrl() const
    {
        return m;
    }
};

template <typename T>
class to< QList<T> >
{
    QList<T> m;
public:
    to(const QVariant &v)
    {
        const QList<QVariant> vl = v.toList();
        for (QList<QVariant>::const_iterator it = vl.begin(), end = vl.end(); it != end; ++it) {
            m.push_back(to<T>(*it));
        }
    }
    operator QList<T> () const
    {
        return m;
    }
};

template <typename T>
class to< std::vector<T> >
{
    std::vector<T> m;
public:
    to(const QVariant &v)
    {
        const QList<QVariant> vl = v.toList();
        m.reserve(m.size() + vl.size());
        for (QList<QVariant>::const_iterator it = vl.begin(), end = vl.end(); it != end; ++it) {
            m.push_back(to<T>(*it));
        }
    }
    operator std::vector<T> () const
    {
        return m;
    }
};

// from<> is the demarshaller. See to<> for why this is a class...

template <typename T>
struct from_helper : public QVariant {
    from_helper(const T &t) : QVariant(t) {}
};

template <typename T>
QVariant from(const T &t)
{
    return from_helper<T>(t);
}

// some special types:
template <> struct from_helper<bool> : public QVariant {
    from_helper(bool b) : QVariant(b) {}
};
template <> struct from_helper<QUrl> : public QVariant {
    from_helper(const QUrl &url) : QVariant(url.path()) {}
};
template <typename T> struct from_helper< QList<T> > : public QVariant {
    from_helper(const QList<T> &l)
    {
        QList<QVariant> result;
        for (typename QList<T>::const_iterator it = l.begin(), end = l.end(); it != end; ++it) {
            result.push_back(from(*it));
        }
        QVariant::operator=(result);
    }
};
template <typename T> struct from_helper< std::vector<T> > : public QVariant {
    from_helper(const std::vector<T> &l)
    {
        QList<QVariant> result;
        for (typename std::vector<T>::const_iterator it = l.begin(), end = l.end(); it != end; ++it) {
            result.push_back(from(*it));
        }
        QVariant::operator=(result);
    }
};

class ChiasmusConfigEntry : public Kleo::CryptoConfigEntry
{
    unsigned int mIdx;
    QVariant mValue;
    bool mDirty;
public:
    ChiasmusConfigEntry(unsigned int i)
        : Kleo::CryptoConfigEntry(),
          mIdx(i), mValue(defaultValue()), mDirty(false)
    {
        assert(i < kleo_chiasmus_config_entries_dim);
    }
    QString name() const Q_DECL_OVERRIDE
    {
        return QLatin1String(kleo_chiasmus_config_entries[mIdx].name);
    }
    QString description() const Q_DECL_OVERRIDE
    {
        return i18n(kleo_chiasmus_config_entries[mIdx].description);
    }
    QString path() const Q_DECL_OVERRIDE
    {
        return name();
    }
    bool isOptional() const Q_DECL_OVERRIDE
    {
        return kleo_chiasmus_config_entries[mIdx].is_optional;
    }
    bool isReadOnly() const Q_DECL_OVERRIDE
    {
        return false;
    }
    bool isList() const Q_DECL_OVERRIDE
    {
        return kleo_chiasmus_config_entries[mIdx].is_list;
    }
    bool isRuntime() const Q_DECL_OVERRIDE
    {
        return kleo_chiasmus_config_entries[mIdx].is_runtime;
    }
    Level level() const Q_DECL_OVERRIDE
    {
        return static_cast<Level>(kleo_chiasmus_config_entries[mIdx].level);
    }
    ArgType argType() const Q_DECL_OVERRIDE
    {
        return static_cast<ArgType>(kleo_chiasmus_config_entries[mIdx].type);
    }
    bool isSet() const Q_DECL_OVERRIDE
    {
        return mValue != defaultValue();
    }
    bool boolValue() const Q_DECL_OVERRIDE
    {
        return mValue.toBool();
    }
    QString stringValue() const Q_DECL_OVERRIDE
    {
        return mValue.toString();
    }
    int intValue() const Q_DECL_OVERRIDE
    {
        return mValue.toInt();
    }
    unsigned int uintValue() const Q_DECL_OVERRIDE
    {
        return mValue.toUInt();
    }
    QUrl urlValue() const Q_DECL_OVERRIDE
    {
        Q_ASSERT(argType() == ArgType_Path || argType() == ArgType_DirPath);
        return QUrl::fromLocalFile(mValue.toString());
    }
    unsigned int numberOfTimesSet() const Q_DECL_OVERRIDE
    {
        return 0;
    }
    std::vector<int> intValueList() const Q_DECL_OVERRIDE
    {
        return to< std::vector<int> >(mValue);
    }
    std::vector<unsigned int> uintValueList() const Q_DECL_OVERRIDE
    {
        return to< std::vector<unsigned int> >(mValue);
    }
    QList<QUrl> urlValueList() const Q_DECL_OVERRIDE
    {
        return to< QList<QUrl> >(mValue);
    }
    void resetToDefault() Q_DECL_OVERRIDE {
        mValue = defaultValue();
        mDirty = false;
    }
    void setBoolValue(bool value) Q_DECL_OVERRIDE {
        setValue(QVariant(value));
    }
    void setStringValue(const QString &value) Q_DECL_OVERRIDE {
        setValue(value);
    }
    void setIntValue(int value) Q_DECL_OVERRIDE {
        setValue(value);
    }
    void setUIntValue(unsigned int value) Q_DECL_OVERRIDE {
        setValue(value);
    }
    void setURLValue(const QUrl &value) Q_DECL_OVERRIDE {
        if (argType() != ArgType_Path)
        {
            setValue(value.url());
        } else {
            setValue(value.path());
        }
    }
    void setNumberOfTimesSet(unsigned int) Q_DECL_OVERRIDE {}
    void setIntValueList(const std::vector<int> &l) Q_DECL_OVERRIDE {
        setValue(from(l));
    }
    void setUIntValueList(const std::vector<unsigned int> &l) Q_DECL_OVERRIDE {
        setValue(from(l));
    }
    void setURLValueList(const QList<QUrl> &l) Q_DECL_OVERRIDE {
        setValue(from(l));
    }
    bool isDirty() const Q_DECL_OVERRIDE
    {
        return mDirty;
    }

    QVariant value() const
    {
        return mValue;
    }

    void sync(KConfigGroup config)
    {
        if (!mDirty) {
            return;
        }
        mDirty = false;
        config.writeEntry(kleo_chiasmus_config_entries[mIdx].name, mValue);
    }
    void read(const KConfigGroup &config)
    {
        mDirty = false;
        mValue = config.readEntry(kleo_chiasmus_config_entries[mIdx].name, defaultValue());
    }
private:
    QVariant defaultValue() const;
    void setValue(const QVariant &value)
    {
        mValue = value;
        mDirty = true;
    }
};

QVariant ChiasmusConfigEntry::defaultValue() const
{
    const kleo_chiasmus_config_data &data = kleo_chiasmus_config_entries[mIdx];
    switch (data.type) {
    default:
        return QVariant();
    case ArgType_None:
        if (isList()) {
            return QList<QVariant>() << QVariant(data.defaults.boolean.value);
        } else {
            return QVariant(data.defaults.boolean.value);
        }
    case ArgType_String:
        if (isList()) {
            return QStringList(QString::fromLatin1(data.defaults.string));
        } else {
            return QString::fromLatin1(data.defaults.string);
        }
    case ArgType_Int:
        if (isList()) {
            return QList<QVariant>() << data.defaults.integer;
        } else {
            return data.defaults.integer;
        }
    case ArgType_UInt:
        if (isList()) {
            return QList<QVariant>() << data.defaults.unsigned_integer;
        } else {
            return data.defaults.unsigned_integer;
        }
    case ArgType_Path:
    case ArgType_DirPath:
        if (isList()) {
            return QList<QVariant>() << QString::fromLatin1(data.defaults.path);
        } else {
            return QString::fromLatin1(data.defaults.path);
        }
    case ArgType_LDAPURL:
        if (isList()) {
            return QList<QVariant>() << QString::fromLatin1(data.defaults.url);
        } else {
            return QString::fromLatin1(data.defaults.url);
        }
    }
}

class ChiasmusGeneralGroup : public Kleo::CryptoConfigGroup
{
    mutable std::map<QString, ChiasmusConfigEntry *> mCache;
    mutable KConfig *mConfigObject;
public:
    ChiasmusGeneralGroup() : Kleo::CryptoConfigGroup(), mConfigObject(0) {}
    ~ChiasmusGeneralGroup()
    {
        clear();
        delete mConfigObject;
    }
    QString name() const Q_DECL_OVERRIDE
    {
        return QStringLiteral("General");
    }
    QString iconName() const Q_DECL_OVERRIDE
    {
        return QStringLiteral("chiasmus_chi");
    }
    QString path() const Q_DECL_OVERRIDE
    {
        return QString();
    }
    QString description() const Q_DECL_OVERRIDE
    {
        return i18n("General");
    }
    Kleo::CryptoConfigEntry::Level level() const Q_DECL_OVERRIDE
    {
        return Kleo::CryptoConfigEntry::Level_Basic;
    }
    QStringList entryList() const Q_DECL_OVERRIDE
    {
        QStringList result;
        for (unsigned int i = 0; i < kleo_chiasmus_config_entries_dim; ++i) {
            result.push_back(QLatin1String(kleo_chiasmus_config_entries[i].name));
        }
        return result;
    }
    Kleo::CryptoConfigEntry *entry(const QString &name) const Q_DECL_OVERRIDE
    {
        if (ChiasmusConfigEntry *entry = mCache[name]) {
            return entry;
        }
        const KConfigGroup group(configObject(), "Chiasmus");
        for (unsigned int i = 0; i < kleo_chiasmus_config_entries_dim; ++i)
            if (name == QLatin1String(kleo_chiasmus_config_entries[i].name)) {
                ChiasmusConfigEntry *entry = new ChiasmusConfigEntry(i);
                entry->read(group);
                return mCache[name] = entry;
            }
        return 0;
    }

    void sync()
    {
        KConfigGroup group(configObject(), "Chiasmus");
        for (std::map<QString, ChiasmusConfigEntry *>::const_iterator it = mCache.begin(), end = mCache.end(); it != end; ++it) {
            it->second->sync(group);
        }
        group.sync();
        clear();
    }
private:
    KConfig *configObject() const
    {
        if (!mConfigObject)
            // this is unsafe. We're a lib, used by concurrent apps.
        {
            mConfigObject = new KConfig(QStringLiteral("chiasmusbackendrc"));
        }
        return mConfigObject;
    }
    void clear()
    {
        for (std::map<QString, ChiasmusConfigEntry *>::const_iterator it = mCache.begin(), end = mCache.end(); it != end; ++it) {
            delete it->second;
        }
        mCache.clear();
    }
};

class ChiasmusComponent : public Kleo::CryptoConfigComponent
{
    mutable ChiasmusGeneralGroup *mGeneralGroup;
public:
    ChiasmusComponent() : Kleo::CryptoConfigComponent(), mGeneralGroup(0) {}
    ~ChiasmusComponent()
    {
        delete mGeneralGroup;
    }

    void sync()
    {
        if (mGeneralGroup) {
            mGeneralGroup->sync();
        }
    }

    QString name() const Q_DECL_OVERRIDE
    {
        return QStringLiteral("Chiasmus");
    }
    QString iconName() const Q_DECL_OVERRIDE
    {
        return QStringLiteral("chiasmus_chi");
    }
    QString description() const Q_DECL_OVERRIDE
    {
        return i18n("Chiasmus");
    }
    QStringList groupList() const Q_DECL_OVERRIDE
    {
        return QStringList() << QStringLiteral("General");
    }
    Kleo::CryptoConfigGroup *group(const QString &name) const Q_DECL_OVERRIDE
    {
        if (name != QLatin1String("General")) {
            return 0;
        }
        if (!mGeneralGroup) {
            mGeneralGroup = new ChiasmusGeneralGroup();
        }
        return mGeneralGroup;
    }
};

}

class Kleo::ChiasmusBackend::CryptoConfig : public Kleo::CryptoConfig
{
    mutable ChiasmusComponent *mComponent;
public:
    CryptoConfig() : Kleo::CryptoConfig(), mComponent(0) {}
    ~CryptoConfig()
    {
        delete mComponent;
    }

    QStringList componentList() const Q_DECL_OVERRIDE
    {
        return QStringList() << QStringLiteral("Chiasmus");
    }
    ChiasmusComponent *component(const QString &name) const Q_DECL_OVERRIDE
    {
        if (name != QLatin1String("Chiasmus")) {
            return 0;
        }
        if (!mComponent) {
            mComponent = new ChiasmusComponent();
        }
        return mComponent;
    }
    void sync(bool) Q_DECL_OVERRIDE {
        if (mComponent)
        {
            mComponent->sync();
        }
    }
    void clear() Q_DECL_OVERRIDE {
        delete mComponent;
        mComponent = 0;
    }
};

class Kleo::ChiasmusBackend::Protocol : public Kleo::CryptoBackend::Protocol
{
    Kleo::CryptoConfig *mCryptoConfig;
public:
    Protocol(Kleo::CryptoConfig *config)
        : Kleo::CryptoBackend::Protocol(), mCryptoConfig(config)
    {
        assert(config);
    }
    ~Protocol() {}

    QString name() const Q_DECL_OVERRIDE
    {
        return QStringLiteral("Chiasmus");
    }
    QString displayName() const Q_DECL_OVERRIDE
    {
        return i18n("Chiasmus command line tool");
    }
    KeyListJob *keyListJob(bool, bool, bool) const Q_DECL_OVERRIDE
    {
        return 0;
    }
    ListAllKeysJob *listAllKeysJob(bool, bool) const Q_DECL_OVERRIDE
    {
        return 0;
    }
    EncryptJob *encryptJob(bool, bool) const Q_DECL_OVERRIDE
    {
        return 0;
    }
    DecryptJob *decryptJob() const Q_DECL_OVERRIDE
    {
        return 0;
    }
    SignJob *signJob(bool, bool) const Q_DECL_OVERRIDE
    {
        return 0;
    }
    VerifyDetachedJob *verifyDetachedJob(bool) const Q_DECL_OVERRIDE
    {
        return 0;
    }
    VerifyOpaqueJob *verifyOpaqueJob(bool) const Q_DECL_OVERRIDE
    {
        return 0;
    }
    KeyGenerationJob *keyGenerationJob() const Q_DECL_OVERRIDE
    {
        return 0;
    }
    ImportFromKeyserverJob *importFromKeyserverJob() const Q_DECL_OVERRIDE
    {
        return 0;
    }
    ImportJob *importJob() const Q_DECL_OVERRIDE
    {
        return 0;
    }
    ExportJob *publicKeyExportJob(bool) const Q_DECL_OVERRIDE
    {
        return 0;
    }
    ExportJob *secretKeyExportJob(bool, const QString &) const Q_DECL_OVERRIDE
    {
        return 0;
    }
    DownloadJob *downloadJob(bool) const Q_DECL_OVERRIDE
    {
        return 0;
    }
    DeleteJob *deleteJob() const Q_DECL_OVERRIDE
    {
        return 0;
    }
    SignEncryptJob *signEncryptJob(bool, bool) const Q_DECL_OVERRIDE
    {
        return 0;
    }
    DecryptVerifyJob *decryptVerifyJob(bool) const Q_DECL_OVERRIDE
    {
        return 0;
    }
    RefreshKeysJob *refreshKeysJob() const Q_DECL_OVERRIDE
    {
        return 0;
    }

    SpecialJob *specialJob(const char *type, const QMap<QString, QVariant> &args) const Q_DECL_OVERRIDE
    {
        if (qstricmp(type, "x-obtain-keys") == 0 && args.size() == 0) {
            return new ObtainKeysJob();
        }
        if (qstricmp(type, "x-encrypt") == 0 && args.size() == 0) {
            return new ChiasmusJob(ChiasmusJob::Encrypt);
        }
        if (qstricmp(type, "x-decrypt") == 0 && args.size() == 0) {
            return new ChiasmusJob(ChiasmusJob::Decrypt);
        }
        qDebug() << "ChiasmusBackend::Protocol: tried to instantiate unknown job type \""
                 << type << "\"";

        return 0;
    }
};

Kleo::ChiasmusBackend *Kleo::ChiasmusBackend::self = 0;

Kleo::ChiasmusBackend::ChiasmusBackend()
    : Kleo::CryptoBackend(),
      mCryptoConfig(0),
      mProtocol(0)
{
    self = this;
}

Kleo::ChiasmusBackend::~ChiasmusBackend()
{
    self = 0;
    delete mCryptoConfig;
    delete mProtocol;
}

QString Kleo::ChiasmusBackend::name() const
{
    return QStringLiteral("Chiasmus");
}

QString Kleo::ChiasmusBackend::displayName() const
{
    return i18n("Chiasmus");
}

Kleo::CryptoConfig *Kleo::ChiasmusBackend::config() const
{
    if (!mCryptoConfig) {
        mCryptoConfig = new CryptoConfig();
    }
    return mCryptoConfig;
}

Kleo::CryptoBackend::Protocol *Kleo::ChiasmusBackend::protocol(const char *name) const
{
    if (qstricmp(name, "Chiasmus") != 0) {
        return 0;
    }
    if (!mProtocol)
        if (checkForChiasmus()) {
            mProtocol = new Protocol(config());
        }
    return mProtocol;
}

bool Kleo::ChiasmusBackend::checkForOpenPGP(QString *reason) const
{
    if (reason) {
        *reason = i18n("Unsupported protocol \"%1\"", QStringLiteral("OpenPGP"));
    }
    return false;
}

bool Kleo::ChiasmusBackend::checkForSMIME(QString *reason) const
{
    if (reason) {
        *reason = i18n("Unsupported protocol \"%1\"", QStringLiteral("SMIME"));
    }
    return false;
}

bool Kleo::ChiasmusBackend::checkForChiasmus(QString *reason) const
{

    // kills the protocol instance when we return false:
    std::unique_ptr<Protocol> tmp(mProtocol);
    mProtocol = 0;

    const CryptoConfigEntry *path = config()->entry(QStringLiteral("Chiasmus"), QStringLiteral("General"), QStringLiteral("path"));
    assert(path); assert(path->argType() == CryptoConfigEntry::ArgType_Path);
    const QString chiasmus = path->urlValue().path();
    const QFileInfo fi(KShell::tildeExpand(chiasmus));
    if (!fi.isExecutable()) {
        if (reason) {
            *reason = i18n("File \"%1\" does not exist or is not executable.", chiasmus);
        }
        return false;
    }

    // FIXME: more checks?
    mProtocol = tmp.release();
    return true;
}

bool Kleo::ChiasmusBackend::checkForProtocol(const char *name, QString *reason) const
{
    if (qstricmp(name, "Chiasmus") == 0) {
        return checkForChiasmus(reason);
    }
    if (reason) {
        *reason = i18n("Unsupported protocol \"%1\"", QLatin1String(name));
    }
    return 0;
}

bool Kleo::ChiasmusBackend::supportsProtocol(const char *name) const
{
    return qstricmp(name, "Chiasmus") == 0;
}

const char *Kleo::ChiasmusBackend::enumerateProtocols(int i) const
{
    return i == 0 ? "Chiasmus" : 0;
}
