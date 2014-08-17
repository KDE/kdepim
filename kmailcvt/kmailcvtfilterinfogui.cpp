/*
  Copyright (c) 2012, 2013 Montel Laurent <montel@kde.org>

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

#include "kmailcvtfilterinfogui.h"
#include <KMessageBox>
#include <QApplication>

KMailCvtFilterInfoGui::KMailCvtFilterInfoGui(KImportPage *dlg, QWidget *parent)
    : MailImporter::FilterInfoGui(),
      m_parent( parent ),
      m_dlg( dlg )
{
}

KMailCvtFilterInfoGui::~KMailCvtFilterInfoGui()
{
}

void KMailCvtFilterInfoGui::setStatusMessage( const QString &status )
{
    m_dlg->widget()->mMailImporterWidget->setStatusMessage( status );
}

void KMailCvtFilterInfoGui::setFrom( const QString &from )
{
    m_dlg->widget()->mMailImporterWidget->setFrom( from );
}

void KMailCvtFilterInfoGui::setTo( const QString &to )
{
    m_dlg->widget()->mMailImporterWidget->setTo( to );
}

void KMailCvtFilterInfoGui::setCurrent( const QString &current )
{
    m_dlg->widget()->mMailImporterWidget->setCurrent( current );
    qApp->processEvents();
}

void  KMailCvtFilterInfoGui::setCurrent( int percent )
{
    m_dlg->widget()->mMailImporterWidget->setCurrent( percent );
    qApp->processEvents(); // Be careful - back & finish buttons disabled, so only user event that can happen is cancel/close button
}

void  KMailCvtFilterInfoGui::setOverall( int percent )
{
    m_dlg->widget()->mMailImporterWidget->setOverall( percent );
}

void KMailCvtFilterInfoGui::addInfoLogEntry( const QString &log )
{
    m_dlg->widget()->mMailImporterWidget->addInfoLogEntry( log );
    m_dlg->widget()->mMailImporterWidget->setLastCurrentItem();
    qApp->processEvents();
}

void KMailCvtFilterInfoGui::addErrorLogEntry( const QString &log )
{
    m_dlg->widget()->mMailImporterWidget->addErrorLogEntry( log );
    m_dlg->widget()->mMailImporterWidget->setLastCurrentItem();
    qApp->processEvents();
}


void KMailCvtFilterInfoGui::clear()
{
    m_dlg->widget()->mMailImporterWidget->clear();
}

void KMailCvtFilterInfoGui::alert( const QString &message )
{
    KMessageBox::information( m_parent, message );
}

QWidget *KMailCvtFilterInfoGui::parent()
{
    return m_parent;
}

