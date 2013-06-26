/* -*- mode: C++; c-file-style: "gnu" -*-

  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>

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
#include "widgets/minimumcombobox.h"
#include "translatorutil.h"
#include "googletranslator.h"
#include <KTextEdit>
#include <KComboBox>
#include <KPushButton>
#include <KLocale>
#include <kio/job.h>
#include <KDebug>
#include <KConfigGroup>
#include <KSeparator>

#include <kpimutils/progressindicatorwidget.h>

#include <QPair>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QRegExp>
#include <QToolButton>
#include <QKeyEvent>
#include <QShortcut>
#include <QPainter>
#include <QSplitter>

using namespace PimCommon;

class TranslatorWidget::TranslatorWidgetPrivate
{
public:
    TranslatorWidgetPrivate()
        : abstractTranslator(0)
    {

    }
    ~TranslatorWidgetPrivate()
    {
        delete abstractTranslator;
    }

    void initLanguage();
    void fillToCombobox( const QString& lang );

    QMap<QString, QMap<QString, QString> > listLanguage;
    QByteArray data;
    TranslatorTextEdit *inputText;
    TranslatorResultTextEdit *translatedText;
    MinimumComboBox *from;
    MinimumComboBox *to;
    KPushButton *translate;
    PimCommon::AbstractTranslator *abstractTranslator;
    KPIMUtils::ProgressIndicatorWidget *progressIndictor;
    QSplitter *splitter;
};

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
    if (!abstractTranslator) {
        return;
    }
    listLanguage = abstractTranslator->initListLanguage(from);
}


TranslatorResultTextEdit::TranslatorResultTextEdit(QWidget *parent)
    : KTextEdit(parent),
      mResultFailed(false)
{
    setReadOnly( true );
}

void TranslatorResultTextEdit::setResultFailed(bool failed)
{
    if (mResultFailed != failed) {
        mResultFailed = failed;
        update();
    }
}

void TranslatorResultTextEdit::paintEvent( QPaintEvent *event )
{
    if ( mResultFailed ) {
        QPainter p( viewport() );

        QFont font = p.font();
        font.setItalic( true );
        p.setFont( font );

        p.setPen( Qt::red );

        p.drawText( QRect( 0, 0, width(), height() ), Qt::AlignCenter, i18n( "Problem when connecting to the translator web site." ) );
    } else {
        KTextEdit::paintEvent( event );
    }
}

TranslatorTextEdit::TranslatorTextEdit(QWidget *parent)
    : KTextEdit(parent)
{
}

void TranslatorTextEdit::dropEvent( QDropEvent *event )
{
    if (event->source() != this ) {
        if ( event->mimeData()->hasText() ) {
            QTextCursor cursor = textCursor();
            cursor.beginEditBlock();
            cursor.insertText(event->mimeData()->text());
            cursor.endEditBlock();
            event->setDropAction(Qt::CopyAction);
            event->accept();
            return;
        }
    }
    QTextEdit::dropEvent(event);
}


TranslatorWidget::TranslatorWidget( QWidget* parent )
    : QWidget( parent ),
      d( new TranslatorWidgetPrivate )
{
    init();
}

TranslatorWidget::TranslatorWidget( const QString& text, QWidget* parent )
    : QWidget( parent ),
      d( new TranslatorWidgetPrivate )
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
    KConfigGroup myGroup( KGlobal::config(), "TranslatorWidget" );
    myGroup.writeEntry( QLatin1String( "FromLanguage" ), d->from->itemData(d->from->currentIndex()).toString() );
    myGroup.writeEntry( "ToLanguage", d->to->itemData(d->to->currentIndex()).toString() );
    myGroup.writeEntry( "mainSplitter", d->splitter->sizes());
    myGroup.sync();
}

void TranslatorWidget::readConfig()
{
    KConfigGroup myGroup( KGlobal::config(), "TranslatorWidget" );
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
    QList<int> size;
    size << 100 << 400;
    d->splitter->setSizes(myGroup.readEntry( "mainSplitter", size));
}

void TranslatorWidget::init()
{
    d->abstractTranslator = new /*BabelFishTranslator*/GoogleTranslator();
    connect(d->abstractTranslator, SIGNAL(translateDone()), SLOT(slotTranslateDone()));
    connect(d->abstractTranslator, SIGNAL(translateFailed(bool)), SLOT(slotTranslateFailed(bool)));

    QVBoxLayout *layout = new QVBoxLayout( this );
    layout->setMargin( 0 );
    QHBoxLayout *hboxLayout = new QHBoxLayout;
    QToolButton * closeBtn = new QToolButton( this );
    closeBtn->setIcon( KIcon( "dialog-close" ) );
    closeBtn->setIconSize( QSize( 16, 16 ) );
    closeBtn->setToolTip( i18n( "Close" ) );

#ifndef QT_NO_ACCESSIBILITY
    closeBtn->setAccessibleName( i18n( "Close" ) );
