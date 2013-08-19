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

#include <QWebPage>
#include <QWebElement>
#include <QWebFrame>
#include <QWebView>
#include <QDebug>

using namespace PimCommon;

GoogleTranslator::GoogleTranslator()
    : AbstractTranslator(),
      mWebPage(0)
{
}

GoogleTranslator::~GoogleTranslator()
{
    delete mWebPage;
    mWebPage = 0;
}


QMap<QString, QMap<QString, QString> > GoogleTranslator::initListLanguage(KComboBox* from)
{
    QMap<QString, QMap<QString, QString> > listLanguage;

    QList<QPair<QString, QString> > fullListLanguage;
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
        const QPair<QString, QString> currentLanguage = fullListLanguage.at(i);
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
    mResult.clear();
    delete mWebPage;
    mWebPage = new QWebPage;
    mWebPage->settings()->setAttribute( QWebSettings::JavaEnabled, false );
    mWebPage->settings()->setAttribute( QWebSettings::PluginsEnabled, false );
    connect(mWebPage, SIGNAL(loadFinished(bool)), SLOT(slotLoadFinished(bool)));

    mUrl = QUrl(QString::fromLatin1("http://translate.google.com/#%1|%2|%3").arg(mFrom, mTo,mInputText));
    mWebPage->mainFrame()->load(mUrl);
}

void GoogleTranslator::slotLoadFinished(bool result)
{
    if (result) {
        QWebElement e = mWebPage->mainFrame()->findFirstElement("span#result_box");
        if (e.isNull()) {
            Q_EMIT translateFailed(result);
        } else {
            mResult = e.toPlainText();
            Q_EMIT translateDone();
        }
    } else {
        Q_EMIT translateFailed(result);
    }
}

void GoogleTranslator::debug()
{
    if (mWebPage) {
        QWebView *view = new QWebView;
        view->setAttribute(Qt::WA_DeleteOnClose);
        view->setPage(mWebPage);
        qDebug()<<" url "<<mUrl.toString();
        view->show();
    } else {
        qDebug()<<" no search done for the moment.";
    }
}

#include "googletranslator.moc"
