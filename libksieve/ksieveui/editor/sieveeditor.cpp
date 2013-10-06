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
#include "sieve-editor.h"
#include "sieveeditortextmodewidget.h"
#include "scriptsparsing/parsingutil.h"
#include "autocreatescripts/sieveeditorgraphicalmodewidget.h"
#include "autocreatescripts/sievescriptparsingerrordialog.h"

#include <klocale.h>
#include <KStandardGuiItem>

#include <QPushButton>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QLabel>
#include <QLineEdit>
#include <QToolBar>
#include <QDebug>
#include <QAction>
#include <QPointer>

using namespace KSieveUi;

SieveEditor::SieveEditor( QWidget * parent )
    : KDialog( parent ),
      mMode(TextMode)
{
    setCaption( i18n( "Edit Sieve Script" ) );
    setButtons( Ok|Cancel );

    setModal( true );
    QWidget *w = new QWidget;

    QVBoxLayout *lay = new QVBoxLayout;

    QToolBar *bar = new QToolBar;
    mCheckSyntax = new QAction(i18n("Check Syntax"), this);
    connect(mCheckSyntax, SIGNAL(triggered(bool)), SLOT(slotCheckSyntax()));
    bar->addAction(mCheckSyntax);
    mSaveAs = bar->addAction(KStandardGuiItem::saveAs().text(), this, SLOT(slotSaveAs()));
    bar->addAction(i18n("Import..."), this, SLOT(slotImport()));

    mAutoGenerateScript = new QAction(i18n("Autogenerate Script..."), this);
    connect(mAutoGenerateScript, SIGNAL(triggered(bool)), SLOT(slotAutoGenerateScripts()));
    bar->addAction(mAutoGenerateScript);
    mSwitchMode = new QAction(i18n("Switch Mode"), this);
    bar->addAction(mSwitchMode);
    connect(mSwitchMode, SIGNAL(triggered(bool)), SLOT(slotSwitchMode()));
#if !defined(NDEBUG)
    //Not necessary to translate it.
    mGenerateXml = new QAction(QLatin1String("Generate xml"), this);
    connect(mGenerateXml, SIGNAL(triggered(bool)), SLOT(slotGenerateXml()));
    bar->addAction(mGenerateXml);
#endif

    lay->addWidget(bar);


    QHBoxLayout *nameLayout = new QHBoxLayout;
    QLabel *label = new QLabel( i18n( "Script name:" ) );
    nameLayout->addWidget( label );
    mScriptName = new QLineEdit;
    mScriptName->setReadOnly( true );
    nameLayout->addWidget( mScriptName );
    lay->addLayout( nameLayout );


    lay->setMargin(0);
    w->setLayout(lay);
    mStackedWidget = new QStackedWidget;

    mTextModeWidget = new SieveEditorTextModeWidget;
    mStackedWidget->addWidget(mTextModeWidget);
    mGraphicalModeWidget = new SieveEditorGraphicalModeWidget;
    mStackedWidget->addWidget(mGraphicalModeWidget);

    lay->addWidget(mStackedWidget);
    connect(mTextModeWidget, SIGNAL(enableButtonOk(bool)), this, SLOT(slotEnableButtonOk(bool)));
    connect(mGraphicalModeWidget, SIGNAL(enableButtonOk(bool)), this, SLOT(slotEnableButtonOk(bool)));
    connect(mGraphicalModeWidget, SIGNAL(switchTextMode(QString)), this, SLOT(slotSwitchTextMode(QString)));
    connect(mTextModeWidget, SIGNAL(switchToGraphicalMode()), SLOT(slotSwitchToGraphicalMode()));
    readConfig();
    setMainWidget( w );
    if (KSieveUi::EditorSettings::useGraphicEditorByDefault()) {
        changeMode(GraphicMode);
    }
}

SieveEditor::~SieveEditor()
{
    writeConfig();
}

void SieveEditor::changeMode(EditorMode mode)
{
    if (mode != mMode) {
        mMode = mode;
        mStackedWidget->setCurrentIndex(static_cast<int>(mode));
        mAutoGenerateScript->setEnabled((mMode == TextMode));
#if !defined(NDEBUG)
        mGenerateXml->setEnabled((mMode == TextMode));
#endif
        if (mMode == GraphicMode)
            mCheckSyntax->setEnabled(false);
        else
            mCheckSyntax->setEnabled(!mTextModeWidget->currentscript().isEmpty());
    }
}