#endif
    closeBtn->setAutoRaise( true );
    hboxLayout->addWidget( closeBtn );
    connect( closeBtn, SIGNAL(clicked()), this, SLOT(slotCloseWidget()) );

    QLabel *label = new QLabel( i18nc( "Translate from language", "From:" ) );
    hboxLayout->addWidget( label );
    d->from = new MinimumComboBox;
    hboxLayout->addWidget( d->from );

    label = new QLabel( i18nc( "Translate to language", "To:" ) );
    hboxLayout->addWidget( label );
    d->to = new MinimumComboBox;
    connect( d->to, SIGNAL(currentIndexChanged(int)), SLOT(slotTranslate()) );
    hboxLayout->addWidget( d->to );

    KSeparator *separator = new KSeparator;
    separator->setOrientation(Qt::Vertical);
    hboxLayout->addWidget( separator );

    KPushButton *invert = new KPushButton(
                i18nc("Invert language choices so that from becomes to and to becomes from", "Invert"),this);
    connect(invert, SIGNAL(clicked()), this, SLOT(slotInvertLanguage()));
    hboxLayout->addWidget(invert);

    KPushButton *clear = new KPushButton(i18n("Clear"),this);
#ifndef QT_NO_ACCESSIBILITY
    clear->setAccessibleName( i18n("Clear") );
#endif
    connect(clear, SIGNAL(clicked()), this, SLOT(slotClear()));
    hboxLayout->addWidget(clear);

    d->translate = new KPushButton( i18n( "Translate" ) );
#ifndef QT_NO_ACCESSIBILITY
    d->translate->setAccessibleName( i18n("Translate") );
#endif


    hboxLayout->addWidget( d->translate );
    connect( d->translate, SIGNAL(clicked()), SLOT(slotTranslate()) );

#if !defined(NDEBUG)
    KPushButton *debug = new KPushButton(i18n("Debug"));
    connect(debug,SIGNAL(clicked()),this,SLOT(slotDebug()));
    hboxLayout->addWidget( debug );
#endif

    d->progressIndictor = new KPIMUtils::ProgressIndicatorWidget(this);
    hboxLayout->addWidget( d->progressIndictor );

    hboxLayout->addItem( new QSpacerItem( 5, 5, QSizePolicy::MinimumExpanding, QSizePolicy::Minimum ) );

    layout->addLayout( hboxLayout );

    d->splitter = new QSplitter;
    d->splitter->setChildrenCollapsible( false );
    d->inputText = new TranslatorTextEdit;
    d->inputText->setAcceptRichText(false);
    d->inputText->setClickMessage(i18n("Drag text that you want to translate."));
    connect( d->inputText, SIGNAL(textChanged()), SLOT(slotTextChanged()) );

    d->splitter->addWidget( d->inputText );
    d->translatedText = new TranslatorResultTextEdit;
    d->translatedText->setReadOnly( true );
    d->splitter->addWidget( d->translatedText );

    layout->addWidget( d->splitter );

    d->initLanguage();
    connect( d->from, SIGNAL(currentIndexChanged(int)), SLOT(slotFromLanguageChanged(int)) );

    d->from->setCurrentIndex( 0 ); //Fill "to" combobox
    slotFromLanguageChanged( 0 );
    slotTextChanged();
    readConfig();
    hide();
    setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed ) );
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
    slotTranslate();
}

void TranslatorWidget::setTextToTranslate( const QString& text)
{
    d->inputText->setPlainText( text );
    d->translatedText->clear();
    slotTranslate();
}


void TranslatorWidget::slotTranslate()
{
    const QString textToTranslate = d->inputText->toPlainText();
    if ( textToTranslate.isEmpty() )
        return;
    d->translatedText->clear();

    const QString from = d->from->itemData(d->from->currentIndex()).toString();
    const QString to = d->to->itemData(d->to->currentIndex()).toString();
    d->translate->setEnabled( false );
    d->progressIndictor->start();

    d->abstractTranslator->setFrom(from);
    d->abstractTranslator->setTo(to);
    d->abstractTranslator->setInputText(d->inputText->toPlainText());
    d->abstractTranslator->translate();
}

void TranslatorWidget::slotTranslateDone()
{
    d->translate->setEnabled( true );
    d->progressIndictor->stop();
    d->translatedText->setResultFailed(false);
    d->translatedText->setPlainText(d->abstractTranslator->resultTranslate());
}

void TranslatorWidget::slotTranslateFailed(bool signalFailed)
{
    d->translate->setEnabled( true );
    d->progressIndictor->stop();
    d->translatedText->setResultFailed(signalFailed);
    d->translatedText->clear();
}

void TranslatorWidget::slotInvertLanguage()
{
    const QString toLanguage = d->to->itemData(d->to->currentIndex()).toString();
    const QString fromLanguage = d->from->itemData(d->from->currentIndex()).toString();

    d->from->blockSignals(true);
    d->to->blockSignals(true);
    const int indexFrom = d->from->findData( toLanguage );
    if ( indexFrom != -1 ) {
        d->from->setCurrentIndex( indexFrom );
    }
    const int indexTo = d->to->findData( fromLanguage );
    if ( indexTo != -1 ) {
        d->to->setCurrentIndex( indexTo );
    }
    d->from->blockSignals(false);
    d->to->blockSignals(false);
    slotTranslate();
}

void TranslatorWidget::slotCloseWidget()
{
    d->inputText->clear();
    d->translatedText->clear();
    d->progressIndictor->stop();
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
    d->translate->setEnabled( false );
}

void TranslatorWidget::slotDebug()
{
    d->abstractTranslator->debug();
}

#include "translatorwidget.moc"



