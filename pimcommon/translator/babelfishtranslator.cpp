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
#include "translatorutil.h"
#include <kio/job.h>
#include <KDebug>
#include <KComboBox>

using namespace MessageViewer;
//Laurent 2012-05-29: Babelfish was replaced by bing translator.
//This code will not get result.

BabelFishTranslator::BabelFishTranslator()
  : AbstractTranslator()
{
}

BabelFishTranslator::~BabelFishTranslator()
{

}

QMap<QString, QMap<QString, QString> > BabelFishTranslator::initListLanguage(KComboBox* from)
{
  QMap<QString, QMap<QString, QString> > listLanguage;

  MessageViewer::TranslatorUtil::addItemToFromComboBox( from, TranslatorUtil::en );
  QMap<QString, QString> enList;
  MessageViewer::TranslatorUtil::addPairToMap( enList, TranslatorUtil::zh );
  MessageViewer::TranslatorUtil::addPairToMap( enList, TranslatorUtil::zt );
  MessageViewer::TranslatorUtil::addPairToMap( enList, TranslatorUtil::nl );
  MessageViewer::TranslatorUtil::addPairToMap( enList, TranslatorUtil::fr );
  MessageViewer::TranslatorUtil::addPairToMap( enList, TranslatorUtil::de );
  MessageViewer::TranslatorUtil::addPairToMap( enList, TranslatorUtil::it );
  MessageViewer::TranslatorUtil::addPairToMap( enList, TranslatorUtil::ja );
  MessageViewer::TranslatorUtil::addPairToMap( enList, TranslatorUtil::ko );
  MessageViewer::TranslatorUtil::addPairToMap( enList, TranslatorUtil::pt );
  MessageViewer::TranslatorUtil::addPairToMap( enList, TranslatorUtil::ru );
  MessageViewer::TranslatorUtil::addPairToMap( enList, TranslatorUtil::es );
  listLanguage.insert( TranslatorUtil::en.second, enList );

  MessageViewer::TranslatorUtil::addItemToFromComboBox( from, TranslatorUtil::nl );
  QMap<QString, QString> nlList;
  MessageViewer::TranslatorUtil::addPairToMap( nlList, TranslatorUtil::en );
  MessageViewer::TranslatorUtil::addPairToMap( nlList, TranslatorUtil::fr );
  listLanguage.insert( TranslatorUtil::nl.second, nlList );

  MessageViewer::TranslatorUtil::addItemToFromComboBox( from, TranslatorUtil::fr );
  QMap<QString, QString> frList;
  MessageViewer::TranslatorUtil::addPairToMap( frList, TranslatorUtil::nl );
  MessageViewer::TranslatorUtil::addPairToMap( frList, TranslatorUtil::en );
  MessageViewer::TranslatorUtil::addPairToMap( frList, TranslatorUtil::de );
  MessageViewer::TranslatorUtil::addPairToMap( frList, TranslatorUtil::el );
  MessageViewer::TranslatorUtil::addPairToMap( frList, TranslatorUtil::it );
  MessageViewer::TranslatorUtil::addPairToMap( frList, TranslatorUtil::pt );
  MessageViewer::TranslatorUtil::addPairToMap( frList, TranslatorUtil::es );
  listLanguage.insert( TranslatorUtil::fr.second, frList );


  MessageViewer::TranslatorUtil::addItemToFromComboBox( from, TranslatorUtil::de );
  QMap<QString, QString> deList;
  MessageViewer::TranslatorUtil::addPairToMap( deList, TranslatorUtil::en );
  MessageViewer::TranslatorUtil::addPairToMap( deList, TranslatorUtil::fr );
  listLanguage.insert( TranslatorUtil::de.second, deList );

  MessageViewer::TranslatorUtil::addItemToFromComboBox( from, TranslatorUtil::el );
  QMap<QString, QString> elList;
  MessageViewer::TranslatorUtil::addPairToMap( elList, TranslatorUtil::en );
  MessageViewer::TranslatorUtil::addPairToMap( elList, TranslatorUtil::fr );
  listLanguage.insert( TranslatorUtil::el.second, elList );



  MessageViewer::TranslatorUtil::addItemToFromComboBox( from, TranslatorUtil::it );
  QMap<QString, QString> itList;
  MessageViewer::TranslatorUtil::addPairToMap( itList, TranslatorUtil::en );
  MessageViewer::TranslatorUtil::addPairToMap( itList, TranslatorUtil::fr );
  listLanguage.insert( TranslatorUtil::it.second, itList );

  MessageViewer::TranslatorUtil::addItemToFromComboBox( from, TranslatorUtil::es );
  QMap<QString, QString> esList;
  MessageViewer::TranslatorUtil::addPairToMap( esList, TranslatorUtil::en );
  MessageViewer::TranslatorUtil::addPairToMap( esList, TranslatorUtil::fr );
  listLanguage.insert( TranslatorUtil::es.second, esList );

  MessageViewer::TranslatorUtil::addItemToFromComboBox( from, TranslatorUtil::pt );
  QMap<QString, QString> ptList;
  MessageViewer::TranslatorUtil::addPairToMap( ptList, TranslatorUtil::en );
  MessageViewer::TranslatorUtil::addPairToMap( ptList, TranslatorUtil::fr );
  listLanguage.insert( TranslatorUtil::pt.second, ptList );

  MessageViewer::TranslatorUtil::addItemToFromComboBox( from, TranslatorUtil::ja );
  QMap<QString, QString> jaList;
  MessageViewer::TranslatorUtil::addPairToMap( jaList, TranslatorUtil::en );
  listLanguage.insert( TranslatorUtil::ja.second, jaList );

  MessageViewer::TranslatorUtil::addItemToFromComboBox( from, TranslatorUtil::ko );
  QMap<QString, QString> koList;
  MessageViewer::TranslatorUtil::addPairToMap( koList, TranslatorUtil::en );
  listLanguage.insert( TranslatorUtil::ko.second, koList );

  MessageViewer::TranslatorUtil::addItemToFromComboBox( from, TranslatorUtil::ru );
  QMap<QString, QString> ruList;
  MessageViewer::TranslatorUtil::addPairToMap( ruList, TranslatorUtil::en );
  listLanguage.insert( TranslatorUtil::ru.second, ruList );


  MessageViewer::TranslatorUtil::addItemToFromComboBox( from, TranslatorUtil::zt );
  QMap<QString, QString> ztList;
  MessageViewer::TranslatorUtil::addPairToMap( ztList, TranslatorUtil::en );
  MessageViewer::TranslatorUtil::addPairToMap( ztList, TranslatorUtil::zh );
  listLanguage.insert( TranslatorUtil::zt.second, ztList );

  MessageViewer::TranslatorUtil::addItemToFromComboBox( from, TranslatorUtil::zh );
  QMap<QString, QString> zhList;
  MessageViewer::TranslatorUtil::addPairToMap( zhList, TranslatorUtil::en );
  MessageViewer::TranslatorUtil::addPairToMap( zhList, TranslatorUtil::zt );
  listLanguage.insert( TranslatorUtil::zh.second, zhList );
  return listLanguage;
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
  if (httpPostJob) {
    const QString data = QString::fromUtf8(httpPostJob->data());
    const QString startTag = QLatin1String("<div style=\"padding:0.6em;\">");
    int index = data.indexOf(startTag);
    if (index != -1) {
      QString newStr = data.right(data.length() - index - startTag.length());
      index = newStr.indexOf(QLatin1String("</div>"));
      mResult = newStr.left(index);
      Q_EMIT translateDone();
    } else {
      Q_EMIT translateFailed();
    }
  }
}

