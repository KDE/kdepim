/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "managesievetreeview.h"

#include <KLocalizedString>

using namespace KSieveUi;

ManageSieveTreeView::ManageSieveTreeView(QWidget *parent)
    : PimCommon::CustomTreeView(parent)
{
    setDefaultText(i18n("No IMAP server configured..."));
    setRootIsDecorated(true);
    setAlternatingRowColors(true);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setHeaderLabel(i18n("Available Scripts"));
    setContextMenuPolicy(Qt::CustomContextMenu);
}

ManageSieveTreeView::~ManageSieveTreeView()
{
}

void ManageSieveTreeView::setNoImapFound(bool found)
{
    if (mShowDefaultText != found) {
        setDefaultText(i18n("No IMAP server configured..."));
        mShowDefaultText = found;
        update();
    }
}

void ManageSieveTreeView::setNetworkDown(bool state)
{
    if (!state) {
        setDefaultText(i18n("Network down."));
    }
    update();
}

