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
#include "sieveconditionwidgetlister.h"

#include <KLocale>

#include <QVBoxLayout>
#include <QListWidget>

using namespace KSieveUi;

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

    mSieveScript = new QListWidget;

    mScriptConditionLister = new SieveConditionWidgetLister;

    vlay->addWidget(mScriptConditionLister);
    setMainWidget( mainWidget );
}

AutoCreateScriptDialog::~AutoCreateScriptDialog()
{

}

QString AutoCreateScriptDialog::script() const
{
    return QString(); //TODO
}

#include "autocreatescriptdialog.moc"
