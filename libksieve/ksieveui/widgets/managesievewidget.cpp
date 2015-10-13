/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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
#include <managescriptsjob/parseuserscriptjob.h>
#include <managescriptsjob/generateglobalscriptjob.h>
#include <util/util.h>

#include <KInputDialog>
#include <KStandardGuiItem>
#include <KLocalizedString>
#include <KMessageBox>

#include <QHBoxLayout>
#include <QMenu>
#include <QTimer>
#include <QDebug>
#include <qmetatype.h>

using namespace KSieveUi;
Q_DECLARE_METATYPE(QTreeWidgetItem*)

ManageSieveWidget::ManageSieveWidget(QWidget *parent)
    : QWidget(parent),
      mClearAll( false ),
      mBlockSignal( false )
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
    connect ( Solid::Networking::notifier(), SIGNAL(statusChanged(Solid::Networking::Status)),
              this, SLOT(slotSystemNetworkStatusChanged(Solid::Networking::Status)) );

    lay->addWidget(mTreeView);
    setLayout(lay);
    QTimer::singleShot(0,this, SLOT(slotCheckNetworkStatus()));
}

ManageSieveWidget::~ManageSieveWidget()
{
    clear();
}

void ManageSieveWidget::slotCheckNetworkStatus()
{
    slotSystemNetworkStatusChanged(Solid::Networking::status());
}

