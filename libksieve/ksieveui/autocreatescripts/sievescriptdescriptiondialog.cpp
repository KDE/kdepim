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

#include "sievescriptdescriptiondialog.h"
#include "pimcommon/widgets/customtextedit.h"

#include <KLocale>

#include <QHBoxLayout>

using namespace KSieveUi;

SieveScriptDescriptionDialog::SieveScriptDescriptionDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n( "Description" ) );
    setButtons( Ok|Cancel );
    mEdit = new PimCommon::CustomTextEdit;
    mEdit->setAcceptRichText(false);
    setMainWidget(mEdit);
    readConfig();
    mEdit->setFocus();
}

SieveScriptDescriptionDialog::~SieveScriptDescriptionDialog()
{
    writeConfig();
}

void SieveScriptDescriptionDialog::setDescription(const QString &desc)
{
    mEdit->setPlainText(desc);
}

QString SieveScriptDescriptionDialog::description() const
{
    return mEdit->toPlainText();
}

void SieveScriptDescriptionDialog::readConfig()
{
    KConfigGroup group( KGlobal::config(), "SieveScriptDescriptionDialog" );
    const QSize sizeDialog = group.readEntry( "Size", QSize(800,600) );
    if ( sizeDialog.isValid() ) {
        resize( sizeDialog );
    }
}

void SieveScriptDescriptionDialog::writeConfig()
{
    KConfigGroup group( KGlobal::config(), "SieveScriptDescriptionDialog" );
    group.writeEntry( "Size", size() );
}


#include "sievescriptdescriptiondialog.moc"
