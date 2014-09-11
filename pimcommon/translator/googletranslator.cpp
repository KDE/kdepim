/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>
  
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

#include "googletranslator.h"
#include "translatorutil.h"
#include "translatordebugdialog.h"

#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QPointer>



using namespace PimCommon;

GoogleTranslator::GoogleTranslator()
    : AbstractTranslator(),
      mNetworkAccessManager(new QNetworkAccessManager(this))
{
    connect(mNetworkAccessManager, &QNetworkAccessManager::finished, this, &GoogleTranslator::slotTranslateFinished);
}

GoogleTranslator::~GoogleTranslator()
{
}

QMap<QString, QMap<QString, QString> > GoogleTranslator::initListLanguage(KComboBox *from)
{
    QMap<QString, QMap<QString, QString> > listLanguage;

    QList<QPair<const char *, QString> > fullListLanguage;
    fullListLanguage.append(TranslatorUtil::automatic);
    fullListLanguage.append(TranslatorUtil::en);
    fullListLanguage.append(TranslatorUtil::nl);
    fullListLanguage.append(TranslatorUtil::fr);
    fullListLanguage.append(TranslatorUtil::de);
    fullListLanguage.append(TranslatorUtil::el);
    fullListLanguage.append(TranslatorUtil::it);
    fullListLanguage.append(TranslatorUtil::ja);
    fullListLanguage.append(TranslatorUtil::ko);
    fullListLanguage.append(TranslatorUtil::pt);
    fullListLanguage.append(TranslatorUtil::ru);
    fullListLanguage.append(TranslatorUtil::es);

    fullListLanguage.append(TranslatorUtil::af);
    fullListLanguage.append(TranslatorUtil::sq);
    fullListLanguage.append(TranslatorUtil::ar);
    fullListLanguage.append(TranslatorUtil::hy);
    fullListLanguage.append(TranslatorUtil::az);
    fullListLanguage.append(TranslatorUtil::eu);
    fullListLanguage.append(TranslatorUtil::be);
    fullListLanguage.append(TranslatorUtil::bg);
    fullListLanguage.append(TranslatorUtil::ca);
    fullListLanguage.append(TranslatorUtil::zh_cn_google); // For google only
    fullListLanguage.append(TranslatorUtil::zh_tw_google); // For google only
    fullListLanguage.append(TranslatorUtil::hr);
    fullListLanguage.append(TranslatorUtil::cs);
    fullListLanguage.append(TranslatorUtil::da);
    fullListLanguage.append(TranslatorUtil::et);
    fullListLanguage.append(TranslatorUtil::tl);
    fullListLanguage.append(TranslatorUtil::fi);
    fullListLanguage.append(TranslatorUtil::gl);
    fullListLanguage.append(TranslatorUtil::ka);
    fullListLanguage.append(TranslatorUtil::ht);
    fullListLanguage.append(TranslatorUtil::iw);
    fullListLanguage.append(TranslatorUtil::hi);
    fullListLanguage.append(TranslatorUtil::hu);
    fullListLanguage.append(TranslatorUtil::is);
    fullListLanguage.append(TranslatorUtil::id);
    fullListLanguage.append(TranslatorUtil::ga);
    fullListLanguage.append(TranslatorUtil::lv);
    fullListLanguage.append(TranslatorUtil::lt);
    fullListLanguage.append(TranslatorUtil::mk);
    fullListLanguage.append(TranslatorUtil::ms);
    fullListLanguage.append(TranslatorUtil::mt);
    fullListLanguage.append(TranslatorUtil::no);
    fullListLanguage.append(TranslatorUtil::fa);
    fullListLanguage.append(TranslatorUtil::pl);
    fullListLanguage.append(TranslatorUtil::ro);
    fullListLanguage.append(TranslatorUtil::sr);
    fullListLanguage.append(TranslatorUtil::sk);
    fullListLanguage.append(TranslatorUtil::sl);
    fullListLanguage.append(TranslatorUtil::sw);
    fullListLanguage.append(TranslatorUtil::sv);
    fullListLanguage.append(TranslatorUtil::th);
    fullListLanguage.append(TranslatorUtil::tr);
    fullListLanguage.append(TranslatorUtil::uk);
    fullListLanguage.append(TranslatorUtil::ur);
    fullListLanguage.append(TranslatorUtil::vi);
    fullListLanguage.append(TranslatorUtil::cy);
    fullListLanguage.append(TranslatorUtil::yi);
    const int fullListLanguageSize(fullListLanguage.count());
    for (int i=0;i<fullListLanguageSize;++i) {
        const QPair<const char *, QString> currentLanguage = fullListLanguage.at(i);
        PimCommon::TranslatorUtil::addItemToFromComboBox( from, currentLanguage );

        QMap<QString, QString> toList;
        for (int j=0;j<fullListLanguageSize;++j) {
            if (j!=0 && j!=i) { //don't add auto and current language
                PimCommon::TranslatorUtil::addPairToMap( toList, fullListLanguage.at(j) );
            }
        }
        listLanguage.insert( currentLanguage.second, toList );
    }

    return listLanguage;
}

