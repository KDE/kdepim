/*
  Copyright (c) 2014 Montel Laurent <montel.org>

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

#include "managesievewidget.h"
#include "managesievetreeview.h"
#include "widgets/sievetreewidgetitem.h"


#include <kmanagesieve/sievejob.h>

#include <KStandardGuiItem>
#include <KLocalizedString>
#include <KMessageBox>

#include <QHBoxLayout>
#include <QMenu>


using namespace KSieveUi;
ManageSieveWidget::ManageSieveWidget(QWidget *parent)
    : QWidget(parent),
      mClearAll( false )
{
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setMargin(0);

    mTreeView = new ManageSieveTreeView;
#ifndef QT_NO_CONTEXTMENU
    connect( mTreeView, SIGNAL(customContextMenuRequested(QPoint)),
             this, SLOT(slotContextMenuRequested(QPoint)) );
#endif
    connect( mTreeView, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
             this, SLOT(slotDoubleClicked(QTreeWidgetItem*)) );
    connect( mTreeView, SIGNAL(itemSelectionChanged()),
             this, SLOT(slotUpdateButtons()) );
    connect( mTreeView, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
             this, SLOT(slotItemChanged(QTreeWidgetItem*,int)));
    lay->addWidget(mTreeView);
    setLayout(lay);
}

ManageSieveWidget::~ManageSieveWidget()
{

}

ManageSieveTreeView *ManageSieveWidget::treeView() const
{
    return mTreeView;
}

void ManageSieveWidget::killAllJobs()
{
    mClearAll = true;
    QMap<KManageSieve::SieveJob*,QTreeWidgetItem*>::const_iterator it = mJobs.constBegin();
    while (it != mJobs.constEnd()) {
        it.key()->kill();
        ++it;
    }
    mClearAll = false;
    mJobs.clear();
}


bool ManageSieveWidget::serverHasError(QTreeWidgetItem *item) const
{
    const QVariant variant = item->data( 0, SIEVE_SERVER_ERROR );
    if ( variant.isValid() && variant.toBool()==true )
        return true;
    return false;
}

void ManageSieveWidget::slotItemChanged(QTreeWidgetItem *item, int col)
{
    if (!item || mBlockSignal || (col != 0) ) {
        return;
    }
    if ( !isFileNameItem( item ) )
        return;
    QTreeWidgetItem *parent = item->parent();
    if ( (mSelectedItems[parent] != item) && itemIsActived( item )) {
        mSelectedItems[parent] = item;
        changeActiveScript( parent, true );
    } else {
        mSelectedItems[parent] = item;
        changeActiveScript( parent, false );
    }
}

void ManageSieveWidget::slotContextMenuRequested( const QPoint& p )
{
    QTreeWidgetItem *item = mTreeView->itemAt( p );
    if ( !item )
        return;
    if ( !item->parent() && !mUrls.count( item ))
        return;
    QMenu menu;
    if ( isFileNameItem( item ) ) {
        // script items:
        menu.addAction( i18n( "Edit Script..." ), this, SLOT(slotEditScript()) );
        menu.addAction( i18n( "Delete Script" ), this, SLOT(slotDeleteScript()) );
        if ( itemIsActived( item ) ) {
            menu.addSeparator();
            menu.addAction( i18n( "Deactivate Script" ), this, SLOT(slotDeactivateScript()) );
        }
    } else if ( !item->parent() ) {
        // top-levels:
        if ( !serverHasError(item) && mJobs.keys(item).isEmpty())
            menu.addAction( i18n( "New Script..." ), this, SLOT(slotNewScript()) );
    }
    if ( !menu.actions().isEmpty() )
        menu.exec( mTreeView->viewport()->mapToGlobal(p) );
}

void ManageSieveWidget::slotDeactivateScript()
{
    QTreeWidgetItem * item = mTreeView->currentItem();
    if ( !isFileNameItem( item ) )
        return;
    QTreeWidgetItem *parent = item->parent();
    if ( itemIsActived( item ) ) {
        mSelectedItems[parent] = item;
        changeActiveScript( parent, false );
    }
}

void ManageSieveWidget::changeActiveScript( QTreeWidgetItem *item, bool activate )
{
    if ( !item )
        return;
    if ( !mUrls.count( item ) )
        return;
    if ( !mSelectedItems.count( item ) )
        return;
    KUrl u = mUrls[item];
    if ( u.isEmpty() )
        return;
    QTreeWidgetItem* selected = mSelectedItems[item];
    if ( !selected )
        return;
    u.setFileName( selected->text(0) );

    KManageSieve::SieveJob * job;
    if ( activate )
        job = KManageSieve::SieveJob::activate( u );
    else
        job = KManageSieve::SieveJob::deactivate( u );
    mBlockSignal = true;
    connect( job, SIGNAL(result(KManageSieve::SieveJob*,bool,QString,bool)),
             this, SLOT(slotRefresh()) );
}

bool ManageSieveWidget::itemIsActived( QTreeWidgetItem *item ) const
{
    Q_ASSERT( item && item->parent() );
    return (item->checkState(0) == Qt::Checked);
}

bool ManageSieveWidget::isFileNameItem( QTreeWidgetItem *item ) const
{
    if ( !item || !item->parent() )
        return false;
    return (item->flags() & Qt::ItemIsEnabled);
}

void ManageSieveWidget::clear()
{
    killAllJobs();
    mSelectedItems.clear();
    mUrls.clear();
    mTreeView->clear();
}

void ManageSieveWidget::slotDeleteScript()
{
    QTreeWidgetItem * currentItem =  mTreeView->currentItem();
    if ( !isFileNameItem( currentItem ) )
        return;

    QTreeWidgetItem *parent = currentItem->parent();
    if ( !parent )
        return;

    if ( !mUrls.count( parent ) )
        return;

    KUrl u = mUrls[parent];
    if ( u.isEmpty() )
        return;

    u.setFileName( currentItem->text(0) );

    if ( KMessageBox::warningContinueCancel( this, i18n( "Really delete script \"%1\" from the server?", u.fileName() ),
                                             i18n( "Delete Sieve Script Confirmation" ),
                                             KStandardGuiItem::del() )
         != KMessageBox::Continue )
        return;
    KManageSieve::SieveJob * job = KManageSieve::SieveJob::del( u );
    connect( job, SIGNAL(result(KManageSieve::SieveJob*,bool,QString,bool)),
             this, SLOT(slotRefresh()) );
}

bool ManageSieveWidget::isProtectedName(const QString &name)
{
    if (name == QLatin1String("master") ||
            name == QLatin1String("user") ||
            name == QLatin1String("management")) {
        return true;
    }
    return false;
}

void ManageSieveWidget::slotRefresh()
{
    mBlockSignal = true;
    clear();
    const bool noImapFound = refreshList();
    /*
    SieveTreeWidgetItem *last = 0;
    Akonadi::AgentInstance::List lst = KSieveUi::Util::imapAgentInstances();
    bool noImapFound = true;
    foreach ( const Akonadi::AgentInstance &type, lst ) {
        if ( type.status() == Akonadi::AgentInstance::Broken )
            continue;

        last = new SieveTreeWidgetItem( mTreeView, last );
        last->setText( 0, type.name() );
        last->setIcon( 0, SmallIcon( QLatin1String("network-server") ) );

        const KUrl u = KSieveUi::Util::findSieveUrlForAccount( type.identifier() );
        if ( u.isEmpty() ) {
            QTreeWidgetItem *item = new QTreeWidgetItem( last );
            item->setText( 0, i18n( "No Sieve URL configured" ) );
            item->setFlags( item->flags() & ~Qt::ItemIsEnabled );
            mTreeView->expandItem( last );
        } else {
            KManageSieve::SieveJob * job = KManageSieve::SieveJob::list( u );
            connect( job, SIGNAL(gotList(KManageSieve::SieveJob*,bool,QStringList,QString)),
                     this, SLOT(slotGotList(KManageSieve::SieveJob*,bool,QStringList,QString)) );
            mJobs.insert( job, last );
            mUrls.insert( last, u );
            last->startAnimation();
        }
        noImapFound = false;
    }
    */
    //FIXME slotUpdateButtons();
    mTreeView->setNoImapFound(noImapFound);
}
