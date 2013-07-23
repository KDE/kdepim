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
#include "sieveeditortextmodewidget.h"
#include "scriptsparsing/parsingutil.h"
#include "autocreatescripts/sieveeditorgraphicalmodewidget.h"

#include <klocale.h>
#include <KStandardGuiItem>

#include <QPushButton>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QLabel>
#include <QLineEdit>
#include <QToolBar>
#include <QDebug>
#include <QAction>

//#define GENERATE_XML_ACTION 1

using namespace KSieveUi;

SieveEditor::SieveEditor( QWidget * parent )
    : KDialog( parent ),
      mMode(TextMode)
{
    setCaption( i18n( "Edit Sieve Script" ) );
    setButtons( None );    
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    mOkButton = buttonBox->button(QDialogButtonBox::Ok);

    connect(buttonBox, SIGNAL(accepted()), this, SIGNAL(okClicked()));
    connect(buttonBox, SIGNAL(rejected()), this, SIGNAL(cancelClicked()));
    connect(this, SIGNAL(accepted()), this, SIGNAL(okClicked()));
    connect(this, SIGNAL(rejected()), this, SIGNAL(cancelClicked()));

    setModal( true );
    QWidget *w = new QWidget;

    QVBoxLayout *lay = new QVBoxLayout;

    QToolBar *bar = new QToolBar;
    mCheckSyntax = new QAction(i18n("Check Syntax"), this);
    connect(mCheckSyntax, SIGNAL(triggered(bool)), SLOT(slotCheckSyntax()));
    bar->addAction(mCheckSyntax);
    bar->addAction(KStandardGuiItem::saveAs().text(), this, SLOT(slotSaveAs()));
    bar->addAction(i18n("Import..."), this, SLOT(slotImport()));
    bar->addAction(i18n("Autogenerate Script..."), this, SLOT(slotAutoGenerateScripts()));
    mSwitchMode = new QAction(i18n("Switch Mode"), this);
    bar->addAction(mSwitchMode);
    connect(mSwitchMode, SIGNAL(triggered(bool)), SLOT(slotSwitchMode()));
#ifdef GENERATE_XML_ACTION
    bar->addAction(QLatin1String("Generate xml"), this, SLOT(slotGenerateXml()));
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
    lay->addWidget(buttonBox);
    connect(mTextModeWidget, SIGNAL(enableButtonOk(bool)), this, SLOT(slotEnableButtonOk(bool)));
    connect(mGraphicalModeWidget, SIGNAL(enableButtonOk(bool)), this, SLOT(slotEnableButtonOk(bool)));
    readConfig();
    setMainWidget( w );
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
    }
}

void SieveEditor::slotEnableButtonOk(bool b)
{
    mOkButton->setEnabled(b);
    mCheckSyntax->setEnabled( b );
}

void SieveEditor::writeConfig()
{
    KConfigGroup group( KGlobal::config(), "SieveEditor" );
    group.writeEntry( "Size", size() );
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
}

QString SieveEditor::script() const
{
    return mTextModeWidget->script();
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

void SieveEditor::setDebugColor( const QColor &col )
{
    mTextModeWidget->setDebugColor( col );
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
    switch (mMode) {
    case TextMode:
        mTextModeWidget->generateXml();
        break;
    case GraphicMode:
        break;
    }
}

void SieveEditor::slotSaveAs()
{
    switch (mMode) {
    case TextMode:
        mTextModeWidget->slotSaveAs();
        break;
    case GraphicMode:
        mTextModeWidget->slotSaveAs();
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
        mTextModeWidget->slotImport();
        break;
    }
}

void SieveEditor::slotSwitchMode()
{
    switch (mMode) {
    case TextMode: {
        bool result = false;
        const QDomDocument doc = ParsingUtil::parseScript(mTextModeWidget->currentscript(), result);
        if (result) {
            mGraphicalModeWidget->loadScript(doc);
            mTextModeWidget->hideEditorWarning();
            changeMode(GraphicMode);
        } else {
            mTextModeWidget->showEditorWarning();
            qDebug() << "can not parse file";
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

#include "sieveeditor.moc"

