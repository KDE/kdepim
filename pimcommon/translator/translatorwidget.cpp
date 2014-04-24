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
#include "pimcommon/texteditor/plaintexteditor/plaintexteditorwidget.h"

#include <KTextEdit>
#include <KPushButton>
#include <KLocalizedString>
#include <kio/job.h>
#include <KDebug>
#include <KConfigGroup>
#include <KSeparator>
#include <KMessageBox>
#include <KToggleAction>
#include <kpimutils/progressindicatorwidget.h>
#include <KIcon>
#include <KGlobal>
#include <QMimeData> 

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
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
        : abstractTranslator(0),
          action(0),
          languageSettingsChanged(false),
          networkUp(true),
          standalone(true)
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
    PimCommon::PlainTextEditorWidget *translatedText;
    TranslatorResultTextEdit *translatorResultTextEdit;
    MinimumComboBox *from;
    MinimumComboBox *to;
    KPushButton *translate;
    PimCommon::AbstractTranslator *abstractTranslator;
    KPIMUtils::ProgressIndicatorWidget *progressIndictor;
    KPushButton *invert;
    QSplitter *splitter;
    KToggleAction *action;
    bool languageSettingsChanged;
    bool networkUp;
    bool standalone;
};

void TranslatorWidget::TranslatorWidgetPrivate::fillToCombobox( const QString &lang )
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
    : PimCommon::PlainTextEditor(parent),
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
        PimCommon::PlainTextEditor::paintEvent( event );
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
            Q_EMIT translateText();
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
    if (d->languageSettingsChanged) {
        myGroup.writeEntry( QLatin1String( "FromLanguage" ), d->from->itemData(d->from->currentIndex()).toString() );
        myGroup.writeEntry( "ToLanguage", d->to->itemData(d->to->currentIndex()).toString() );
    }
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
    d->abstractTranslator = new GoogleTranslator();
    connect(d->abstractTranslator, SIGNAL(translateDone()), SLOT(slotTranslateDone()));
    connect(d->abstractTranslator, SIGNAL(translateFailed(bool,QString)), SLOT(slotTranslateFailed(bool,QString)));

    QVBoxLayout *layout = new QVBoxLayout( this );
    layout->setMargin( 2 );
    QHBoxLayout *hboxLayout = new QHBoxLayout;
    QToolButton * closeBtn = new QToolButton( this );
    closeBtn->setObjectName(QLatin1String("close-button"));
    closeBtn->setIcon( KIcon( QLatin1String("dialog-close") ) );
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
    d->from->setObjectName(QLatin1String("from"));
    hboxLayout->addWidget( d->from );

    label = new QLabel( i18nc( "Translate to language", "To:" ) );
    hboxLayout->addWidget( label );
    d->to = new MinimumComboBox;
    d->to->setObjectName(QLatin1String("to"));
    connect( d->to, SIGNAL(currentIndexChanged(int)), SLOT(slotTranslate()) );
    connect( d->to, SIGNAL(currentIndexChanged(int)), SLOT(slotConfigChanged()));
    hboxLayout->addWidget( d->to );

    KSeparator *separator = new KSeparator;
    separator->setOrientation(Qt::Vertical);
    hboxLayout->addWidget( separator );

    d->invert = new KPushButton(
                i18nc("Invert language choices so that from becomes to and to becomes from", "Invert"),this);
    d->invert->setObjectName(QLatin1String("invert-button"));
    connect(d->invert, SIGNAL(clicked()), this, SLOT(slotInvertLanguage()));
    hboxLayout->addWidget(d->invert);

    KPushButton *clear = new KPushButton(i18n("Clear"),this);
    clear->setObjectName(QLatin1String("clear-button"));
#ifndef QT_NO_ACCESSIBILITY
    clear->setAccessibleName( i18n("Clear") );
#endif
    connect(clear, SIGNAL(clicked()), this, SLOT(slotClear()));
    hboxLayout->addWidget(clear);

    d->translate = new KPushButton( i18n( "Translate" ) );
    d->translate->setObjectName(QLatin1String("translate-button"));
#ifndef QT_NO_ACCESSIBILITY
    d->translate->setAccessibleName( i18n("Translate") );
#endif


    hboxLayout->addWidget( d->translate );
    connect( d->translate, SIGNAL(clicked()), SLOT(slotTranslate()) );

#if !defined(NDEBUG)
    QPushButton *debugButton = new QPushButton( i18n("Debug") );
    hboxLayout->addWidget( debugButton );
    connect( debugButton, SIGNAL(clicked()), SLOT(slotDebug()));
