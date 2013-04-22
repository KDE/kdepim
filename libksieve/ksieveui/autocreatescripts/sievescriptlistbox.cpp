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
#include <KPushButton>

#include <QVBoxLayout>
#include <QListWidget>
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

QString SieveScriptListItem::generatedScript(QStringList &requires) const
{
    QString script;
    if (!mDescription.isEmpty()) {
        script = QLatin1Char('#') + i18n("Description:") + QLatin1Char('\n') + mDescription;
        script.replace(QLatin1Char('\n'), QLatin1String("\n#"));
        script += QLatin1Char('\n');
    }
    if (mScriptPage) {
        mScriptPage->generatedScript(script, requires);
    }
    return script;
}

SieveScriptListBox::SieveScriptListBox(const QString &title, QWidget *parent)
    : QGroupBox(title, parent)
{
    QVBoxLayout *layout = new QVBoxLayout();
    mSieveListScript = new QListWidget;
    layout->addWidget(mSieveListScript);

    //----------- the first row of buttons
    KHBox *hb = new KHBox( this );
    hb->setSpacing( 4 );

    mBtnTop = new KPushButton( QString(), hb );
    mBtnTop->setIcon( KIcon( QLatin1String("go-top") ) );
    mBtnTop->setIconSize( QSize( KIconLoader::SizeSmall, KIconLoader::SizeSmall ) );
    mBtnTop->setMinimumSize( mBtnTop->sizeHint() * 1.2 );

    mBtnUp = new KPushButton( QString(), hb );
    mBtnUp->setAutoRepeat( true );
    mBtnUp->setIcon( KIcon( QLatin1String("go-up") ) );
    mBtnUp->setIconSize( QSize( KIconLoader::SizeSmall, KIconLoader::SizeSmall ) );
    mBtnUp->setMinimumSize( mBtnUp->sizeHint() * 1.2 );
    mBtnDown = new KPushButton( QString(), hb );
    mBtnDown->setAutoRepeat( true );
    mBtnDown->setIcon( KIcon( QLatin1String("go-down") ) );
    mBtnDown->setIconSize( QSize( KIconLoader::SizeSmall, KIconLoader::SizeSmall ) );
    mBtnDown->setMinimumSize( mBtnDown->sizeHint() * 1.2 );

    mBtnBottom = new KPushButton( QString(), hb );
    mBtnBottom->setIcon( KIcon( QLatin1String("go-bottom") ) );
    mBtnBottom->setIconSize( QSize( KIconLoader::SizeSmall, KIconLoader::SizeSmall ) );
    mBtnBottom->setMinimumSize( mBtnBottom->sizeHint() * 1.2 );

    mBtnUp->setToolTip( i18nc( "Move selected filter up.", "Up" ) );
    mBtnDown->setToolTip( i18nc( "Move selected filter down.", "Down" ) );
    mBtnTop->setToolTip( i18nc( "Move selected filter to the top.", "Top" ) );
    mBtnBottom->setToolTip( i18nc( "Move selected filter to the bottom.", "Bottom" ) );

    layout->addWidget( hb );

    hb = new KHBox( this );
    hb->setSpacing( 4 );

    mBtnNew = new KPushButton( QString(), hb );
    mBtnNew->setIcon( KIcon( QLatin1String("document-new") ) );
    mBtnNew->setIconSize( QSize( KIconLoader::SizeSmall, KIconLoader::SizeSmall ) );
    mBtnNew->setMinimumSize( mBtnNew->sizeHint() * 1.2 );

    mBtnDelete = new KPushButton( QString(), hb );
    mBtnDelete->setIcon( KIcon( QLatin1String("edit-delete") ) );
    mBtnDelete->setIconSize( QSize( KIconLoader::SizeSmall, KIconLoader::SizeSmall ) );
    mBtnDelete->setMinimumSize( mBtnDelete->sizeHint() * 1.2 );

    mBtnRename = new KPushButton( i18n( "Rename..." ), hb );

    mBtnDescription = new KPushButton( i18n( "Edit description..." ), hb );


    layout->addWidget( hb );
    setLayout( layout );

    connect( mBtnNew, SIGNAL(clicked()), this, SLOT(slotNew()));
    connect( mBtnDelete, SIGNAL(clicked()), this, SLOT(slotDelete()));
    connect( mBtnRename, SIGNAL(clicked()), this, SLOT(slotRename()));
    connect( mBtnDescription, SIGNAL(clicked()), this, SLOT(slotEditDescription()));

    connect( mBtnUp, SIGNAL(clicked()), this, SLOT(slotUp()) );
    connect( mBtnDown, SIGNAL(clicked()), this, SLOT(slotDown()) );
    connect( mBtnTop, SIGNAL(clicked()), this, SLOT(slotTop()) );
    connect( mBtnBottom, SIGNAL(clicked()), this, SLOT(slotBottom()) );


    connect( mSieveListScript, SIGNAL(itemSelectionChanged()), SLOT(updateButtons()));
    connect( mSieveListScript, SIGNAL(itemActivated(QListWidgetItem*)), SLOT(slotItemActived(QListWidgetItem*)));
    connect( mSieveListScript, SIGNAL(itemDoubleClicked(QListWidgetItem*)), SLOT(slotEditDescription()));
    updateButtons();
}

SieveScriptListBox::~SieveScriptListBox()
{
}

