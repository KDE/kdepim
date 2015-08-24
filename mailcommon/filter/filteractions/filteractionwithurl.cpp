/*
 * Copyright (c) 1996-1998 Stefan Taferner <taferner@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "filteractionwithurl.h"

#include <KUrlRequester>
#include <KLocalizedString>
#include <QUrl>

#include <QHBoxLayout>
#include <QWhatsThis>
#include <QTextDocument>

using namespace MailCommon;

FilterActionWithUrlHelpButton::FilterActionWithUrlHelpButton(QWidget *parent)
    : QToolButton(parent)
{
    setToolTip(i18n("Help"));
    setIcon(QIcon::fromTheme(QStringLiteral("help-hint")));
}

FilterActionWithUrlHelpButton::~FilterActionWithUrlHelpButton()
{

}

FilterActionWithUrl::FilterActionWithUrl(const QString &name, const QString &label, QObject *parent)
    : FilterAction(name, label, parent),
      mHelpButton(Q_NULLPTR)
{
}

FilterActionWithUrl::~FilterActionWithUrl()
{
}

bool FilterActionWithUrl::isEmpty() const
{
    return mParameter.trimmed().isEmpty();
}

QWidget *FilterActionWithUrl::createParamWidget(QWidget *parent) const
{
    QWidget *widget = new QWidget(parent);
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);
    widget->setLayout(layout);
    KUrlRequester *requester = new KUrlRequester(parent);
    requester->setUrl(QUrl::fromLocalFile(mParameter));
    requester->setObjectName(QStringLiteral("requester"));
    layout->addWidget(requester);
    mHelpButton = new FilterActionWithUrlHelpButton(parent);
    mHelpButton->setObjectName(QStringLiteral("helpbutton"));
    connect(mHelpButton, &QAbstractButton::clicked, this, &FilterActionWithUrl::slotHelp);
    layout->addWidget(mHelpButton);

    connect(requester, &KUrlRequester::textChanged, this, &FilterActionWithUrl::filterActionModified);

    return widget;
}

void FilterActionWithUrl::slotHelp()
{
    QString fullWhatsThis = i18n("You can get specific header when you use %{headername}.");
    QWhatsThis::showText(QCursor::pos(), fullWhatsThis, mHelpButton);
}

void FilterActionWithUrl::applyParamWidgetValue(QWidget *paramWidget)
{
    KUrlRequester *requester = paramWidget->findChild<KUrlRequester *>(QStringLiteral("requester"));
    Q_ASSERT(requester);

    if (QUrl(requester->text()).isRelative()) {
        mParameter = requester->text();
    } else {
        const QUrl url = requester->url();
        mParameter = (url.isLocalFile() ? url.toLocalFile() : url.path());
    }
}

void FilterActionWithUrl::setParamWidgetValue(QWidget *paramWidget) const
{
    KUrlRequester *requester = paramWidget->findChild<KUrlRequester *>(QStringLiteral("requester"));
    Q_ASSERT(requester);

    requester->setUrl(QUrl::fromLocalFile(mParameter));
}

void FilterActionWithUrl::clearParamWidget(QWidget *paramWidget) const
{
    KUrlRequester *requester = paramWidget->findChild<KUrlRequester *>(QStringLiteral("requester"));
    Q_ASSERT(requester);
    requester->clear();
}

void FilterActionWithUrl::argsFromString(const QString &argsStr)
{
    mParameter = argsStr;
}

QString FilterActionWithUrl::argsAsString() const
{
    return mParameter;
}

QString FilterActionWithUrl::displayString() const
{
    return label() + QLatin1String(" \"") + argsAsString().toHtmlEscaped() + QLatin1String("\"");
}

