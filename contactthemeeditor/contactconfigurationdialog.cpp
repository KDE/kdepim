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

#include "contactconfigurationdialog.h"
#include "contacteditorutil.h"

#include "configurewidget.h"

#include <KLocale>
#include <KUrlRequester>
#include <KConfig>
#include <KGlobal>
#include <KConfigGroup>
#include <KTextEdit>

#include <QVBoxLayout>
#include <QLabel>

ContactConfigureDialog::ContactConfigureDialog(QWidget *parent)
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

    QLabel *lab = new QLabel(i18n("Default contact:"));
    lay->addWidget(lab);

    mDefaultContact = new KTextEdit;
    mDefaultContact->setAcceptRichText(false);
    lay->addWidget(mDefaultContact);

    tab->addTab(w, i18n("General"));

    mDefaultTemplate = new KTextEdit;
    mDefaultTemplate->setAcceptRichText(false);
    tab->addTab(mDefaultTemplate, i18n("Default Template"));

    setMainWidget(tab);
    connect(this, SIGNAL(defaultClicked()), this, SLOT(slotDefaultClicked()));
    connect(this, SIGNAL(okClicked()), this, SLOT(slotOkClicked()));
    readConfig();
}

ContactConfigureDialog::~ContactConfigureDialog()
{
    KSharedConfig::Ptr config = KGlobal::config();

    KConfigGroup group = config->group( QLatin1String("ContactConfigureDialog") );
    group.writeEntry( "Size", size() );
}

void ContactConfigureDialog::slotDefaultClicked()
{
    mConfigureWidget->setDefault();
    mDefaultContact->setPlainText(contacteditorutil::defaultContact());
    mDefaultTemplate->clear();
}

void ContactConfigureDialog::slotOkClicked()
{
    writeConfig();
}

void ContactConfigureDialog::readConfig()
{
    KSharedConfig::Ptr config = KGlobal::config();

    if (config->hasGroup(QLatin1String("Global"))) {
        KConfigGroup group = config->group(QLatin1String("Global"));
        mDefaultContact->setPlainText(group.readEntry("defaultContact",contacteditorutil::defaultContact()));
        mDefaultTemplate->setPlainText(group.readEntry("defaultTemplate",QString()));
    } else {
        mDefaultContact->setPlainText(contacteditorutil::defaultContact());
    }

    mConfigureWidget->readConfig();

    KConfigGroup group = KConfigGroup( config, "ContactConfigureDialog" );
    const QSize sizeDialog = group.readEntry( "Size", QSize() );
    if ( sizeDialog.isValid() ) {
        resize( sizeDialog );
    } else {
        resize( 600,400);
    }
}

void ContactConfigureDialog::writeConfig()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group(QLatin1String("Global"));
    group.writeEntry("defaultContact", mDefaultContact->toPlainText());    
    group.writeEntry("defaultTemplate", mDefaultTemplate->toPlainText());
    mConfigureWidget->writeConfig();
}

#include "contactconfigurationdialog.moc"
