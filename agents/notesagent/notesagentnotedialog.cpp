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


#include "notesagentnotedialog.h"

#include <KLocalizedString>
#include <KSharedConfig>

#include <QVBoxLayout>
#include <QTextEdit>


NotesAgentNoteDialog::NotesAgentNoteDialog(QWidget *parent)
    : KDialog(parent)
{
    setButtons(Close);
    setAttribute(Qt::WA_DeleteOnClose);
    QWidget *w = new QWidget;
    QVBoxLayout *vbox = new QVBoxLayout;
    w->setLayout(vbox);
    mNote = new QTextEdit;
    vbox->addWidget(mNote);
    setMainWidget(w);
    readConfig();
}

NotesAgentNoteDialog::~NotesAgentNoteDialog()
{
    writeConfig();
}

void NotesAgentNoteDialog::readConfig()
{
    KConfigGroup grp( KGlobal::config(), "NotesAgentNoteDialog" );
    const QSize size = grp.readEntry( "Size", QSize(300, 200) );
    if ( size.isValid() ) {
        resize( size );
    }
}

void NotesAgentNoteDialog::writeConfig()
{
    KConfigGroup grp( KGlobal::config(), "NotesAgentNoteDialog" );
    grp.writeEntry( "Size", size() );
    grp.sync();
}
