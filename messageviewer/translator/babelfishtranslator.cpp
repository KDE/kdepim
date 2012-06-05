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
#include <KLocale>

using namespace MessageViewer;

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
  const QPair<QString, QString> en( i18n("English"), QLatin1String( "en" ) );
  const QPair<QString, QString> zh( i18n("Chinese (Simplified)"), QLatin1String( "zh" ) );
  const QPair<QString, QString> zt( i18n("Chinese (Traditional)"), QLatin1String( "zt" ) );
  const QPair<QString, QString> nl( i18n("Dutch"), QLatin1String( "nl" ) );
  const QPair<QString, QString> fr( i18n("French"), QLatin1String( "fr" ) );
  const QPair<QString, QString> de( i18n("German"), QLatin1String( "de" ) );
  const QPair<QString, QString> el( i18n("Greek"), QLatin1String( "el" ) );
  const QPair<QString, QString> it( i18n("Italian"), QLatin1String( "it" ) );
  const QPair<QString, QString> ja( i18n("Japanese"), QLatin1String( "ja" ) );
  const QPair<QString, QString> ko( i18n("Korean"), QLatin1String( "ko" ) );
  const QPair<QString, QString> pt( i18n("Portuguese"), QLatin1String( "pt" ) );
  const QPair<QString, QString> ru( i18n("Russian"), QLatin1String( "ru" ) );
  const QPair<QString, QString> es( i18n("Spanish"), QLatin1String( "es" ) );

  MessageViewer::TranslatorUtil::addItemToFromComboBox( from, en );
  QMap<QString, QString> enList;
  MessageViewer::TranslatorUtil::addPairToMap( enList, zh );
  MessageViewer::TranslatorUtil::addPairToMap( enList, zt );
  MessageViewer::TranslatorUtil::addPairToMap( enList, nl );
  MessageViewer::TranslatorUtil::addPairToMap( enList, fr );
  MessageViewer::TranslatorUtil::addPairToMap( enList, de );
  MessageViewer::TranslatorUtil::addPairToMap( enList, it );
  MessageViewer::TranslatorUtil::addPairToMap( enList, ja );
  MessageViewer::TranslatorUtil::addPairToMap( enList, ko );
  MessageViewer::TranslatorUtil::addPairToMap( enList, pt );
  MessageViewer::TranslatorUtil::addPairToMap( enList, ru );
  MessageViewer::TranslatorUtil::addPairToMap( enList, es );
  listLanguage.insert( en.second, enList );

  MessageViewer::TranslatorUtil::addItemToFromComboBox( from, nl );
  QMap<QString, QString> nlList;
  MessageViewer::TranslatorUtil::addPairToMap( nlList, en );
  MessageViewer::TranslatorUtil::addPairToMap( nlList, fr );
  listLanguage.insert( nl.second, nlList );

  MessageViewer::TranslatorUtil::addItemToFromComboBox( from, fr );
  QMap<QString, QString> frList;
  MessageViewer::TranslatorUtil::addPairToMap( frList, nl );
  MessageViewer::TranslatorUtil::addPairToMap( frList, en );
  MessageViewer::TranslatorUtil::addPairToMap( frList, de );
  MessageViewer::TranslatorUtil::addPairToMap( frList, el );
  MessageViewer::TranslatorUtil::addPairToMap( frList, it );
  MessageViewer::TranslatorUtil::addPairToMap( frList, pt );
  MessageViewer::TranslatorUtil::addPairToMap( frList, es );
  listLanguage.insert( fr.second, frList );


  MessageViewer::TranslatorUtil::addItemToFromComboBox( from, de );
  QMap<QString, QString> deList;
  MessageViewer::TranslatorUtil::addPairToMap( deList, en );
  MessageViewer::TranslatorUtil::addPairToMap( deList, fr );
  listLanguage.insert( de.second, deList );

  MessageViewer::TranslatorUtil::addItemToFromComboBox( from, el );
  QMap<QString, QString> elList;
  MessageViewer::TranslatorUtil::addPairToMap( elList, en );
  MessageViewer::TranslatorUtil::addPairToMap( elList, fr );
  listLanguage.insert( el.second, elList );



  MessageViewer::TranslatorUtil::addItemToFromComboBox( from, it );
  QMap<QString, QString> itList;
  MessageViewer::TranslatorUtil::addPairToMap( itList, en );
  MessageViewer::TranslatorUtil::addPairToMap( itList, fr );
  listLanguage.insert( it.second, itList );

  MessageViewer::TranslatorUtil::addItemToFromComboBox( from, es );
  QMap<QString, QString> esList;
  MessageViewer::TranslatorUtil::addPairToMap( esList, en );
  MessageViewer::TranslatorUtil::addPairToMap( esList, fr );
  listLanguage.insert( es.second, esList );

  MessageViewer::TranslatorUtil::addItemToFromComboBox( from, pt );
  QMap<QString, QString> ptList;
  MessageViewer::TranslatorUtil::addPairToMap( ptList, en );
  MessageViewer::TranslatorUtil::addPairToMap( ptList, fr );
  listLanguage.insert( pt.second, ptList );

  MessageViewer::TranslatorUtil::addItemToFromComboBox( from, ja );
  QMap<QString, QString> jaList;
  MessageViewer::TranslatorUtil::addPairToMap( jaList, en );
  listLanguage.insert( ja.second, jaList );

  MessageViewer::TranslatorUtil::addItemToFromComboBox( from, ko );
  QMap<QString, QString> koList;
  MessageViewer::TranslatorUtil::addPairToMap( koList, en );
  listLanguage.insert( ko.second, koList );

  MessageViewer::TranslatorUtil::addItemToFromComboBox( from, ru );
  QMap<QString, QString> ruList;
  MessageViewer::TranslatorUtil::addPairToMap( ruList, en );
  listLanguage.insert( ru.second, ruList );


  MessageViewer::TranslatorUtil::addItemToFromComboBox( from, zt );
  QMap<QString, QString> ztList;
  MessageViewer::TranslatorUtil::addPairToMap( ztList, en );
  MessageViewer::TranslatorUtil::addPairToMap( ztList, zh );
  listLanguage.insert( zt.second, ztList );

  MessageViewer::TranslatorUtil::addItemToFromComboBox( from, zh );
  QMap<QString, QString> zhList;
  MessageViewer::TranslatorUtil::addPairToMap( zhList, en );
  MessageViewer::TranslatorUtil::addPairToMap( zhList, zt );
  listLanguage.insert( zh.second, zhList );
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
