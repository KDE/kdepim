/*
    qgpgmebackend.cpp

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004,2005 Klarälvdalens Datakonsult AB

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

#include "qgpgmebackend.h"

#include "qgpgmecryptoconfig.h"
#include "qgpgmenewcryptoconfig.h"

#include "qgpgmekeygenerationjob.h"
#include "qgpgmekeylistjob.h"
#include "qgpgmelistallkeysjob.h"
#include "qgpgmedecryptjob.h"
#include "qgpgmedecryptverifyjob.h"
#include "qgpgmerefreshkeysjob.h"
#include "qgpgmedeletejob.h"
#include "qgpgmesecretkeyexportjob.h"
#include "qgpgmedownloadjob.h"
#include "qgpgmesignencryptjob.h"
#include "qgpgmeencryptjob.h"
#include "qgpgmesignjob.h"
#include "qgpgmesignkeyjob.h"
#include "qgpgmeexportjob.h"
#include "qgpgmeverifydetachedjob.h"
#include "qgpgmeimportjob.h"
#include "qgpgmeimportfromkeyserverjob.h"
#include "qgpgmeverifyopaquejob.h"
#include "qgpgmechangeexpiryjob.h"
#include "qgpgmechangeownertrustjob.h"
#include "qgpgmechangepasswdjob.h"
#include "qgpgmeadduseridjob.h"

#include <gpgme++/error.h>
#include <gpgme++/engineinfo.h>

#include <KLocalizedString>

#include <QFile>
#include <QString>

namespace
{

class Protocol : public Kleo::CryptoBackend::Protocol
{
    GpgME::Protocol mProtocol;
public:
    explicit Protocol(GpgME::Protocol proto) : mProtocol(proto) {}

    QString name() const Q_DECL_OVERRIDE
    {
        switch (mProtocol) {
        case GpgME::OpenPGP: return QStringLiteral("OpenPGP");
        case GpgME::CMS:     return QStringLiteral("SMIME");
        default:             return QString();
        }
    }

    QString displayName() const Q_DECL_OVERRIDE
    {
        switch (mProtocol) {
        case GpgME::OpenPGP: return QStringLiteral("gpg");
        case GpgME::CMS:     return QStringLiteral("gpgsm");
        default:             return i18n("unknown");
        }
    }

    Kleo::SpecialJob *specialJob(const char *, const QMap<QString, QVariant> &) const Q_DECL_OVERRIDE
    {
        return 0;
    }

    Kleo::KeyListJob *keyListJob(bool remote, bool includeSigs, bool validate) const Q_DECL_OVERRIDE
    {
        GpgME::Context *context = GpgME::Context::createForProtocol(mProtocol);
        if (!context) {
            return 0;
        }

        unsigned int mode = context->keyListMode();
        if (remote) {
            mode |= GpgME::Extern;
            mode &= ~GpgME::Local;
        } else {
            mode |= GpgME::Local;
            mode &= ~GpgME::Extern;
        }
        if (includeSigs) {
            mode |= GpgME::Signatures;
        }
        if (validate) {
            mode |= GpgME::Validate;
        }
        context->setKeyListMode(mode);
        return new Kleo::QGpgMEKeyListJob(context);
    }

    Kleo::ListAllKeysJob *listAllKeysJob(bool includeSigs, bool validate) const Q_DECL_OVERRIDE
    {
        GpgME::Context *context = GpgME::Context::createForProtocol(mProtocol);
        if (!context) {
            return 0;
        }

        unsigned int mode = context->keyListMode();
        mode |= GpgME::Local;
        mode &= ~GpgME::Extern;
        if (includeSigs) {
            mode |= GpgME::Signatures;
        }
        if (validate) {
            mode |= GpgME::Validate;
            /* Setting the context to offline mode disables CRL / OCSP checks in
               this Job. Otherwise we would try to fetch the CRL's for all CMS
               keys in the users keyring because GpgME::Validate includes remote
               resources by default in the validity check.
               This setting only has any effect if gpgsm >= 2.1.6 is used.
               */
            context->setOffline(true);
        }
        context->setKeyListMode(mode);
        return new Kleo::QGpgMEListAllKeysJob(context);
    }

    Kleo::EncryptJob *encryptJob(bool armor, bool textmode) const Q_DECL_OVERRIDE
    {
        GpgME::Context *context = GpgME::Context::createForProtocol(mProtocol);
        if (!context) {
            return 0;
        }

        context->setArmor(armor);
        context->setTextMode(textmode);
        return new Kleo::QGpgMEEncryptJob(context);
    }

    Kleo::DecryptJob *decryptJob() const Q_DECL_OVERRIDE
    {
        GpgME::Context *context = GpgME::Context::createForProtocol(mProtocol);
        if (!context) {
            return 0;
        }
        return new Kleo::QGpgMEDecryptJob(context);
    }

    Kleo::SignJob *signJob(bool armor, bool textMode) const Q_DECL_OVERRIDE
    {
        GpgME::Context *context = GpgME::Context::createForProtocol(mProtocol);
        if (!context) {
            return 0;
        }

        context->setArmor(armor);
        context->setTextMode(textMode);
        return new Kleo::QGpgMESignJob(context);
    }

    Kleo::VerifyDetachedJob *verifyDetachedJob(bool textMode) const Q_DECL_OVERRIDE
    {
        GpgME::Context *context = GpgME::Context::createForProtocol(mProtocol);
        if (!context) {
            return 0;
        }

        context->setTextMode(textMode);
        return new Kleo::QGpgMEVerifyDetachedJob(context);
    }

    Kleo::VerifyOpaqueJob *verifyOpaqueJob(bool textMode) const Q_DECL_OVERRIDE
    {
        GpgME::Context *context = GpgME::Context::createForProtocol(mProtocol);
        if (!context) {
            return 0;
        }

        context->setTextMode(textMode);
        return new Kleo::QGpgMEVerifyOpaqueJob(context);
    }

    Kleo::KeyGenerationJob *keyGenerationJob() const Q_DECL_OVERRIDE
    {
        GpgME::Context *context = GpgME::Context::createForProtocol(mProtocol);
        if (!context) {
            return 0;
        }
        return new Kleo::QGpgMEKeyGenerationJob(context);
    }

    Kleo::ImportJob *importJob() const Q_DECL_OVERRIDE
    {
        GpgME::Context *context = GpgME::Context::createForProtocol(mProtocol);
        if (!context) {
            return 0;
        }
        return new Kleo::QGpgMEImportJob(context);
    }

    Kleo::ImportFromKeyserverJob *importFromKeyserverJob() const Q_DECL_OVERRIDE
    {
        GpgME::Context *context = GpgME::Context::createForProtocol(mProtocol);
        if (!context) {
            return 0;
        }
        return new Kleo::QGpgMEImportFromKeyserverJob(context);
    }

    Kleo::ExportJob *publicKeyExportJob(bool armor) const Q_DECL_OVERRIDE
    {
        GpgME::Context *context = GpgME::Context::createForProtocol(mProtocol);
        if (!context) {
            return 0;
        }

        context->setArmor(armor);
        return new Kleo::QGpgMEExportJob(context);
    }

    Kleo::ExportJob *secretKeyExportJob(bool armor, const QString &charset) const Q_DECL_OVERRIDE
    {
        if (mProtocol != GpgME::CMS) { // fixme: add support for gpg, too
            return 0;
        }

        // this operation is not supported by gpgme, so we have to call gpgsm ourselves:
        return new Kleo::QGpgMESecretKeyExportJob(armor, charset);
    }

    Kleo::RefreshKeysJob *refreshKeysJob() const Q_DECL_OVERRIDE
    {
        if (mProtocol != GpgME::CMS) { // fixme: add support for gpg, too
            return 0;
        }

        // this operation is not supported by gpgme, so we have to call gpgsm ourselves:
        return new Kleo::QGpgMERefreshKeysJob();
    }

    Kleo::DownloadJob *downloadJob(bool armor) const Q_DECL_OVERRIDE
    {
        GpgME::Context *context = GpgME::Context::createForProtocol(mProtocol);
        if (!context) {
            return 0;
        }

        context->setArmor(armor);
        // this is the hackish interface for downloading from keyserers currently:
        context->setKeyListMode(GpgME::Extern);
        return new Kleo::QGpgMEDownloadJob(context);
    }

    Kleo::DeleteJob *deleteJob() const Q_DECL_OVERRIDE
    {
        GpgME::Context *context = GpgME::Context::createForProtocol(mProtocol);
        if (!context) {
            return 0;
        }
        return new Kleo::QGpgMEDeleteJob(context);
    }

    Kleo::SignEncryptJob *signEncryptJob(bool armor, bool textMode) const Q_DECL_OVERRIDE
    {
        GpgME::Context *context = GpgME::Context::createForProtocol(mProtocol);
        if (!context) {
            return 0;
        }

        context->setArmor(armor);
        context->setTextMode(textMode);
        return new Kleo::QGpgMESignEncryptJob(context);
    }

    Kleo::DecryptVerifyJob *decryptVerifyJob(bool textMode) const Q_DECL_OVERRIDE
    {
        GpgME::Context *context = GpgME::Context::createForProtocol(mProtocol);
        if (!context) {
            return 0;
        }

        context->setTextMode(textMode);
        return new Kleo::QGpgMEDecryptVerifyJob(context);
    }

    Kleo::ChangeExpiryJob *changeExpiryJob() const Q_DECL_OVERRIDE
    {
        if (mProtocol != GpgME::OpenPGP) {
            return 0;    // only supported by gpg
        }

        GpgME::Context *context = GpgME::Context::createForProtocol(mProtocol);
        if (!context) {
            return 0;
        }
        return new Kleo::QGpgMEChangeExpiryJob(context);
    }

    Kleo::ChangePasswdJob *changePasswdJob() const Q_DECL_OVERRIDE
    {
        if (!GpgME::hasFeature(GpgME::PasswdFeature, 0)) {
            return 0;
        }
        GpgME::Context *context = GpgME::Context::createForProtocol(mProtocol);
        if (!context) {
            return 0;
        }
        return new Kleo::QGpgMEChangePasswdJob(context);
    }

    Kleo::SignKeyJob *signKeyJob() const Q_DECL_OVERRIDE
    {
        if (mProtocol != GpgME::OpenPGP) {
            return 0;    // only supported by gpg
        }

        GpgME::Context *context = GpgME::Context::createForProtocol(mProtocol);
        if (!context) {
            return 0;
        }
        return new Kleo::QGpgMESignKeyJob(context);
    }

    Kleo::ChangeOwnerTrustJob *changeOwnerTrustJob() const Q_DECL_OVERRIDE
    {
        if (mProtocol != GpgME::OpenPGP) {
            return 0;    // only supported by gpg
        }

        GpgME::Context *context = GpgME::Context::createForProtocol(mProtocol);
        if (!context) {
            return 0;
        }
        return new Kleo::QGpgMEChangeOwnerTrustJob(context);
    }

    Kleo::AddUserIDJob *addUserIDJob() const Q_DECL_OVERRIDE
    {
        if (mProtocol != GpgME::OpenPGP) {
            return 0;    // only supported by gpg
        }

        GpgME::Context *context = GpgME::Context::createForProtocol(mProtocol);
        if (!context) {
            return 0;
        }
        return new Kleo::QGpgMEAddUserIDJob(context);
    }

};

}

