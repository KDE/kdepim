/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>

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

#include "importfilterinfogui.h"
#include "importmailpage.h"

#include "mailimporter/importmailswidget.h"

#include <KMessageBox>
#include <QApplication>

#include <QListWidgetItem>

ImportFilterInfoGui::ImportFilterInfoGui(ImportMailPage* parent)
    : MailImporter::FilterInfoGui(),
      m_parent( parent )
{
}

ImportFilterInfoGui::~ImportFilterInfoGui()
{
}

void ImportFilterInfoGui::setStatusMessage( const QString& status )
{
    m_parent->mailWidget()->setStatusMessage(status);
}

void ImportFilterInfoGui::setFrom( const QString& from )
{
    m_parent->mailWidget()->setFrom(from);
}

void ImportFilterInfoGui::setTo( const QString& to )
{
    m_parent->mailWidget()->setTo(to);
}

void ImportFilterInfoGui::setCurrent( const QString& current )
{
    m_parent->mailWidget()->setCurrent(current);
    qApp->processEvents();
}

void  ImportFilterInfoGui::setCurrent( int percent )
{
    m_parent->mailWidget()->setCurrent(percent);
    qApp->processEvents(); // Be careful - back & finish buttons disabled, so only user event that can happen is cancel/close button
}

void  ImportFilterInfoGui::setOverall( int percent )
{
    m_parent->mailWidget()->setOverall(percent);
}

void ImportFilterInfoGui::addInfoLogEntry( const QString& log )
{
    QListWidgetItem* item =new QListWidgetItem(log);
    item->setForeground(Qt::blue);
    m_parent->mailWidget()->addItem( item );
    m_parent->mailWidget()->setLastCurrentItem();
    qApp->processEvents();
}

void ImportFilterInfoGui::addErrorLogEntry( const QString& log )
{
    QListWidgetItem* item =new QListWidgetItem(log);
    item->setForeground(Qt::red);
    m_parent->mailWidget()->addItem( item );
    m_parent->mailWidget()->setLastCurrentItem();
    qApp->processEvents();
}


void ImportFilterInfoGui::clear()
{
    m_parent->mailWidget()->clear();
}

void ImportFilterInfoGui::alert( const QString& message )
{
    KMessageBox::information( m_parent, message );
}

QWidget *ImportFilterInfoGui::parent()
{
    return m_parent;
}

