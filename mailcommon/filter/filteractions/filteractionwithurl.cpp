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
    setIcon( QIcon::fromTheme( QLatin1String("help-hint") ) );
}

FilterActionWithUrlHelpButton::~FilterActionWithUrlHelpButton()
{

}

FilterActionWithUrl::FilterActionWithUrl( const QString &name, const QString &label, QObject *parent )
    : FilterAction( name, label, parent )
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
    widget->setLayout(layout);
    KUrlRequester *requester = new KUrlRequester( parent );
    requester->setUrl( QUrl::fromLocalFile( mParameter ) );
    layout->addWidget(requester);
    mHelpButton = new FilterActionWithUrlHelpButton(parent);
    connect(mHelpButton, SIGNAL(clicked()), this, SLOT(slotHelp()));
    layout->addWidget(mHelpButton);

    connect(requester, &KUrlRequester::textChanged, this, &FilterActionWithUrl::filterActionModified);

    return widget;
}

void FilterActionWithUrl::slotHelp()
{
    QString fullWhatsThis = i18n("You can get specific header when when you use %{headername}.");
    QWhatsThis::showText( QCursor::pos(), fullWhatsThis, mHelpButton );
}

void FilterActionWithUrl::applyParamWidgetValue( QWidget *paramWidget )
{
    const QUrl url = static_cast<KUrlRequester *>(paramWidget)->url();

    mParameter = (url.isLocalFile() ? url.toLocalFile() : url.path());
}

void FilterActionWithUrl::setParamWidgetValue(QWidget *paramWidget) const
{
    static_cast<KUrlRequester *>(paramWidget)->setUrl(QUrl::fromLocalFile(mParameter));
}

void FilterActionWithUrl::clearParamWidget(QWidget *paramWidget) const
{
    static_cast<KUrlRequester *>(paramWidget)->clear();
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