void SieveEditor::slotEnableButtonOk(bool b)
{
    enableButtonOk(b);
    mSaveAs->setEnabled(b);
    if (mMode == TextMode) {
        mCheckSyntax->setEnabled(b);
    } else {
        mCheckSyntax->setEnabled(false);
    }
}

void SieveEditor::writeConfig()
{
    KConfigGroup group( KGlobal::config(), "SieveEditor" );
    group.writeEntry( "Size", size() );
}

void SieveEditor::readConfig()
{
    KConfigGroup group( KGlobal::config(), "SieveEditor" );
    const QSize sizeDialog = group.readEntry( "Size", QSize(800,600) );
    if ( sizeDialog.isValid() ) {
        resize( sizeDialog );
    }
}

QString SieveEditor::script() const
{
    QString currentScript;
    switch (mMode) {
    case TextMode:
        currentScript = mTextModeWidget->script();
        break;
    case GraphicMode:
        currentScript = mGraphicalModeWidget->currentscript();
        break;
    }
    return currentScript;
}

QString SieveEditor::originalScript() const
{
    return mOriginalScript;
}

void SieveEditor::setScript( const QString &script )
{
    mTextModeWidget->setScript( script );
    mOriginalScript = script;
}

void SieveEditor::setDebugScript( const QString &debug )
{
    mTextModeWidget->setDebugScript( debug );
}

void SieveEditor::setScriptName( const QString &name )
{
    mScriptName->setText( name );
}  

void SieveEditor::resultDone()
{
    mCheckSyntax->setEnabled(true);
}

void SieveEditor::setSieveCapabilities( const QStringList &capabilities )
{
    mTextModeWidget->setSieveCapabilities(capabilities);
    mGraphicalModeWidget->setSieveCapabilities(capabilities);
}

void SieveEditor::slotAutoGenerateScripts()
{
    switch (mMode) {
    case TextMode:
        mTextModeWidget->autoGenerateScripts();
        break;
    case GraphicMode:
        break;
    }
}

void SieveEditor::slotCheckSyntax()
{
    switch (mMode) {
    case TextMode:
        mCheckSyntax->setEnabled(false);
        Q_EMIT checkSyntax();
        break;
    case GraphicMode:
        break;
    }
}

void SieveEditor::slotGenerateXml()
{
#if !defined(NDEBUG)
    switch (mMode) {
    case TextMode:
        mTextModeWidget->generateXml();
        break;
    case GraphicMode:
        break;
    }
#endif
}

void SieveEditor::slotSaveAs()
{
    switch (mMode) {
    case TextMode:
        mTextModeWidget->slotSaveAs();
        break;
    case GraphicMode:
        mGraphicalModeWidget->slotSaveAs();
        break;
    }
}

void SieveEditor::slotImport()
{
    switch (mMode) {
    case TextMode:
        mTextModeWidget->slotImport();
        break;
    case GraphicMode:
        mGraphicalModeWidget->slotImport();
        break;
    }
}

void SieveEditor::slotSwitchToGraphicalMode()
{
    mTextModeWidget->hideEditorWarning();
    changeMode(GraphicMode);
}

void SieveEditor::slotSwitchMode()
{
    switch (mMode) {
    case TextMode: {
        bool result = false;
        const QDomDocument doc = ParsingUtil::parseScript(mTextModeWidget->currentscript(), result);
        if (result) {
            QString error;
            mGraphicalModeWidget->loadScript(doc, error);
            mTextModeWidget->hideEditorWarning();
            if (!error.isEmpty()) {
                mTextModeWidget->setParsingEditorWarningError(mTextModeWidget->currentscript(), error);
                mTextModeWidget->showParsingEditorWarning();
            } else {
                changeMode(GraphicMode);
            }
        } else {
            mTextModeWidget->showEditorWarning();
            qDebug() << "cannot parse file";
        }
        break;
    }
    case GraphicMode: {
        const QString script = mGraphicalModeWidget->currentscript();
        changeMode(TextMode);
        mTextModeWidget->setScript(script);
        break;
    }
    }
}

void SieveEditor::slotSwitchTextMode(const QString &script)
{
    changeMode(TextMode);
    mTextModeWidget->setScript(script);
}

#include "sieveeditor.moc"