Kleo::QGpgMEBackend::QGpgMEBackend()
    : Kleo::CryptoBackend(),
      mCryptoConfig(0),
      mOpenPGPProtocol(0),
      mSMIMEProtocol(0)
{
    GpgME::initializeLibrary();
}

Kleo::QGpgMEBackend::~QGpgMEBackend()
{
    delete mCryptoConfig; mCryptoConfig = 0;
    delete mOpenPGPProtocol; mOpenPGPProtocol = 0;
    delete mSMIMEProtocol; mSMIMEProtocol = 0;
}

QString Kleo::QGpgMEBackend::name() const
{
    return QStringLiteral("gpgme");
}

QString Kleo::QGpgMEBackend::displayName() const
{
    return i18n("GpgME");
}

Kleo::CryptoConfig *Kleo::QGpgMEBackend::config() const
{
    if (!mCryptoConfig) {
#ifdef _WIN32_WCE // for now...
        if (GpgME::hasFeature(GpgME::GpgConfEngineFeature, 0)) {
            mCryptoConfig = new QGpgMENewCryptoConfig;
        } else
#endif
            if (!QGpgMECryptoConfig::gpgConfPath().isEmpty()) {
                mCryptoConfig = new QGpgMECryptoConfig();
            }
    }
    return mCryptoConfig;
}

