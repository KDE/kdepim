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

#include <KDE/KUrlRequester>

#include <QHBoxLayout>
#include <QWhatsThis>
#include <QTextDocument>

using namespace MailCommon;

FilterActionWithUrlHelpButton::FilterActionWithUrlHelpButton(QWidget *parent)
    : QToolButton(parent)
{
    //KF5 add i18n
    setToolTip(QLatin1String("Help"));
    setIcon( KIcon( QLatin1String("help-hint") ) );
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

QWidget* FilterActionWithUrl::createParamWidget( QWidget *parent ) const
{
    QWidget *widget = new QWidget(parent);
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);
    widget->setLayout(layout);
    KUrlRequester *requester = new KUrlRequester( parent );
    requester->setObjectName(QLatin1String("requester"));
    requester->setUrl( KUrl( mParameter ) );
    layout->addWidget(requester);
    mHelpButton = new FilterActionWithUrlHelpButton(parent);
    mHelpButton->setObjectName(QLatin1String("helpbutton"));
    connect(mHelpButton, SIGNAL(clicked()), this, SLOT(slotHelp()));
    layout->addWidget(mHelpButton);

    connect( requester, SIGNAL(textChanged(QString)), this, SIGNAL(filterActionModified()) );

    return widget;
}

void FilterActionWithUrl::slotHelp()
{
    //KF5 add i18n / Improve it
    QString fullWhatsThis = QLatin1String("You can get specific header when when you use %{headername}.");
    QWhatsThis::showText( QCursor::pos(), fullWhatsThis, mHelpButton );
}

void FilterActionWithUrl::applyParamWidgetValue( QWidget *paramWidget )
{
    KUrlRequester *requester = paramWidget->findChild<KUrlRequester*>( QLatin1String("requester") );
    Q_ASSERT( requester );

    if (QUrl(requester->text()).isRelative()) {
        mParameter = requester->text();
    } else {
        const KUrl url = requester->url();
        mParameter = (url.isLocalFile() ? url.toLocalFile() : url.path());
    }
}

void FilterActionWithUrl::setParamWidgetValue( QWidget *paramWidget ) const
{
    KUrlRequester *requester = paramWidget->findChild<KUrlRequester*>( QLatin1String("requester") );
    Q_ASSERT( requester );
    requester->setUrl( KUrl( mParameter ) );
}

void FilterActionWithUrl::clearParamWidget( QWidget *paramWidget ) const
{
    KUrlRequester *requester = paramWidget->findChild<KUrlRequester*>( QLatin1String("requester") );
    Q_ASSERT( requester );
    requester->clear();
}

void FilterActionWithUrl::argsFromString( const QString &argsStr )
{
    mParameter = argsStr;
}

QString FilterActionWithUrl::argsAsString() const
{
    return mParameter;
}

QString FilterActionWithUrl::displayString() const
{
    return label() + QLatin1String( " \"" ) + Qt::escape( argsAsString() ) + QLatin1String( "\"" );
}


