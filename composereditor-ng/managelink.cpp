/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>

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

#include "managelink.h"
#include <KLineEdit>
#include <KLocale>

#include <QVBoxLayout>
#include <QLabel>

using namespace ComposerEditorNG;

ManageLink::ManageLink(QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n( "Create Link" ) );
    initialize();
}

ManageLink::ManageLink(const QWebElement& element, QWidget *parent)
    : KDialog(parent),
      mWebElement(element)
{
    setCaption( i18n( "Edit Link" ) );
    initialize();
    mLinkLocation->setText(mWebElement.attribute(QLatin1String("href")));
    mLinkText->setText(mWebElement.toInnerXml());
}

ManageLink::~ManageLink()
{
}

void ManageLink::initialize()
{

    setButtons( Ok | Cancel );

    QVBoxLayout *layout = new QVBoxLayout( mainWidget() );

    QLabel *label = new QLabel(i18n("Enter text to display for the link:"));
    layout->addWidget( label );

    mLinkText = new KLineEdit;
    mLinkText->setReadOnly(!mWebElement.isNull());
    mLinkText->setClearButtonShown(true);
    layout->addWidget( mLinkText );

    label = new QLabel(i18n("Enter the location:"));
    layout->addWidget( label );
    mLinkLocation = new KLineEdit;
    mLinkLocation->setClearButtonShown(true);
    layout->addWidget( mLinkLocation );
    connect(this,SIGNAL(okClicked()),this,SLOT(slotOkClicked()));
}

void ManageLink::setLinkText(const QString& link)
{
    mLinkText->setText(link);
}

QString ManageLink::linkText() const
{
    return mLinkText->text();
}

void ManageLink::setLinkLocation(const QString &location)
{
    mLinkLocation->setText(location);
}

QString ManageLink::linkLocation() const
{
    return mLinkLocation->text();
}

void ManageLink::slotOkClicked()
{
    if(!mWebElement.isNull()) {
        if(mLinkLocation->text().isEmpty()) {
            mWebElement.removeAttribute(QLatin1String("href"));
        } else {
            mWebElement.setAttribute(QLatin1String("href"), mLinkLocation->text());
        }
    }
    accept();
}

#include "managelink.moc"
