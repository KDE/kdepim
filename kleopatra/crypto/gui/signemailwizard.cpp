/* -*- mode: c++; c-basic-offset:4 -*-
    crypto/gui/signemailwizard.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2008 Klarälvdalens Datakonsult AB

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

#include <config-kleopatra.h>

#include "signemailwizard.h"

#include "signerresolvepage.h"

#include <utils/formatting.h>

#include <KLocalizedString>

#include <gpgme++/key.h>

#include <cassert>

using namespace Kleo;
using namespace Kleo::Crypto::Gui;
using namespace GpgME;

namespace
{

class SignerResolveValidator : public SignerResolvePage::Validator
{
public:
    explicit SignerResolveValidator(SignerResolvePage *page);
    bool isComplete() const;
    QString explanation() const;
    void update() const;
    QString customWindowTitle() const
    {
        return QString();
    }

private:
    SignerResolvePage *m_page;
    mutable QString expl;
    mutable bool complete;
};
}

SignerResolveValidator::SignerResolveValidator(SignerResolvePage *page) : SignerResolvePage::Validator(), m_page(page), complete(true)
{
    assert(m_page);
}

void SignerResolveValidator::update() const
{
    const bool haveSelected = !m_page->selectedProtocols().empty();
    const std::vector<Protocol> missing = m_page->selectedProtocolsWithoutSigningCertificate();

    complete = haveSelected && missing.empty();
    expl.clear();
    if (complete) {
        return;
    }
    if (!haveSelected) {
        expl = i18n("You need to select a signing certificate to proceed.");
        return;
    }

    assert(missing.size() <= 2);
    if (missing.size() == 1) {
        expl = i18n("You need to select an %1 signing certificate to proceed.", Formatting::displayName(missing[0]));
    } else {
        expl = i18n("You need to select %1 and %2 signing certificates to proceed.", Formatting::displayName(missing[0]), Formatting::displayName(missing[1]));
    }
}

QString SignerResolveValidator::explanation() const
{
    update();
    return expl;
}

bool SignerResolveValidator::isComplete() const
{
    update();
    return complete;
}

class SignEMailWizard::Private
{
    friend class ::Kleo::Crypto::Gui::SignEMailWizard;
    SignEMailWizard *const q;
public:
    explicit Private(SignEMailWizard *qq);
    ~Private();

    void operationSelected();

    bool m_quickMode;
};

SignEMailWizard::Private::Private(SignEMailWizard *qq)
    : q(qq), m_quickMode(false)
{
    q->setWindowTitle(i18n("Sign Mail Message"));

    std::vector<int> pageOrder;
    q->setSignerResolvePageValidator(boost::shared_ptr<SignerResolveValidator>(new SignerResolveValidator(q->signerResolvePage())));
    pageOrder.push_back(SignEncryptWizard::ResolveSignerPage);
    pageOrder.push_back(SignEncryptWizard::ResultPage);
    q->setPageOrder(pageOrder);
    q->setCommitPage(SignEncryptWizard::ResolveSignerPage);
    q->setEncryptionSelected(false);
    q->setEncryptionUserMutable(false);
    q->setSigningSelected(true);
    q->setSigningUserMutable(false);
    q->signerResolvePage()->setProtocolSelectionUserMutable(false);
    q->setMultipleProtocolsAllowed(false);
}

SignEMailWizard::Private::~Private() {}

SignEMailWizard::SignEMailWizard(QWidget *parent, Qt::WindowFlags f)
    : SignEncryptWizard(parent, f), d(new Private(this))
{
}

bool SignEMailWizard::quickMode() const
{
    return d->m_quickMode;
}

void SignEMailWizard::setQuickMode(bool quick)
{
    if (quick == d->m_quickMode) {
        return;
    }
    d->m_quickMode = quick;
    signerResolvePage()->setAutoAdvance(quick);
    setKeepResultPageOpenWhenDone(!quick);
}

SignEMailWizard::~SignEMailWizard() {}

