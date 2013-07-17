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

#include <klocale.h>

#include <QPushButton>
#include <QDialogButtonBox>
#include <QVBoxLayout>

using namespace KSieveUi;

SieveEditor::SieveEditor( QWidget * parent )
    : KDialog( parent )
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
    lay->setMargin(0);
    w->setLayout(lay);
    mTextModeWidget = new SieveEditorTextModeWidget;
    lay->addWidget(mTextModeWidget);
    lay->addWidget(buttonBox);
    connect(mTextModeWidget, SIGNAL(checkSyntax()), SIGNAL(checkSyntax()));
    connect(mTextModeWidget, SIGNAL(enableButtonOk(bool)), this, SLOT(slotEnableButtonOk(bool)));
    readConfig();
    setMainWidget( w );
}

SieveEditor::~SieveEditor()
{
    writeConfig();
}

void SieveEditor::slotEnableButtonOk(bool b)
{
    mOkButton->setEnabled(b);
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
    mTextModeWidget->setScriptName( name );
}  

void SieveEditor::resultDone()
{
    mTextModeWidget->resultDone();
}

void SieveEditor::setSieveCapabilities( const QStringList &capabilities )
{
    mTextModeWidget->setSieveCapabilities(capabilities);
}

#include "sieveeditor.moc"