void ManageSieveWidget::slotSystemNetworkStatusChanged(Solid::Networking::Status status)
{
    if ( status == Solid::Networking::Connected || status == Solid::Networking::Unknown) {
        mTreeView->setEnabled(true);
        slotRefresh();
    } else {
        mTreeView->setEnabled(false);
        mTreeView->setNetworkDown(false);
    }
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

void ManageSieveWidget::slotNewScript()
{
    QTreeWidgetItem *currentItem = mTreeView->currentItem();
    if ( !currentItem )
        return;
    if ( currentItem->parent() )
        currentItem = currentItem->parent();
    if ( !currentItem )
        return;

    if ( !mUrls.count( currentItem ) )
        return;

    KUrl u = mUrls[currentItem];
    if ( u.isEmpty() )
        return;

    bool ok = false;
    const QString name = KInputDialog::getText( i18n( "New Sieve Script" ),
                                                i18n( "Please enter a name for the new Sieve script:" ),
                                                i18n( "unnamed" ), &ok, this );
    if ( !ok || name.isEmpty() )
        return;

    if (Util::isKep14ProtectedName(name)) {
        KMessageBox::error(this, i18n("You cannot use protected name."), i18n("New Script"));
        return;
    }

    u.setFileName( name );

    QTreeWidgetItem * parentItem = currentItem;
    if (parentItem) {
        const int numberOfElement(parentItem->childCount());
        for (int i = 0; i <numberOfElement; ++i) {
            if (parentItem->child(i)->text(0) == name) {
                KMessageBox::error(
                            this,
                            i18n( "Script name already used \"%1\".", name ),
                            i18n( "New Script" ) );
                return;
            }
        }
    }

    const QStringList currentCapabilities = currentItem->data(0, SIEVE_SERVER_CAPABILITIES).toStringList();

    mBlockSignal = true;
    QTreeWidgetItem *newItem = new QTreeWidgetItem( currentItem );
    newItem->setFlags(newItem->flags() & (Qt::ItemIsUserCheckable|Qt::ItemIsEnabled|Qt::ItemIsSelectable));
    newItem->setText(0,name);
    newItem->setCheckState(0,Qt::Unchecked);
    mBlockSignal = false;
    Q_EMIT newScript(u, currentCapabilities);
}

void ManageSieveWidget::slotEditScript()
{
    QTreeWidgetItem *currentItem = mTreeView->currentItem();
    if ( !isFileNameItem( currentItem ) )
        return;
    QTreeWidgetItem* parent = currentItem->parent();
    if ( !mUrls.count( parent ) )
        return;
    KUrl url = mUrls[parent];
    if ( url.isEmpty() )
        return;
    url.setFileName( currentItem->text(0) );
    const QStringList currentCapabilities = parent->data(0, SIEVE_SERVER_CAPABILITIES).toStringList();
    Q_EMIT editScript(url, currentCapabilities);
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

    if (item->data(0, SIEVE_SERVER_MODE).toInt() == Kep14EditorMode) {
        QStringList activeScripts;
        for(int i=0; i < item->childCount(); i++) {
            QTreeWidgetItem *j = item->child(i);
            if (itemIsActived(j)) {
                activeScripts << j->text(0);
            }
        }
        GenerateGlobalScriptJob *job = new GenerateGlobalScriptJob(u);
        job->addUserActiveScripts(activeScripts);
        connect( job, SIGNAL(success()), SLOT(slotRefresh()));
        connect( job, SIGNAL(error(QString)), SLOT(slotRefresh()));
        job->start();
        return;
    }

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
    Q_EMIT scriptDeleted(u);
}

void ManageSieveWidget::slotRefresh()
{
    mBlockSignal = true;
    clear();
    const bool noImapFound = refreshList();
    slotUpdateButtons();
    mTreeView->setNoImapFound(noImapFound);
    if (!noImapFound)
       mBlockSignal = false;
}

void ManageSieveWidget::slotUpdateButtons()
{
    Q_EMIT updateButtons(mTreeView->currentItem());
}

void ManageSieveWidget::slotGotList(KManageSieve::SieveJob *job, bool success, const QStringList &listScript, const QString &activeScript)
{
    qDebug()<<"void ManageSieveWidget::slotGotList(KManageSieve::SieveJob *job, bool success, const QStringList &listScript, const QString &activeScript) success: "<<success<<" listScript"<<listScript;
    if (mClearAll)
        return;
    QTreeWidgetItem * parent = mJobs[job];
    if ( !parent )
        return;
    (static_cast<SieveTreeWidgetItem*>(parent))->stopAnimation();

    mJobs.remove( job );
    if (!success) {
        mBlockSignal = false;
        parent->setData( 0, SIEVE_SERVER_ERROR, true );
        QTreeWidgetItem * item = new QTreeWidgetItem( parent );
        item->setText( 0, i18n( "Failed to fetch the list of scripts" ) );
        item->setFlags( item->flags() & ~Qt::ItemIsEnabled );
        mTreeView->expandItem( parent );
        return;
    }


    mBlockSignal = true; // don't trigger slotItemChanged
    Q_FOREACH (const QString &script, listScript) {
        //Hide protected name.
        if (Util::isKep14ProtectedName(script))
            continue;
        QTreeWidgetItem* item = new QTreeWidgetItem( parent );
        item->setFlags(item->flags() & (Qt::ItemIsUserCheckable|Qt::ItemIsEnabled|Qt::ItemIsSelectable));

        item->setText(0, script);
        const bool isActive = (script == activeScript);
        item->setCheckState(0, isActive ? Qt::Checked : Qt::Unchecked);
        if ( isActive ) {
            mSelectedItems[parent] = item;
        }
    }
    mBlockSignal = false;

    const bool hasKep14EditorMode = Util::hasKep14Support(job->sieveCapabilities(), listScript, activeScript);
    if (hasKep14EditorMode) {
        KUrl u = mUrls[parent];
        u.setFileName(QLatin1String("USER"));
        ParseUserScriptJob *parseJob = new ParseUserScriptJob(u);
        parseJob->setProperty(QLatin1String("parentItem").latin1(), QVariant::fromValue<QTreeWidgetItem*>(parent));
        connect(parseJob, SIGNAL(finished(ParseUserScriptJob*)), SLOT(setActiveScripts(ParseUserScriptJob*)));
        parseJob->start();
        (static_cast<SieveTreeWidgetItem*>(parent))->startAnimation();
    }

    parent->setData( 0, SIEVE_SERVER_CAPABILITIES, job->sieveCapabilities() );
    parent->setData( 0, SIEVE_SERVER_ERROR, false );
    parent->setData( 0, SIEVE_SERVER_MODE, hasKep14EditorMode ? Kep14EditorMode : NormalEditorMode);
    mTreeView->expandItem( parent );
}

void ManageSieveWidget::setActiveScripts(ParseUserScriptJob *job)
{
    QTreeWidgetItem * parent = job->property(QLatin1String("parentItem").latin1()).value<QTreeWidgetItem*>();
    if ( !parent ) {
        return;
    }
    (static_cast<SieveTreeWidgetItem*>(parent))->stopAnimation();

    if (!job->error().isEmpty()) {
        qWarning() << job->error();
        return;
    }

    mBlockSignal = true; // don't trigger slotItemChanged
    const QStringList activeScriptList = job->activeScriptList();
    QStringList scriptOrder = activeScriptList;
    QMap<QString, QTreeWidgetItem*> scriptMap;

    const int children = parent->childCount();
    for(int i=0; i < children; i++) {
        QTreeWidgetItem *item = parent->takeChild(0);
        scriptMap.insert(item->text(0), item);
        const bool isActive = activeScriptList.contains(item->text(0));
        item->setCheckState(0, isActive ? Qt::Checked : Qt::Unchecked);
        if (!isActive) {
            scriptOrder <<  item->text(0);
        }
    }

    foreach(const QString &scriptName, scriptOrder) {
        parent->addChild(scriptMap[scriptName]);
    }

    mBlockSignal = false;
}


void ManageSieveWidget::slotDoubleClicked( QTreeWidgetItem * item )
{
    if ( !isFileNameItem( item ) )
        return;
    slotEditScript();
}

void ManageSieveWidget::enableDisableActions(bool &newScriptAction, bool &editScriptAction, bool &deleteScriptAction, bool &desactivateScriptAction)
{
    QTreeWidgetItem * item = mTreeView->currentItem();

    bool enabled = true;
    if ( !item )
        enabled = false;
    else if ( !item->parent() && !mUrls.count( item ))
        enabled = false;

    if ( !enabled ) {
        newScriptAction = false;
        editScriptAction = false;
        deleteScriptAction = false;
        desactivateScriptAction = false;
    } else {
        if ( serverHasError(item) || !mJobs.keys(item).isEmpty())
            newScriptAction =  false;
        else
            newScriptAction = mUrls.count( item );
        enabled = isFileNameItem( item );
        editScriptAction = enabled;
        deleteScriptAction = enabled;
        desactivateScriptAction = enabled && itemIsActived( item );
    }
}
