/*
    This file is part of libkpimexchange
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <qlayout.h>
#include <qlabel.h>
#include <qcombobox.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <kapplication.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <ksimpleconfig.h>

#include "exchangeprogress.h"
using namespace KPIM;

ExchangeProgress::ExchangeProgress(QWidget *parent)
  : KProgressDialog(parent, "", i18n("Exchange Download Progress"), i18n("Exchange Plugin"), "text" )
{
  m_finished = 0;
  m_total = 0;  
  setAutoClose( false );
  setLabel( i18n( "Listing appointments" ) );
}

ExchangeProgress::~ExchangeProgress()
{
}

void ExchangeProgress::slotTransferStarted()
{
  m_total++;
  progressBar()->setTotalSteps( m_total );
  updateLabel();
}

void ExchangeProgress::slotTransferFinished()
{
  m_finished++;
  updateLabel();
  if ( m_finished == m_total ) {
    emit complete( this );
  }
}

void ExchangeProgress::updateLabel()
{
  progressBar()->setValue( m_finished );
  QString str = i18n( "Downloading, %1 of %2" ).arg( m_finished ).arg( m_total );
  setLabel( str );
}

#include "exchangeprogress.moc"
