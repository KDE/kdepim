/* -*- mode: C++; c-file-style: "gnu" -*-
  kmsearchpatternedit.cpp
  Author: Marc Mutz <Marc@Mutz.com>

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

#include <QStackedWidget>
#include "rulewidgethandlermanager.h"
using MailCommon::RuleWidgetHandlerManager;
#include "minimumcombobox.h"

#include <kcombobox.h>
#include <klocale.h>
#include <kdialog.h>
#include <kdebug.h>

#include <QButtonGroup>
#include <QByteArray>
#include <QHBoxLayout>
#include <QRadioButton>

#include <assert.h>
#include <QResizeEvent>

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

  QString getLocalizedDisplayName() const { return i18nc(context, displayName); };

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
  { "Organization",  I18N_NOOP( "Organization" )  }
};
static const int SpecialRuleFieldsCount =
  sizeof( SpecialRuleFields ) / sizeof( *SpecialRuleFields );

//=============================================================================
//
// class SearchRuleWidget
//
//=============================================================================

SearchRuleWidget::SearchRuleWidget( QWidget *parent, SearchRule::Ptr aRule,
                                        bool headersOnly,
                                        bool absoluteDates )
  : QWidget( parent ),
    mRuleField( 0 ),
    mFunctionStack( 0 ),
    mValueStack( 0 ),
    mAbsoluteDates( absoluteDates )
{
  initFieldList( headersOnly, absoluteDates );
  initWidget();

  if ( aRule )
    setRule( aRule );
  else
    reset();
}

void SearchRuleWidget::setHeadersOnly( bool headersOnly )
{
  SearchRule::Ptr srule = rule();
  QByteArray currentText = srule->field();

  initFieldList( headersOnly, mAbsoluteDates );

  mRuleField->clear();
  mRuleField->addItems( mFilterFieldList );
  mRuleField->setMaxCount( mRuleField->count() );
  mRuleField->adjustSize();

  if (( currentText != "<message>") &&
      ( currentText != "<body>"))
    mRuleField->setItemText( 0, QString::fromAscii( currentText ) );
  else
    mRuleField->setItemText( 0, QString() );
}

void SearchRuleWidget::initWidget()
{
  QHBoxLayout * hlay = new QHBoxLayout( this );
  hlay->setSpacing( KDialog::spacingHint() );
  hlay->setMargin( 0 );

  // initialize the header field combo box
  mRuleField = new MinimumComboBox( this );
  mRuleField->setObjectName( "mRuleField" );
  mRuleField->setEditable( true );
  mRuleField->addItems( mFilterFieldList );
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

  RuleWidgetHandlerManager::instance()->createWidgets( mFunctionStack,
                                                       mValueStack,
                                                       this );

  // redirect focus to the header field combo box
  setFocusProxy( mRuleField );

  connect( mRuleField, SIGNAL(activated(QString)),
           this, SLOT(slotRuleFieldChanged(QString)) );
  connect( mRuleField, SIGNAL(editTextChanged(QString)),
           this, SLOT(slotRuleFieldChanged(QString)) );
  connect( mRuleField, SIGNAL(editTextChanged(QString)),
           this, SIGNAL(fieldChanged(QString)) );
}


void SearchRuleWidget::setRule( SearchRule::Ptr aRule )
{
  assert ( aRule );

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

  RuleWidgetHandlerManager::instance()->setRule( mFunctionStack, mValueStack,
                                                 aRule );
}

SearchRule::Ptr SearchRuleWidget::rule() const {
  const QByteArray ruleField = ruleFieldToEnglish( mRuleField->currentText() );
  const SearchRule::Function function =
    RuleWidgetHandlerManager::instance()->function( ruleField,
                                                    mFunctionStack );
  const QString value =
    RuleWidgetHandlerManager::instance()->value( ruleField, mFunctionStack,
                                                 mValueStack );

  return SearchRule::createInstance( ruleField, function, value );
}

void SearchRuleWidget::reset()
{
  mRuleField->blockSignals( true );
  mRuleField->setItemText( 0, "" );
  mRuleField->setCurrentIndex( 0 );
  mRuleField->blockSignals( false );

  RuleWidgetHandlerManager::instance()->reset( mFunctionStack, mValueStack );
}

void SearchRuleWidget::slotFunctionChanged()
{
  const QByteArray ruleField = ruleFieldToEnglish( mRuleField->currentText() );
  RuleWidgetHandlerManager::instance()->update( ruleField,
                                                mFunctionStack,
                                                mValueStack );
  const QString prettyValue = RuleWidgetHandlerManager::instance()->prettyValue( ruleField,
                                                                                 mFunctionStack,
                                                                                 mValueStack );
  emit contentsChanged( prettyValue );
}

void SearchRuleWidget::slotValueChanged()
{
  const QByteArray ruleField = ruleFieldToEnglish( mRuleField->currentText() );
  const QString prettyValue =
    RuleWidgetHandlerManager::instance()->prettyValue( ruleField,
                                                       mFunctionStack,
                                                       mValueStack );
  emit contentsChanged( prettyValue );
}

QByteArray SearchRuleWidget::ruleFieldToEnglish( const QString & i18nVal )
{
  for ( int i = 0; i < SpecialRuleFieldsCount; ++i ) {
    if ( i18nVal == SpecialRuleFields[i].getLocalizedDisplayName() )
      return SpecialRuleFields[i].internalName;
  }
  return i18nVal.toLatin1();
}

int SearchRuleWidget::ruleFieldToId( const QString & i18nVal )
{
  for ( int i = 0; i < SpecialRuleFieldsCount; ++i ) {
    if ( i18nVal == SpecialRuleFields[i].getLocalizedDisplayName() )
      return i;
  }
  return -1; // no pseudo header
}

static QString displayNameFromInternalName( const QString & internal )
{
  for ( int i = 0; i < SpecialRuleFieldsCount; ++i ) {
    if ( internal == SpecialRuleFields[i].internalName )
      return SpecialRuleFields[i].getLocalizedDisplayName();
  }
  return internal.toLatin1();
}



int SearchRuleWidget::indexOfRuleField( const QByteArray & aName ) const
{
  if ( aName.isEmpty() )
    return -1;

  const QString i18n_aName = displayNameFromInternalName( aName );
  const int nbRuleField = mRuleField->count();
  for ( int i = 1; i < nbRuleField; ++i ) {
    if ( mRuleField->itemText( i ) == i18n_aName )
      return i;
  }

  return -1;
}

void SearchRuleWidget::initFieldList( bool headersOnly, bool absoluteDates )
{
  mFilterFieldList.clear();
  mFilterFieldList.append(""); // empty entry for user input
  if( !headersOnly ) {
    mFilterFieldList.append( SpecialRuleFields[Message].getLocalizedDisplayName() );
    mFilterFieldList.append( SpecialRuleFields[Body].getLocalizedDisplayName()    );
  }
  mFilterFieldList.append( SpecialRuleFields[AnyHeader].getLocalizedDisplayName()  );
  mFilterFieldList.append( SpecialRuleFields[Recipients].getLocalizedDisplayName() );
  mFilterFieldList.append( SpecialRuleFields[Size].getLocalizedDisplayName()       );
  if ( !absoluteDates )
    mFilterFieldList.append( SpecialRuleFields[AgeInDays].getLocalizedDisplayName() );
  mFilterFieldList.append( SpecialRuleFields[Subject].getLocalizedDisplayName() );
  mFilterFieldList.append( SpecialRuleFields[From].getLocalizedDisplayName()    );
  mFilterFieldList.append( SpecialRuleFields[To].getLocalizedDisplayName()      );
  mFilterFieldList.append( SpecialRuleFields[CC].getLocalizedDisplayName()      );
  mFilterFieldList.append( SpecialRuleFields[Status].getLocalizedDisplayName()  );
#ifndef KDEPIM_NO_NEPOMUK  
  mFilterFieldList.append( SpecialRuleFields[Tag].getLocalizedDisplayName()     );
#endif  
  mFilterFieldList.append( i18n( SpecialRuleFields[ReplyTo].displayName ) );
  mFilterFieldList.append( i18n( SpecialRuleFields[Organization].displayName ) );

  // these others only represent message headers and you can add to
  // them as you like
  mFilterFieldList.append("List-Id");
  mFilterFieldList.append("Resent-From");
  mFilterFieldList.append("X-Loop");
  mFilterFieldList.append("X-Mailing-List");
  mFilterFieldList.append("X-Spam-Flag");
}

void SearchRuleWidget::slotRuleFieldChanged( const QString & field )
{
  RuleWidgetHandlerManager::instance()->update( ruleFieldToEnglish( field ),
                                                mFunctionStack,
                                                mValueStack );
}

//=============================================================================
//
// class KMFilterActionWidgetLister (the filter action editor)
//
//=============================================================================

SearchRuleWidgetLister::SearchRuleWidgetLister( QWidget *parent, const char*, bool headersOnly, bool absoluteDates )
  : KWidgetLister( 2, FILTER_MAX_RULES, parent )
{
  mRuleList = 0;
  mHeadersOnly = headersOnly;
  mAbsoluteDates = absoluteDates;
}

SearchRuleWidgetLister::~SearchRuleWidgetLister()
{
}

void SearchRuleWidgetLister::setRuleList( QList<SearchRule::Ptr> *aList )
{
  assert ( aList );

  if ( mRuleList && mRuleList != aList )
    regenerateRuleListFromWidgets();

  mRuleList = aList;

  if ( !widgets().isEmpty() ) // move this below next 'if'?
    widgets().first()->blockSignals(true);

  if ( aList->isEmpty() ) {
    slotClear();
    widgets().first()->blockSignals(false);
    return;
  }

  int superfluousItems = (int)mRuleList->count() - widgetsMaximum();
  if ( superfluousItems > 0 ) {
    kDebug() << "Clipping rule list to" << widgetsMaximum() << "items!";

    for ( ; superfluousItems ; superfluousItems-- )
      mRuleList->removeLast();
  }

  // HACK to workaround regression in Qt 3.1.3 and Qt 3.2.0 (fixes bug #63537)
  setNumberOfShownWidgetsTo( qMax((int)mRuleList->count(), widgetsMinimum())+1 );
  // set the right number of widgets
  setNumberOfShownWidgetsTo( qMax((int)mRuleList->count(), widgetsMinimum()) );

  // load the actions into the widgets
  QList<QWidget*> widgetList = widgets();
  QList<SearchRule::Ptr>::const_iterator rIt;
  QList<QWidget*>::const_iterator wIt = widgetList.constBegin();
  for ( rIt = mRuleList->constBegin();
        rIt != mRuleList->constEnd() && wIt != widgetList.constEnd(); ++rIt, ++wIt ) {
    qobject_cast<SearchRuleWidget*>( *wIt )->setRule( (*rIt) );
  }
  for ( ; wIt != widgetList.constEnd() ; ++wIt )
    qobject_cast<SearchRuleWidget*>( *wIt )->reset();

  assert( !widgets().isEmpty() );
  widgets().first()->blockSignals(false);
}

void SearchRuleWidgetLister::setHeadersOnly( bool headersOnly )
{
  foreach ( QWidget *w, widgets() ) {
    qobject_cast<SearchRuleWidget*>( w )->setHeadersOnly( headersOnly );
  }
}

void SearchRuleWidgetLister::reset()
{
  if ( mRuleList )
    regenerateRuleListFromWidgets();

  mRuleList = 0;
  slotClear();
}

QWidget* SearchRuleWidgetLister::createWidget( QWidget *parent )
{
  return new SearchRuleWidget(parent, SearchRule::Ptr(),  mHeadersOnly, mAbsoluteDates);
}

void SearchRuleWidgetLister::clearWidget( QWidget *aWidget )
{
  if ( aWidget )
    ((SearchRuleWidget*)aWidget)->reset();
}

void SearchRuleWidgetLister::regenerateRuleListFromWidgets()
{
  if ( !mRuleList ) return;

  mRuleList->clear();

  foreach ( const QWidget *w, widgets() ) {
    SearchRule::Ptr r = qobject_cast<const SearchRuleWidget*>( w )->rule();
    if ( r && !r->isEmpty() )
      mRuleList->append( r );
  }
}




//=============================================================================
//
// class SearchPatternEdit
//
//=============================================================================

SearchPatternEdit::SearchPatternEdit( QWidget *parent, bool headersOnly,
                                          bool absoluteDates )
  : QWidget( parent )
{
  setObjectName( "SearchPatternEdit" );
  initLayout( headersOnly, absoluteDates );
}


SearchPatternEdit::~SearchPatternEdit()
{
}

void SearchPatternEdit::initLayout(bool headersOnly, bool absoluteDates)
{
  QVBoxLayout *layout = new QVBoxLayout( this );

  //------------the radio buttons
  mAllRBtn = new QRadioButton( i18n("Match a&ll of the following"), this );
  mAnyRBtn = new QRadioButton( i18n("Match an&y of the following"), this );

  mAllRBtn->setObjectName( "mAllRBtn" );
  mAllRBtn->setChecked(true);
  mAnyRBtn->setObjectName( "mAnyRBtn" );
  mAnyRBtn->setChecked(false);

  layout->addWidget( mAllRBtn );
  layout->addWidget( mAnyRBtn );

  QButtonGroup *bg = new QButtonGroup( this );
  bg->addButton( mAllRBtn );
  bg->addButton( mAnyRBtn );

  //------------connect a few signals
  connect( bg, SIGNAL(buttonClicked(QAbstractButton*)),
	   this, SLOT(slotRadioClicked(QAbstractButton*)) );

  //------------the list of SearchRuleWidget's
  mRuleLister = new SearchRuleWidgetLister( this, "swl", headersOnly, absoluteDates );
  mRuleLister->slotClear();

  if ( !mRuleLister->widgets().isEmpty() ) {
    for (int i = 0; i < mRuleLister->widgets().count(); i++) {
      SearchRuleWidget *srw = static_cast<SearchRuleWidget*>( mRuleLister->widgets().at(i) );
      connect( srw, SIGNAL(fieldChanged(QString)),
               this, SLOT(slotAutoNameHack()) );
      connect( srw, SIGNAL(contentsChanged(QString)),
               this, SLOT(slotAutoNameHack()) );
    }
  } else
    kDebug() << "No first SearchRuleWidget, though slotClear() has been called!";

  connect( mRuleLister, SIGNAL(widgetAdded(QWidget*)),
           this, SLOT(slotRuleAdded(QWidget*)) );
  connect( mRuleLister, SIGNAL(widgetRemoved()), this, SIGNAL(patternChanged()) );
  
  layout->addWidget( mRuleLister );
}

void SearchPatternEdit::setSearchPattern( SearchPattern *aPattern )
{
  assert( aPattern );

  mRuleLister->setRuleList( aPattern );

  mPattern = aPattern;

  blockSignals(true);
  if ( mPattern->op() == SearchPattern::OpOr )
    mAnyRBtn->setChecked(true);
  else
    mAllRBtn->setChecked(true);
  blockSignals(false);

  setEnabled( true );
  emit patternChanged();
}

void SearchPatternEdit::setHeadersOnly( bool headersOnly )
{
  mRuleLister->setHeadersOnly( headersOnly );
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

void SearchPatternEdit::slotRadioClicked(QAbstractButton *aRBtn)
{
  if ( mPattern ) {
    if ( aRBtn == mAllRBtn )
      mPattern->setOp( SearchPattern::OpAnd );
    else
      mPattern->setOp( SearchPattern::OpOr );

    emit patternChanged();
  }
}

void SearchPatternEdit::slotAutoNameHack()
{
  mRuleLister->regenerateRuleListFromWidgets();
  emit maybeNameChanged();
  emit patternChanged();
}

void SearchPatternEdit::slotRuleAdded(QWidget* newRuleWidget)
{
  SearchRuleWidget *srw = static_cast<SearchRuleWidget*>( newRuleWidget );
  connect( srw, SIGNAL(fieldChanged(QString)), this, SLOT(slotAutoNameHack()) );
  connect( srw, SIGNAL(contentsChanged(QString)), this, SLOT(slotAutoNameHack()) );
  emit patternChanged();
}
#include "searchpatternedit.moc"
