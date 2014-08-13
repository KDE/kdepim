/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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
#include "pimcommon/texteditor/plaintexteditor/plaintexteditorwidget.h"
#include "pimcommon/texteditor/plaintexteditor/plaintexteditor.h"

#include <KLocalizedString>
#include <KSharedConfig>
#include <QDialogButtonBox>
#include <KConfigGroup>
#include <QPushButton>
#include <QVBoxLayout>

using namespace KSieveUi;

ParsingResultDialog::ParsingResultDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle( i18n( "Sieve Parsing" ) );
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    QPushButton *user1Button = new QPushButton;
    buttonBox->addButton(user1Button, QDialogButtonBox::ActionRole);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    user1Button->setText(i18n("Save As..."));

    mTextEdit = new PimCommon::PlainTextEditorWidget( this );
    new XMLPrintingSyntaxHighLighter(mTextEdit->editor()->document());
    mTextEdit->setReadOnly( true );
    mainLayout->addWidget(mTextEdit);
    mainLayout->addWidget(buttonBox);

    connect(user1Button, SIGNAL(clicked()), this, SLOT(slotSaveAs()));
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
    KConfigGroup group( KSharedConfig::openConfig(), "ParsingResultDialog" );
    const QSize sizeDialog = group.readEntry( "Size", QSize(800,600) );
    if ( sizeDialog.isValid() ) {
        resize( sizeDialog );
    }
}

void ParsingResultDialog::writeConfig()
{
    KConfigGroup group( KSharedConfig::openConfig(), "ParsingResultDialog" );
    group.writeEntry( "Size", size() );
}

void ParsingResultDialog::slotSaveAs()
{
    const QString filter = i18n( "*.xml|xml files (*.xml)\n*|all files (*)" );
    PimCommon::Util::saveTextAs(mTextEdit->toPlainText(), filter, this);
}