#endif

    d->progressIndictor = new KPIMUtils::ProgressIndicatorWidget(this);
    hboxLayout->addWidget( d->progressIndictor );

    hboxLayout->addItem( new QSpacerItem( 5, 5, QSizePolicy::MinimumExpanding, QSizePolicy::Minimum ) );

    layout->addLayout( hboxLayout );

    d->splitter = new QSplitter;
    d->splitter->setChildrenCollapsible( false );
    d->inputText = new TranslatorTextEdit;
    d->inputText->setObjectName(QLatin1String("inputtext"));
    d->inputText->enableFindReplace(false);
    d->inputText->setAcceptRichText(false);
    d->inputText->setClickMessage(i18n("Drag text that you want to translate."));
    connect( d->inputText, SIGNAL(textChanged()), SLOT(slotTextChanged()) );
    connect( d->inputText, SIGNAL(translateText()), SLOT(slotTranslate()));

    d->splitter->addWidget( d->inputText );
    d->translatorResultTextEdit = new TranslatorResultTextEdit;
    d->translatedText = new PimCommon::PlainTextEditorWidget(d->translatorResultTextEdit, this);
    d->translatedText->setObjectName(QLatin1String("translatedtext"));
    d->translatedText->setReadOnly( true );
    d->splitter->addWidget( d->translatedText );

    layout->addWidget( d->splitter );

    d->initLanguage();
    connect( d->from, SIGNAL(currentIndexChanged(int)), SLOT(slotFromLanguageChanged(int)) );
    connect( d->from, SIGNAL(currentIndexChanged(int)), SLOT(slotConfigChanged()));

    d->from->setCurrentIndex( 0 ); //Fill "to" combobox
    slotFromLanguageChanged( 0, true );
    slotTextChanged();
    readConfig();
    hide();
    setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed ) );
    d->languageSettingsChanged = false;

    connect ( Solid::Networking::notifier(), SIGNAL(statusChanged(Solid::Networking::Status)),
              this, SLOT(slotSystemNetworkStatusChanged(Solid::Networking::Status)) );
    Solid::Networking::Status networkStatus = Solid::Networking::status();
    if ( ( networkStatus == Solid::Networking::Unconnected ) ||
         ( networkStatus == Solid::Networking::Disconnecting ) ||
         ( networkStatus == Solid::Networking::Connecting ))
        d->networkUp = false;
    else
        d->networkUp = true;
}

KToggleAction *TranslatorWidget::toggleAction()
{
    if (!d->action) {
        d->action = new KToggleAction( i18n("&Translator"), this );
        d->action->setShortcut( QKeySequence( Qt::CTRL + Qt::ALT + Qt::Key_T ) );
        d->action->setChecked(false);
    }
    return d->action;
}

void TranslatorWidget::slotConfigChanged()
{
    d->languageSettingsChanged = true;
}

void TranslatorWidget::slotTextChanged()
{
    d->translate->setEnabled( !d->inputText->document()->isEmpty() );
}

void TranslatorWidget::slotFromLanguageChanged(int index, bool initialize)
{
    const QString lang = d->from->itemData(index).toString();
    d->invert->setEnabled(lang != QLatin1String("auto"));
    const QString to = d->to->itemData(d->to->currentIndex()).toString();
    d->to->blockSignals(true);
    d->fillToCombobox( lang );
    d->to->blockSignals(false);
    const int indexTo = d->to->findData( to );
    if ( indexTo != -1 ) {
        d->to->setCurrentIndex( indexTo );
    }
    if (!initialize)
        slotTranslate();
}

void TranslatorWidget::setTextToTranslate( const QString& text)
{
    d->inputText->setPlainText( text );
    d->translatorResultTextEdit->clear();
    slotTranslate();
}

void TranslatorWidget::slotTranslate()
{
    if (!d->networkUp) {
        KMessageBox::information(this, i18n("No network connection detected, we cannot translate text."), i18n("No network"));
        return;
    }
    const QString textToTranslate = d->inputText->toPlainText();
    if ( textToTranslate.trimmed().isEmpty() )
        return;

    d->translatorResultTextEdit->clear();

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
    d->translatorResultTextEdit->setResultFailed(false);
    d->translatorResultTextEdit->setPlainText(d->abstractTranslator->resultTranslate());
}

void TranslatorWidget::slotTranslateFailed(bool signalFailed, const QString &message)
{
    d->translate->setEnabled( true );
    d->progressIndictor->stop();
    d->translatorResultTextEdit->setResultFailed(signalFailed);
    d->translatorResultTextEdit->clear();
    if (!message.isEmpty()) {
        KMessageBox::error(this, message, i18n("Translate error"));
    }
}

void TranslatorWidget::slotInvertLanguage()
{
    const QString fromLanguage = d->from->itemData(d->from->currentIndex()).toString();
    // don't invert when fromLanguage == auto
    if (fromLanguage == QLatin1String("auto"))
        return;

    const QString toLanguage = d->to->itemData(d->to->currentIndex()).toString();
    const int indexFrom = d->from->findData( toLanguage );
    if ( indexFrom != -1 ) {
        d->from->setCurrentIndex( indexFrom );
    }
    const int indexTo = d->to->findData( fromLanguage );
    if ( indexTo != -1 ) {
        d->to->setCurrentIndex( indexTo );
    }
    slotTranslate();
}

void TranslatorWidget::setStandalone(bool b)
{
    d->standalone = b;
}

void TranslatorWidget::slotCloseWidget()
{
    if (isHidden())
        return;
    d->inputText->clear();
    d->translatorResultTextEdit->clear();
    d->progressIndictor->stop();
    if (d->standalone)
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
    d->translatorResultTextEdit->clear();
    d->translate->setEnabled( false );
    d->abstractTranslator->clear();
}

void TranslatorWidget::slotDebug()
{
#if !defined(NDEBUG)
    d->abstractTranslator->debug();
#endif
}

void TranslatorWidget::slotSystemNetworkStatusChanged( Solid::Networking::Status status )
{
    if ( status == Solid::Networking::Connected || status == Solid::Networking::Unknown) {
        d->networkUp = true;
    } else {
        d->networkUp = false;
    }
}




