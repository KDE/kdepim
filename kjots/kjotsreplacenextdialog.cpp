//
//  kjots
//
//  Copyright (C) 2008 Stephen Kelly <steveire@gmail.com>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#include "kjotsreplacenextdialog.h"

#include <QLabel>

#include <KLocalizedString>

KJotsReplaceNextDialog::KJotsReplaceNextDialog(QWidget *parent) :
    KDialog(parent), m_answer(Close)
{
    setModal(true);
    setCaption(i18n("Replace"));
    setButtons(User3 | User2 | User1 | Close);
    setButtonGuiItem(User1, KGuiItem(i18n("&All")));
    setButtonGuiItem(User2, KGuiItem(i18n("&Skip")));
    setButtonGuiItem(User3, KGuiItem(i18n("Replace")));
    setDefaultButton(User3);
    showButtonSeparator(false);

    m_mainLabel = new QLabel(this);
    setMainWidget(m_mainLabel);

    connect(this, SIGNAL(user1Clicked()), SLOT(onHandleAll()));
    connect(this, SIGNAL(user2Clicked()), SLOT(onHandleSkip()));
    connect(this, SIGNAL(user3Clicked()), SLOT(onHandleReplace()));
}

void KJotsReplaceNextDialog::setLabel(const QString &pattern, const QString &replacement)
{
    m_mainLabel->setText(i18n("Replace '%1' with '%2'?", pattern, replacement));
}

void KJotsReplaceNextDialog::onHandleAll()
{
    m_answer = User1;
    accept();
}

void KJotsReplaceNextDialog::onHandleSkip()
{
    m_answer = User2;
    accept();
}

void KJotsReplaceNextDialog::onHandleReplace()
{
    m_answer = User3;
    accept();
}

