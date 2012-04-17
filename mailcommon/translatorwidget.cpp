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

#include "translatorwidget.h"
#include <KTextEdit>
#include <KComboBox>
#include <KPushButton>
#include <KLocale>
#include <kio/job.h>
#include <KDebug>

#include <QPair>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QRegExp>

using namespace MailCommon;

class TranslatorWidget::TranslatorWidgetPrivate
{
public:
  TranslatorWidgetPrivate()
    : job( 0 ) {
    
  }
  void initLanguage();
  void fillToCombobox( const QString& lang );
  
  QMap<QString, QMap<QString, QString> > listLanguage;
  QByteArray data;
  KTextEdit *inputText;
  KTextEdit *translatedText;
  KComboBox *from;
  KComboBox *to;
  KPushButton *translate;
  KIO::Job *job;
};

static void addPairToMap( QMap<QString, QString>& map, const QPair<QString, QString>& pair )
{
  map.insert( pair.first, pair.second );
}

static void addItemToFromComboBox( KComboBox *combo, const QPair<QString, QString>& pair )
{
  combo->addItem( pair.first, pair.second );
}


void TranslatorWidget::TranslatorWidgetPrivate::fillToCombobox( const QString& lang )
{
  to->clear();
  const QMap<QString, QString> list = listLanguage.value( lang );
  QMap<QString, QString>::const_iterator i = list.constBegin();
  while (i != list.constEnd()) {
    to->addItem( i.key(), i.value() );
    ++i;
 }
}

void TranslatorWidget::TranslatorWidgetPrivate::initLanguage()
{
  const QPair<QString, QString> en( i18n("English"), QLatin1String( "en" ) );
  const QPair<QString, QString> zh( i18n("Chinese (Simplified)"), QLatin1String( "zh" ) );
  const QPair<QString, QString> zt( i18n("Chinese (Simplified)"), QLatin1String( "zt" ) );
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

  addItemToFromComboBox( from, en );
  QMap<QString, QString> enList;
  addPairToMap( enList, zh );
  addPairToMap( enList, zt );
  addPairToMap( enList, nl );
  addPairToMap( enList, fr );
  addPairToMap( enList, de );
  addPairToMap( enList, it );
  addPairToMap( enList, ja );
  addPairToMap( enList, ko );
  addPairToMap( enList, pt );
  addPairToMap( enList, ru );
  addPairToMap( enList, es );
  listLanguage.insert( en.second, enList );

  addItemToFromComboBox( from, nl );
  QMap<QString, QString> nlList;
  addPairToMap( nlList, en );
  addPairToMap( nlList, fr );  
  listLanguage.insert( nl.second, nlList );

  addItemToFromComboBox( from, fr );  
  QMap<QString, QString> frList;
  addPairToMap( frList, nl );
  addPairToMap( frList, en );
  addPairToMap( frList, de );
  addPairToMap( frList, el );
  addPairToMap( frList, it );
  addPairToMap( frList, pt );
  addPairToMap( frList, es );
  listLanguage.insert( fr.second, frList );


  addItemToFromComboBox( from, de );
  QMap<QString, QString> deList;
  addPairToMap( deList, en );
  addPairToMap( deList, fr );
  listLanguage.insert( de.second, deList );

  addItemToFromComboBox( from, el );
  QMap<QString, QString> elList;
  addPairToMap( elList, en );
  addPairToMap( elList, fr );
  listLanguage.insert( el.second, elList );



  addItemToFromComboBox( from, it );
  QMap<QString, QString> itList;
  addPairToMap( itList, en );
  addPairToMap( itList, fr );
  listLanguage.insert( it.second, itList );

  addItemToFromComboBox( from, es );
  QMap<QString, QString> esList;
  addPairToMap( esList, en );
  addPairToMap( esList, fr );
  listLanguage.insert( es.second, esList );

  addItemToFromComboBox( from, pt );
  QMap<QString, QString> ptList;
  addPairToMap( ptList, en );
  addPairToMap( ptList, fr );
  listLanguage.insert( pt.second, ptList );

  addItemToFromComboBox( from, ja );
  QMap<QString, QString> jaList;
  addPairToMap( jaList, en );
  listLanguage.insert( ja.second, jaList );

  addItemToFromComboBox( from, ko );
  QMap<QString, QString> koList;
  addPairToMap( koList, ko );
  listLanguage.insert( ko.second, koList );

  addItemToFromComboBox( from, ru );
  QMap<QString, QString> ruList;
  addPairToMap( ruList, ru );
  listLanguage.insert( ru.second, ruList );


  addItemToFromComboBox( from, zt );
  QMap<QString, QString> ztList;
  addPairToMap( ztList, en );
  addPairToMap( ztList, zh );
  listLanguage.insert( zt.second, ztList );

  addItemToFromComboBox( from, zh );
  QMap<QString, QString> zhList;
  addPairToMap( zhList, en );
  addPairToMap( zhList, zt );
  listLanguage.insert( zh.second, zhList );
}