void GoogleTranslator::translate()
{
    if (mFrom == mTo) {
        Q_EMIT translateFailed(false, i18n("You used same language for from and to language."));
        return;
    }

    mResult.clear();

    QNetworkRequest request(QUrl(QLatin1String("http://www.google.com/translate_a/t")));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/x-www-form-urlencoded"));

    QUrl postData;
    postData.addQueryItem(QLatin1String("client"), QLatin1String("t"));
    postData.addQueryItem(QLatin1String("sl"), mFrom);
    postData.addQueryItem(QLatin1String("tl"), mTo);
    postData.addQueryItem(QLatin1String("text"), mInputText);

    QNetworkReply *reply = mNetworkAccessManager->post(request, postData.encodedQuery());
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &GoogleTranslator::slotError);
}

void GoogleTranslator::slotError(QNetworkReply::NetworkError /*error*/)
{
    Q_EMIT translateFailed(false);
}

void GoogleTranslator::slotTranslateFinished(QNetworkReply *reply)
{
//QT5
#if 0
    mJsonData = QString::fromUtf8(reply->readAll());
    reply->deleteLater();
    //  jsonData contains arrays like this: ["foo",,"bar"]
    //  but this is not valid JSON for QJSON, it expects empty strings: ["foo","","bar"]
    mJsonData = mJsonData.replace(QRegExp(QLatin1String(",{3,3}")), QLatin1String(",\"\",\"\","));
    mJsonData = mJsonData.replace(QRegExp(QLatin1String(",{2,2}")), QLatin1String(",\"\","));
    //qDebug() << mJsonData;

    QJson::Parser parser;
    bool ok;

    const QVariantList json = parser.parse(mJsonData.toUtf8(), &ok).toList();
    if (!ok) {
        Q_EMIT translateFailed(ok);
        return;
    }
    //qDebug()<<" json"<<json;
    bool oldVersion = true;
    QMultiMap<int, QPair<QString, double> > sentences;

    // we are going recursively through the nested json-array
    // level0 contains the data of the outer array, level1 of the next one and so on
    Q_FOREACH (const QVariant& level0, json) {
        const QVariantList listLevel0 = level0.toList();
        if (listLevel0.isEmpty()) {
            continue;
        }
        Q_FOREACH (const QVariant& level1, listLevel0) {
            if (level1.toList().size() <= 2 || level1.toList().at(2).toList().isEmpty()) {
                continue;
            }
            const int indexLevel1 = listLevel0.indexOf(level1);
            const QVariantList listLevel1 = level1.toList().at(2).toList();
            foreach (const QVariant& level2, listLevel1) {
                const QVariantList listLevel2 = level2.toList();

                // The JSON we get from Google has not always the same structure.
                // There is a version with additional information like synonyms and frequency,
                // this is called newVersion oldVersion doesn't cointain something like this.

                const bool foundWordNew = (listLevel2.size() > 1) && (!listLevel2.at(1).toList().isEmpty());
                const bool foundWordOld = (listLevel2.size() == 4) && (oldVersion == true) && (listLevel2.at(1).toDouble() > 0);

                if (foundWordNew || foundWordOld) {
                    if (!level1.toList().at(0).toString().isEmpty() && foundWordOld) {
                        // sentences are translated phrase by phrase
                        // first we have to add all phrases to sentences and then rebuild them
                        sentences.insert(indexLevel1, qMakePair(listLevel2.at(0).toString(), listLevel2.at(1).toDouble() / 1000));
                    } else {
                        if (foundWordNew) {
                            oldVersion = false;
                            mResult = listLevel2.at(0).toString();
                        }
                    }
                }
            }
        }
    }

    if (!sentences.isEmpty()) {
        QPair<QString, double> pair;
        QMapIterator<int, QPair<QString, double> > it(sentences);
        int currentKey = -1;
        double currentRel = 1;
        QString currentString;

        while (it.hasNext()) {
            pair = it.next().value();

            // we're on to another key, process previous results, if any
            if (currentKey != it.key()) {
                currentKey = it.key();
                currentRel = 1;
                currentString.append(QLatin1Char(' ')).append(pair.first);
                currentRel *= pair.second;
            }
        }
        if (!currentString.isEmpty()) {
            mResult = currentString;
            Q_EMIT translateDone();
        }
    } else {
        //Same value
        mResult = mInputText;
        Q_EMIT translateDone();
    }
#endif
}

void GoogleTranslator::debug()
{
#if !defined(NDEBUG)
    QPointer<TranslatorDebugDialog> dlg = new TranslatorDebugDialog;
    dlg->setDebug(mJsonData);
    dlg->exec();
    delete dlg;
#endif
}

void GoogleTranslator::clear()
{
    mJsonData.clear();
}
