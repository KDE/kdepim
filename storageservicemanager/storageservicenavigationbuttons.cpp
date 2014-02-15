/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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

#include "storageservicenavigationbuttons.h"

#include <KLocalizedString>
#include <KIcon>
#include <KStandardShortcut>

#include <QHBoxLayout>
#include <QAction>

StorageServiceNavigationButtons::StorageServiceNavigationButtons(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *hbox = new QHBoxLayout;
    setLayout(hbox);

    mGoBack = new QAction(KIcon(QLatin1String("go-previous")),i18n("Back"), this);
    connect(mGoBack, SIGNAL(triggered()), SLOT(slotGoBackClicked()));
    mGoBack->setShortcuts( KStandardShortcut::shortcut(KStandardShortcut::Back) );
    mGoBack->setEnabled(false);

    mGoForward = new QAction(KIcon(QLatin1String("go-next")),i18n("Forward"), this);
    mGoForward->setShortcuts( KStandardShortcut::shortcut(KStandardShortcut::Forward) );
    connect(mGoForward, SIGNAL(triggered()), SLOT(slotGoForwardClicked()));
    mGoForward->setEnabled(false);
}

QAction *StorageServiceNavigationButtons::goBack() const
{
    return mGoBack;
}

QAction *StorageServiceNavigationButtons::goForward() const
{
    return mGoForward;
}

QList<InformationUrl> StorageServiceNavigationButtons::backUrls() const
{
    return mBackUrls;
}

void StorageServiceNavigationButtons::setBackUrls(const QList<InformationUrl> &value)
{
    if (mBackUrls != value) {
        mBackUrls = value;
        updateButtons();
    }
}

QList<InformationUrl> StorageServiceNavigationButtons::forwardUrls() const
{
    return mForwardUrls;
}

void StorageServiceNavigationButtons::setForwardUrls(const QList<InformationUrl> &value)
{
    if (mForwardUrls != value) {
        mForwardUrls = value;
        updateButtons();
    }
}

void StorageServiceNavigationButtons::clear()
{
    mBackUrls.clear();
    mForwardUrls.clear();
    updateButtons();
}

void StorageServiceNavigationButtons::updateButtons()
{
    mGoForward->setEnabled(!mForwardUrls.isEmpty());
    mGoBack->setEnabled(!mBackUrls.isEmpty());
}

void StorageServiceNavigationButtons::slotGoBackClicked()
{
    if (!mBackUrls.isEmpty()) {
        Q_EMIT changeUrl(mBackUrls.first());
    }
}

void StorageServiceNavigationButtons::slotGoForwardClicked()
{
    if (!mForwardUrls.isEmpty()) {
        Q_EMIT changeUrl(mForwardUrls.first());
    }
}
