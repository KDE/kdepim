/* -*- mode: C++; c-file-style: "gnu" -*-

  Copyright (c) 2012 Montel Laurent <montel@kde.org>

  The program is free software; you can redistribute it and/or modify it
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

#include "translatewidget.h"
#include <KTextEdit>
#include <KComboBox>
#include <KPushButton>
#include <KLocale>
#include <kio/job.h>
#include <KDebug>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QRegExp>

using namespace MailCommon;

class TranslateWidget::TranslateWidgetPrivate
{
public:
  TranslateWidgetPrivate() {
  }
  QByteArray data;
  KTextEdit *inputText;
  KTextEdit *translatedText;
  KComboBox *from;
  KComboBox *to;
  KPushButton *translate;
  KIO::Job *job;
};

TranslateWidget::TranslateWidget( QWidget* parent )
  :QWidget( parent ), d( new TranslateWidgetPrivate )
{
   QHBoxLayout *layout = new QHBoxLayout( this );
   QVBoxLayout *vboxLayout = new QVBoxLayout;
   d->translate = new KPushButton( i18n( "Translate" ) );
   vboxLayout->addWidget( d->translate );
   connect( d->translate, SIGNAL( clicked() ), SLOT( slotTranslate() ) );
   QLabel *label = new QLabel( i18n( "From:" ) );
   vboxLayout->addWidget( label );
   d->from = new KComboBox;
   vboxLayout->addWidget( d->from );

   label = new QLabel( i18n( "To:" ) );
   vboxLayout->addWidget( label );
   d->to = new KComboBox;
   vboxLayout->addWidget( d->to );
   layout->addLayout( vboxLayout );

   vboxLayout = new QVBoxLayout;
   d->inputText = new KTextEdit;
   vboxLayout->addWidget( d->inputText );
   d->translatedText = new KTextEdit;
   d->translatedText->setReadOnly( true );
   vboxLayout->addWidget( d->translatedText );

   layout->addLayout( vboxLayout );
   
   initLanguage();
}

TranslateWidget::~TranslateWidget()
{
  delete d;
}


void TranslateWidget::initLanguage()
{
  //TODO
}


void TranslateWidget::slotTranslate()
{
  const QString textToTranslate = d->inputText->toPlainText();
  if ( textToTranslate.isEmpty() )
    return;
  //FIXME
  const QString from = QLatin1String( "fr" );
  const QString to = QLatin1String( "fr_en" );
  d->translate->setEnabled( false );
  //TODO
  KUrl geturl ( "http://babelfish.yahoo.com/translate_txt" );

  QString body = QUrl::toPercentEncoding( textToTranslate );
  body.replace("%20", "+");

  QByteArray postData = QString( "ei=UTF-8&doit=done&fr=bf-res&intl=1&tt=urltext&trtext=%1&lp=%2_%3&btnTrTxt=Translate").arg( body, from, to ).toLocal8Bit();
  kDebug(14308) << "URL:" << geturl << "(post data" << postData << ")";

  KIO::TransferJob *job = KIO::http_post( geturl, postData );
  job->addMetaData( "content-type", "Content-Type: application/x-www-form-urlencoded" );
  job->addMetaData( "referrer", "http://babelfish.yahoo.com/translate_txt" );

  connect( job, SIGNAL(data(KIO::Job*,QByteArray)), this, SLOT(slotDataReceived(KIO::Job*,QByteArray)) );
  connect( job, SIGNAL(result(KJob*)), this, SLOT(slotJobDone(KJob*)) );
}

void TranslateWidget::slotDataReceived ( KIO::Job *job, const QByteArray &data )
{
  if ( job == d->job )
    d->data.append(data);
}

void TranslateWidget::slotJobDone ( KJob *job )
{
  if ( job == d->job )
  {
    disconnect( job, SIGNAL(data(KIO::Job*,QByteArray)), this, SLOT(slotDataReceived(KIO::Job*,QByteArray)) );
    disconnect( job, SIGNAL(result(KJob*)), this, SLOT(slotJobDone(KJob*)) );

    d->translate->setEnabled( false );
    QRegExp re( "<div style=\"padding:0.6em;\">(.*)</div>" );
    re.setMinimal( true );
    re.indexIn( d->data );
    d->translatedText->setText( re.cap( 1 ) );
  }
}

#include "translatewidget.moc"



