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

#include "sievescriptparsingerrordialog.h"

#include <KTextEdit>
#include <KLocale>

SieveScriptParsingErrorDialog::SieveScriptParsingErrorDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n( "Sieve Parsing Error" ) );
    setButtons( Close );

    mTextEdit = new KTextEdit( this );
    mTextEdit->setReadOnly( true );
    setMainWidget( mTextEdit );
    readConfig();
}

SieveScriptParsingErrorDialog::~SieveScriptParsingErrorDialog()
{
    writeConfig();
}

void SieveScriptParsingErrorDialog::setError(const QString &script, const QString &error)
{
    mTextEdit->setPlainText(error);
}

void SieveScriptParsingErrorDialog::readConfig()
{
    KConfigGroup group( KGlobal::config(), "SieveScriptParsingErrorDialog" );
    const QSize sizeDialog = group.readEntry( "Size", QSize() );
    if ( sizeDialog.isValid() ) {
        resize( sizeDialog );
    } else {
        resize( 800,600);
    }
}

void SieveScriptParsingErrorDialog::writeConfig()
{
    KConfigGroup group( KGlobal::config(), "SieveScriptParsingErrorDialog" );
    group.writeEntry( "Size", size() );
}


#include "sievescriptparsingerrordialog.moc"

