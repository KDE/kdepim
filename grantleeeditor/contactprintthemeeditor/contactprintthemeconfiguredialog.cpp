/*
   Copyright (C) 2015-2016 Montel Laurent <montel@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "contactprintthemeconfiguredialog.h"
#include "contactprintthemeeditorutil.h"
#include "kpimtextedit/plaintexteditorwidget.h"

#include "configurewidget.h"

#include <Akonadi/Contact/ContactEditor>

#include <KContacts/VCardConverter>

#include <KLocalizedString>
#include <KConfig>

#include <KConfigGroup>

#include <QVBoxLayout>
#include <QLabel>
#include <QTabWidget>
#include <KSharedConfig>
#include <QDialogButtonBox>
#include <QPushButton>

ContactPrintThemeConfigureDialog::ContactPrintThemeConfigureDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18n("Configure"));

    QTabWidget *tab = new QTabWidget;

    QWidget *w = new QWidget;
    QVBoxLayout *lay = new QVBoxLayout;
    w->setLayout(lay);

    mConfigureWidget = new GrantleeThemeEditor::ConfigureWidget;
    lay->addWidget(mConfigureWidget);

    QLabel *lab = new QLabel(i18n("Default contact:"));
    lay->addWidget(lab);

    mDefaultContact = new Akonadi::ContactEditor(Akonadi::ContactEditor::CreateMode, Akonadi::ContactEditor::VCardMode);
    lay->addWidget(mDefaultContact);

    tab->addTab(w, i18n("General"));

    mDefaultTemplate = new KPIMTextEdit::PlainTextEditorWidget;
    tab->addTab(mDefaultTemplate, i18n("Default Template"));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(tab);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::RestoreDefaults);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ContactPrintThemeConfigureDialog::reject);
    mainLayout->addWidget(buttonBox);
    okButton->setFocus();

    connect(buttonBox->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked, this, &ContactPrintThemeConfigureDialog::slotDefaultClicked);
    connect(buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, this, &ContactPrintThemeConfigureDialog::slotOkClicked);
    readConfig();
}

ContactPrintThemeConfigureDialog::~ContactPrintThemeConfigureDialog()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();

    KConfigGroup group = config->group(QStringLiteral("ContactPrintThemeConfigureDialog"));
    group.writeEntry("Size", size());
}

void ContactPrintThemeConfigureDialog::slotDefaultClicked()
{
    mConfigureWidget->setDefault();

    ContactPrintThemeEditorutil contactUtil;
    if (!contactUtil.defaultContact().isEmpty()) {
        KContacts::VCardConverter converter;
        KContacts::Addressee addr = converter.parseVCard(contactUtil.defaultContact().toUtf8());
        mDefaultContact->setContactTemplate(addr);
    } else {
        mDefaultContact->setContactTemplate(KContacts::Addressee());
    }
    mDefaultTemplate->setPlainText(contactUtil.defaultTemplate());
}

void ContactPrintThemeConfigureDialog::slotOkClicked()
{
    writeConfig();
    accept();
}

void ContactPrintThemeConfigureDialog::readConfig()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();

    ContactPrintThemeEditorutil contactUtil;
    if (config->hasGroup(QStringLiteral("Global"))) {
        KConfigGroup group = config->group(QStringLiteral("Global"));

        const QString defaultContact = group.readEntry("defaultContact", contactUtil.defaultContact());
        if (!defaultContact.isEmpty()) {
            KContacts::VCardConverter converter;
            KContacts::Addressee addr = converter.parseVCard(defaultContact.toUtf8());
            mDefaultContact->setContactTemplate(addr);
        } else {
            mDefaultContact->setContactTemplate(KContacts::Addressee());
        }
        mDefaultTemplate->setPlainText(group.readEntry("defaultTemplate", contactUtil.defaultTemplate()));
    } else {
        if (!contactUtil.defaultContact().isEmpty()) {
            KContacts::VCardConverter converter;
            KContacts::Addressee addr = converter.parseVCard(contactUtil.defaultContact().toUtf8());
            mDefaultContact->setContactTemplate(addr);
        } else {
            mDefaultContact->setContactTemplate(KContacts::Addressee());
        }
        mDefaultTemplate->setPlainText(contactUtil.defaultTemplate());
    }

    mConfigureWidget->readConfig();

    KConfigGroup group = KConfigGroup(config, "ContactPrintThemeConfigureDialog");
    const QSize sizeDialog = group.readEntry("Size", QSize(600, 400));
    if (sizeDialog.isValid()) {
        resize(sizeDialog);
    }
}

void ContactPrintThemeConfigureDialog::writeConfig()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group = config->group(QStringLiteral("Global"));
    const KContacts::Addressee addr = mDefaultContact->contact();
    KContacts::VCardConverter converter;
    const QByteArray data = converter.exportVCard(addr, KContacts::VCardConverter::v3_0);
    group.writeEntry("defaultContact", data);

    group.writeEntry("defaultTemplate", mDefaultTemplate->toPlainText());
    mConfigureWidget->writeConfig();
}

