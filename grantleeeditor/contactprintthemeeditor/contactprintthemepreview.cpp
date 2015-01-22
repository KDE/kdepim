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

#include "contactprintthemepreview.h"
#include <KSharedConfig>
#include <KConfigGroup>
#include <KContacts/VCardConverter>

ContactPrintThemePreview::ContactPrintThemePreview(const QString &projectDirectory, QWidget *parent)
    : QWidget(parent)
{
    loadConfig();
}

ContactPrintThemePreview::~ContactPrintThemePreview()
{

}

void ContactPrintThemePreview::updateViewer()
{

}

void ContactPrintThemePreview::createScreenShot(const QStringList &fileName)
{

}

void ContactPrintThemePreview::setThemePath(const QString &projectDirectory)
{

}

void ContactPrintThemePreview::loadConfig()
{
#if 0
    KSharedConfig::Ptr config = KSharedConfig::openConfig();

    if (config->hasGroup(QStringLiteral("Global"))) {
        KConfigGroup group = config->group(QStringLiteral("Global"));
        const QString defaultContact = group.readEntry("defaultContact", contacteditorutil::defaultContact());
        if (!defaultContact.isEmpty()) {
            KContacts::VCardConverter converter;
            mContact = converter.parseVCard(defaultContact.toUtf8());
        } else {
            mContact = KContacts::Addressee();
        }
    } else {
        if (!contacteditorutil::defaultContact().isEmpty()) {
            KContacts::VCardConverter converter;
            mContact = converter.parseVCard(contacteditorutil::defaultContact().toUtf8());
        } else {
            mContact = KContacts::Addressee();
        }
    }
#endif
}