void SieveScriptListBox::slotItemActived(QListWidgetItem* item)
{
    if (item) {
        SieveScriptListItem *itemScript = static_cast<SieveScriptListItem*>(item);
        Q_EMIT activatePage(itemScript->scriptPage());
    }
}

void SieveScriptListBox::updateButtons()
{
    const int currentIndex = mSieveListScript->currentRow();
    const bool theFirst = ( currentIndex == 0 );
    const int numberOfElement( mSieveListScript->count() );
    const bool theLast = ( currentIndex >= numberOfElement - 1 );

    const QList<QListWidgetItem*> lst = mSieveListScript->selectedItems();
    mBtnDelete->setEnabled(!lst.isEmpty());
    mBtnRename->setEnabled(lst.count() == 1);
    mBtnDescription->setEnabled(lst.count() == 1);
    mBtnBottom->setEnabled(!lst.isEmpty() && !theLast);
    mBtnTop->setEnabled(!lst.isEmpty() && !theFirst);
    mBtnDown->setEnabled(!lst.isEmpty() && !theLast);
    mBtnUp->setEnabled(!lst.isEmpty() && !theFirst);
}

void SieveScriptListBox::slotNew()
{
    const QString newName = KInputDialog::getText(i18n("New Script"), i18n("Add new name:"));
    if (!newName.isEmpty()) {
        SieveScriptListItem *item = new SieveScriptListItem(newName, mSieveListScript);
        SieveScriptPage *page = new SieveScriptPage(mSieveCapabilities);
        item->setScriptPage(page);
        Q_EMIT addNewPage(page);
        mSieveListScript->setCurrentItem(item);
        updateButtons();
    }
}

void SieveScriptListBox::slotDelete()
{
    QListWidgetItem *item = mSieveListScript->currentItem();
    if (item) {
        if (KMessageBox::warningYesNo(this, i18n("Do you want to delete \"%1\" script?", item->text()), i18n("Delete script")) == KMessageBox::Yes) {
            SieveScriptListItem *itemScript = static_cast<SieveScriptListItem*>(item);
            Q_EMIT removePage(itemScript->scriptPage());
            delete item;
            updateButtons();
        }
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
    QListWidgetItem *item = mSieveListScript->currentItem();
    if (item) {
        SieveScriptListItem *sieveItem = static_cast<SieveScriptListItem*>(item);
        QPointer<SieveScriptDescriptionDialog> dlg = new SieveScriptDescriptionDialog(this);
        dlg->setDescription(sieveItem->description());
        if (dlg->exec()) {
            sieveItem->setDescription(dlg->description());
        }
        delete dlg;
    }
}

void SieveScriptListBox::slotTop()
{
    QListWidgetItem *item = mSieveListScript->currentItem();
    if (item) {
        const int currentIndex = mSieveListScript->currentRow();
        if (currentIndex != 0) {
            item = mSieveListScript->takeItem( currentIndex );
            mSieveListScript->insertItem( 0, item );
            mSieveListScript->setCurrentItem(item);
        }
    }
}

void SieveScriptListBox::slotBottom()
{
    QListWidgetItem *item = mSieveListScript->currentItem();
    if (item) {
        const int currentIndex = mSieveListScript->currentRow();
        if (currentIndex != mSieveListScript->count() - 1 ) {
            item = mSieveListScript->takeItem( currentIndex );
            mSieveListScript->insertItem( mSieveListScript->count() , item );
            mSieveListScript->setCurrentItem(item);
        }
    }
}

void SieveScriptListBox::slotDown()
{
    QListWidgetItem *item = mSieveListScript->currentItem();
    if (item) {
        const int currentIndex = mSieveListScript->currentRow();
        if (currentIndex < mSieveListScript->count() - 1 ) {
            item = mSieveListScript->takeItem( currentIndex );
            mSieveListScript->insertItem( currentIndex + 1, item );
            mSieveListScript->setCurrentItem(item);
        }
    }
}

void SieveScriptListBox::slotUp()
{
    QListWidgetItem *item = mSieveListScript->currentItem();
    if (item) {
        const int currentIndex = mSieveListScript->currentRow();
        if (currentIndex >= 1 ) {
            item = mSieveListScript->takeItem( currentIndex );
            mSieveListScript->insertItem( currentIndex - 1, item );
            mSieveListScript->setCurrentItem(item);
        }
    }
}


QString SieveScriptListBox::generatedScript(QString &requires) const
{
    QString resultScript;
    QStringList lstRequires;
    const int numberOfScripts(mSieveListScript->count());
    for (int i = 0; i< numberOfScripts; ++i) {
        SieveScriptListItem* item = static_cast<SieveScriptListItem*>(mSieveListScript->item(i));
        if (i != 0)
            resultScript += QLatin1Char('\n');
        resultScript += QLatin1Char('#') + i18n("Script name: %1",item->text()) + QLatin1String("\n\n");
        resultScript += item->generatedScript(lstRequires);
    }

    Q_FOREACH (const QString &r, lstRequires) {
        requires += QString::fromLatin1("require \"%1\";\n").arg(r);
    }

    return resultScript;
}

void SieveScriptListBox::setSieveCapabilities( const QStringList &capabilities )
{
    mSieveCapabilities = capabilities;
}

#include "sievescriptlistbox.moc"
