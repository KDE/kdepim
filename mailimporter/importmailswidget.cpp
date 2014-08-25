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

#include "importmailswidget.h"
#include "ui_importmailswidget.h"

#include <QAbstractTextDocumentLayout>

using namespace MailImporter;



ImportMailsWidget::ImportMailsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ImportMailsWidget)
{
    ui->setupUi(this);
}

ImportMailsWidget::~ImportMailsWidget()
{
    delete ui;
}

void ImportMailsWidget::setStatusMessage( const QString &status )
{
    ui->textStatus->setText( status );
}

void ImportMailsWidget::setFrom( const QString &from )
{
    ui->from->setText( from );
}

void ImportMailsWidget::setTo( const QString &to )
{
    ui->to->setText( to );
}

void ImportMailsWidget::setCurrent( const QString &current )
{
    ui->current->setText( current );
}

void  ImportMailsWidget::setCurrent( int percent )
{
    ui->done_current->setValue( percent );
}

void  ImportMailsWidget::setOverall( int percent )
{
    ui->done_overall->setValue( percent );
}

void ImportMailsWidget::addItem( QListWidgetItem* item )
{
    ui->log->addItem(item);
}

void ImportMailsWidget::setLastCurrentItem()
{
    ui->log->setCurrentItem(ui->log->item(ui->log->count() - 1 ));
}

void ImportMailsWidget::addInfoLogEntry( const QString &log )
{
    ui->log->addInfoLogEntry( log );
}

void ImportMailsWidget::addErrorLogEntry( const QString &log )
{
    ui->log->addErrorLogEntry( log );
}

void ImportMailsWidget::clear()
{
    ui->log->clear();
    setCurrent(0);
    setOverall(0);
    setCurrent( QString() );
    setFrom( QString() );
    setTo( QString() );
}

