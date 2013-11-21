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


#include "nepomukdebugdialog.h"
#include "utils.h"

#include "pimcommon/nepomukdebug/searchdebugnepomukshowdialog.h"
#include "pimcommon/nepomukdebug/akonadiresultlistview.h"
#include "pimcommon/texteditor/plaintexteditor/plaintexteditorwidget.h"
#include "pimcommon/texteditor/plaintexteditor/plaintexteditor.h"
#include <Akonadi/Item>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>

#include <KLocale>
#include <KMessageBox>

#include <QStringListModel>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QPointer>

NepomukDebugDialog::NepomukDebugDialog(QItemSelectionModel *selectionModel, QWidget *parent)
    : KDialog(parent)
{
    setCaption(i18n("Nepomuk Debug"));
    setButtons(User1|Close);
    setButtonText(User1, i18n("Search info with nepomukshow..."));
    setDefaultButton( Close );

    QWidget *w = new QWidget;
    QHBoxLayout *lay = new QHBoxLayout;
    w->setLayout(lay);
    mResult = new PimCommon::PlainTextEditorWidget;
    mResult->setReadOnly(true);

    mListView = new PimCommon::AkonadiResultListView;
    QStringListModel *resultModel = new QStringListModel( this );
    mListView->setModel( resultModel );

    lay->addWidget(mListView);
    lay->addWidget(mResult);

    setMainWidget( w );
    const Akonadi::Item::List lst = Utils::collectSelectedContactsItem(selectionModel);
    QStringList uidList;
    Q_FOREACH ( const Akonadi::Item &item, lst ) {
        uidList << QString::number( item.id() );
    }
    resultModel->setStringList( uidList );
    readConfig();
    connect(this, SIGNAL(user1Clicked()), this, SLOT(slotSearchInfoWithNepomuk()));
    connect( mListView, SIGNAL(activated(QModelIndex)), this, SLOT(slotShowItem(QModelIndex)) );
}

NepomukDebugDialog::~NepomukDebugDialog()
{
    writeConfig();
}

void NepomukDebugDialog::slotShowItem(const QModelIndex &index)
{
    if ( !index.isValid() )
        return;

    const QString uid = index.data( Qt::DisplayRole ).toString();
    Akonadi::ItemFetchJob *fetchJob = new Akonadi::ItemFetchJob( Akonadi::Item( uid.toLongLong() ) );
    fetchJob->fetchScope().fetchFullPayload();
    connect( fetchJob, SIGNAL(result(KJob*)), this, SLOT(slotItemFetched(KJob*)) );
}

void NepomukDebugDialog::slotItemFetched(KJob *job)
{
    mResult->editor()->clear();

    if ( job->error() ) {
        KMessageBox::error( this, i18n("Error on fetching item") );
        return;
    }

    Akonadi::ItemFetchJob *fetchJob = qobject_cast<Akonadi::ItemFetchJob*>( job );
    if ( !fetchJob->items().isEmpty() ) {
        const Akonadi::Item item = fetchJob->items().first();
        mResult->editor()->setPlainText( QString::fromUtf8( item.payloadData() ) );
    }
}

void NepomukDebugDialog::readConfig()
{
    KConfigGroup grp( KGlobal::config(), "NepomukDebugDialog" );
    const QSize size = grp.readEntry( "Size", QSize(300, 200) );
    if ( size.isValid() ) {
        resize( size );
    }
}

void NepomukDebugDialog::writeConfig()
{
    KConfigGroup grp( KGlobal::config(), "NepomukDebugDialog");
    grp.writeEntry( "Size", size() );
    grp.sync();
}

void NepomukDebugDialog::slotSearchInfoWithNepomuk()
{
    QString defaultValue;
    if (mResult->editor()->textCursor().hasSelection()) {
        defaultValue = mResult->editor()->textCursor().selectedText().trimmed();
    }
    const QString nepomukId = QInputDialog::getText(this, i18n("Search with nepomukshow"), i18n("Nepomuk id:"), QLineEdit::Normal, defaultValue);
    if (nepomukId.isEmpty())
        return;
    QPointer<PimCommon::SearchDebugNepomukShowDialog> dlg = new PimCommon::SearchDebugNepomukShowDialog(nepomukId, this);
    dlg->exec();
    delete dlg;
}

#include "moc_nepomukdebugdialog.cpp"
