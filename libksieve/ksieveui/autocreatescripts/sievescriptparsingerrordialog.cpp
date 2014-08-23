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

#include "sievescriptparsingerrordialog.h"
#include "pimcommon/util/pimutil.h"
#include "pimcommon/texteditor/richtexteditor/richtexteditorwidget.h"

#include <KLocalizedString>
#include <KSharedConfig>

#include <errno.h>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

SieveScriptParsingErrorDialog::SieveScriptParsingErrorDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle( i18n( "Sieve Parsing Error" ) );
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    QPushButton *user1Button = new QPushButton;
    buttonBox->addButton(user1Button, QDialogButtonBox::ActionRole);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &SieveScriptParsingErrorDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &SieveScriptParsingErrorDialog::reject);
    mainLayout->addWidget(buttonBox);
    user1Button->setText(i18n("Save As..."));

    mTextEdit = new PimCommon::RichTextEditorWidget( this );
    
    mTextEdit->setReadOnly( true );
    readConfig();
    connect(user1Button, &QPushButton::clicked, this, &SieveScriptParsingErrorDialog::slotSaveAs);
    mainLayout->addWidget(mTextEdit);
    mainLayout->addWidget(buttonBox);
}

SieveScriptParsingErrorDialog::~SieveScriptParsingErrorDialog()
{
    writeConfig();
}

void SieveScriptParsingErrorDialog::setError(QString script, QString error)
{
    QString str;
    str = QLatin1String("<b>") + i18n("Sieve script:") + QLatin1String("</b><br>");
    str += script.replace(QLatin1Char('\n'), QLatin1String("<br>")) + QLatin1String("<br><br>");
    str += QLatin1String("<b>") + i18n("Errors reported:") + QLatin1String("</b><br>");
    str += error.replace(QLatin1Char('\n'), QLatin1String("<br>")) + QLatin1String("<br>");
    mTextEdit->setHtml(str);
}

void SieveScriptParsingErrorDialog::readConfig()
{
    KConfigGroup group( KSharedConfig::openConfig(), "SieveScriptParsingErrorDialog" );
    const QSize sizeDialog = group.readEntry( "Size", QSize(800,600) );
    if ( sizeDialog.isValid() ) {
        resize( sizeDialog );
    }
}

void SieveScriptParsingErrorDialog::writeConfig()
{
    KConfigGroup group( KSharedConfig::openConfig(), "SieveScriptParsingErrorDialog" );
    group.writeEntry( "Size", size() );
}

void SieveScriptParsingErrorDialog::slotSaveAs()
{
    const QString filter = i18n( "all files (*)" );
    PimCommon::Util::saveTextAs(mTextEdit->toPlainText(), filter, this, QUrl(), i18n("Save Log To File"));
}


