/* -*- mode: C++; c-file-style: "gnu" -*-

  Author: Marc Mutz <mutz@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "searchpatternedit.h"
#include "rulewidgethandlermanager.h"
using MailCommon::RuleWidgetHandlerManager;

#include <pimcommon/widgets/minimumcombobox.h>
#include <KComboBox>
#include <KDebug>
#include <KDialog>
#include <KLocale>
#include <KPushButton>
#include <KLineEdit>

#include <QButtonGroup>
#include <QByteArray>
#include <QHBoxLayout>
#include <QRadioButton>
#include <QResizeEvent>
#include <QStackedWidget>

// Definition of special rule field strings
// Note: Also see SearchRule::matches() and ruleFieldToEnglish() if
//       you change the following i18n-ized strings!
// Note: The index of the values in the following array has to correspond to
//       the value of the entries in the enum in SearchRuleWidget.

#undef I18N_NOOP
#define I18N_NOOP(t) 0, t
#undef I18N_NOOP2
#define I18N_NOOP2(c,t) c, t

using namespace MailCommon;

static const struct {
  const char *internalName;
  const char *context;
  const char *displayName;

  QString getLocalizedDisplayName() const
  {
    return i18nc( context, displayName );
  }

} SpecialRuleFields[] = {
  { "<message>",     I18N_NOOP( "Complete Message" )       },
  { "<body>",        I18N_NOOP( "Body of Message" )          },
  { "<any header>",  I18N_NOOP( "Anywhere in Headers" )    },
  { "<recipients>",  I18N_NOOP( "All Recipients" )    },
  { "<size>",        I18N_NOOP( "Size in Bytes" ) },
  { "<age in days>", I18N_NOOP( "Age in Days" )   },
  { "<status>",      I18N_NOOP( "Message Status" )        },
  { "<tag>",         I18N_NOOP( "Message Tag" ) },
  { "Subject",       I18N_NOOP2( "Subject of an email.", "Subject" )  },
  { "From",          I18N_NOOP( "From" )  },
  { "To",            I18N_NOOP2( "Receiver of an email.", "To" )  },
  { "CC",            I18N_NOOP( "CC" )  },
  { "Reply-To",      I18N_NOOP( "Reply To" )  },
  { "Organization",  I18N_NOOP( "Organization" )  }, 
  { "<date>",  I18N_NOOP( "Date" )  }
};
static const int SpecialRuleFieldsCount =
  sizeof( SpecialRuleFields ) / sizeof( *SpecialRuleFields );

//=============================================================================
//
// class SearchRuleWidget
//
//=============================================================================

SearchRuleWidget::SearchRuleWidget(QWidget *parent, SearchRule::Ptr aRule,
                                   SearchPatternEdit::SearchPatternEditOptions options ,
                                   SearchPatternEdit::SearchModeType modeType)
    : QWidget( parent ),
    mRuleField( 0 ),
    mFunctionStack( 0 ),
    mValueStack( 0 )
{
  initFieldList( options );
  initWidget( modeType );

  if ( aRule ) {
    setRule( aRule );
  } else {
    reset();
  }
}

void SearchRuleWidget::setPatternEditOptions( SearchPatternEdit::SearchPatternEditOptions options )
{
  SearchRule::Ptr srule = rule();
  QByteArray currentText = srule->field();

  initFieldList( options );

  mRuleField->clear();
  mRuleField->addItems( mFilterFieldList );
  KCompletion *comp = mRuleField->completionObject();
  comp->clear();
  comp->insertItems(mFilterFieldList);
  mRuleField->setMaxCount( mRuleField->count() );
  mRuleField->adjustSize();

  const bool headersOnly = ( options & MailCommon::SearchPatternEdit::HeadersOnly );
  const bool notShowSize = ( options & MailCommon::SearchPatternEdit::NotShowSize );
  const bool notShowDate = ( options & MailCommon::SearchPatternEdit::NotShowDate );
  const bool absoluteDates = ( options & MailCommon::SearchPatternEdit::AbsoluteDate );

  if ( headersOnly && ( currentText != "<message>") && ( currentText != "<body>" ) ) {
    mRuleField->setItemText( 0, QString::fromLatin1( currentText ) );
  } else {
    mRuleField->setItemText( 0, QString() );
  }

  if ( notShowSize && ( currentText != "<size>") ) {
    mRuleField->setItemText( 0, QString::fromLatin1( currentText ) );
  } else {
    mRuleField->setItemText( 0, QString() );
  }

  if ( notShowDate && ( currentText != "<date>") ) {
    mRuleField->setItemText( 0, QString::fromLatin1( currentText ) );
  } else {
    mRuleField->setItemText( 0, QString() );
  }

  if ( !absoluteDates && ( currentText != "<age in days>") ) {
    mRuleField->setItemText( 0, QString::fromLatin1( currentText ) );
  } else {
    mRuleField->setItemText( 0, QString() );
  }
}



void SearchRuleWidget::initWidget(SearchPatternEdit::SearchModeType modeType)
{
  QHBoxLayout *hlay = new QHBoxLayout( this );
  hlay->setSpacing( KDialog::spacingHint() );
  hlay->setMargin( 0 );

  // initialize the header field combo box
  mRuleField = new PimCommon::MinimumComboBox( this );
  mRuleField->setObjectName( QLatin1String("mRuleField") );
  mRuleField->setEditable( true );
  KLineEdit *edit = new KLineEdit;
  edit->setClickMessage( i18n("Choose or type your own criteria"));
  mRuleField->setToolTip(i18n("Choose or type your own criteria"));
  edit->setClearButtonShown(true);
  mRuleField->setLineEdit(edit);
  mRuleField->setTrapReturnKey(true);

  mRuleField->addItems( mFilterFieldList );
  KCompletion *comp = mRuleField->completionObject();
  comp->setIgnoreCase(true);
  comp->insertItems(mFilterFieldList);
  comp->setCompletionMode(KGlobalSettings::CompletionPopupAuto);

  // don't show sliders when popping up this menu
  mRuleField->setMaxCount( mRuleField->count() );
  mRuleField->adjustSize();
  hlay->addWidget( mRuleField );

  // initialize the function/value widget stack
  mFunctionStack = new QStackedWidget( this );
  //Don't expand the widget in vertical direction
  mFunctionStack->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );

  hlay->addWidget( mFunctionStack );

  mValueStack = new QStackedWidget( this );
  hlay->addWidget( mValueStack );
  hlay->setStretchFactor( mValueStack, 10 );

  mAdd = new KPushButton( this );
  mAdd->setIcon( KIcon( QLatin1String("list-add") ) );
  mAdd->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );
  hlay->addWidget( mAdd );

  mRemove = new KPushButton( this );
  mRemove->setIcon( KIcon( QLatin1String("list-remove") ) );
  mRemove->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );
  hlay->addWidget( mRemove );

  RuleWidgetHandlerManager::instance()->setIsNepomukSearch(modeType==SearchPatternEdit::NepomukMode);

  RuleWidgetHandlerManager::instance()->createWidgets( mFunctionStack, mValueStack, this );

  // redirect focus to the header field combo box
  setFocusProxy( mRuleField );

  connect( mRuleField, SIGNAL(activated(QString)),
           this, SLOT(slotRuleFieldChanged(QString)) );
  connect( mRuleField, SIGNAL(editTextChanged(QString)),
           this, SLOT(slotRuleFieldChanged(QString)) );
  connect( mRuleField, SIGNAL(editTextChanged(QString)),
           this, SIGNAL(fieldChanged(QString)) );

  connect( mAdd, SIGNAL(clicked()),
           this, SLOT(slotAddWidget()) );
  connect( mRemove, SIGNAL(clicked()),
           this, SLOT(slotRemoveWidget()) );
}

void SearchRuleWidget::updateAddRemoveButton( bool addButtonEnabled, bool removeButtonEnabled )
{
  mAdd->setEnabled( addButtonEnabled );
  mRemove->setEnabled( removeButtonEnabled );
}

void SearchRuleWidget::slotAddWidget()
{
  emit addWidget( this );
}

void SearchRuleWidget::slotRemoveWidget()
{
  emit removeWidget( this );
}

void SearchRuleWidget::setRule( SearchRule::Ptr aRule )
{
  Q_ASSERT( aRule );

  kDebug() << "(" << aRule->asString() << ")";

  //--------------set the field
  int i = indexOfRuleField( aRule->field() );

  mRuleField->blockSignals( true );

  if ( i < 0 ) { // not found -> user defined field
    mRuleField->setItemText( 0, QString::fromLatin1( aRule->field() ) );
    i = 0;
  } else { // found in the list of predefined fields
    mRuleField->setItemText( 0, QString() );
  }

  mRuleField->setCurrentIndex( i );
  mRuleField->blockSignals( false );

  RuleWidgetHandlerManager::instance()->setRule( mFunctionStack, mValueStack, aRule );
}

SearchRule::Ptr SearchRuleWidget::rule() const
{
  const QByteArray ruleField = ruleFieldToEnglish( mRuleField->currentText() );

  const SearchRule::Function function =
    RuleWidgetHandlerManager::instance()->function( ruleField, mFunctionStack );

  const QString value =
    RuleWidgetHandlerManager::instance()->value( ruleField, mFunctionStack, mValueStack );

  return SearchRule::createInstance( ruleField, function, value );
}

void SearchRuleWidget::reset()
{
  mRuleField->blockSignals( true );
  mRuleField->setItemText( 0, QString() );
  mRuleField->setCurrentIndex( 0 );
  mRuleField->blockSignals( false );

  RuleWidgetHandlerManager::instance()->reset( mFunctionStack, mValueStack );
}

void SearchRuleWidget::slotFunctionChanged()
{
  const QByteArray ruleField = ruleFieldToEnglish( mRuleField->currentText() );
  RuleWidgetHandlerManager::instance()->update( ruleField, mFunctionStack, mValueStack );
  const QString prettyValue =
    RuleWidgetHandlerManager::instance()->prettyValue( ruleField, mFunctionStack, mValueStack );

  emit contentsChanged( prettyValue );
}

void SearchRuleWidget::slotValueChanged()
{
  const QByteArray ruleField = ruleFieldToEnglish( mRuleField->currentText() );

  const QString prettyValue =
    RuleWidgetHandlerManager::instance()->prettyValue( ruleField, mFunctionStack, mValueStack );

  emit contentsChanged( prettyValue );
}

void SearchRuleWidget::slotReturnPressed()
{
  emit returnPressed();
}

QByteArray SearchRuleWidget::ruleFieldToEnglish( const QString & i18nVal )
{
  for ( int i = 0; i < SpecialRuleFieldsCount; ++i ) {
    if ( i18nVal == SpecialRuleFields[i].getLocalizedDisplayName() ) {
      return SpecialRuleFields[i].internalName;
    }
  }
  return i18nVal.toLatin1();
}

int SearchRuleWidget::ruleFieldToId( const QString & i18nVal )
{
  for ( int i = 0; i < SpecialRuleFieldsCount; ++i ) {
    if ( i18nVal == SpecialRuleFields[i].getLocalizedDisplayName() ) {
      return i;
    }
  }
  return -1; // no pseudo header
}

static QString displayNameFromInternalName( const QString & internal )
{
  for ( int i = 0; i < SpecialRuleFieldsCount; ++i ) {
    if ( internal == QLatin1String(SpecialRuleFields[i].internalName) ) {
      return SpecialRuleFields[i].getLocalizedDisplayName();
    }
  }
  return QLatin1String(internal.toLatin1());
}

int SearchRuleWidget::indexOfRuleField( const QByteArray &aName ) const
{
  if ( aName.isEmpty() ) {
    return -1;
  }

  const QString i18n_aName = displayNameFromInternalName( QLatin1String(aName) );
  const int nbRuleField = mRuleField->count();
  for ( int i = 1; i < nbRuleField; ++i ) {
    if ( mRuleField->itemText( i ) == i18n_aName ) {
      return i;
    }
  }

  return -1;
}

void SearchRuleWidget::initFieldList( SearchPatternEdit::SearchPatternEditOptions options )
{
  const bool headersOnly = ( options & MailCommon::SearchPatternEdit::HeadersOnly );
  const bool absoluteDates = ( options & MailCommon::SearchPatternEdit::AbsoluteDate );
  const bool notShowSize = ( options & MailCommon::SearchPatternEdit::NotShowSize );
  const bool notShowDate = ( options & MailCommon::SearchPatternEdit::NotShowDate );

  mFilterFieldList.clear();
  mFilterFieldList.append( QString() ); // empty entry for user input

  if ( !headersOnly ) {
    mFilterFieldList.append( SpecialRuleFields[Message].getLocalizedDisplayName() );
    mFilterFieldList.append( SpecialRuleFields[Body].getLocalizedDisplayName() );
  }
  mFilterFieldList.append( SpecialRuleFields[AnyHeader].getLocalizedDisplayName() );
  mFilterFieldList.append( SpecialRuleFields[Recipients].getLocalizedDisplayName() );
  if ( !notShowSize )
    mFilterFieldList.append( SpecialRuleFields[Size].getLocalizedDisplayName() );
  if ( !absoluteDates ) {
    mFilterFieldList.append( SpecialRuleFields[AgeInDays].getLocalizedDisplayName() );
  }

  mFilterFieldList.append( SpecialRuleFields[Subject].getLocalizedDisplayName() );
  mFilterFieldList.append( SpecialRuleFields[From].getLocalizedDisplayName() );
  mFilterFieldList.append( SpecialRuleFields[To].getLocalizedDisplayName() );
  mFilterFieldList.append( SpecialRuleFields[CC].getLocalizedDisplayName() );
  mFilterFieldList.append( SpecialRuleFields[Status].getLocalizedDisplayName() );
  mFilterFieldList.append( SpecialRuleFields[Tag].getLocalizedDisplayName() );
  mFilterFieldList.append( i18n( SpecialRuleFields[ReplyTo].displayName ) );
  mFilterFieldList.append( i18n( SpecialRuleFields[Organization].displayName ) );

  if ( !notShowDate )
      mFilterFieldList.append( i18n( SpecialRuleFields[Date].displayName ) );

  // these others only represent message headers and you can add to
  // them as you like
  mFilterFieldList.append( QLatin1String("List-Id") );
  mFilterFieldList.append( QLatin1String("Resent-From") );
  mFilterFieldList.append( QLatin1String("X-Loop") );
  mFilterFieldList.append( QLatin1String("X-Mailing-List") );
  mFilterFieldList.append( QLatin1String("X-Spam-Flag") );
}

void SearchRuleWidget::slotRuleFieldChanged( const QString &field )
{
  RuleWidgetHandlerManager::instance()->update(
    ruleFieldToEnglish( field ), mFunctionStack, mValueStack );
}

//=============================================================================
//
// class KMFilterActionWidgetLister (the filter action editor)
//
//=============================================================================

SearchRuleWidgetLister::SearchRuleWidgetLister( QWidget *parent, SearchPatternEdit::SearchPatternEditOptions options, SearchPatternEdit::SearchModeType modeType)
  : KWidgetLister( false, 2, FILTER_MAX_RULES, parent )
{
  mRuleList = 0;

  mTypeMode = modeType;
  mOptions = options;
}

SearchRuleWidgetLister::~SearchRuleWidgetLister()
{
}

void SearchRuleWidgetLister::setPatternEditOptions( SearchPatternEdit::SearchPatternEditOptions options )
{
    mOptions = options;
    foreach ( QWidget *w, widgets() ) {
        qobject_cast<SearchRuleWidget*>( w )->setPatternEditOptions( options );
    }
}

void SearchRuleWidgetLister::setRuleList( QList<SearchRule::Ptr> *aList )
{
  Q_ASSERT( aList );

  if ( mRuleList && mRuleList != aList ) {
    regenerateRuleListFromWidgets();
  }

  mRuleList = aList;

  if ( !widgets().isEmpty() ) { // move this below next 'if'?
    widgets().first()->blockSignals( true );
  }

  if ( aList->isEmpty() ) {
    slotClear();
    widgets().first()->blockSignals( false );
    return;
  }

  int superfluousItems = (int)mRuleList->count() - widgetsMaximum();
  if ( superfluousItems > 0 ) {
    kDebug() << "Clipping rule list to" << widgetsMaximum() << "items!";

    for ( ; superfluousItems ; superfluousItems-- ) {
      mRuleList->removeLast();
    }
  }

  // set the right number of widgets
  setNumberOfShownWidgetsTo( qMax( (int)mRuleList->count(), widgetsMinimum() ) );

  // load the actions into the widgets
  QList<QWidget*> widgetList = widgets();
  QList<SearchRule::Ptr>::const_iterator rIt;
  QList<SearchRule::Ptr>::const_iterator rItEnd( mRuleList->constEnd() );
  QList<QWidget*>::const_iterator wIt = widgetList.constBegin();
  QList<QWidget*>::const_iterator wItEnd = widgetList.constEnd();
  for ( rIt = mRuleList->constBegin();
        rIt != rItEnd && wIt != wItEnd; ++rIt, ++wIt ) {
    qobject_cast<SearchRuleWidget*>( *wIt )->setRule( (*rIt) );
  }
  for ( ; wIt != wItEnd; ++wIt ) {
    qobject_cast<SearchRuleWidget*>( *wIt )->reset();
  }

  Q_ASSERT( !widgets().isEmpty() );
  widgets().first()->blockSignals(false);
  updateAddRemoveButton();
}

void SearchRuleWidgetLister::slotAddWidget( QWidget *w )
{
  addWidgetAfterThisWidget( w );
  updateAddRemoveButton();
}

void SearchRuleWidgetLister::slotRemoveWidget( QWidget *w )
{
  removeWidget( w );
  updateAddRemoveButton();
}

void SearchRuleWidgetLister::reconnectWidget( SearchRuleWidget *w )
{
  connect( w, SIGNAL(addWidget(QWidget*)),
           this, SLOT(slotAddWidget(QWidget*)), Qt::UniqueConnection );
  connect( w, SIGNAL(removeWidget(QWidget*)),
           this, SLOT(slotRemoveWidget(QWidget*)), Qt::UniqueConnection );
}

void SearchRuleWidgetLister::updateAddRemoveButton()
{
  QList<QWidget*> widgetList = widgets();
  const int numberOfWidget( widgetList.count() );
  bool addButtonEnabled = false;
  bool removeButtonEnabled = false;
  if ( numberOfWidget <= widgetsMinimum() ) {
    addButtonEnabled = true;
    removeButtonEnabled = false;
  } else if ( numberOfWidget >= widgetsMaximum() ) {
    addButtonEnabled = false;
    removeButtonEnabled = true;
  } else {
    addButtonEnabled = true;
    removeButtonEnabled = true;
  }
  QList<QWidget*>::ConstIterator wIt = widgetList.constBegin();
  QList<QWidget*>::ConstIterator wEnd = widgetList.constEnd();
  for ( ; wIt != wEnd ;++wIt ) {
    SearchRuleWidget *w = qobject_cast<SearchRuleWidget*>( *wIt );
    w->updateAddRemoveButton( addButtonEnabled, removeButtonEnabled );
  }
}

void SearchRuleWidgetLister::reset()
{
  if ( mRuleList ) {
    regenerateRuleListFromWidgets();
  }

  mRuleList = 0;
  slotClear();
  updateAddRemoveButton();
}

QWidget *SearchRuleWidgetLister::createWidget( QWidget *parent )
{
  SearchRuleWidget *w =
    new SearchRuleWidget( parent, SearchRule::Ptr(), mOptions, mTypeMode );
  reconnectWidget( w );
  return w;
}

void SearchRuleWidgetLister::clearWidget( QWidget *aWidget )
{
  if ( aWidget ) {
    SearchRuleWidget *w = static_cast<SearchRuleWidget*>( aWidget );
    w->reset();
    reconnectWidget( w );
    updateAddRemoveButton();
  }
}

void SearchRuleWidgetLister::regenerateRuleListFromWidgets()
{
  if ( !mRuleList ) {
    return;
  }

  mRuleList->clear();

  foreach ( const QWidget *w, widgets() ) {
    SearchRule::Ptr r = qobject_cast<const SearchRuleWidget*>( w )->rule();
    if ( r && !r->isEmpty() ) {
      mRuleList->append( r );
    }
  }
  updateAddRemoveButton();
}

//=============================================================================
//
// class SearchPatternEdit
//
//=============================================================================

SearchPatternEdit::SearchPatternEdit( QWidget *parent, SearchPatternEditOptions options, SearchModeType modeType )
  : QWidget( parent ), mAllMessageRBtn( 0 )
{
  setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
  setObjectName( QLatin1String("SearchPatternEdit") );
  initLayout( options, modeType );
}

SearchPatternEdit::~SearchPatternEdit()
{
}

void SearchPatternEdit::updateSearchPattern()
{
  mRuleLister->regenerateRuleListFromWidgets();
}

void SearchPatternEdit::setPatternEditOptions( SearchPatternEdit::SearchPatternEditOptions options )
{
    mRuleLister->setPatternEditOptions(options);
}


void SearchPatternEdit::initLayout( SearchPatternEditOptions options, SearchModeType modeType )
{
  QVBoxLayout *layout = new QVBoxLayout( this );
  layout->setMargin( 0 );

  const bool matchAllMessages = ( options & MailCommon::SearchPatternEdit::MatchAllMessages );
  //------------the radio buttons
  mAllRBtn = new QRadioButton( i18n( "Match a&ll of the following" ), this );
  mAnyRBtn = new QRadioButton( i18n( "Match an&y of the following" ), this );
  if ( matchAllMessages ) {
    mAllMessageRBtn = new QRadioButton( i18n( "Match all messages" ), this );
  }

  mAllRBtn->setObjectName( QLatin1String("mAllRBtn") );
  mAllRBtn->setChecked( true );
  mAnyRBtn->setObjectName( QLatin1String("mAnyRBtn") );
  mAnyRBtn->setChecked( false );
  if ( matchAllMessages ) {
    mAllMessageRBtn->setObjectName( QLatin1String("mAllMessageRBtn") );
    mAllMessageRBtn->setChecked(false);
  }
  layout->addWidget( mAllRBtn );
  layout->addWidget( mAnyRBtn );
  if ( matchAllMessages ) {
    layout->addWidget( mAllMessageRBtn );
  }

  QButtonGroup *bg = new QButtonGroup( this );
  bg->addButton( mAllRBtn );
  bg->addButton( mAnyRBtn );
  if ( matchAllMessages ) {
    bg->addButton( mAllMessageRBtn );
  }

  //------------connect a few signals
  connect( bg, SIGNAL(buttonClicked(QAbstractButton*)),
           this, SLOT(slotRadioClicked(QAbstractButton*)) );

  //------------the list of SearchRuleWidget's
  mRuleLister =
    new SearchRuleWidgetLister(
      this, options, modeType);

  mRuleLister->slotClear();

  if ( !mRuleLister->widgets().isEmpty() ) {
    const int numberOfWidget( mRuleLister->widgets().count() );
    for ( int i = 0; i < numberOfWidget; ++i ) {
      SearchRuleWidget *srw = static_cast<SearchRuleWidget*>( mRuleLister->widgets().at(i) );
      connect( srw, SIGNAL(fieldChanged(QString)),
               this, SLOT(slotAutoNameHack()) );
      connect( srw, SIGNAL(contentsChanged(QString)),
               this, SLOT(slotAutoNameHack()) );
      connect( srw, SIGNAL(returnPressed()),
               this, SIGNAL(returnPressed()) );
    }
  } else {
    kDebug() << "No first SearchRuleWidget, though slotClear() has been called!";
  }

  connect( mRuleLister, SIGNAL(widgetAdded(QWidget*)),
           this, SLOT(slotRuleAdded(QWidget*)) );
  connect( mRuleLister, SIGNAL(widgetRemoved()), this, SIGNAL(patternChanged()) );
  connect( mRuleLister, SIGNAL(clearWidgets()), this, SIGNAL(patternChanged()));

  layout->addWidget( mRuleLister );
}

void SearchPatternEdit::setSearchPattern( SearchPattern *aPattern )
{
  Q_ASSERT( aPattern );

  mRuleLister->setRuleList( aPattern );

  mPattern = aPattern;

  blockSignals( true );
  if ( mPattern->op() == SearchPattern::OpOr ) {
    mAnyRBtn->setChecked( true );
  } else if ( mPattern->op() == SearchPattern::OpAnd ) {
    mAllRBtn->setChecked( true );
  } else if ( mAllMessageRBtn &&  ( mPattern->op() == SearchPattern::OpAll ) ) {
    mAllMessageRBtn->setChecked( true );
  }
  mRuleLister->setEnabled( mPattern->op() != SearchPattern::OpAll );
  blockSignals( false );

  setEnabled( true );
  emit patternChanged();
}

void SearchPatternEdit::reset()
{
  mRuleLister->reset();

  blockSignals(true);
  mAllRBtn->setChecked( true );
  blockSignals(false);

  setEnabled( false );
  emit patternChanged();
}

void SearchPatternEdit::slotRadioClicked( QAbstractButton *aRBtn )
{
  if ( mPattern ) {
    if ( aRBtn == mAllRBtn ) {
      mPattern->setOp( SearchPattern::OpAnd );
    } else if ( aRBtn == mAnyRBtn ) {
      mPattern->setOp( SearchPattern::OpOr );
    } else if ( aRBtn == mAllMessageRBtn ) {
      mPattern->setOp( SearchPattern::OpAll );
    }
    mRuleLister->setEnabled( mPattern->op() != SearchPattern::OpAll );
    emit patternChanged();
  }
}

void SearchPatternEdit::slotAutoNameHack()
{
  mRuleLister->regenerateRuleListFromWidgets();
  emit maybeNameChanged();
  emit patternChanged();
}

void SearchPatternEdit::slotRuleAdded( QWidget *newRuleWidget )
{
  SearchRuleWidget *srw = static_cast<SearchRuleWidget*>( newRuleWidget );
  connect( srw, SIGNAL(fieldChanged(QString)), this, SLOT(slotAutoNameHack()) );
  connect( srw, SIGNAL(contentsChanged(QString)), this, SLOT(slotAutoNameHack()) );
  connect( srw, SIGNAL(returnPressed()), this, SIGNAL(returnPressed()) );
  emit patternChanged();
}
#include "searchpatternedit.moc"