static bool check(GpgME::Protocol proto, QString *reason)
{
    if (!GpgME::checkEngine(proto)) {
        return true;
    }
    if (!reason) {
        return false;
    }
    // error, check why:
    const GpgME::EngineInfo ei = GpgME::engineInfo(proto);
    if (ei.isNull()) {
        *reason = i18n("GPGME was compiled without support for %1.", proto == GpgME::CMS ? QLatin1String("S/MIME") : QLatin1String("OpenPGP"));
    } else if (ei.fileName() && !ei.version()) {
        *reason = i18n("Engine %1 is not installed properly.", QFile::decodeName(ei.fileName()));
    } else if (ei.fileName() && ei.version() && ei.requiredVersion())
        *reason = i18n("Engine %1 version %2 installed, "
                       "but at least version %3 is required.",
                       QFile::decodeName(ei.fileName()), QLatin1String(ei.version()), QLatin1String(ei.requiredVersion()));
    else {
        *reason = i18n("Unknown problem with engine for protocol %1.", proto == GpgME::CMS ? QLatin1String("S/MIME") : QLatin1String("OpenPGP"));
    }
    return false;
}

bool Kleo::QGpgMEBackend::checkForOpenPGP(QString *reason) const
{
    return check(GpgME::OpenPGP, reason);
}

