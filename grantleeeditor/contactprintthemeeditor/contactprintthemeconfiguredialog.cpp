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

#include "contactprintthemeconfiguredialog.h"
#include "themeeditorutil.h"
#include "pimcommon/texteditor/richtexteditor/richtexteditorwidget.h"
#include "pimcommon/texteditor/richtexteditor/richtexteditor.h"

#include "configurewidget.h"

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

    QLabel *lab = new QLabel(i18n("Default email:"));
    lay->addWidget(lab);

    mDefaultEmail = new PimCommon::RichTextEditorWidget;
    mDefaultEmail->setAcceptRichText(false);
    lay->addWidget(mDefaultEmail);
    tab->addTab(w, i18n("General"));

    mDefaultTemplate = new PimCommon::RichTextEditorWidget;
    mDefaultTemplate->setAcceptRichText(false);
    tab->addTab(mDefaultTemplate, i18n("Default Template"));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(tab);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::RestoreDefaults);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &ContactPrintThemeConfigureDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ContactPrintThemeConfigureDialog::reject);
    mainLayout->addWidget(buttonBox);
    buttonBox->button(QDialogButtonBox::Ok)->setFocus();

    connect(buttonBox->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked, this, &ContactPrintThemeConfigureDialog::slotDefaultClicked);
    connect(buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, this, &ContactPrintThemeConfigureDialog::slotOkClicked);
    readConfig();
}

ContactPrintThemeConfigureDialog::~ContactPrintThemeConfigureDialog()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();

    KConfigGroup group = config->group(QStringLiteral("ThemeConfigureDialog"));
    group.writeEntry("Size", size());
}

void ContactPrintThemeConfigureDialog::slotDefaultClicked()
{
    mConfigureWidget->setDefault();
    mDefaultEmail->setPlainText(themeeditorutil::defaultContact());
    mDefaultTemplate->editor()->clear();
}

void ContactPrintThemeConfigureDialog::slotOkClicked()
{
    writeConfig();
}

void ContactPrintThemeConfigureDialog::readConfig()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    if (config->hasGroup(QStringLiteral("Global"))) {
        KConfigGroup group = config->group(QStringLiteral("Global"));
        mConfigureWidget->readConfig();
        mDefaultEmail->setPlainText(group.readEntry("defaultContact", themeeditorutil::defaultContact()));
        mDefaultTemplate->setPlainText(group.readEntry("defaultTemplate", QString()));
    } else {
        mDefaultEmail->setPlainText(themeeditorutil::defaultContact());
    }

    KConfigGroup group = KConfigGroup(config, "ThemeConfigureDialog");
    const QSize sizeDialog = group.readEntry("Size", QSize(600, 400));
    if (sizeDialog.isValid()) {
        resize(sizeDialog);
    }
}

void ContactPrintThemeConfigureDialog::writeConfig()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group = config->group(QStringLiteral("Global"));
    group.writeEntry("defaultContact", mDefaultEmail->toPlainText());
    group.writeEntry("defaultTemplate", mDefaultTemplate->toPlainText());
    mConfigureWidget->writeConfig();
}

