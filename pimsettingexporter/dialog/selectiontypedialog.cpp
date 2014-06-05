/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>
  
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

#include "selectiontypedialog.h"
#include "widgets/selectiontypetreewidget.h"

#include <KGlobal>
#include <KLocalizedString>
#include <QHBoxLayout>
#include <QPushButton>
#include <KSharedConfig>

SelectionTypeDialog::SelectionTypeDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n( "Select Type" ) );
    setButtons( Ok|Cancel );
    setDefaultButton( Ok );
    setModal( true );
    QWidget *mainWidget = new QWidget( this );
    QVBoxLayout *mainLayout = new QVBoxLayout( mainWidget );
    mainLayout->setSpacing( KDialog::spacingHint() );
    mainLayout->setMargin( KDialog::marginHint() );
    mSelectionTreeWidget = new SelectionTypeTreeWidget(this);
    mainLayout->addWidget(mSelectionTreeWidget);

    QHBoxLayout *hbox = new QHBoxLayout;
    QPushButton *selectAll = new QPushButton(i18n("Select All"));
    connect(selectAll, SIGNAL(clicked()), this, SLOT(slotSelectAll()));
    hbox->addWidget(selectAll);

    QPushButton *unselectAll = new QPushButton(i18n("Unselect All"));
    connect(unselectAll, SIGNAL(clicked()), this, SLOT(slotUnselectAll()));
    hbox->addWidget(unselectAll);

    QPushButton *saveTemplate = new QPushButton(i18n("Save as Template..."));
    connect(saveTemplate, SIGNAL(clicked()), this, SLOT(slotSaveAsTemplate()));
    hbox->addWidget(saveTemplate);

    QPushButton *loadTemplate = new QPushButton(i18n("Load Template..."));
    connect(loadTemplate, SIGNAL(clicked()), this, SLOT(slotLoadTemplate()));
    hbox->addWidget(loadTemplate);


    mainLayout->addLayout(hbox);

    setMainWidget(mainWidget);
    readConfig();
}

SelectionTypeDialog::~SelectionTypeDialog()
{
    writeConfig();
}

void SelectionTypeDialog::writeConfig()
{
    KConfigGroup group( KSharedConfig::openConfig(), "SelectionTypeDialog" );
    group.writeEntry( "Size", size() );
}

void SelectionTypeDialog::readConfig()
{
    KConfigGroup group( KSharedConfig::openConfig(), "SelectionTypeDialog" );
    const QSize sizeDialog = group.readEntry( "Size", QSize(600,400) );
    if ( sizeDialog.isValid() ) {
        resize( sizeDialog );
    }
}

QHash<Utils::AppsType, Utils::importExportParameters> SelectionTypeDialog::storedType() const
{
    return mSelectionTreeWidget->storedType();
}

void SelectionTypeDialog::loadTemplate(const QString &fileName)
{
    if (!fileName.isEmpty())
        mSelectionTreeWidget->loadTemplate(fileName);
}

void SelectionTypeDialog::slotSelectAll()
{
    mSelectionTreeWidget->selectAllItems();
}

void SelectionTypeDialog::slotUnselectAll()
{
    mSelectionTreeWidget->unSelectAllItems();
}

void SelectionTypeDialog::slotSaveAsTemplate()
{
    mSelectionTreeWidget->saveAsTemplate();
}

void SelectionTypeDialog::slotLoadTemplate()
{
    mSelectionTreeWidget->loadTemplate();
}
