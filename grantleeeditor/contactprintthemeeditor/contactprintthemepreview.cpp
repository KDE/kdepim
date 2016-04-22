/*
  Copyright (c) 2015-2016 Montel Laurent <montel@kde.org>

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
#include "contactprintthemeeditorutil.h"
#include <KSharedConfig>
#include <KConfigGroup>
#include <QHBoxLayout>
#ifdef QTWEBENGINE_EXPERIMENTAL_OPTION
#include <QWebEngineView>
#else
#include <QWebView>
#endif
#include <KContacts/VCardConverter>
#include <kaddressbookgrantlee/grantleeprint.h>
#include "contactprintthemeeditor_debug.h"

ContactPrintThemePreview::ContactPrintThemePreview(const QString &projectDirectory, QWidget *parent)
    : GrantleeThemeEditor::PreviewWidget(parent)
{
    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->setMargin(0);
#ifdef QTWEBENGINE_EXPERIMENTAL_OPTION
    mViewer = new QWebEngineView;
#else
    mViewer = new QWebView;
#endif
    hbox->addWidget(mViewer);
    setLayout(hbox);
    mGrantleePrint = new KAddressBookGrantlee::GrantleePrint(this);
    loadConfig();
    if (!projectDirectory.isEmpty()) {
        mGrantleePrint->changeGrantleePath(projectDirectory);
    }
}

ContactPrintThemePreview::~ContactPrintThemePreview()
{

}

void ContactPrintThemePreview::updateViewer()
{
    KContacts::AddresseeList lst;
    lst << mContact;
    mGrantleePrint->refreshTemplate();
    const QString html = mGrantleePrint->contactsToHtml(lst);
    mViewer->setHtml(html);
}

void ContactPrintThemePreview::createScreenShot(const QStringList &fileName)
{
    //TODO
}

void ContactPrintThemePreview::setThemePath(const QString &projectDirectory, const QString &mainPageFileName)
{
    Q_UNUSED(mainPageFileName);
    mGrantleePrint->changeGrantleePath(projectDirectory);
}

void ContactPrintThemePreview::loadConfig()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();

    if (config->hasGroup(QStringLiteral("Global"))) {
        KConfigGroup group = config->group(QStringLiteral("Global"));
        ContactPrintThemeEditorutil contactEditorUtil;
        const QString defaultContact = group.readEntry("defaultContact", contactEditorUtil.defaultContact());
        if (!defaultContact.isEmpty()) {
            KContacts::VCardConverter converter;
            mContact = converter.parseVCard(defaultContact.toUtf8());
        } else {
            mContact = KContacts::Addressee();
        }
    } else {
        ContactPrintThemeEditorutil contactEditorUtil;
        if (!contactEditorUtil.defaultContact().isEmpty()) {
            KContacts::VCardConverter converter;
            mContact = converter.parseVCard(contactEditorUtil.defaultContact().toUtf8());
        } else {
            mContact = KContacts::Addressee();
        }
    }
}

