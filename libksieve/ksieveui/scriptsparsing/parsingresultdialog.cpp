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

#include "parsingresultdialog.h"
#include "xmlprintingsyntaxhighlighter.h"
#include "pimcommon/util/pimutil.h"

#include <KTextEdit>
#include <KLocale>

using namespace KSieveUi;

ParsingResultDialog::ParsingResultDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n( "Sieve Parsing" ) );
    setButtons( Close|User1 );
    setButtonText(User1, i18n("Save As..."));

    mTextEdit = new KTextEdit( this );
    new XMLPrintingSyntaxHighLighter(mTextEdit->document());
    mTextEdit->setReadOnly( true );
    setMainWidget( mTextEdit );
    connect(this, SIGNAL(user1Clicked()), this, SLOT(slotSaveAs()));
    readConfig();
}

ParsingResultDialog::~ParsingResultDialog()
{
    writeConfig();
}

void ParsingResultDialog::setResultParsing(const QString &result)
{
    mTextEdit->setPlainText(result);
}

void ParsingResultDialog::readConfig()
{
    KConfigGroup group( KGlobal::config(), "ParsingResultDialog" );
    const QSize sizeDialog = group.readEntry( "Size", QSize() );
    if ( sizeDialog.isValid() ) {
        resize( sizeDialog );
    } else {
        resize( 800,600);
    }
}

void ParsingResultDialog::writeConfig()
{
    KConfigGroup group( KGlobal::config(), "ParsingResultDialog" );
    group.writeEntry( "Size", size() );
}

void ParsingResultDialog::slotSaveAs()
{
    const QString filter = i18n( "all files (*)" );
    PimCommon::Util::saveTextAs(mTextEdit->toPlainText(), filter, this);
}

#include "parsingresultdialog.moc"
