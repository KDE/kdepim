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

#include "babelfishtranslator.h"
#include <kio/job.h>
#include <KDebug>

using namespace MessageViewer;

BabelFishTranslator::BabelFishTranslator()
  : AbstractTranslator()
{
}

BabelFishTranslator::~BabelFishTranslator()
{

}

void BabelFishTranslator::translate()
{
  mResult.clear();
  KUrl geturl ( "http://babelfish.yahoo.com/translate_txt" );

  QString body = QUrl::toPercentEncoding( mInputText );
  body.replace(QLatin1String( "%20" ), QLatin1String( "+" ));

  QByteArray postData = QString( "ei=UTF-8&doit=done&fr=bf-res&intl=1&tt=urltext&trtext=%1&lp=%2_%3&btnTrTxt=Translate").arg( body, mFrom, mTo ).toLocal8Bit();
  kDebug() << "URL:" << geturl << "(post data" << postData << ")";

  KIO::StoredTransferJob *job = KIO::storedHttpPost(postData,geturl);
  job->addMetaData( "content-type", "Content-Type: application/x-www-form-urlencoded" );
  job->addMetaData( "referrer", "http://babelfish.yahoo.com/translate_txt" );
  connect( job, SIGNAL(result(KJob*)), this, SLOT(slotJobDone(KJob*)) );
}

void BabelFishTranslator::slotJobDone ( KJob *job )
{
  KIO::StoredTransferJob *httpPostJob = dynamic_cast<KIO::StoredTransferJob *>(job);
  if(httpPostJob) {
    //d->translate->setEnabled( true );
    const QString data = QString::fromUtf8(httpPostJob->data());
    const QString startTag = QLatin1String("<div style=\"padding:0.6em;\">");
    int index = data.indexOf(startTag);
    if(index != -1) {
      QString newStr = data.right(data.length() - index - startTag.length());
      index = newStr.indexOf(QLatin1String("</div>"));
      mResult = newStr.left(index);
      Q_EMIT translateDone();
      //d->translatedText->setHtml(newStr.left(index));
    } else {
      Q_EMIT translateFailed();
      //d->translatedText->clear();
    }
  }
}

#include "babelfishtranslator.moc"
