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
#include "contacteditorutil.h"

#include <KABC/VCardConverter>

#include "kaddressbookgrantlee/formatter/grantleecontactformatter.h"
#include "kaddressbookgrantlee/formatter/grantleecontactgroupformatter.h"

#include <Akonadi/Contact/ContactGroupViewer>
#include <Akonadi/Contact/ContactViewer>
#include <Akonadi/Item>
#include <KLocale>
#include <KGlobal>
#include <KConfigGroup>

#include <QTabWidget>
#include <QHBoxLayout>
#include <QPainter>

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

    mFormatter = new KAddressBookGrantlee::GrantleeContactFormatter;

    mContactViewer->setContactFormatter( mFormatter );

    mGroupFormatter = new KAddressBookGrantlee::GrantleeContactGroupFormatter;

    mGroupViewer->setContactGroupFormatter( mGroupFormatter );

    loadConfig();
    if (!projectDirectory.isEmpty()) {
        mGroupFormatter->setAbsoluteThemePath(projectDirectory);
        mFormatter->setAbsoluteThemePath(projectDirectory);
    }
}

ContactPreviewWidget::~ContactPreviewWidget()
{
}

void ContactPreviewWidget::updateViewer()
{
    mContactViewer->setRawContact(mContact);
    //mGroupViewer->setContactGroup();
    //TODO
}

void ContactPreviewWidget::createScreenShot(const QStringList &fileName)
{
    for (int i = 0; i <fileName.count();++i) {
        if (i == 0) {
            QImage image(mContactViewer->size(), QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::transparent);

            QPainter painter(&image);
            painter.setRenderHint(QPainter::Antialiasing, true);
            painter.setRenderHint(QPainter::TextAntialiasing, true);
            painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
            mContactViewer->render(&painter);
            painter.end();
            image.save(fileName.at(i));
        } else if (i == 1) {
            QImage image(mContactViewer->size(), QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::transparent);

            QPainter painter(&image);
            painter.setRenderHint(QPainter::Antialiasing, true);
            painter.setRenderHint(QPainter::TextAntialiasing, true);
            painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
            mGroupViewer->render(&painter);
            painter.end();
            image.save(fileName.at(i));
        }
    }
}

void ContactPreviewWidget::loadConfig()
{
    KSharedConfig::Ptr config = KGlobal::config();

    if (config->hasGroup(QLatin1String("Global"))) {
        KConfigGroup group = config->group(QLatin1String("Global"));
        const QString defaultContact = group.readEntry("defaultContact",contacteditorutil::defaultContact());
        if (!defaultContact.isEmpty()) {
            KABC::VCardConverter converter;
            mContact = converter.parseVCard( defaultContact.toUtf8() );
        } else {
            mContact = KABC::Addressee();
        }
    } else {
        if (!contacteditorutil::defaultContact().isEmpty()) {
            KABC::VCardConverter converter;
            mContact = converter.parseVCard( contacteditorutil::defaultContact().toUtf8() );
        } else {
            mContact = KABC::Addressee();
        }
    }
}

void ContactPreviewWidget::setThemePath(const QString &projectDirectory)
{
    mGroupFormatter->setAbsoluteThemePath(projectDirectory);
    mFormatter->setAbsoluteThemePath(projectDirectory);
}

#include "contactpreviewwidget.moc"
