/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>

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
#include <KApplication>

KMailCvtFilterInfoGui::KMailCvtFilterInfoGui(KImportPage* dlg, QWidget* parent)
  : MailImporter::FilterInfoGui(),
    m_parent( parent ),
    m_dlg( dlg )
{
}

KMailCvtFilterInfoGui::~KMailCvtFilterInfoGui()
{
}
void KMailCvtFilterInfoGui::setStatusMsg( const QString& status )
{
  m_dlg->mWidget->_textStatus->setText( status );
}

void KMailCvtFilterInfoGui::setFrom( const QString& from )
{
  m_dlg->mWidget->_from->setText( from );
}

void KMailCvtFilterInfoGui::setTo( const QString& to )
{
  m_dlg->mWidget->_to->setText( to );
}

void KMailCvtFilterInfoGui::setCurrent( const QString& current )
{
  m_dlg->mWidget->_current->setText( current );
  kapp->processEvents();
}

void  KMailCvtFilterInfoGui::setCurrent( int percent )
{
  m_dlg->mWidget->_done_current->setValue( percent );
  kapp->processEvents(); // Be careful - back & finish buttons disabled, so only user event that can happen is cancel/close button
}

void  KMailCvtFilterInfoGui::setOverall( int percent )
{
  m_dlg->mWidget->_done_overall->setValue( percent );
}

void KMailCvtFilterInfoGui::addInfoLogEntry( const QString& log )
{
  m_dlg->mWidget->_log->addItem( log );
  m_dlg->mWidget->_log->setCurrentItem( m_dlg->mWidget->_log->item(m_dlg->mWidget->_log->count() - 1 ));
  kapp->processEvents();
}

void KMailCvtFilterInfoGui::addErrorLogEntry( const QString& log )
{
  m_dlg->mWidget->_log->addItem( log );
  m_dlg->mWidget->_log->setCurrentItem( m_dlg->mWidget->_log->item(m_dlg->mWidget->_log->count() - 1 ));
  kapp->processEvents();
}


void KMailCvtFilterInfoGui::clear()
{
  m_dlg->mWidget->_log->clear();
  setCurrent();
  setOverall();
  setCurrent( QString() );
  setFrom( QString() );
  setTo( QString() );
}

void KMailCvtFilterInfoGui::alert( const QString& message )
{
  KMessageBox::information( m_parent, message );
}

QWidget *KMailCvtFilterInfoGui::parent()
{
  return m_parent;
}
  
