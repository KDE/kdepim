/******************************************************************************
 *
 *  Copyright 2008 Szymon Tomasz Stefanek <pragma@kvirc.net>
 *
 *  This program is free softhisare; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Softhisare Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Softhisare
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *******************************************************************************/

#include "utils/optionseteditor.h"
#include "core/optionset.h"

#include <QLabel>
#include <QGridLayout>

#include <KLineEdit>
#include <KLocalizedString>
#include <KTextEdit>

using namespace MessageList::Utils;
using namespace MessageList::Core;

OptionSetEditor::OptionSetEditor( QWidget *parent )
    : QTabWidget( parent )
{
    // General tab
    QWidget * tab = new QWidget( this );
    addTab( tab, i18nc( "@title:tab General options of a view mode", "General" ) );

    QGridLayout * tabg = new QGridLayout( tab );

    QLabel * l = new QLabel( i18nc( "@label:textbox Name of the option", "Name:" ), tab );
    tabg->addWidget( l, 0, 0 );

    mNameEdit = new KLineEdit( tab );

    tabg->addWidget( mNameEdit, 0, 1 );

    connect( mNameEdit, SIGNAL(textEdited(QString)),
             SLOT(slotNameEditTextEdited(QString)) );

    l = new QLabel( i18nc( "@label:textbox Description of the option", "Description:" ), tab );
    tabg->addWidget( l, 1, 0 );

    mDescriptionEdit = new KTextEdit( tab );
    mDescriptionEdit->setAcceptRichText(false);
    tabg->addWidget( mDescriptionEdit, 1, 1, 2, 1 );

    tabg->setColumnStretch( 1, 1 );
    tabg->setRowStretch( 2, 1 );

}

OptionSetEditor::~OptionSetEditor()
{
}

void OptionSetEditor::setReadOnly( bool readOnly )
{
    mDescriptionEdit->setReadOnly( readOnly );
    mNameEdit->setReadOnly( readOnly );
}

KTextEdit *OptionSetEditor::descriptionEdit() const
{
    return mDescriptionEdit;
}

KLineEdit * OptionSetEditor::nameEdit() const
{
    return mNameEdit;
}