bool Kleo::QGpgMEBackend::checkForSMIME(QString *reason) const
{
    return check(GpgME::CMS, reason);
}

bool Kleo::QGpgMEBackend::checkForProtocol(const char *name, QString *reason) const
{
    if (qstricmp(name, OpenPGP) == 0) {
        return check(GpgME::OpenPGP, reason);
    }
    if (qstricmp(name, SMIME) == 0) {
        return check(GpgME::CMS, reason);
    }
    if (reason) {
        *reason = i18n("Unsupported protocol \"%1\"", QLatin1String(name));
    }
    return false;
}

Kleo::CryptoBackend::Protocol *Kleo::QGpgMEBackend::openpgp() const
{
    if (!mOpenPGPProtocol)
        if (checkForOpenPGP()) {
            mOpenPGPProtocol = new ::Protocol(GpgME::OpenPGP);
        }
    return mOpenPGPProtocol;
}

Kleo::CryptoBackend::Protocol *Kleo::QGpgMEBackend::smime() const
{
    if (!mSMIMEProtocol)
        if (checkForSMIME()) {
            mSMIMEProtocol = new ::Protocol(GpgME::CMS);
        }
    return mSMIMEProtocol;
}

Kleo::CryptoBackend::Protocol *Kleo::QGpgMEBackend::protocol(const char *name) const
{
    if (qstricmp(name, OpenPGP) == 0) {
        return openpgp();
    }
    if (qstricmp(name, SMIME) == 0) {
        return smime();
    }
    return 0;
}

bool Kleo::QGpgMEBackend::supportsProtocol(const char *name) const
{
    return qstricmp(name, OpenPGP) == 0 || qstricmp(name, SMIME) == 0;
}

const char *Kleo::QGpgMEBackend::enumerateProtocols(int i) const
{
    switch (i) {
    case 0: return OpenPGP;
    case 1: return SMIME;
    default: return 0;
    }
}
