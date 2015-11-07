/*
    messagebox.cpp

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004 Klarälvdalens Datakonsult AB

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

#include "messagebox.h"
#include "auditlogviewer.h"

#include <QDebug>
#include "kleo_ui_debug.h"
#include "libkleo/job.h"

#include <gpgme++/signingresult.h>
#include <gpgme++/encryptionresult.h>

#include <QFileDialog>

#include <QPushButton>
#include <qdialog.h>
#include <qdialogbuttonbox.h>
#include <KLocalizedString>
#include <QSaveFile>
#include <kguiitem.h>
#include <ktextedit.h>

#include <qtextstream.h>

#include <gpg-error.h>
#include <KSharedConfig>

using namespace Kleo;
using namespace Kleo::Private;
using namespace GpgME;

// static
void MessageBox::auditLog(QWidget *parent, const Job *job, const QString &caption)
{

    if (!job) {
        return;
    }

    if (!GpgME::hasFeature(AuditLogFeature, 0) || !job->isAuditLogSupported()) {
        KMessageBox::information(parent, i18n("Your system does not have support for GnuPG Audit Logs"),
                                 i18n("System Error"));
        return;
    }

    const GpgME::Error err = job->auditLogError();

    if (err && err.code() != GPG_ERR_NO_DATA) {
        KMessageBox::information(parent, i18n("An error occurred while trying to retrieve the GnuPG Audit Log:\n%1",
                                              QString::fromLocal8Bit(err.asString())),
                                 i18n("GnuPG Audit Log Error"));
        return;
    }

    const QString log = job->auditLogAsHtml();

    if (log.isEmpty()) {
        KMessageBox::information(parent, i18n("No GnuPG Audit Log available for this operation."),
                                 i18n("No GnuPG Audit Log"));
        return;
    }

    auditLog(parent, log, caption);
}

// static
void MessageBox::auditLog(QWidget *parent, const QString &log, const QString &caption)
{
    AuditLogViewer *const alv = new AuditLogViewer(log, parent);
    alv->setAttribute(Qt::WA_DeleteOnClose);
    alv->setObjectName(QStringLiteral("alv"));
    alv->setWindowTitle(caption);
    alv->show();
}

// static
void MessageBox::auditLog(QWidget *parent, const Job *job)
{
    auditLog(parent, job, i18n("GnuPG Audit Log Viewer"));
}

// static
void MessageBox::auditLog(QWidget *parent, const QString &log)
{
    auditLog(parent, log, i18n("GnuPG Audit Log Viewer"));
}

static QString to_information_string(const SigningResult &result)
{
    return result.error()
           ? i18n("Signing failed: %1", QString::fromLocal8Bit(result.error().asString()))
           : i18n("Signing successful");
}

static QString to_error_string(const SigningResult &result)
{
    return to_information_string(result);
}

static QString to_information_string(const EncryptionResult &result)
{
    return result.error()
           ? i18n("Encryption failed: %1", QString::fromLocal8Bit(result.error().asString()))
           : i18n("Encryption successful");
}

static QString to_error_string(const EncryptionResult &result)
{
    return to_information_string(result);
}

static QString to_information_string(const SigningResult &sresult, const EncryptionResult &eresult)
{
    return to_information_string(sresult) + QLatin1Char('\n') + to_information_string(eresult);
}

static QString to_error_string(const SigningResult &sresult, const EncryptionResult &eresult)
{
    return to_information_string(sresult, eresult);
}

// static
void MessageBox::information(QWidget *parent, const SigningResult &result, const Job *job, KMessageBox::Options options)
{
    information(parent, result, job, i18n("Signing Result"), options);
}

// static
void MessageBox::information(QWidget *parent, const SigningResult &result, const Job *job, const QString &caption, KMessageBox::Options options)
{
    make(parent, QMessageBox::Information, to_information_string(result), job, caption, options);
}

// static
void MessageBox::error(QWidget *parent, const SigningResult &result, const Job *job, KMessageBox::Options options)
{
    error(parent, result, job, i18n("Signing Error"), options);
}

// static
void MessageBox::error(QWidget *parent, const SigningResult &result, const Job *job, const QString &caption, KMessageBox::Options options)
{
    make(parent, QMessageBox::Critical, to_error_string(result), job, caption, options);
}

// static
void MessageBox::information(QWidget *parent, const EncryptionResult &result, const Job *job, KMessageBox::Options options)
{
    information(parent, result, job, i18n("Encryption Result"), options);
}

// static
void MessageBox::information(QWidget *parent, const EncryptionResult &result, const Job *job, const QString &caption, KMessageBox::Options options)
{
    make(parent, QMessageBox::Information, to_information_string(result), job, caption, options);
}

// static
void MessageBox::error(QWidget *parent, const EncryptionResult &result, const Job *job, KMessageBox::Options options)
{
    error(parent, result, job, i18n("Encryption Error"), options);
}

// static
void MessageBox::error(QWidget *parent, const EncryptionResult &result, const Job *job, const QString &caption, KMessageBox::Options options)
{
    make(parent, QMessageBox::Critical, to_error_string(result), job, caption, options);
}

// static
void MessageBox::information(QWidget *parent, const SigningResult &sresult, const EncryptionResult &eresult, const Job *job, KMessageBox::Options options)
{
    information(parent, sresult, eresult, job, i18n("Encryption Result"), options);
}

// static
void MessageBox::information(QWidget *parent, const SigningResult &sresult, const EncryptionResult &eresult, const Job *job, const QString &caption, KMessageBox::Options options)
{
    make(parent, QMessageBox::Information, to_information_string(sresult, eresult), job, caption, options);
}

// static
void MessageBox::error(QWidget *parent, const SigningResult &sresult, const EncryptionResult &eresult, const Job *job, KMessageBox::Options options)
{
    error(parent, sresult, eresult, job, i18n("Encryption Error"), options);
}

// static
void MessageBox::error(QWidget *parent, const SigningResult &sresult, const EncryptionResult &eresult, const Job *job, const QString &caption, KMessageBox::Options options)
{
    make(parent, QMessageBox::Critical, to_error_string(sresult, eresult), job, caption, options);
}

// static
bool MessageBox::showAuditLogButton(const Kleo::Job *job)
{
    if (!job) {
        qCDebug(KLEO_UI_LOG) << "not showing audit log button (no job instance)";
        return false;
    }
    if (!GpgME::hasFeature(GpgME::AuditLogFeature, 0)) {
        qCDebug(KLEO_UI_LOG) << "not showing audit log button (gpgme too old)";
        return false;
    }
    if (!job->isAuditLogSupported()) {
        qCDebug(KLEO_UI_LOG) << "not showing audit log button (not supported)";
        return false;
    }
    if (job->auditLogError().code() == GPG_ERR_NO_DATA) {
        qCDebug(KLEO_UI_LOG) << "not showing audit log button (GPG_ERR_NO_DATA)";
        return false;
    }
    if (!job->auditLogError() && job->auditLogAsHtml().isEmpty()) {
        qCDebug(KLEO_UI_LOG) << "not showing audit log button (success, but result empty)";
        return false;
    }
    return true;
}

// static
void MessageBox::make(QWidget *parent, QMessageBox::Icon icon, const QString &text, const Job *job, const QString &caption, KMessageBox::Options options)
{
    QDialog *dialog = new QDialog(parent);
    dialog->setWindowTitle(caption);
    QDialogButtonBox *box = new QDialogButtonBox(showAuditLogButton(job) ? (QDialogButtonBox::Yes | QDialogButtonBox::No) : QDialogButtonBox::Yes, parent);
    QPushButton *yesButton = box->button(QDialogButtonBox::Yes);
    yesButton->setDefault(true);
    //dialog->setEscapeButton(KDialog::Yes);
    dialog->setObjectName(QStringLiteral("error"));
    dialog->setModal(true);
    KGuiItem::assign(yesButton, KStandardGuiItem::ok());
    if (GpgME::hasFeature(GpgME::AuditLogFeature, 0)) {
        KGuiItem::assign(box->button(QDialogButtonBox::No), KGuiItem(i18n("&Show Audit Log")));
    }

    if (QDialogButtonBox::No == KMessageBox::createKMessageBox(dialog, box, icon, text, QStringList(), QString(), 0, options)) {
        auditLog(0, job);
    }
}
