/* -*- mode: c++; c-basic-offset:4 -*-
    importcertificatefromclipboardcommand.cpp

    This clipboard is part of Kleopatra, the KDE keymanager
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
    Qt.  If you modify this clipboard, you may extend this exception to
    your version of the clipboard, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include <config-kleopatra.h>

#include "importcertificatefromclipboardcommand.h"

#ifndef QT_NO_CLIPBOARD

#include "importcertificatescommand_p.h"

#include "utils/classify.h"

#include <gpgme++/global.h>

#include <KLocalizedString>

#include <QByteArray>
#include <QClipboard>
#include <QApplication>
#include <QMimeData>

using namespace GpgME;
using namespace Kleo;

class ImportCertificateFromClipboardCommand::Private : public ImportCertificatesCommand::Private
{
    friend class ::ImportCertificateFromClipboardCommand;
    ImportCertificateFromClipboardCommand *q_func() const
    {
        return static_cast<ImportCertificateFromClipboardCommand *>(q);
    }
public:
    explicit Private(ImportCertificateFromClipboardCommand *qq, KeyListController *c);
    ~Private();

    bool ensureHaveClipboard();

private:
    QByteArray input;
};

ImportCertificateFromClipboardCommand::Private *ImportCertificateFromClipboardCommand::d_func()
{
    return static_cast<Private *>(d.get());
}
const ImportCertificateFromClipboardCommand::Private *ImportCertificateFromClipboardCommand::d_func() const
{
    return static_cast<const Private *>(d.get());
}

ImportCertificateFromClipboardCommand::Private::Private(ImportCertificateFromClipboardCommand *qq, KeyListController *c)
    : ImportCertificatesCommand::Private(qq, c)
{

}

ImportCertificateFromClipboardCommand::Private::~Private() {}

// static
bool ImportCertificateFromClipboardCommand::canImportCurrentClipboard()
{
    if (const QClipboard *clip = QApplication::clipboard())
        if (const QMimeData *mime = clip->mimeData())
            return mime->hasText()
                   && mayBeAnyCertStoreType(classifyContent(mime->text().toUtf8()));
    return false;
}

#define d d_func()
#define q q_func()

ImportCertificateFromClipboardCommand::ImportCertificateFromClipboardCommand(KeyListController *p)
    : ImportCertificatesCommand(new Private(this, p))
{

}

ImportCertificateFromClipboardCommand::ImportCertificateFromClipboardCommand(QAbstractItemView *v, KeyListController *p)
    : ImportCertificatesCommand(v, new Private(this, p))
{

}

ImportCertificateFromClipboardCommand::~ImportCertificateFromClipboardCommand() {}

void ImportCertificateFromClipboardCommand::doStart()
{

    if (!d->ensureHaveClipboard()) {
        emit canceled();
        d->finished();
        return;
    }

    d->setWaitForMoreJobs(true);
    const unsigned int classification = classifyContent(d->input);
    if (!mayBeAnyCertStoreType(classification)) {
        d->error(i18n("Clipboard contents do not look like a certificate."),
                 i18n("Certificate Import Failed"));
    } else {
        const GpgME::Protocol protocol = findProtocol(classification);
        if (protocol == GpgME::UnknownProtocol) {
            d->error(i18n("Could not determine certificate type of clipboard contents."), i18n("Certificate Import Failed"));
        } else {
            d->startImport(protocol, d->input, i18n("Clipboard"));
        }
    }
    d->setWaitForMoreJobs(false);
}

bool ImportCertificateFromClipboardCommand::Private::ensureHaveClipboard()
{
    if (input.isEmpty())
        if (const QClipboard *cb = qApp->clipboard()) {
            input = cb->text().toUtf8();
        }
    return !input.isEmpty();
}

#undef d
#undef q

#endif // QT_NO_CLIPBOARD
