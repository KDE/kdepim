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

#include "mboximporterinfogui.h"
#include "mboximportwidget.h"

#include "mailimporter/importmailswidget.h"

#include <KMessageBox>

#include <QListWidgetItem>
#include <QApplication>

MBoxImporterInfoGui::MBoxImporterInfoGui(MBoxImportWidget *parent)
    : MailImporter::FilterInfoGui(),
      mParent(parent)
{
}

MBoxImporterInfoGui::~MBoxImporterInfoGui()
{
}

void MBoxImporterInfoGui::setStatusMessage(const QString &status)
{
    mParent->mailWidget()->setStatusMessage(status);
}

void MBoxImporterInfoGui::setFrom(const QString &from)
{
    mParent->mailWidget()->setFrom(from);
}

void MBoxImporterInfoGui::setTo(const QString &to)
{
    mParent->mailWidget()->setTo(to);
}

void MBoxImporterInfoGui::setCurrent(const QString &current)
{
    mParent->mailWidget()->setCurrent(current);
    qApp->processEvents();
}

void  MBoxImporterInfoGui::setCurrent(int percent)
{
    mParent->mailWidget()->setCurrent(percent);
    qApp->processEvents(); // Be careful - back & finish buttons disabled, so only user event that can happen is cancel/close button
}

void  MBoxImporterInfoGui::setOverall(int percent)
{
    mParent->mailWidget()->setOverall(percent);
}

void MBoxImporterInfoGui::addInfoLogEntry(const QString &log)
{
    QListWidgetItem *item = new QListWidgetItem(log);
    item->setForeground(Qt::blue);
    mParent->mailWidget()->addItem(item);
    mParent->mailWidget()->setLastCurrentItem();
    qApp->processEvents();
}

void MBoxImporterInfoGui::addErrorLogEntry(const QString &log)
{
    QListWidgetItem *item = new QListWidgetItem(log);
    item->setForeground(Qt::red);
    mParent->mailWidget()->addItem(item);
    mParent->mailWidget()->setLastCurrentItem();
    qApp->processEvents();
}

void MBoxImporterInfoGui::clear()
{
    mParent->mailWidget()->clear();
}

void MBoxImporterInfoGui::alert(const QString &message)
{
    KMessageBox::information(mParent, message);
}

QWidget *MBoxImporterInfoGui::parent()
{
    return mParent;
}

