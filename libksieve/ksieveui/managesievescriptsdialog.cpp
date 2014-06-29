/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#include "managesievescriptsdialog.h"
#include "widgets/managesievetreeview.h"
#include "editor/sievetextedit.h"
#include "editor/sieveeditor.h"
#include "widgets/sievetreewidgetitem.h"

#include <klocale.h>
#include <kiconloader.h>
#include <kwindowsystem.h>
#include <QPushButton>
#include <QPushButton>
#include <kmessagebox.h>

#include <agentinstance.h>
#include <kmanagesieve/sievejob.h>
#include <ksieveui/util/util.h>

#include <QApplication>
#include <QTreeWidget>
#include <QVBoxLayout>

#include <errno.h>
#include <KSharedConfig>
#include <KGuiItem>
#include <KStandardGuiItem>

using namespace KSieveUi;

CustomManageSieveWidget::CustomManageSieveWidget(QWidget *parent)
    : KSieveUi::ManageSieveWidget(parent)
{

}

CustomManageSieveWidget::~CustomManageSieveWidget()
{

}

bool CustomManageSieveWidget::refreshList()
{
    bool noImapFound = true;
    SieveTreeWidgetItem *last = 0;
    Akonadi::AgentInstance::List lst = KSieveUi::Util::imapAgentInstances();
    foreach ( const Akonadi::AgentInstance &type, lst ) {
        if ( type.status() == Akonadi::AgentInstance::Broken )
            continue;

        QString serverName = type.name();
        last = new SieveTreeWidgetItem( treeView(), last );
        last->setIcon( 0, SmallIcon( QLatin1String("network-server") ) );

        const KUrl u = KSieveUi::Util::findSieveUrlForAccount( type.identifier() );
        if ( u.isEmpty() ) {
            QTreeWidgetItem *item = new QTreeWidgetItem( last );
            item->setText( 0, i18n( "No Sieve URL configured" ) );
            item->setFlags( item->flags() & ~Qt::ItemIsEnabled );
            treeView()->expandItem( last );
        } else {
            serverName += QString::fromLatin1(" (%1)").arg(u.userName());
            KManageSieve::SieveJob * job = KManageSieve::SieveJob::list( u );
            connect( job, SIGNAL(gotList(KManageSieve::SieveJob*,bool,QStringList,QString)),
                     this, SLOT(slotGotList(KManageSieve::SieveJob*,bool,QStringList,QString)) );
            mJobs.insert( job, last );
            mUrls.insert( last, u );
            last->startAnimation();
        }
        last->setText( 0, serverName );
        noImapFound = false;
    }
    return noImapFound;
}


ManageSieveScriptsDialog::ManageSieveScriptsDialog( QWidget * parent )
    : QDialog( parent ),
      mSieveEditor( 0 ),
      mIsNewScript( false ),
      mWasActive( false )
{
    setWindowTitle( i18n( "Manage Sieve Scripts" ) );
    setModal( false );
    setAttribute( Qt::WA_GroupLeader );
    setAttribute( Qt::WA_DeleteOnClose );
    KWindowSystem::setIcons( winId(), qApp->windowIcon().pixmap(IconSize(KIconLoader::Desktop),IconSize(KIconLoader::Desktop)), qApp->windowIcon().pixmap(IconSize(KIconLoader::Small),IconSize(KIconLoader::Small)) );
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QFrame *frame =new QFrame;
    mainLayout->addWidget( frame );
    QVBoxLayout * vlay = new QVBoxLayout( frame );
    vlay->setSpacing( 0 );
    vlay->setMargin( 0 );

    mTreeView = new CustomManageSieveWidget( frame);
    connect(mTreeView, SIGNAL(editScript(KUrl,QStringList)), SLOT(slotEditScript(KUrl,QStringList)));
    connect(mTreeView, SIGNAL(newScript(KUrl,QStringList)), SLOT(slotNewScript(KUrl,QStringList)));
    connect(mTreeView, SIGNAL(updateButtons(QTreeWidgetItem*)), SLOT(slotUpdateButtons(QTreeWidgetItem*)));
    vlay->addWidget( mTreeView );

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    vlay->addLayout( buttonLayout );

    mNewScript = new QPushButton( i18nc( "create a new sieve script", "New..." ) );
    connect( mNewScript, SIGNAL(clicked()), mTreeView, SLOT(slotNewScript()) );
    buttonLayout->addWidget( mNewScript );

    mEditScript = new QPushButton( i18n( "Edit..." ) );
    connect( mEditScript, SIGNAL(clicked()), mTreeView, SLOT(slotEditScript()) );
    buttonLayout->addWidget( mEditScript );

    mDeleteScript = new QPushButton( i18n( "Delete" ) );
    connect( mDeleteScript, SIGNAL(clicked()), mTreeView, SLOT(slotDeleteScript()) );
    buttonLayout->addWidget( mDeleteScript );

    mDeactivateScript = new QPushButton( i18n( "Deactivate" ) );
    connect( mDeactivateScript, SIGNAL(clicked()), mTreeView, SLOT(slotDeactivateScript()) );
    buttonLayout->addWidget( mDeactivateScript );

    QPushButton *close = new QPushButton;
    KGuiItem::assign(close, KStandardGuiItem::close() );
    connect( close, SIGNAL(clicked()), this, SLOT(accept()) );
    buttonLayout->addWidget( close );

    KConfigGroup group( KSharedConfig::openConfig(), "ManageSieveScriptsDialog" );
    const QSize size = group.readEntry( "Size", QSize() );
    if ( size.isValid() ) {
        resize( size );
    } else {
        resize( sizeHint().width(), sizeHint().height() );
    }

    mTreeView->slotRefresh();
}

