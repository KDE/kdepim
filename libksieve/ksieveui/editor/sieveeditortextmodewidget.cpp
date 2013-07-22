/* Copyright (C) 2013 Laurent Montel <montel@kde.org>
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

#include "sieveeditortextmodewidget.h"
#include "templates/sievetemplatewidget.h"
#include "autocreatescripts/autocreatescriptdialog.h"
#include "editor/sieveinfowidget.h"
#include "editor/sievefindbar.h"
#include "editor/sievetextedit.h"

#include "scriptsparsing/xmlprintingscriptbuilder.h"
#include "scriptsparsing/parsingresultdialog.h"

#include <ksieve/parser.h>
#include <ksieve/error.h>
#include <ksieve/scriptbuilder.h>


#include <KLocale>
#include <KTextEdit>
#include <KStandardGuiItem>
#include <KFileDialog>
#include <KMessageBox>

#include <QToolBar>
#include <QAction>
#include <QLabel>
#include <QSplitter>
#include <QShortcut>
#include <QHBoxLayout>
#include <QPointer>
#include <QPushButton>
#include <QTextStream>
#include <QDebug>

#include <errno.h>

using namespace KSieveUi;

SieveEditorTextModeWidget::SieveEditorTextModeWidget(QWidget *parent)
    : SieveEditorAbstractWidget(parent)
{
    QVBoxLayout *lay = new QVBoxLayout;
    setLayout( lay );

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

    readConfig();

    mTextEdit->setFocus();
}

SieveEditorTextModeWidget::~SieveEditorTextModeWidget()
{
    writeConfig();
}

void SieveEditorTextModeWidget::writeConfig()
{
    KConfigGroup group( KGlobal::config(), "SieveEditor" );
    group.writeEntry( "mainSplitter", mMainSplitter->sizes());
    group.writeEntry( "extraSplitter", mExtraSplitter->sizes());
    group.writeEntry( "templateSplitter", mTemplateSplitter->sizes());
}

void SieveEditorTextModeWidget::readConfig()
{
    KConfigGroup group( KGlobal::config(), "SieveEditor" );
    QList<int> size;
    size << 400 << 100;

    mMainSplitter->setSizes(group.readEntry( "mainSplitter", size));
    mExtraSplitter->setSizes(group.readEntry( "extraSplitter", size));
    mTemplateSplitter->setSizes(group.readEntry( "templateSplitter", size));
}

void SieveEditorTextModeWidget::generateXml()
{
    const QByteArray script = mTextEdit->toPlainText().toUtf8();
    KSieve::Parser parser( script.begin(),
                           script.begin() + script.length() );
    KSieveUi::XMLPrintingScriptBuilder psb;
    parser.setScriptBuilder( &psb );
    const bool result = parser.parse();
    QPointer<ParsingResultDialog> dlg = new  ParsingResultDialog;
    if (result) {
        dlg->setResultParsing(psb.toDom().toString());
    } else {
        dlg->setResultParsing(QLatin1String("Error during parsing"));
    }
    dlg->exec();
    delete dlg;
}

void SieveEditorTextModeWidget::autoGenerateScripts()
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

void SieveEditorTextModeWidget::slotFind()
{
    if ( mTextEdit->textCursor().hasSelection() )
        mFindBar->setText( mTextEdit->textCursor().selectedText() );
    mTextEdit->moveCursor(QTextCursor::Start);
    mFindBar->show();
    mFindBar->focusAndSetCursor();
}

QString SieveEditorTextModeWidget::currentscript()
{
    return mTextEdit->toPlainText();
}

void SieveEditorTextModeWidget::setImportScript( const QString &script )
{
    mTextEdit->setPlainText( script );
}

void SieveEditorTextModeWidget::slotTextChanged()
{
    const bool enabled = !script().isEmpty();
    Q_EMIT enableButtonOk( enabled );
}

QString SieveEditorTextModeWidget::script() const
{
    return mTextEdit->toPlainText();
}

void SieveEditorTextModeWidget::setScript( const QString &script )
{
    mTextEdit->setPlainText( script );
}

void SieveEditorTextModeWidget::setDebugColor( const QColor &col )
{
    mDebugTextEdit->setTextColor( col );
}

void SieveEditorTextModeWidget::setDebugScript( const QString &debug )
{
    mDebugTextEdit->setText( debug );
}

void SieveEditorTextModeWidget::setSieveCapabilities( const QStringList &capabilities )
{
    mSieveCapabilities = capabilities;
    mTextEdit->setSieveCapabilities(mSieveCapabilities);
    mSieveInfo->setServerInfo(capabilities);
}


#include "sieveeditortextmodewidget.moc"
