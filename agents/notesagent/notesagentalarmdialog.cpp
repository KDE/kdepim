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

#include "notesagentalarmdialog.h"
#include "notesagentnotedialog.h"
#include "noteshared/widget/notelistwidget.h"
#include "noteshared/attributes/notealarmattribute.h"

#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/ItemModifyJob>

#include <KLocalizedString>
#include <KGlobal>
#include <KLocale>
#include <KDateTime>
#include <KMenu>
#include <KAction>

#include <QListWidget>
#include <QLabel>
#include <QVBoxLayout>

NotesAgentAlarmDialog::NotesAgentAlarmDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n( "Alarm" ) );
    setWindowIcon( KIcon( QLatin1String("knotes") ) );
    setButtons( Close );
    setDefaultButton( Close );
    QWidget *w = new QWidget;
    QVBoxLayout *vbox = new QVBoxLayout;
    w->setLayout(vbox);

    mCurrentDateTime = new QLabel;
    mCurrentDateTime->setText(KGlobal::locale()->formatDateTime(QDateTime::currentDateTime()));
    vbox->addWidget(mCurrentDateTime);

    QLabel *lab = new QLabel(i18n("The following notes triggered alarms:"));
    vbox->addWidget(lab);
    mListWidget = new NoteShared::NoteListWidget;
    mListWidget->setContextMenuPolicy( Qt::CustomContextMenu );
    connect(mListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(slotItemDoubleClicked(QListWidgetItem*)));
    connect( mListWidget, SIGNAL(customContextMenuRequested(QPoint)),
             this, SLOT(slotCustomContextMenuRequested(QPoint)) );

    vbox->addWidget(mListWidget);
    setMainWidget(w);
    readConfig();
}

NotesAgentAlarmDialog::~NotesAgentAlarmDialog()
{
    writeConfig();
}

void NotesAgentAlarmDialog::slotCustomContextMenuRequested(const QPoint &pos)
{
    if ( mListWidget->selectedItems().isEmpty() )
        return;
    Q_UNUSED(pos);
    KMenu *entriesContextMenu = new KMenu;
    KAction *removeAlarm = new KAction(i18n("Remove Alarm"), entriesContextMenu);
    connect(removeAlarm, SIGNAL(triggered()), this, SLOT(slotRemoveAlarm()));
    KAction *showNote = new KAction(i18n("Show Note..."), entriesContextMenu);
    connect(showNote, SIGNAL(triggered()), this, SLOT(slotShowAlarm()));
    entriesContextMenu->addAction( showNote );
    entriesContextMenu->addAction( removeAlarm );
    entriesContextMenu->exec( QCursor::pos() );
    delete entriesContextMenu;
}

void NotesAgentAlarmDialog::readConfig()
{
    KConfigGroup grp( KGlobal::config(), "NotesAgentAlarmDialog" );
    const QSize size = grp.readEntry( "Size", QSize(300, 200) );
    if ( size.isValid() ) {
        resize( size );
    }
}

void NotesAgentAlarmDialog::writeConfig()
{
    KConfigGroup grp( KGlobal::config(), "NotesAgentAlarmDialog" );
    grp.writeEntry( "Size", size() );
    grp.sync();
}


void NotesAgentAlarmDialog::addListAlarm(const Akonadi::Item::List &lstAlarm)
{
    mListWidget->setNotes(lstAlarm);
    mCurrentDateTime->setText(KGlobal::locale()->formatDateTime(QDateTime::currentDateTime()));
}

void NotesAgentAlarmDialog::slotItemDoubleClicked(QListWidgetItem *item)
{
    if (item) {
        NotesAgentNoteDialog *dlg = new NotesAgentNoteDialog;
        dlg->show();
    }
}

void NotesAgentAlarmDialog::slotShowAlarm()
{
    NotesAgentNoteDialog *dlg = new NotesAgentNoteDialog;
    dlg->show();
}

void NotesAgentAlarmDialog::slotRemoveAlarm()
{
    Akonadi::Item::Id id = mListWidget->currentItemId();
    if (id!=-1) {
        Akonadi::Item item(id);
        Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob( item, this );
        job->fetchScope().fetchAttribute<NoteShared::NoteAlarmAttribute>();
        connect( job, SIGNAL(result(KJob*)), SLOT(slotFetchItem(KJob*)) );
    }
}

void NotesAgentAlarmDialog::slotFetchItem(KJob *job)
{
    if ( job->error() ) {
        qDebug()<<"fetch item failed "<<job->errorString();
        return;
    }
    Akonadi::ItemFetchJob *itemFetchJob = static_cast<Akonadi::ItemFetchJob *>(job);
    Akonadi::Item::List items = itemFetchJob->items();
    if (!items.isEmpty()) {
        Akonadi::Item item = items.first();
        item.removeAttribute<NoteShared::NoteAlarmAttribute>();
        Akonadi::ItemModifyJob *modify = new Akonadi::ItemModifyJob(item);
        connect( modify, SIGNAL(result(KJob*)), SLOT(slotModifyItem(KJob*)) );
    }
}

void NotesAgentAlarmDialog::slotModifyItem(KJob *job)
{
    if ( job->error() ) {
        qDebug()<<"modify item failed "<<job->errorString();
        return;
    }
}
