#include "importmailswidget.h"
#include "ui_importmailswidget.h"

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

void ImportMailsWidget::setStatusMessage( const QString& status )
{
    ui->textStatus->setText( status );
}

void ImportMailsWidget::setFrom( const QString& from )
{
  ui->from->setText( from );
}

void ImportMailsWidget::setTo( const QString& to )
{
  ui->to->setText( to );
}

void ImportMailsWidget::setCurrent( const QString& current )
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
