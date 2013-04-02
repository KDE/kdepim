/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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
#include "sievescriptlistbox.h"
#include "sievescriptdescriptiondialog.h"
#include "sievescriptpage.h"

#include <KHBox>
#include <KMessageBox>
#include <KLocale>
#include <KInputDialog>

#include <QVBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QPointer>


using namespace KSieveUi;

SieveScriptListItem::SieveScriptListItem( const QString &text, QListWidget *parent )
    : QListWidgetItem( text, parent ),
      mScriptPage(0)
{

}

SieveScriptListItem::~SieveScriptListItem()
{
}

void SieveScriptListItem::setDescription(const QString & desc)
{
    mDescription = desc;
}

QString SieveScriptListItem::description() const
{
    return mDescription;
}

SieveScriptPage *SieveScriptListItem::scriptPage() const
{
    return mScriptPage;
}

void SieveScriptListItem::setScriptPage(SieveScriptPage *page)
{
    mScriptPage = page;
}

QString SieveScriptListItem::generatedScript() const
{
    QString script;
    if (!mDescription.isEmpty()) {
        script = QLatin1Char('#') + i18n("Description:") + QLatin1Char('\n') + mDescription;
        script.replace(QLatin1Char('\n'), QLatin1String("\n#"));
    }
    if (mScriptPage) {
        mScriptPage->generatedScript(script);
    }
    return script;
}

SieveScriptListBox::SieveScriptListBox(const QString &title, QWidget *parent)
    : QGroupBox(title, parent)
{
    QVBoxLayout *layout = new QVBoxLayout();
    mSieveListScript = new QListWidget;
    layout->addWidget(mSieveListScript);

    KHBox *hb = new KHBox( this );
    hb->setSpacing( 4 );

    mBtnNew = new QPushButton( QString(), hb );
    mBtnNew->setIcon( KIcon( "document-new" ) );
    mBtnNew->setIconSize( QSize( KIconLoader::SizeSmall, KIconLoader::SizeSmall ) );
    mBtnNew->setMinimumSize( mBtnNew->sizeHint() * 1.2 );

    mBtnDelete = new QPushButton( QString(), hb );
    mBtnDelete->setIcon( KIcon( "edit-delete" ) );
    mBtnDelete->setIconSize( QSize( KIconLoader::SizeSmall, KIconLoader::SizeSmall ) );
    mBtnDelete->setMinimumSize( mBtnDelete->sizeHint() * 1.2 );

    mBtnRename = new QPushButton( i18n( "Rename..." ), hb );


    layout->addWidget( hb );
    setLayout( layout );

    connect( mBtnNew, SIGNAL(clicked()), this, SLOT(slotNew()) );
    connect( mBtnDelete, SIGNAL(clicked()), this, SLOT(slotDelete()) );
    connect( mBtnRename, SIGNAL(clicked()), this, SLOT(slotRename()) );
    connect( mSieveListScript, SIGNAL(itemSelectionChanged()), SLOT(updateButtons()));
}

SieveScriptListBox::~SieveScriptListBox()
{
}

void SieveScriptListBox::updateButtons()
{
    const QList<QListWidgetItem*> lst = mSieveListScript->selectedItems();
    mBtnDelete->setEnabled(!lst.isEmpty());
    mBtnRename->setEnabled(lst.count() == 1);
}

void SieveScriptListBox::slotNew()
{
    const QString newName = KInputDialog::getText(i18n("New Script"), i18n("Add new name:"));
    if (!newName.isEmpty()) {
        SieveScriptListItem *item = new SieveScriptListItem(newName, mSieveListScript);
        //TODO create uniq name
    }
}

void SieveScriptListBox::slotDelete()
{
    QListWidgetItem *item = mSieveListScript->currentItem();
    if (item) {
        delete item;
    }
}

void SieveScriptListBox::slotRename()
{
    QListWidgetItem *item = mSieveListScript->currentItem();
    if (item) {
        const QString newName = KInputDialog::getText(i18n("Rename"), i18n("Add new name:"), item->text());
        if (!newName.isEmpty()) {
            item->setText(newName);
        }
    }
}

void SieveScriptListBox::slotEditDescription()
{
    if (mSieveListScript->currentItem()) {
        SieveScriptListItem *item = static_cast<SieveScriptListItem*>(mSieveListScript->currentItem());
        QPointer<SieveScriptDescriptionDialog> dlg = new SieveScriptDescriptionDialog(this);
        dlg->setDescription(item->description());
        if (dlg->exec()) {
            item->setDescription(dlg->description());
        }
        delete dlg;
    }
}

QString SieveScriptListBox::generatedScript() const
{
    QString resultScript;
    for (int i = 0; i< mSieveListScript->count(); ++i) {
        SieveScriptListItem* item = static_cast<SieveScriptListItem*>(mSieveListScript->item(i));
        if (i != 0)
            resultScript += QLatin1Char('\n');
        resultScript += QLatin1Char('#') + i18n("Script name: %1",item->text()) + QLatin1Char('\n');
        resultScript = item->generatedScript();
    }
    return resultScript;
}

#include "sievescriptlistbox.moc"
