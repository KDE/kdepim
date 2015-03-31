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
#include "invalidfilterlistwidget.h"
#include <KLocalizedString>
#include <QVBoxLayout>
#include <QLabel>

using namespace MailCommon;

InvalidFilterWidget::InvalidFilterWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *vbox = new QVBoxLayout(this);
    //KF5 add i18n
    QLabel *lab = new QLabel(QLatin1String("text"));
    lab->setObjectName(QLatin1String("label"));
    vbox->addWidget(lab);

    mInvalidFilterListWidget = new InvalidFilterListWidget(this);
    mInvalidFilterListWidget->setObjectName(QLatin1String("invalidfilterlist"));
    vbox->addWidget(mInvalidFilterListWidget);
}

InvalidFilterWidget::~InvalidFilterWidget()
{

}

void InvalidFilterWidget::setInvalidFilter(const QStringList &lst)
{
    mInvalidFilterListWidget->setInvalidFilter(lst);
}
