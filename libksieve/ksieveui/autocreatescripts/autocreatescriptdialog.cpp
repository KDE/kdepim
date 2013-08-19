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

#include "autocreatescriptdialog.h"
#include "sievescriptlistbox.h"
#include "sieveconditionwidgetlister.h"
#include "sieveactionwidgetlister.h"

#include <KLocale>

#include <QVBoxLayout>
#include <QListWidget>
#include <QSplitter>
#include <QStackedWidget>

using namespace KSieveUi;

QStringList AutoCreateScriptDialog::sCapabilities = QStringList();

AutoCreateScriptDialog::AutoCreateScriptDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n( "Create sieve filter" ) );
    setButtons( Ok|Cancel );
    setButtonFocus( Ok );
    QWidget *mainWidget = new QWidget( this );
    QVBoxLayout *vlay = new QVBoxLayout( mainWidget );
    vlay->setSpacing( KDialog::spacingHint() );
    vlay->setMargin( KDialog::marginHint() );

    mSplitter = new QSplitter;
    mSplitter->setChildrenCollapsible(false);
    mSieveScript = new SieveScriptListBox( i18n("Sieve Script"));
    connect(mSieveScript, SIGNAL(addNewPage(QWidget*)), SLOT(slotAddScriptPage(QWidget*)));
    connect(mSieveScript, SIGNAL(removePage(QWidget*)), SLOT(slotRemoveScriptPage(QWidget*)));
    connect(mSieveScript, SIGNAL(activatePage(QWidget*)), SLOT(slotActivateScriptPage(QWidget*)));
    mSplitter->addWidget(mSieveScript);
    vlay->addWidget(mSplitter);

    mStackWidget = new QStackedWidget;
    mSplitter->addWidget(mStackWidget);

    setMainWidget( mainWidget );
    readConfig();
}

AutoCreateScriptDialog::~AutoCreateScriptDialog()
{
    writeConfig();
}

void AutoCreateScriptDialog::setSieveCapabilities( const QStringList &capabilities )
{
    sCapabilities = capabilities;
}

QStringList AutoCreateScriptDialog::sieveCapabilities()
{
    return sCapabilities;
}

QString AutoCreateScriptDialog::script(QString &requires) const
{
    return mSieveScript->generatedScript(requires);
}

void AutoCreateScriptDialog::slotAddScriptPage(QWidget *page)
{
    mStackWidget->addWidget(page);
    mStackWidget->setCurrentWidget(page);
}

void AutoCreateScriptDialog::slotRemoveScriptPage(QWidget *page)
{
    mStackWidget->removeWidget(page);
}

void AutoCreateScriptDialog::slotActivateScriptPage(QWidget *page)
{
    mStackWidget->setCurrentWidget(page);
}

void AutoCreateScriptDialog::readConfig()
{
    KConfigGroup group( KGlobal::config(), "AutoCreateScriptDialog" );
    const QSize sizeDialog = group.readEntry( "Size", QSize() );
    if ( sizeDialog.isValid() ) {
        resize( sizeDialog );
    } else {
        resize( 800,600);
    }
    QList<int> size;
    size << 100 << 400;
    mSplitter->setSizes(group.readEntry( "mainSplitter", size));
}

void AutoCreateScriptDialog::writeConfig()
{
    KConfigGroup group( KGlobal::config(), "AutoCreateScriptDialog" );
    group.writeEntry( "Size", size() );
    group.writeEntry( "mainSplitter", mSplitter->sizes());
}

#include "autocreatescriptdialog.moc"
