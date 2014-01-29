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
#include "sieveeditorwidget.h"

#include <klocale.h>
#include <KStandardGuiItem>

#include <QPushButton>
#include <QDebug>

using namespace KSieveUi;

SieveEditor::SieveEditor( QWidget * parent )
    : KDialog( parent )
{
    setCaption( i18n( "Edit Sieve Script" ) );
    setButtons( Ok|Cancel );

    setModal( true );
    mSieveEditorWidget = new SieveEditorWidget;
    connect(mSieveEditorWidget, SIGNAL(valueChanged()), this, SIGNAL(valueChanged()));
    setMainWidget(mSieveEditorWidget);
    connect(mSieveEditorWidget, SIGNAL(enableButtonOk(bool)), this, SLOT(slotEnableButtonOk(bool)));
    connect(mSieveEditorWidget, SIGNAL(checkSyntax()), this, SIGNAL(checkSyntax()));
    readConfig();
}

SieveEditor::~SieveEditor()
{
    writeConfig();
}


void SieveEditor::slotEnableButtonOk(bool b)
{
    enableButtonOk(b);
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
    return mSieveEditorWidget->script();
}

QString SieveEditor::originalScript() const
{
    return mSieveEditorWidget->originalScript();
}

void SieveEditor::setScript( const QString &script )
{
    mSieveEditorWidget->setScript(script);
}

void SieveEditor::setDebugScript( const QString &debug )
{
    mSieveEditorWidget->setDebugScript(debug);
}

void SieveEditor::setScriptName( const QString &name )
{
    mSieveEditorWidget->setScriptName(name);
}  

void SieveEditor::resultDone()
{
    mSieveEditorWidget->resultDone();
}

void SieveEditor::setSieveCapabilities( const QStringList &capabilities )
{
    mSieveEditorWidget->setSieveCapabilities(capabilities);
}

void SieveEditor::addFailedMessage(const QString &err)
{
    mSieveEditorWidget->addFailedMessage(err);
}

void SieveEditor::addOkMessage(const QString &msg)
{
    mSieveEditorWidget->addOkMessage(msg);
}


#include "moc_sieveeditor.cpp"
