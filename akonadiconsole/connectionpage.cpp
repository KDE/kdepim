/*
    This file is part of Akonadi.

    Copyright (c) 2006 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#include "connectionpage.h"

#include <KTextEdit>

#include <QVBoxLayout>
#include <QFontDatabase>

#include "tracernotificationinterface.h"

ConnectionPage::ConnectionPage(const QString &identifier, QWidget *parent)
    : QWidget(parent), mIdentifier(identifier), mShowAllConnections(false)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    mDataView = new KTextEdit(this);
    mDataView->setReadOnly(true);
    mDataView->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    layout->addWidget(mDataView);

    org::freedesktop::Akonadi::TracerNotification *iface = new org::freedesktop::Akonadi::TracerNotification(QString(), QStringLiteral("/tracing/notifications"), QDBusConnection::sessionBus(), this);

    connect(iface, &OrgFreedesktopAkonadiTracerNotificationInterface::connectionDataInput,
            this, &ConnectionPage::connectionDataInput);
    connect(iface, &OrgFreedesktopAkonadiTracerNotificationInterface::connectionDataOutput,
            this, &ConnectionPage::connectionDataOutput);
}

void ConnectionPage::connectionDataInput(const QString &identifier, const QString &msg)
{
    QString str = QStringLiteral("<font color=\"green\">%2</font>").arg(identifier) + QLatin1Char(' ');
    if (mShowAllConnections || identifier == mIdentifier) {
        str += QStringLiteral("<font color=\"red\">%1</font>").arg(msg.toHtmlEscaped());
        mDataView->append(str);
    }
}

void ConnectionPage::connectionDataOutput(const QString &identifier, const QString &msg)
{
    QString str = QStringLiteral("<font color=\"green\">%2</font>").arg(identifier) + QLatin1Char(' ');
    if (mShowAllConnections || identifier == mIdentifier) {
        str += msg.toHtmlEscaped();
        mDataView->append(str);
    }
}

void ConnectionPage::showAllConnections(bool show)
{
    mShowAllConnections = show;
}

QString ConnectionPage::toHtml() const
{
    return mDataView->toHtml();
}

void ConnectionPage::clear()
{
    mDataView->clear();
}