ManageSieveScriptsDialog::~ManageSieveScriptsDialog()
{
    KConfigGroup group( KSharedConfig::openConfig(), "ManageSieveScriptsDialog" );
    group.writeEntry( "Size", size() );
}


void ManageSieveScriptsDialog::slotUpdateButtons(QTreeWidgetItem *item)
{
    Q_UNUSED(item);
    bool newScriptAction;
    bool editScriptAction;
    bool deleteScriptAction;
    bool desactivateScriptAction;
    mTreeView->enableDisableActions(newScriptAction, editScriptAction, deleteScriptAction, desactivateScriptAction);
    mNewScript->setEnabled( newScriptAction );
    mEditScript->setEnabled( editScriptAction );
    mDeleteScript->setEnabled( deleteScriptAction );
    mDeactivateScript->setEnabled( desactivateScriptAction );
}


void ManageSieveScriptsDialog::slotEditScript(const KUrl &url, const QStringList &capabilities)
{
    mCurrentURL = url;
    mCurrentCapabilities = capabilities;
    mIsNewScript = false;
    KManageSieve::SieveJob * job = KManageSieve::SieveJob::get( url );
    connect( job, SIGNAL(result(KManageSieve::SieveJob*,bool,QString,bool)),
             this, SLOT(slotGetResult(KManageSieve::SieveJob*,bool,QString,bool)) );
}

void ManageSieveScriptsDialog::slotNewScript(const KUrl &url, const QStringList &capabilities)
{
    mCurrentCapabilities = capabilities;
    mCurrentURL = url;
    mIsNewScript = true;
    slotGetResult( 0, true, QString(), false );
}

void ManageSieveScriptsDialog::slotGetResult( KManageSieve::SieveJob *, bool success, const QString & script, bool isActive )
{
    if ( !success )
        return;

    if ( mSieveEditor )
        return;

    disableManagerScriptsDialog(true);
    mSieveEditor = new SieveEditor;
    mSieveEditor->setScriptName( mCurrentURL.fileName() );
    mSieveEditor->setSieveCapabilities(mCurrentCapabilities);
    mSieveEditor->setScript( script );
    connect( mSieveEditor, SIGNAL(okClicked()), this, SLOT(slotSieveEditorOkClicked()) );
    connect( mSieveEditor, SIGNAL(cancelClicked()), this, SLOT(slotSieveEditorCancelClicked()) );
    connect( mSieveEditor, SIGNAL(checkSyntax()), this, SLOT(slotSieveEditorCheckSyntaxClicked()) );
    mSieveEditor->show();
    mWasActive = isActive;
}

void ManageSieveScriptsDialog::slotSieveEditorCheckSyntaxClicked()
{
    if ( !mSieveEditor )
        return;
    KManageSieve::SieveJob * job = KManageSieve::SieveJob::put( mCurrentURL,mSieveEditor->script(), mWasActive, mWasActive );
    job->setInteractive( false );
    connect( job, SIGNAL(errorMessage(KManageSieve::SieveJob*,bool,QString)),
             this, SLOT(slotPutResultDebug(KManageSieve::SieveJob*,bool,QString)) );
}

void ManageSieveScriptsDialog::slotSieveEditorOkClicked()
{
    if ( !mSieveEditor )
        return;
    disableManagerScriptsDialog(false);
    KManageSieve::SieveJob * job = KManageSieve::SieveJob::put( mCurrentURL,mSieveEditor->script(), mWasActive, mWasActive );
    connect( job, SIGNAL(result(KManageSieve::SieveJob*,bool,QString,bool)),
             this, SLOT(slotPutResult(KManageSieve::SieveJob*,bool)) );
}

void ManageSieveScriptsDialog::slotSieveEditorCancelClicked()
{
    disableManagerScriptsDialog(false);
    mSieveEditor->deleteLater();
    mSieveEditor = 0;
    mCurrentURL = KUrl();
    if ( mIsNewScript ) {
        mTreeView->slotRefresh();
    }
}

void ManageSieveScriptsDialog::slotPutResultDebug(KManageSieve::SieveJob *,bool success ,const QString &errorMsg)
{
    if ( success ) {
        mSieveEditor->addOkMessage( i18n( "No errors found." ) );
    } else {
        if ( errorMsg.isEmpty() )
            mSieveEditor->addFailedMessage( i18n( "An unknown error was encountered." ) );
        else
            mSieveEditor->addFailedMessage( errorMsg );
    }
    //Put original script after check otherwise we will put a script even if we don't click on ok
    KManageSieve::SieveJob * job = KManageSieve::SieveJob::put( mCurrentURL, mSieveEditor->originalScript(), mWasActive, mWasActive );
    job->setInteractive( false );
    mSieveEditor->resultDone();
}

void ManageSieveScriptsDialog::slotPutResult( KManageSieve::SieveJob *, bool success )
{
    if ( success ) {
        KMessageBox::information( this, i18n( "The Sieve script was successfully uploaded." ),
                                  i18n( "Sieve Script Upload" ) );
        mSieveEditor->deleteLater();
        mSieveEditor = 0;
        mCurrentURL = KUrl();
    } else {
        mSieveEditor->show();
    }
}

void ManageSieveScriptsDialog::disableManagerScriptsDialog(bool disable)
{
    setDisabled(disable);
}



