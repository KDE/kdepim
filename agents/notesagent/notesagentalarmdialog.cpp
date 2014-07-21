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

#include "notesagentalarmdialog.h"
#include "notesagentnotedialog.h"
#include "noteshared/widget/notelistwidget.h"
#include "noteshared/attributes/notealarmattribute.h"
#include "noteshared/alarms/notealarmdialog.h"

#include <KMime/KMimeMessage>

#include <ItemFetchJob>
#include <ItemFetchScope>
#include <ItemModifyJob>

#include <KLocalizedString>
#include <KLocale>
#include <KDateTime>
#include <QMenu>
#include <QAction>
#include <KMessageBox>
#include <QIcon>

#include <QListWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QPointer>
#include <KSharedConfig>
#include <QDialogButtonBox>
#include <KConfigGroup>
#include <QPushButton>

NotesAgentAlarmDialog::NotesAgentAlarmDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle( i18n( "Alarm" ) );
    setWindowIcon( QIcon::fromTheme( QLatin1String("knotes") ) );
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    setAttribute(Qt::WA_DeleteOnClose);
    buttonBox->button(QDialogButtonBox::Close)->setDefault(true);
    
    QWidget *w = new QWidget;
    QVBoxLayout *vbox = new QVBoxLayout;
    w->setLayout(vbox);

    mCurrentDateTime = new QLabel;
    mCurrentDateTime->setText(KLocale::global()->formatDateTime(QDateTime::currentDateTime()));
    vbox->addWidget(mCurrentDateTime);

    QLabel *lab = new QLabel(i18n("The following notes triggered alarms:"));
    vbox->addWidget(lab);
    mListWidget = new NoteShared::NoteListWidget;
    mListWidget->setContextMenuPolicy( Qt::CustomContextMenu );
    connect(mListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(slotItemDoubleClicked(QListWidgetItem*)));
    connect( mListWidget, SIGNAL(customContextMenuRequested(QPoint)),
             this, SLOT(slotCustomContextMenuRequested(QPoint)) );

    vbox->addWidget(mListWidget);
    mainLayout->addWidget(w);
    mainLayout->addWidget(buttonBox);
    readConfig();
}

NotesAgentAlarmDialog::~NotesAgentAlarmDialog()
{
    writeConfig();
}

void NotesAgentAlarmDialog::removeAlarm(const Akonadi::Item &note)
{
    mListWidget->removeNote(note);
    if (mListWidget->count() == 0) {
        close();
    }
}

void NotesAgentAlarmDialog::slotCustomContextMenuRequested(const QPoint &pos)
{
    if ( mListWidget->selectedItems().isEmpty() )
        return;
    Q_UNUSED(pos);
    QMenu *entriesContextMenu = new QMenu;
    QAction *removeAlarm = new QAction(QIcon::fromTheme(QLatin1String("edit-delete")), i18n("Remove Alarm"), entriesContextMenu);
    connect(removeAlarm, SIGNAL(triggered()), this, SLOT(slotRemoveAlarm()));
    QAction *showNote = new QAction(i18n("Show Note..."), entriesContextMenu);
    connect(showNote, SIGNAL(triggered()), this, SLOT(slotShowNote()));
    QAction *modifyAlarm = new QAction(i18n("Modify Alarm..."), entriesContextMenu);
    connect(modifyAlarm, SIGNAL(triggered()), this, SLOT(slotModifyAlarm()));
    entriesContextMenu->addAction( showNote );
    entriesContextMenu->addAction( modifyAlarm );

    entriesContextMenu->addSeparator();
    entriesContextMenu->addAction( removeAlarm );
    entriesContextMenu->exec( QCursor::pos() );
    delete entriesContextMenu;
}

void NotesAgentAlarmDialog::readConfig()
{
    KConfigGroup grp( KSharedConfig::openConfig(), "NotesAgentAlarmDialog" );
    const QSize size = grp.readEntry( "Size", QSize(300, 200) );
    if ( size.isValid() ) {
        resize( size );
    }
}

void NotesAgentAlarmDialog::writeConfig()
{
    KConfigGroup grp( KSharedConfig::openConfig(), "NotesAgentAlarmDialog" );
    grp.writeEntry( "Size", size() );
    grp.sync();
}

void NotesAgentAlarmDialog::addListAlarm(const Akonadi::Item::List &lstAlarm)
{
    mListWidget->setNotes(lstAlarm);
    mCurrentDateTime->setText(KLocale::global()->formatDateTime(QDateTime::currentDateTime()));
}

void NotesAgentAlarmDialog::slotItemDoubleClicked(QListWidgetItem *item)
{
    if (item) {
        slotShowNote();
    }
}

void NotesAgentAlarmDialog::slotShowNote()
{
    //deleted on close
    const Akonadi::Item::Id id = mListWidget->currentItemId();
    if (id!=-1) {
        NotesAgentNoteDialog *dlg = new NotesAgentNoteDialog;
        dlg->setNoteId(id);
        dlg->show();
    }
}

void NotesAgentAlarmDialog::slotRemoveAlarm()
{
    if (KMessageBox::Yes == KMessageBox::warningYesNo(this, i18n("Are you sure to remove alarm?"), i18nc("@title:window", "Remove Alarm")) ) {
        const Akonadi::Item::Id id = mListWidget->currentItemId();
        if (id!=-1) {
            Akonadi::Item item(id);
            Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob( item, this );
            job->fetchScope().fetchAttribute<NoteShared::NoteAlarmAttribute>();
            connect( job, SIGNAL(result(KJob*)), SLOT(slotFetchItem(KJob*)) );
        }
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

void NotesAgentAlarmDialog::slotModifyAlarm()
{
    Akonadi::Item::Id id = mListWidget->currentItemId();
    if (id!=-1) {
        Akonadi::Item item(id);
        Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob( item, this );
        job->fetchScope().fetchFullPayload( true );
        job->fetchScope().fetchAttribute<NoteShared::NoteAlarmAttribute>();
        connect( job, SIGNAL(result(KJob*)), SLOT(slotFetchAlarmItem(KJob*)) );
    }
}

void NotesAgentAlarmDialog::slotFetchAlarmItem(KJob *job)
{
    if ( job->error() ) {
        qDebug()<<"fetch item failed "<<job->errorString();
        return;
    }
    Akonadi::ItemFetchJob *itemFetchJob = static_cast<Akonadi::ItemFetchJob *>(job);
    Akonadi::Item::List items = itemFetchJob->items();
    if (!items.isEmpty()) {
        Akonadi::Item item = items.first();
        NoteShared::NoteAlarmAttribute *attr = item.attribute<NoteShared::NoteAlarmAttribute>();
        if (attr) {
            KMime::Message::Ptr noteMessage = item.payload<KMime::Message::Ptr>();
            if (!noteMessage)
                return;
            const KMime::Headers::Subject * const subject = noteMessage->subject(false);
            QString caption;
            if (subject) {
                caption = subject->asUnicodeString();
            }
            QPointer<NoteShared::NoteAlarmDialog> dlg = new NoteShared::NoteAlarmDialog(caption, this);
            dlg->setAlarm(attr->dateTime());
            if (dlg->exec()) {
                const KDateTime date = dlg->alarm();
                if (date.isValid()) {
                    attr->setDateTime(dlg->alarm());
                } else {
                    item.removeAttribute<NoteShared::NoteAlarmAttribute>();
                }
                Akonadi::ItemModifyJob *modify = new Akonadi::ItemModifyJob(item);
                connect(modify, SIGNAL(result(KJob*)), SLOT(slotModifyItem(KJob*)));
            }
            delete dlg;
        }
    }
}
