/*
    Copyright (c) 2016 Klarälvdalens Datakonsult AB

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "cryptopage.h"
#include "dialog.h"

#include <Libkleo/DefaultKeyFilter>
#include <Libkleo/Job>
#include <Libkleo/ImportJob>
#include <Libkleo/CryptoBackendFactory>
#include <Libkleo/DefaultKeyGenerationJob>
#include <Libkleo/ProgressDialog>
#include <Libkleo/Classify>

#include <gpgme++/context.h>
#include <gpgme++/keygenerationresult.h>
#include <gpgme++/importresult.h>

#include <KMessageBox>

#include <QFileDialog>

class KeyGenerationJob : public Kleo::Job
{
    Q_OBJECT

public:
    KeyGenerationJob(const QString &name, const QString &email, Kleo::KeySelectionCombo *parent)
        : Kleo::Job(parent)
        , mName(name)
        , mEmail(email)
        , mJob(Q_NULLPTR)
    {
    }

    ~KeyGenerationJob()
    {
    }

    void slotCancel() Q_DECL_OVERRIDE {
        if (mJob)
        {
            mJob->slotCancel();
        }
    }

    void start()
    {
        auto job = new Kleo::DefaultKeyGenerationJob(this);
        connect(job, &Kleo::DefaultKeyGenerationJob::result,
                this, &KeyGenerationJob::keyGenerated);
        job->start(mEmail, mName);
        mJob = job;
    }

    void keyGenerated(const GpgME::KeyGenerationResult &result)
    {
        mJob = Q_NULLPTR;
        if (result.error()) {
            KMessageBox::error(qobject_cast<QWidget *>(parent()),
                               i18n("Error while generating new key pair: %1", QString::fromUtf8(result.error().asString())),
                               i18n("Key Generation Error"));
            Q_EMIT done();
            return;
        }

        auto combo = qobject_cast<Kleo::KeySelectionCombo *>(parent());
        combo->setDefaultKey(QLatin1String(result.fingerprint()));
        connect(combo, &Kleo::KeySelectionCombo::keyListingFinished,
                this, &KeyGenerationJob::done);
        combo->refreshKeys();
    }

private:
    QString mName;
    QString mEmail;
    Kleo::Job *mJob;
};

class KeyImportJob : public Kleo::Job
{
    Q_OBJECT
public:
    KeyImportJob(const QString &file, Kleo::KeySelectionCombo *parent)
        : Kleo::Job(parent)
        , mFile(file)
        , mJob(Q_NULLPTR)
    {
    }

    ~KeyImportJob()
    {
    }

    void slotCancel() Q_DECL_OVERRIDE {
        if (mJob)
        {
            mJob->slotCancel();
        }
    }

    void start()
    {
        Kleo::ImportJob *job = Q_NULLPTR;
        switch (Kleo::findProtocol(mFile)) {
        case GpgME::OpenPGP:
            job = Kleo::CryptoBackendFactory::instance()->openpgp()->importJob();
            break;
        case GpgME::CMS:
            job = Kleo::CryptoBackendFactory::instance()->smime()->importJob();
            break;
        default:
            job = Q_NULLPTR;
            break;
        }

        if (!job) {
            KMessageBox::error(qobject_cast<QWidget *>(parent()),
                               i18n("Could not detect valid key type"),
                               i18n("Import error"));
            Q_EMIT done();
            return;
        }

        QFile keyFile(mFile);
        if (!keyFile.open(QIODevice::ReadOnly)) {
            KMessageBox::error(qobject_cast<QWidget *>(parent()),
                               i18n("Cannot read data from the certificate file: %1", keyFile.errorString()),
                               i18n("Import error"));
            Q_EMIT done();
            return;
        }

        connect(job, &Kleo::ImportJob::result,
                this, &KeyImportJob::keyImported);
        job->start(keyFile.readAll());
        mJob = job;
    }

    void keyImported(const GpgME::ImportResult &result)
    {
        mJob = Q_NULLPTR;
        if (result.error()) {
            KMessageBox::error(qobject_cast<QWidget *>(parent()),
                               i18n("Failed to import key: %1", QString::fromUtf8(result.error().asString())),
                               i18n("Import error"));
            Q_EMIT done();
            return;
        }

        const auto imports = result.imports();
        if (imports.size() == 0) {
            KMessageBox::error(qobject_cast<QWidget *>(parent()),
                               i18n("Failed to import key."),
                               i18n("Import error"));
            Q_EMIT done();
            return;
        }

        auto combo = qobject_cast<Kleo::KeySelectionCombo *>(parent());
        combo->setDefaultKey(QLatin1String(result.import(0).fingerprint()));
        connect(combo, &Kleo::KeySelectionCombo::keyListingFinished,
                this, &KeyGenerationJob::done);
        combo->refreshKeys();
    }

private:
    QString mFile;
    Kleo::Job *mJob;
};

CryptoPage::CryptoPage(Dialog *parent)
    : Page(parent)
    , mSetupManager(parent->setupManager())
{
    ui.setupUi(this);
    // TODO: Enable once we implement key publishing
    ui.publishCheckbox->setChecked(false);
    ui.publishCheckbox->setEnabled(false);

    boost::shared_ptr<Kleo::DefaultKeyFilter> filter(new Kleo::DefaultKeyFilter);
    filter->setCanSign(Kleo::DefaultKeyFilter::Set);
    filter->setCanEncrypt(Kleo::DefaultKeyFilter::Set);
    filter->setHasSecret(Kleo::DefaultKeyFilter::Set);
    ui.keyCombo->setKeyFilter(filter);
    ui.keyCombo->prependCustomItem(QIcon(), i18n("No key"), NoKey);
    ui.keyCombo->appendCustomItem(QIcon::fromTheme(QStringLiteral("document-new")),
                                  i18n("Generate a new key pair"), GenerateKey);
    ui.keyCombo->appendCustomItem(QIcon::fromTheme(QStringLiteral("document-import")),
                                  i18n("Import a key"), ImportKey);

    connect(ui.keyCombo, &Kleo::KeySelectionCombo::customItemSelected,
            this, &CryptoPage::customItemSelected);
    connect(ui.keyCombo, &Kleo::KeySelectionCombo::currentKeyChanged,
    this, [this](const GpgME::Key & key) {
        setValid(!key.isNull());
    });
}

void CryptoPage::enterPageNext()
{
    ui.keyCombo->setIdFilter(mSetupManager->email());
}

void CryptoPage::leavePageNext()
{
    mSetupManager->setKey(ui.keyCombo->currentKey());
}

void CryptoPage::customItemSelected(const QVariant &data)
{
    switch (data.toInt()) {
    case NoKey:
        setValid(true);
        return;
    case GenerateKey:
        setValid(false);
        generateKeyPair();
        break;
    case ImportKey:
        setValid(false);
        importKey();
        break;
    }
}

namespace
{

template<typename T, typename ... Args>
void runJob(Kleo::KeySelectionCombo *combo, const QString &title, const Args   &... args)
{
    auto job = new T(args ..., combo);
    new Kleo::ProgressDialog(job, title, combo->parentWidget());
    combo->setEnabled(false);
    QObject::connect(job, &KeyGenerationJob::done,
    combo, [combo]() {
        combo->setEnabled(true);
    });
    job->start();
}

}

void CryptoPage::generateKeyPair()
{
    runJob<KeyGenerationJob>(ui.keyCombo, i18n("Generating new key pair..."),
                             mSetupManager->name(), mSetupManager->email());
}

void CryptoPage::importKey()
{
    const QString certificateFilter = i18n("Certificates") + QLatin1String(" (*.asc *.cer *.cert *.crt *.der *.pem *.gpg *.p7c *.p12 *.pfx *.pgp)");
    const QString anyFilesFilter = i18n("Any files") + QLatin1String(" (*)");

    const QString file = QFileDialog::getOpenFileName(this, i18n("Select Certificate File"),
                         QString(),
                         certificateFilter + QLatin1String(";;") + anyFilesFilter);
    if (file.isEmpty()) {
        return;
    }

    runJob<KeyImportJob>(ui.keyCombo, i18n("Importing key..."), file);
}

#include "cryptopage.moc"
