/***************************************************************************
   Copyright (C) 2007
   by Marco Gulino <marco@kmobiletools.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/
#include "kserialdeviceemulatorwidget.h"

#include <qlabel.h>
#include <ktextbrowser.h>
#include <kurlrequester.h>
#include <kcombobox.h>
#include <kpushbutton.h>
#include <qregexp.h>
#include <qfile.h>
#include <qtimer.h>
#include <kdebug.h>

#include "commandslist.h"

KSerialDeviceEmulatorWidget::KSerialDeviceEmulatorWidget(QWidget* parent, const char* name, Qt::WFlags fl)
        : QWidget(parent,fl)
{
    setupUi(this);
    connect(b_loadlog, SIGNAL(clicked()), this, SLOT(loadClicked()));
    connect(cmdListView, SIGNAL(clicked(Q3ListViewItem*)), this, SLOT(commandClicked(Q3ListViewItem*)));
    connect(b_remove, SIGNAL(clicked()), this, SLOT(removeCmd()));
    connect(b_removeall, SIGNAL(clicked()), this, SLOT(removeAllCmds()) );
    connect(u_logfile, SIGNAL(urlSelected( const QString &)), this, SLOT(loadClicked()));
    connect(eventBox, SIGNAL(returnPressed(const QString &)), SLOT(slotSendEvent(const QString &)));
    connect(eventBox, SIGNAL(activated(int)), this, SLOT(eventSelected(int)));
    connect(b_event, SIGNAL(clicked()), this, SLOT(slotSendEvent()));
    b_remove->setEnabled(false);
}

KSerialDeviceEmulatorWidget::~KSerialDeviceEmulatorWidget()
{}



#include "kserialdeviceemulatorwidget.moc"



/*!
    \fn KSerialDeviceEmulatorWidget::addToLog(const QString &text, const QString &color)
 */
void KSerialDeviceEmulatorWidget::addToLog(const QString &text, const QString &color)
{
//     QString curText=tb_rawlog->text();
//     if( tb_rawlog->lines() > 128 && false ) curText=curText.mid( curText.find( "\n" ) + 1);
//     tb_rawlog->setText(curText);
//     tb_rawlog->scrollToBottom();
       tb_rawlog->append("<font color=\"" + color + "\">" + text + "</font><br>\n");
}



/*!
    \fn KSerialDeviceEmulatorWidget::loadClicked()
 */
void KSerialDeviceEmulatorWidget::loadClicked()
{
    emit loadFile(u_logfile->url());
}


/*!
    \fn KSerialDeviceEmulatorWidget::updateCommandListView()
 */
void KSerialDeviceEmulatorWidget::updateCommandListView()
{
    cmdListView->clear();
    for(Q3ValueListConstIterator<Command> it=CommandsList::instance()->begin(); it!=CommandsList::instance()->end(); ++it)
        new CommandListViewItem(cmdListView, *it);
}

CommandListViewItem::CommandListViewItem(K3ListView *parent, const Command &command)
    : K3ListViewItem(parent)
{
    cmd=command;
    setText(0, QString("%1").arg(cmd.origPos(), 3).replace(" ", "0"));
    setText(1, cmd.cmd());
    QStringList slanswer= QStringList::split( QRegExp("[\\n]|[\\r]"), cmd.answer() );
    QString answ.clear();
    for(QStringList::Iterator it=slanswer.begin(); it!=slanswer.end(); ++it)
        if((*it).length() >= answ.length()) answ=*it;
    setText(2,answ);
}


/*!
    \fn KSerialDeviceEmulatorWidget::commandClicked ( QListViewItem * item )
 */
void KSerialDeviceEmulatorWidget::commandClicked ( Q3ListViewItem * item )
{
    if(!item)
    {
        b_remove->setEnabled(false);
        return;
    }
    b_remove->setEnabled(true);
    CommandListViewItem *citem=static_cast<CommandListViewItem*> (item);
    cmdDesc->setText(QString("<qt><center><font color=\"red\">%1</font></center><br><font color=\"blue\">%2</font></qt>")
            .arg(citem->command().cmd())
            .arg(citem->command().answer()) );
}


/*!
    \fn KSerialDeviceEmulatorWidget::removeCmd()
 */
void KSerialDeviceEmulatorWidget::removeCmd()
{
    cmdDesc->setText(QString() );
    if(!cmdListView->selectedItem()) return;
    CommandListViewItem *citem=static_cast<CommandListViewItem*> (cmdListView->selectedItem());
//     for(QValueList<Command>::ConstIterator it=cl->begin(); it!=cl->end(); ++it)
//     {
//         if((*it).cmd() == citem->command().cmd() && (*it).answer()==citem->command().answer() )
//             CommandsList::instance()->remove(it);
//     }
//     delete citem;
    CommandsList::instance()->remove( citem->command() );
    updateCommandListView();
}


/*!
    \fn KSerialDeviceEmulatorWidget::removeAllCmds()
 */
void KSerialDeviceEmulatorWidget::removeAllCmds()
{
    CommandsList::instance()->clear();
    updateCommandListView();
}


/*!
    \fn KSerialDeviceEmulatorWidget::sendEvent()
 */
void KSerialDeviceEmulatorWidget::slotSendEvent()
{
    slotSendEvent(eventBox->currentText());
}

void KSerialDeviceEmulatorWidget::slotSendEvent(const QString &event)
{
    if(!event.length()) return;
    QTimer::singleShot( 150, this, SLOT(resetEvent()));
    emit sendEvent(eventBox->currentText() );
}


void KSerialDeviceEmulatorWidget::resetEvent()
{
    eventBox->setCurrentItem(0);
}

/*!
    \fn KSerialDeviceEmulatorWidget::eventSelected(int)
 */
void KSerialDeviceEmulatorWidget::eventSelected(int i)
{
    kDebug() <<"Selected" << i;
    if(!i) return;
    switch( i ){
        case 1:
            resetEvent();
            eventBox->setCurrentText("RING");
            break;
    }
}
