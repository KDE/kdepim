/*
    Copyright (c) 2006 - 2007 Volker Krause <vkrause@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "transportmgr.h"

#include <mailtransport/transportconfigdialog.h>
#include <mailtransport/transportmanager.h>
#include <mailtransport/transportmanagementwidget.h>
#include <mailtransport/transportjob.h>

#include <KApplication>
#include <KCmdLineArgs>
#include <KLineEdit>

#include <QPushButton>
#include <QTextEdit>

using namespace KPIM;

TransportMgr::TransportMgr() :
    mCurrentJob( 0 )
{
  new TransportManagementWidget( this );
  mComboBox = new TransportComboBox( this );
  QPushButton *b = new QPushButton( "&Edit", this );
  connect( b, SIGNAL(clicked(bool)), SLOT(editBtnClicked()) );
  mSenderEdit = new KLineEdit( this );
  mSenderEdit->setClickMessage( "Sender" );
  mToEdit = new KLineEdit( this );
  mToEdit->setClickMessage( "To" );
  mCcEdit = new KLineEdit( this );
  mCcEdit->setClickMessage( "Cc" );
  mBccEdit = new KLineEdit( this );
  mBccEdit->setClickMessage( "Bcc" );
  mMailEdit = new QTextEdit( this );
  mMailEdit->setAcceptRichText( false );
  mMailEdit->setLineWrapMode( QTextEdit::NoWrap );
  b = new QPushButton( "&Send", this );
  connect( b, SIGNAL(clicked(bool)), SLOT(sendBtnClicked()) );
  b = new QPushButton( "&Cancel", this );
  connect( b, SIGNAL(clicked(bool)), SLOT(cancelBtnClicked()) );
}

void TransportMgr::editBtnClicked()
{
  TransportConfigDialog *t = new TransportConfigDialog( TransportManager::self()->transportById( mComboBox->currentTransportId() ), this );
  t->exec();
  delete t;
}

void TransportMgr::sendBtnClicked()
{
  TransportJob *job = TransportManager::self()->createTransportJob( mComboBox->currentTransportId() );
  Q_ASSERT( job );
  job->setSender( mSenderEdit->text() );
  job->setTo( mToEdit->text().isEmpty() ? QStringList() : mToEdit->text().split(',') );
  job->setCc( mCcEdit->text().isEmpty() ? QStringList() : mCcEdit->text().split(',') );
  job->setBcc( mBccEdit->text().isEmpty() ? QStringList() : mBccEdit->text().split(',') );
  job->setData( mMailEdit->document()->toPlainText().toLatin1() );
  connect( job, SIGNAL(result(KJob*)), SLOT(jobResult(KJob*)) );
  connect( job, SIGNAL(percent(KJob*,unsigned long)), SLOT(jobPercent(KJob*,unsigned long)) );
  mCurrentJob = job;
  TransportManager::self()->schedule( job );
}

void TransportMgr::cancelBtnClicked()
{
  if ( mCurrentJob )
    kDebug() << k_funcinfo << "kill success: " << mCurrentJob->kill() << endl;
  mCurrentJob = 0;
}

int main( int argc, char** argv )
{
  KCmdLineArgs::init(argc, argv, "transportmgr", "transportmgr", "Mail Transport Manager Demo", "0" );
  KApplication app;
  TransportMgr* t = new TransportMgr();
  t->show();
  app.exec();
}

void TransportMgr::jobResult( KJob* job )
{
  kDebug() << k_funcinfo << job->error() << job->errorText() << endl;
  mCurrentJob = 0;
}

void TransportMgr::jobPercent(KJob * job, unsigned long percent)
{
  Q_UNUSED( job );
  kDebug() << k_funcinfo << percent << "%" << endl;
}

#include "transportmgr.moc"

