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
#include "globalsettings.h"

#include <KTextEdit>
#include <KComboBox>
#include <KPushButton>
#include <KLocale>
#include <kio/job.h>
#include <KDebug>
#include <KConfigGroup>

#include <QPair>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QRegExp>
#include <QToolButton>
#include <QKeyEvent>

using namespace MessageViewer;

class TranslatorWidget::TranslatorWidgetPrivate
{
public:
  TranslatorWidgetPrivate()
  {
    
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
  QMap<QString, QString>::const_iterator end = list.constEnd();
  while (i != end) {
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
  addPairToMap( koList, en );
  listLanguage.insert( ko.second, koList );

  addItemToFromComboBox( from, ru );
  QMap<QString, QString> ruList;
  addPairToMap( ruList, en );
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

TranslatorWidget::~TranslatorWidget()
{
  writeConfig();
  delete d;
}

void TranslatorWidget::writeConfig()
{
  KConfig *config = GlobalSettings::self()->config();
  KConfigGroup myGroup( config, "TranslatorWidget" );
  myGroup.writeEntry( QLatin1String( "FromLanguage" ), d->from->itemData(d->from->currentIndex()).toString() );
  myGroup.writeEntry( "ToLanguage", d->to->itemData(d->to->currentIndex()).toString() );
  config->sync();
}

void TranslatorWidget::readConfig()
{
  KConfig *config = GlobalSettings::self()->config();
  KConfigGroup myGroup( config, "TranslatorWidget" );
  const QString from = myGroup.readEntry( QLatin1String( "FromLanguage" ) );
  const QString to = myGroup.readEntry( QLatin1String( "ToLanguage" ) );
  if ( from.isEmpty() )
    return;
  const int indexFrom = d->from->findData( from );
  if ( indexFrom != -1 ) {
    d->from->setCurrentIndex( indexFrom );
  }
  const int indexTo = d->to->findData( to );
  if ( indexTo != -1 ) {
    d->to->setCurrentIndex( indexTo );
  }
}

void TranslatorWidget::init()
{
  QVBoxLayout *layout = new QVBoxLayout( this );
  layout->setMargin( 0 );
  QHBoxLayout *hboxLayout = new QHBoxLayout;
  QToolButton * closeBtn = new QToolButton( this );
  closeBtn->setIcon( KIcon( "dialog-close" ) );
  closeBtn->setIconSize( QSize( 24, 24 ) );
  closeBtn->setToolTip( i18n( "Close" ) );

#ifndef QT_NO_ACCESSIBILITY
  closeBtn->setAccessibleName( i18n( "Close" ) );
#endif
  closeBtn->setAutoRaise( true );
  hboxLayout->addWidget( closeBtn );
  connect( closeBtn, SIGNAL(clicked()), this, SLOT(slotCloseWidget()) );

  d->translate = new KPushButton( i18n( "Translate" ) );
  hboxLayout->addWidget( d->translate );
  connect( d->translate, SIGNAL(clicked()), SLOT(slotTranslate()) );
  QLabel *label = new QLabel( i18n( "From:" ) );
  hboxLayout->addWidget( label );
  d->from = new KComboBox;
  hboxLayout->addWidget( d->from );

  label = new QLabel( i18n( "To:" ) );
  hboxLayout->addWidget( label );
  d->to = new KComboBox;
  hboxLayout->addWidget( d->to );

  KPushButton *invert = new KPushButton(i18n("Invert"),this);
  connect(invert,SIGNAL(clicked()),this,SLOT(slotInvertLanguage()));
  hboxLayout->addWidget(invert);

  KPushButton *clear = new KPushButton(i18n("Clear"),this);
  connect(clear,SIGNAL(clicked()),this,SLOT(slotClear()));
  hboxLayout->addWidget(clear);


  hboxLayout->addItem( new QSpacerItem( 5, 5, QSizePolicy::MinimumExpanding, QSizePolicy::Minimum ) );
   
  layout->addLayout( hboxLayout );

  hboxLayout = new QHBoxLayout;
  d->inputText = new KTextEdit;
  d->inputText->setAcceptRichText(false);
  d->inputText->setClickMessage(i18n("Drag text that you want to translate."));
  connect( d->inputText, SIGNAL(textChanged()), SLOT(slotTextChanged()) );

  hboxLayout->addWidget( d->inputText );
  d->translatedText = new KTextEdit;
  d->translatedText->setReadOnly( true );
  hboxLayout->addWidget( d->translatedText );

  layout->addLayout( hboxLayout );
   
  d->initLanguage();
  connect( d->from, SIGNAL(currentIndexChanged(int)), SLOT(slotFromLanguageChanged(int)) );

  d->from->setCurrentIndex( 0 ); //Fill "to" combobox
  slotFromLanguageChanged( 0 );
  slotTextChanged();
  readConfig();
  hide();
}

void TranslatorWidget::slotTextChanged()
{
  d->translate->setEnabled( !d->inputText->document()->isEmpty() );
}

void TranslatorWidget::slotFromLanguageChanged( int index )
{
  const QString lang = d->from->itemData(index).toString();
  const QString to = d->to->itemData(d->to->currentIndex()).toString();
  d->to->blockSignals(true);
  d->fillToCombobox( lang );
  d->to->blockSignals(false);
  const int indexTo = d->to->findData( to );
  if ( indexTo != -1 ) {
    d->to->setCurrentIndex( indexTo );
  }
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
  kDebug() << "URL:" << geturl << "(post data" << postData << ")";

  KIO::StoredTransferJob *job = KIO::storedHttpPost(postData,geturl);
  job->addMetaData( "content-type", "Content-Type: application/x-www-form-urlencoded" );
  job->addMetaData( "referrer", "http://babelfish.yahoo.com/translate_txt" );
  connect( job, SIGNAL(result(KJob*)), this, SLOT(slotJobDone(KJob*)) );
}

void TranslatorWidget::slotJobDone ( KJob *job )
{
  KIO::StoredTransferJob *httpPostJob = dynamic_cast<KIO::StoredTransferJob *>(job);
  if(httpPostJob) {
    d->translate->setEnabled( true );
    const QString data = QString::fromUtf8(httpPostJob->data());
    const QString startTag = QLatin1String("<div style=\"padding:0.6em;\">");
    int index = data.indexOf(startTag);
    if(index != -1) {
      QString newStr = data.right(data.length() - index - startTag.length());
      index = newStr.indexOf(QLatin1String("</div>"));
      d->translatedText->setHtml(newStr.left(index));
    } else {
      d->translatedText->clear();
    }
  }
}

void TranslatorWidget::slotInvertLanguage()
{
  const QString toLanguage = d->to->itemData(d->to->currentIndex()).toString();
  const int indexFrom = d->from->findData( toLanguage );
  if ( indexFrom != -1 ) {
    d->from->setCurrentIndex( indexFrom );
  }
  const QString fromLanguage = d->to->itemData(d->from->currentIndex()).toString();
  const int indexTo = d->to->findData( fromLanguage );
  if ( indexTo != -1 ) {
    d->to->setCurrentIndex( indexTo );
  }
}

void TranslatorWidget::slotCloseWidget()
{
  d->inputText->clear();
  d->translatedText->clear();
  hide();
  Q_EMIT translatorWasClosed();
}

bool TranslatorWidget::event(QEvent* e)
{
  // Close the bar when pressing Escape.
  // Not using a QShortcut for this because it could conflict with
  // window-global actions (e.g. Emil Sedgh binds Esc to "close tab").
  // With a shortcut override we can catch this before it gets to kactions.
  if (e->type() == QEvent::ShortcutOverride || e->type() == QEvent::KeyPress ) {
    QKeyEvent* kev = static_cast<QKeyEvent* >(e);
    if (kev->key() == Qt::Key_Escape) {
      e->accept();
      slotCloseWidget();
      return true;
    }
  }
  return QWidget::event(e);
}

void TranslatorWidget::slotClear()
{
  d->inputText->clear();
  d->translatedText->clear();
}

#include "translatorwidget.moc"



