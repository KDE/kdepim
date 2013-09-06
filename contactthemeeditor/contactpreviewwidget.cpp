/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#include "contactpreviewwidget.h"

#include "kaddressbook/grantlee/grantleecontactformatter.h"
#include "kaddressbook/grantlee/grantleecontactgroupformatter.h"

#include <Akonadi/Contact/ContactGroupViewer>
#include <Akonadi/Contact/ContactViewer>
#include <Akonadi/Item>
#include <KABC/Addressee>
#include <KLocale>

#include <QTabWidget>
#include <QHBoxLayout>

ContactPreviewWidget::ContactPreviewWidget(const QString &projectDirectory, QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);
    mTabWidget = new QTabWidget;

    mContactViewer = new Akonadi::ContactViewer;
    mTabWidget->addTab(mContactViewer, i18n("Contact"));

    mGroupViewer = new Akonadi::ContactGroupViewer;
    mTabWidget->addTab(mGroupViewer, i18n("Group"));

    lay->addWidget(mTabWidget);
    setLayout(lay);

    mFormatter = new Akonadi::GrantleeContactFormatter;

    mContactViewer->setContactFormatter( mFormatter );

    mGroupFormatter = new Akonadi::GrantleeContactGroupFormatter;

    mGroupViewer->setContactGroupFormatter( mGroupFormatter );
}

ContactPreviewWidget::~ContactPreviewWidget()
{
}

void ContactPreviewWidget::updateViewer()
{
    Akonadi::Item item;
    KABC::Addressee contact;

    item.setMimeType( KABC::Addressee::mimeType() );
    contact.setGivenName( QLatin1String("Konqi") );
    contact.setFamilyName( QLatin1String("Kde") );

    item.setPayload<KABC::Addressee>( contact );

    mContactViewer->setContact(item);
    //mGroupViewer->setContactGroup();
    //TODO
}

void ContactPreviewWidget::createScreenShot(const QString &fileName)
{
    //TODO
}

#include "contactpreviewwidget.moc"
