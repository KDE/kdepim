/* -*- mode: c++; c-basic-offset:4 -*-
    crypto/gui/wizardpage.cpp

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

#include <config-kleopatra.h>

#include "wizardpage.h"

#include <KGuiItem>

using namespace Kleo::Crypto::Gui;

class WizardPage::Private
{
    friend class ::WizardPage;
    WizardPage *const q;
public:
    explicit Private(WizardPage *qq);
    ~Private();

private:
    bool commitPage;
    bool autoAdvance;
    QString title;
    QString subTitle;
    QString explanation;
    KGuiItem customNextButton;
};

WizardPage::Private::Private(WizardPage *qq)
    : q(qq), commitPage(false), autoAdvance(false)
{

}

WizardPage::Private::~Private() {}

WizardPage::WizardPage(QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f), d(new Private(this))
{

}

bool WizardPage::isCommitPage() const
{
    return d->commitPage;
}

void WizardPage::setCommitPage(bool commitPage)
{
    d->commitPage = commitPage;
}

bool WizardPage::autoAdvance() const
{
    return d->autoAdvance;
}

void WizardPage::setAutoAdvance(bool enabled)
{
    if (d->autoAdvance == enabled) {
        return;
    }
    d->autoAdvance = enabled;
    Q_EMIT autoAdvanceChanged();
}

QString WizardPage::title() const
{
    return d->title;
}

void WizardPage::setTitle(const QString &title)
{
    if (d->title == title) {
        return;
    }
    d->title = title;
    Q_EMIT titleChanged();
}

QString WizardPage::subTitle() const
{
    return d->subTitle;
}

void WizardPage::setSubTitle(const QString &subTitle)
{
    if (d->subTitle == subTitle) {
        return;
    }
    d->subTitle = subTitle;
    Q_EMIT subTitleChanged();
}

QString WizardPage::explanation() const
{
    return d->explanation;
}

void WizardPage::setExplanation(const QString &explanation)
{
    if (d->explanation == explanation) {
        return;
    }
    d->explanation = explanation;
    Q_EMIT explanationChanged();
}

KGuiItem WizardPage::customNextButton() const
{
    return d->customNextButton;
}
void WizardPage::setCustomNextButton(const KGuiItem &item)
{
    d->customNextButton = item;
}

WizardPage::~WizardPage() {}

void WizardPage::onNext() {}

