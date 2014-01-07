/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/

#include "sieveeditorwidget.h"

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

using namespace KSieveUi;

SieveEditorWidget::SieveEditorWidget(QWidget *parent)
    : QWidget(parent),
      mMode(TextMode)
{
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
    setLayout(lay);
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
    if (KSieveUi::EditorSettings::useGraphicEditorByDefault()) {
        changeMode(GraphicMode);
    }
}

SieveEditorWidget::~SieveEditorWidget()
{
}

void SieveEditorWidget::changeMode(EditorMode mode)
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

void SieveEditorWidget::slotEnableButtonOk(bool b)
{
    Q_EMIT enableButtonOk(b);
    mSaveAs->setEnabled(b);
    if (mMode == TextMode) {
        mCheckSyntax->setEnabled(b);
    } else {
        mCheckSyntax->setEnabled(false);
    }
}

QString SieveEditorWidget::script() const
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

QString SieveEditorWidget::originalScript() const
{
    return mOriginalScript;
}

void SieveEditorWidget::setScript( const QString &script )
{
    mTextModeWidget->setScript( script );
    mOriginalScript = script;
}

void SieveEditorWidget::addFailedMessage(const QString &err)
{
    addMessageEntry( err, QColor( Qt::darkRed ) );
}

void SieveEditorWidget::addOkMessage(const QString &msg)
{
    addMessageEntry( msg, QColor( Qt::darkGreen ) );
}

void SieveEditorWidget::addMessageEntry( const QString &errorMsg, const QColor &color )
{
    const QString logText = QString::fromLatin1( "<font color=%1>%2</font>" )
           .arg( color.name() ).arg(errorMsg);

    setDebugScript( logText );
}

void SieveEditorWidget::setDebugScript( const QString &debug )
{
    mTextModeWidget->setDebugScript( debug );
}

void SieveEditorWidget::setScriptName( const QString &name )
{
    mScriptName->setText( name );
}

void SieveEditorWidget::resultDone()
{
    mCheckSyntax->setEnabled(true);
}

void SieveEditorWidget::setSieveCapabilities( const QStringList &capabilities )
{
    mTextModeWidget->setSieveCapabilities(capabilities);
    mGraphicalModeWidget->setSieveCapabilities(capabilities);
}

void SieveEditorWidget::slotAutoGenerateScripts()
{
    switch (mMode) {
    case TextMode:
        mTextModeWidget->autoGenerateScripts();
        break;
    case GraphicMode:
        break;
    }
}

void SieveEditorWidget::slotCheckSyntax()
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

void SieveEditorWidget::slotGenerateXml()
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

void SieveEditorWidget::slotSaveAs()
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

void SieveEditorWidget::slotImport()
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

void SieveEditorWidget::slotSwitchToGraphicalMode()
{
    mTextModeWidget->hideEditorWarning();
    changeMode(GraphicMode);
}

void SieveEditorWidget::slotSwitchMode()
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

void SieveEditorWidget::slotSwitchTextMode(const QString &script)
{
    changeMode(TextMode);
    mTextModeWidget->setScript(script);
}


