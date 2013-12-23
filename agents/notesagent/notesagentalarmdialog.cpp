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
#include "noteshared/widget/notelistwidget.h"

#include <KLocalizedString>
#include <KGlobal>
#include <KLocale>
#include <KDateTime>

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
    vbox->addWidget(mListWidget);
    setMainWidget(w);
}

NotesAgentAlarmDialog::~NotesAgentAlarmDialog()
{

}

void NotesAgentAlarmDialog::addListAlarm(const Akonadi::Item::List &lstAlarm)
{
    mListWidget->addNotes(lstAlarm);
    mCurrentDateTime->setText(KGlobal::locale()->formatDateTime(QDateTime::currentDateTime()));
}