TranslatorWidget::TranslatorWidget( QWidget* parent )
  :QWidget( parent ), d( new TranslatorWidgetPrivate )
{
  init();
}

TranslatorWidget::TranslatorWidget( const QString& text, QWidget* parent )
  :QWidget( parent ), d( new TranslatorWidgetPrivate )
{
  init();
  d->inputText->setPlainText( text );
}

void TranslatorWidget::init()
{
  QVBoxLayout *layout = new QVBoxLayout( this );
  QHBoxLayout *hboxLayout = new QHBoxLayout;
  d->translate = new KPushButton( i18n( "Translate" ) );
  hboxLayout->addWidget( d->translate );
  connect( d->translate, SIGNAL( clicked() ), SLOT( slotTranslate() ) );
  QLabel *label = new QLabel( i18n( "From:" ) );
  hboxLayout->addWidget( label );
  d->from = new KComboBox;
  hboxLayout->addWidget( d->from );

  label = new QLabel( i18n( "To:" ) );
  hboxLayout->addWidget( label );
  d->to = new KComboBox;
  hboxLayout->addWidget( d->to );
  hboxLayout->addItem( new QSpacerItem( 5, 5, QSizePolicy::MinimumExpanding, QSizePolicy::Minimum ) );
   
  layout->addLayout( hboxLayout );

  hboxLayout = new QHBoxLayout;
  d->inputText = new KTextEdit;
  connect( d->inputText, SIGNAL(textChanged() ), SLOT( slotTextChanged() ) );

  hboxLayout->addWidget( d->inputText );
  d->translatedText = new KTextEdit;
  d->translatedText->setReadOnly( true );
  hboxLayout->addWidget( d->translatedText );

  layout->addLayout( hboxLayout );
   
  d->initLanguage();
  connect( d->from, SIGNAL(currentIndexChanged(int) ), SLOT( slotFromLanguageChanged( int ) ) );
  //TODO restore previous config
  d->from->setCurrentIndex( 0 ); //Fill "to" combobox
  slotTextChanged();
}

TranslatorWidget::~TranslatorWidget()
{
  delete d;
}

void TranslatorWidget::slotTextChanged()
{
  d->translate->setEnabled( !d->inputText->document()->isEmpty() );
}

void TranslatorWidget::slotFromLanguageChanged( int index )
{
  const QString lang = d->from->itemData(index).toString();
  d->fillToCombobox( lang );
}

void TranslatorWidget::setTextToTranslate( const QString& text)
{
  d->inputText->setPlainText( text );
}


void TranslatorWidget::slotTranslate()
{
  const QString textToTranslate = d->inputText->toPlainText();
  if ( textToTranslate.isEmpty() )
    return;
  const QString from = d->from->itemData(d->from->currentIndex()).toString();
  const QString to = d->to->itemData(d->to->currentIndex()).toString();
  d->translate->setEnabled( false );
  KUrl geturl ( "http://babelfish.yahoo.com/translate_txt" );

  QString body = QUrl::toPercentEncoding( textToTranslate );
  body.replace(QLatin1String( "%20" ), QLatin1String( "+" ));

  QByteArray postData = QString( "ei=UTF-8&doit=done&fr=bf-res&intl=1&tt=urltext&trtext=%1&lp=%2_%3&btnTrTxt=Translate").arg( body, from, to ).toLocal8Bit();
  kDebug(14308) << "URL:" << geturl << "(post data" << postData << ")";

  KIO::TransferJob *job = KIO::http_post( geturl, postData );
  job->addMetaData( "content-type", "Content-Type: application/x-www-form-urlencoded" );
  job->addMetaData( "referrer", "http://babelfish.yahoo.com/translate_txt" );
  d->data.clear();
  d->job = job;
  connect( job, SIGNAL(data(KIO::Job*,QByteArray)), this, SLOT(slotDataReceived(KIO::Job*,QByteArray)) );
  connect( job, SIGNAL(result(KJob*)), this, SLOT(slotJobDone(KJob*)) );
}

void TranslatorWidget::slotDataReceived ( KIO::Job *job, const QByteArray &data )
{
  if ( job == d->job )
    d->data.append(data);
}

void TranslatorWidget::slotJobDone ( KJob *job )
{
  if ( job == d->job )
  {
    disconnect( job, SIGNAL(data(KIO::Job*,QByteArray)), this, SLOT(slotDataReceived(KIO::Job*,QByteArray)) );
    disconnect( job, SIGNAL(result(KJob*)), this, SLOT(slotJobDone(KJob*)) );

    d->translate->setEnabled( true );
    QRegExp re( "<div style=\"padding:0.6em;\">(.*)</div>" );
    re.setMinimal( true );
    re.indexIn( d->data );
    d->translatedText->setText( re.cap( 1 ) );
  }
}

#include "translatorwidget.moc"



