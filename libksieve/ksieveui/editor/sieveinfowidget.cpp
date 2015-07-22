/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#include "sieveinfowidget.h"

#include <KLocalizedString>

#include <QHBoxLayout>
#include <KTextEdit>

using namespace KSieveUi;
SieveInfoWidget::SieveInfoWidget(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *lay = new QHBoxLayout;
    mInfo = new QTextEdit;
    mInfo->setReadOnly(true);
    mInfo->setAcceptRichText(true);
    lay->addWidget(mInfo);
    setLayout(lay);
}

SieveInfoWidget::~SieveInfoWidget()
{

}

void SieveInfoWidget::setServerInfo(QStringList serverInfos)
{
    serverInfos.sort();
    QString result = QLatin1String("<qt><b>") + i18n("Sieve server supports:") + QLatin1String("</b><ul>");
    Q_FOREACH (const QString &info, serverInfos) {
        result += QLatin1String("<li>") + info;
    }
    result += QLatin1String("</ul></qt>");
    mInfo->setHtml(result);
}

