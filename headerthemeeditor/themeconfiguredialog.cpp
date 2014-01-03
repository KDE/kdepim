/*
  Copyright (c) 2013, 2014 Montel Laurent <montel.org>

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

#include "themeconfiguredialog.h"
#include "themeeditorutil.h"

#include "configurewidget.h"

#include <KLocale>
#include <KConfig>
#include <KGlobal>
#include <KConfigGroup>
#include <KTextEdit>

#include <QVBoxLayout>
#include <QLabel>
#include <QTabWidget>

ThemeConfigureDialog::ThemeConfigureDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n( "Configure" ) );
    setButtons( Default|Ok|Cancel );
    setButtonFocus( Ok );

    QTabWidget *tab = new QTabWidget;

    QWidget *w = new QWidget;

    QVBoxLayout *lay = new QVBoxLayout;
    w->setLayout(lay);

    mConfigureWidget = new GrantleeThemeEditor::ConfigureWidget;
    lay->addWidget(mConfigureWidget);

    QLabel *lab = new QLabel(i18n("Default email:"));
    lay->addWidget(lab);

    mDefaultEmail = new KTextEdit;
    mDefaultEmail->setAcceptRichText(false);
    lay->addWidget(mDefaultEmail);
    tab->addTab(w, i18n("General"));

    mDefaultTemplate = new KTextEdit;
    mDefaultTemplate->setAcceptRichText(false);
    tab->addTab(mDefaultTemplate, i18n("Default Template"));

    setMainWidget(tab);
    connect(this, SIGNAL(defaultClicked()), this, SLOT(slotDefaultClicked()));
    connect(this, SIGNAL(okClicked()), this, SLOT(slotOkClicked()));
    readConfig();
}

ThemeConfigureDialog::~ThemeConfigureDialog()
{
    KSharedConfig::Ptr config = KGlobal::config();

    KConfigGroup group = config->group( QLatin1String("ThemeConfigureDialog") );
    group.writeEntry( "Size", size() );
}

void ThemeConfigureDialog::slotDefaultClicked()
{
    mConfigureWidget->setDefault();
    mDefaultEmail->setPlainText(themeeditorutil::defaultMail());
    mDefaultTemplate->clear();
}

void ThemeConfigureDialog::slotOkClicked()
{
    writeConfig();
}

void ThemeConfigureDialog::readConfig()
{
    KSharedConfig::Ptr config = KGlobal::config();
    if (config->hasGroup(QLatin1String("Global"))) {
        KConfigGroup group = config->group(QLatin1String("Global"));
        mConfigureWidget->readConfig();
        mDefaultEmail->setPlainText(group.readEntry("defaultEmail",themeeditorutil::defaultMail()));
        mDefaultTemplate->setPlainText(group.readEntry("defaultTemplate",QString()));
    } else {
        mDefaultEmail->setPlainText(themeeditorutil::defaultMail());
    }

    KConfigGroup group = KConfigGroup( config, "ThemeConfigureDialog" );
    const QSize sizeDialog = group.readEntry( "Size", QSize(600,400) );
    if ( sizeDialog.isValid() ) {
        resize( sizeDialog );
    }
}

void ThemeConfigureDialog::writeConfig()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group(QLatin1String("Global"));
    group.writeEntry("defaultEmail", mDefaultEmail->toPlainText());
    group.writeEntry("defaultTemplate", mDefaultTemplate->toPlainText());
    mConfigureWidget->writeConfig();
}

