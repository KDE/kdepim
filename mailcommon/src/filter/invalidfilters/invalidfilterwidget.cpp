/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#include "invalidfilterwidget.h"
#include "invalidfilterlistview.h"
#include <KLocalizedString>
#include <QVBoxLayout>
#include <QLabel>

using namespace MailCommon;

InvalidFilterWidget::InvalidFilterWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox->setMargin(0);
    QLabel *lab = new QLabel(i18n("The following filters are invalid (e.g. containing no actions "
                                  "or no search rules). Discard or edit invalid filters?"));
    lab->setWordWrap(true);
    lab->setObjectName(QStringLiteral("label"));
    vbox->addWidget(lab);

    mInvalidFilterListWidget = new InvalidFilterListView(this);
    mInvalidFilterListWidget->setObjectName(QStringLiteral("invalidfilterlist"));
    connect(mInvalidFilterListWidget, &InvalidFilterListView::showDetails, this, &InvalidFilterWidget::showDetails);
    connect(mInvalidFilterListWidget, &InvalidFilterListView::hideInformationWidget, this, &InvalidFilterWidget::hideInformationWidget);
    vbox->addWidget(mInvalidFilterListWidget);
}

InvalidFilterWidget::~InvalidFilterWidget()
{

}

void InvalidFilterWidget::setInvalidFilters(const QVector<InvalidFilterInfo> &lst)
{
    mInvalidFilterListWidget->setInvalidFilters(lst);
}
