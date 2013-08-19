/* Copyright (C) 2011, 2012, 2013 Laurent Montel <montel@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "sieveeditor.h"
#include "sievefindbar.h"
#include "sieveinfowidget.h"
#include "templates/sievetemplatewidget.h"
#include "autocreatescripts/autocreatescriptdialog.h"

#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <KTextEdit>
#include <KPushButton>
#include <errno.h>

#include <QSplitter>
#include <QDialogButtonBox>
#include <QTextStream>
#include <QPointer>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QShortcut>

using namespace KSieveUi;

SieveEditor::SieveEditor( QWidget * parent )
    : KDialog( parent )
{
    setCaption( i18n( "Edit Sieve Script" ) );
    setButtons( None );
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    mCheckSyntax = new KPushButton(i18n("Check Syntax"), this);
    connect(mCheckSyntax, SIGNAL(clicked(bool)), SLOT(slotCheckSyntax()));
    KPushButton *saveAs = new KPushButton(KStandardGuiItem::saveAs(), this);
    connect(saveAs, SIGNAL(clicked(bool)), SLOT(slotSaveAs()));
    KPushButton *import = new KPushButton(i18n( "Import..." ), this);
    connect(import, SIGNAL(clicked(bool)), SLOT(slotImport()));
    KPushButton *autogenerateScript = new KPushButton(i18n("Autogenerate Script..."), this);
    connect(autogenerateScript, SIGNAL(clicked(bool)), SLOT(slotAutoGenerateScripts()));
    mOkButton = buttonBox->button(QDialogButtonBox::Ok);

    connect(buttonBox, SIGNAL(accepted()), this, SIGNAL(okClicked()));
    connect(buttonBox, SIGNAL(rejected()), this, SIGNAL(cancelClicked()));
    connect(this, SIGNAL(accepted()), this, SIGNAL(okClicked()));
    connect(this, SIGNAL(rejected()), this, SIGNAL(cancelClicked()));

    buttonBox->addButton(mCheckSyntax, QDialogButtonBox::ActionRole);
    buttonBox->addButton(saveAs, QDialogButtonBox::ActionRole);
    buttonBox->addButton(import, QDialogButtonBox::ActionRole);
    buttonBox->addButton(autogenerateScript, QDialogButtonBox::ActionRole);

    setModal( true );

    QWidget *mainWidget = new QWidget;
    QVBoxLayout *lay = new QVBoxLayout;
    mainWidget->setLayout( lay );
    QHBoxLayout *nameLayout = new QHBoxLayout;
    QLabel * label = new QLabel( i18n( "Script name:" ) );
    nameLayout->addWidget( label );
    mScriptName = new QLineEdit;
    mScriptName->setReadOnly( true );
    nameLayout->addWidget( mScriptName );
    lay->addLayout( nameLayout );

    mMainSplitter = new QSplitter;
    mMainSplitter->setOrientation( Qt::Vertical );
    lay->addWidget( mMainSplitter );

    mTemplateSplitter = new QSplitter;
    mTemplateSplitter->setOrientation( Qt::Horizontal );
    //
    SieveTemplateWidget *sieveTemplateWidget = new SieveTemplateWidget(i18n("Sieve Template:"));

    mSieveInfo = new SieveInfoWidget;

    mExtraSplitter = new QSplitter;
    mExtraSplitter->setOrientation( Qt::Vertical );

    mExtraSplitter->addWidget(sieveTemplateWidget);
    mExtraSplitter->addWidget(mSieveInfo);
    mExtraSplitter->setChildrenCollapsible(false);


    QWidget *textEditWidget = new QWidget;
    QVBoxLayout * textEditLayout = new QVBoxLayout;
    mTextEdit = new SieveTextEdit;
    textEditLayout->addWidget(mTextEdit);
    mFindBar = new SieveFindBar( mTextEdit, textEditWidget );
    textEditLayout->addWidget(mFindBar);
    textEditWidget->setLayout(textEditLayout);

    mTemplateSplitter->addWidget(textEditWidget);
    mTemplateSplitter->addWidget(mExtraSplitter);
    mTemplateSplitter->setChildrenCollapsible(false);

    connect(sieveTemplateWidget, SIGNAL(insertTemplate(QString)), mTextEdit, SLOT(insertPlainText(QString)));

    //
    QShortcut *shortcut = new QShortcut( this );
    shortcut->setKey( Qt::Key_F+Qt::CTRL );
    connect( shortcut, SIGNAL(activated()), SLOT(slotFind()) );
    connect( mTextEdit, SIGNAL(findText()), SLOT(slotFind()) );

    mDebugTextEdit = new KTextEdit;
    mDebugTextEdit->setReadOnly( true );
    mMainSplitter->addWidget( mTemplateSplitter );
    mMainSplitter->addWidget( mDebugTextEdit );
    mMainSplitter->setChildrenCollapsible(false);
    connect( mTextEdit, SIGNAL(textChanged()), SLOT(slotTextChanged()) );

    lay->addWidget(buttonBox);
    setMainWidget( mainWidget );
    readConfig();

    mTextEdit->setFocus();
}

SieveEditor::~SieveEditor()
{
    writeConfig();
}

void SieveEditor::writeConfig()
{
    KConfigGroup group( KGlobal::config(), "SieveEditor" );
    group.writeEntry( "Size", size() );
    group.writeEntry( "mainSplitter", mMainSplitter->sizes());
    group.writeEntry( "extraSplitter", mExtraSplitter->sizes());
    group.writeEntry( "templateSplitter", mTemplateSplitter->sizes());
}

void SieveEditor::readConfig()
{
    KConfigGroup group( KGlobal::config(), "SieveEditor" );
    const QSize sizeDialog = group.readEntry( "Size", QSize() );
    if ( sizeDialog.isValid() ) {
        resize( sizeDialog );
    } else {
        resize( 800,600);
    }
    QList<int> size;
    size << 400 << 100;

    mMainSplitter->setSizes(group.readEntry( "mainSplitter", size));
    mExtraSplitter->setSizes(group.readEntry( "extraSplitter", size));
    mTemplateSplitter->setSizes(group.readEntry( "templateSplitter", size));
}

void SieveEditor::slotAutoGenerateScripts()
{
    QPointer<AutoCreateScriptDialog> dlg = new AutoCreateScriptDialog(this);
    dlg->setSieveCapabilities(mSieveCapabilities);
    if ( dlg->exec()) {
        QString requires;
        const QString script = dlg->script(requires);
        QString newPlainText = mTextEdit->toPlainText() + script;
        if (!requires.isEmpty()) {
            newPlainText.prepend(requires);
        }
        mTextEdit->setPlainText(newPlainText);
    }
    delete dlg;
}

void SieveEditor::slotFind()
{
    if ( mTextEdit->textCursor().hasSelection() )
        mFindBar->setText( mTextEdit->textCursor().selectedText() );
    mTextEdit->moveCursor(QTextCursor::Start);
    mFindBar->show();
    mFindBar->focusAndSetCursor();
}

void SieveEditor::slotSaveAs()
{
    KUrl url;
    const QString filter = i18n( "*.siv|sieve files (*.siv)\n*|all files (*)" );
    QPointer<KFileDialog> fdlg( new KFileDialog( url, filter, this) );

    fdlg->setMode( KFile::File );
    fdlg->setOperationMode( KFileDialog::Saving );
    fdlg->setConfirmOverwrite(true);
    if ( fdlg->exec() == QDialog::Accepted && fdlg ) {
        const QString fileName = fdlg->selectedFile();
        if ( !saveToFile( fileName ) ) {
            KMessageBox::error( this,
                                i18n( "Could not write the file %1:\n"
                                      "\"%2\" is the detailed error description.",
                                      fileName,
                                      QString::fromLocal8Bit( strerror( errno ) ) ),
                                i18n( "Sieve Editor Error" ) );
        }
    }
    delete fdlg;

}

bool SieveEditor::saveToFile( const QString &filename )
{
    QFile file( filename );
    if ( !file.open( QIODevice::WriteOnly|QIODevice::Text ) )
        return false;
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << mTextEdit->toPlainText();
    return true;
}

void SieveEditor::slotImport()
{
    if ( !mTextEdit->toPlainText().isEmpty() ) {
        if ( KMessageBox::warningYesNo(this, i18n( "You will overwrite script. Do you want to continue?" ), i18n( "Import Script" ) ) == KMessageBox::No )
            return;
    }
    KUrl url;
    const QString filter = i18n( "*.siv|sieve files (*.siv)\n*|all files (*)" );
    QPointer<KFileDialog> fdlg( new KFileDialog( url, filter, this) );

    fdlg->setMode( KFile::File );
    fdlg->setOperationMode( KFileDialog::Opening );
    if ( fdlg->exec() == QDialog::Accepted && fdlg ) {
        const QString fileName = fdlg->selectedFile();
        if ( !loadFromFile( fileName ) ) {
            KMessageBox::error( this,
                                i18n( "Could not load the file %1:\n"
                                      "\"%2\" is the detailed error description.",
                                      fileName,
                                      QString::fromLocal8Bit( strerror( errno ) ) ),
                                i18n( "Sieve Editor Error" ) );
        }
    }
    delete fdlg;
}

bool SieveEditor::loadFromFile( const QString &filename )
{
    QFile file( filename );
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QTextStream in(&file);
    QString line = in.readLine();
    QString scriptText;
    while (!line.isNull()) {
        if ( scriptText.isEmpty() )
            scriptText = line;
        else
            scriptText += QLatin1Char( '\n' ) + line;
        line = in.readLine();
    }
    mTextEdit->setPlainText( scriptText );
    return true;
}

void SieveEditor::slotTextChanged()
{
    const bool enabled = !script().isEmpty();
    mOkButton->setEnabled(enabled);
    mCheckSyntax->setEnabled( enabled );
    enableButtonOk( enabled );
}

QString SieveEditor::script() const
{
    return mTextEdit->toPlainText();
}

QString SieveEditor::originalScript() const
{
    return mOriginalScript;
}

void SieveEditor::setScript( const QString &script )
{
    mTextEdit->setPlainText( script );
    mOriginalScript = script;
}

void SieveEditor::setDebugColor( const QColor &col )
{
    mDebugTextEdit->setTextColor( col );
}

void SieveEditor::setDebugScript( const QString &debug )
{
    mDebugTextEdit->setText( debug );
}

void SieveEditor::setScriptName( const QString &name )
{
    mScriptName->setText( name );
}  

void SieveEditor::resultDone()
{
    mCheckSyntax->setEnabled(true);
}

void SieveEditor::slotCheckSyntax()
{
    mCheckSyntax->setEnabled(false);
    Q_EMIT checkSyntax();
}

void SieveEditor::setSieveCapabilities( const QStringList &capabilities )
{
    mSieveCapabilities = capabilities;
    mTextEdit->setSieveCapabilities(mSieveCapabilities);
    mSieveInfo->setServerInfo(capabilities);
}

#include "sieveeditor.moc"

