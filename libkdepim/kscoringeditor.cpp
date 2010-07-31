/*
    kscoringeditor.cpp

    Copyright (c) 2001 Mathias Waack
    Copyright (C) 2005 by Volker Krause <volker.krause@rwth-aachen.de>

    Author: Mathias Waack <mathias@atoll-net.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#undef QT_NO_COMPAT

#include "kscoring.h"
#include "kscoringeditor.h"

#include <kdebug.h>
#include <klocale.h>
#include <kcombobox.h>
#include <kcolorcombo.h>
#include <kiconloader.h>
#include <kregexpeditorinterface.h>
#include <ktrader.h>
#include <kparts/componentfactory.h>


#include <tqlabel.h>
#include <tqpushbutton.h>
#include <tqlayout.h>
#include <tqtooltip.h>
#include <tqcheckbox.h>
#include <tqbuttongroup.h>
#include <tqradiobutton.h>
#include <tqwidgetstack.h>
#include <tqapplication.h>
#include <tqtimer.h>
#include <tqhbox.h>

// works for both ListBox and ComboBox
template <class T> static int setCurrentItem(T *box, const TQString& s)
{
  int cnt = box->count();
  for (int i=0;i<cnt;++i) {
    if (box->text(i) == s) {
      box->setCurrentItem(i);
      return i;
    }
  }
  return -1;
}


//============================================================================
//
// class SingleConditionWidget (editor for one condition, used in ConditionEditWidget)
//
//============================================================================
SingleConditionWidget::SingleConditionWidget(KScoringManager *m,TQWidget *p, const char *n)
  : TQFrame(p,n), manager(m)
{
  TQBoxLayout *topL = new TQVBoxLayout(this,5);
  TQBoxLayout *firstRow = new TQHBoxLayout(topL);
  neg = new TQCheckBox(i18n("Not"),this);
  TQToolTip::add(neg,i18n("Negate this condition"));
  firstRow->addWidget(neg);
  headers = new KComboBox(this);
  headers->insertStringList(manager->getDefaultHeaders());
  headers->setEditable( true );
  TQToolTip::add(headers,i18n("Select the header to match this condition against"));
  firstRow->addWidget(headers,1);
  matches = new KComboBox(this);
  matches->insertStringList(KScoringExpression::conditionNames());
  TQToolTip::add(matches,i18n("Select the type of match"));
  firstRow->addWidget(matches,1);
  connect( matches, TQT_SIGNAL( activated( int ) ), TQT_SLOT( toggleRegExpButton( int ) ) );
  TQHBoxLayout *secondRow = new TQHBoxLayout( topL );
  secondRow->setSpacing( 1 );
  expr = new KLineEdit( this );
  TQToolTip::add(expr,i18n("The condition for the match"));
  // reserve space for at least 20 characters
  expr->setMinimumWidth(fontMetrics().maxWidth()*20);
  secondRow->addWidget( expr );
  regExpButton = new TQPushButton( i18n("Edit..."), this );
  secondRow->addWidget( regExpButton );
  connect( regExpButton, TQT_SIGNAL( clicked() ), TQT_SLOT( showRegExpDialog() ) );

  // occupy at much width as possible
  setSizePolicy(TQSizePolicy(TQSizePolicy::Expanding,TQSizePolicy::Fixed));
  setFrameStyle(Box | Sunken);
  setLineWidth(1);
}

SingleConditionWidget::~SingleConditionWidget()
{}

void SingleConditionWidget::setCondition(KScoringExpression *e)
{
  neg->setChecked(e->isNeg());
  headers->setCurrentText( e->getHeader() );
  setCurrentItem(matches,KScoringExpression::getNameForCondition(e->getCondition()));
  toggleRegExpButton( matches->currentItem() );
  expr->setText(e->getExpression());
}

KScoringExpression* SingleConditionWidget::createCondition() const
{
  TQString head = headers->currentText();
  TQString match = matches->currentText();
  int condType = KScoringExpression::getConditionForName(match);
  match = KScoringExpression::getTypeString(condType);
  TQString cond = expr->text();
  TQString negs = (neg->isChecked())?"1":"0";
  return new KScoringExpression(head,match,cond,negs);
}

void SingleConditionWidget::clear()
{
  neg->setChecked(false);
  expr->clear();
}

void SingleConditionWidget::toggleRegExpButton( int selected )
{
  bool isRegExp = (KScoringExpression::MATCH == selected ||
      KScoringExpression::MATCHCS == selected) &&
      !KTrader::self()->query("KRegExpEditor/KRegExpEditor").isEmpty();
  regExpButton->setEnabled( isRegExp );
}

void SingleConditionWidget::showRegExpDialog()
{
  TQDialog *editorDialog = KParts::ComponentFactory::createInstanceFromQuery<TQDialog>( "KRegExpEditor/KRegExpEditor" );
  if ( editorDialog ) {
    KRegExpEditorInterface *editor = static_cast<KRegExpEditorInterface *>( editorDialog->qt_cast( "KRegExpEditorInterface" ) );
    Q_ASSERT( editor ); // This should not fail!
    editor->setRegExp( expr->text() );
    editorDialog->exec();
    expr->setText( editor->regExp() );
  }
}

//============================================================================
//
// class ConditionEditWidget (the widget to edit the conditions of a rule)
//
//============================================================================
ConditionEditWidget::ConditionEditWidget(KScoringManager *m, TQWidget *p, const char *n)
  : KWidgetLister(1,8,p,n), manager(m)
{
  // create one initial widget
  addWidgetAtEnd();
}

ConditionEditWidget::~ConditionEditWidget()
{}

TQWidget* ConditionEditWidget::createWidget(TQWidget *parent)
{
  return new SingleConditionWidget(manager,parent);
}

void ConditionEditWidget::clearWidget(TQWidget *w)
{
  Q_ASSERT( w->isA("SingleConditionWidget") );
  SingleConditionWidget *sw = dynamic_cast<SingleConditionWidget*>(w);
  if (sw)
    sw->clear();
}

void ConditionEditWidget::slotEditRule(KScoringRule *rule)
{
  KScoringRule::ScoreExprList l;
  if (rule) l = rule->getExpressions();
  if (!rule || l.count() == 0) {
    slotClear();
  } else {
    setNumberOfShownWidgetsTo(l.count());
    KScoringExpression *e = l.first();
    SingleConditionWidget *scw = static_cast<SingleConditionWidget*>(mWidgetList.first());
    while (e && scw) {
      scw->setCondition(e);
      e = l.next();
      scw = static_cast<SingleConditionWidget*>(mWidgetList.next());
    }
  }
}

void ConditionEditWidget::updateRule(KScoringRule *rule)
{
  rule->cleanExpressions();
  for(TQWidget *w = mWidgetList.first(); w; w = mWidgetList.next()) {
    if (! w->isA("SingleConditionWidget")) {
      kdWarning(5100) << "there is a widget in ConditionEditWidget "
                      << "which isn't a SingleConditionWidget" << endl;
    } else {
      SingleConditionWidget *saw = dynamic_cast<SingleConditionWidget*>(w);
	  if (saw)
	    rule->addExpression(saw->createCondition());
    }
  }
}

//============================================================================
//
// class SingleActionWidget (editor for one action, used in ActionEditWidget)
//
//============================================================================
SingleActionWidget::SingleActionWidget(KScoringManager *m,TQWidget *p, const char *n)
  : TQWidget(p,n), notifyEditor(0), scoreEditor(0), colorEditor(0),manager(m)
{
  TQHBoxLayout *topL = new TQHBoxLayout(this,0,5);
  types = new KComboBox(this);
  types->setEditable(false);
  topL->addWidget(types);
  stack = new TQWidgetStack(this);
  topL->addWidget(stack);

  dummyLabel = new TQLabel(i18n("Select an action."), stack);
  stack->addWidget(dummyLabel, 0);

  // init widget stack and the types combo box
  int index = 1;
  types->insertItem(TQString::null);
  TQStringList l = ActionBase::userNames();
  for ( TQStringList::Iterator it = l.begin(); it != l.end(); ++it ) {
    TQString name = *it;
    int feature = ActionBase::getTypeForUserName(name);
    if (manager->hasFeature(feature)) {
      types->insertItem(name);
      TQWidget *w=0;
      switch (feature) {
        case ActionBase::SETSCORE:
          w = scoreEditor = new KIntSpinBox(-99999,99999,1,0,10, stack);
          break;
        case ActionBase::NOTIFY:
          w = notifyEditor = new KLineEdit(stack);
          break;
        case ActionBase::COLOR:
          w = colorEditor = new KColorCombo(stack);
          break;
        case ActionBase::MARKASREAD:
          w = new TQLabel( stack ); // empty dummy
          break;
      }
      if ( w )
        stack->addWidget(w,index++);
    }
  }

  connect(types,TQT_SIGNAL(activated(int)),stack,TQT_SLOT(raiseWidget(int)));

  // raise the dummy label
  types->setCurrentItem(0);
  stack->raiseWidget(dummyLabel);
}

SingleActionWidget::~SingleActionWidget()
{
}

void SingleActionWidget::setAction(ActionBase *act)
{
  kdDebug(5100) << "SingleActionWidget::setAction()" << endl;
  setCurrentItem(types,ActionBase::userName(act->getType()));
  int index = types->currentItem();
  stack->raiseWidget(index);
  switch (act->getType()) {
    case ActionBase::SETSCORE:
      scoreEditor->setValue(act->getValueString().toInt());
      break;
    case ActionBase::NOTIFY:
      notifyEditor->setText(act->getValueString());
      break;
    case ActionBase::COLOR:
      colorEditor->setColor(TQColor(act->getValueString()));
      break;
    case ActionBase::MARKASREAD:
      // nothing
      break;
    default:
      kdWarning(5100) << "unknown action type in SingleActionWidget::setAction()" << endl;
  }
}

ActionBase* SingleActionWidget::createAction() const
{
  // no action selected...
  if (types->currentText().isEmpty())
    return 0;

  int type = ActionBase::getTypeForUserName(types->currentText());
  switch (type) {
    case ActionBase::SETSCORE:
      return new ActionSetScore(scoreEditor->value());
    case ActionBase::NOTIFY:
      return new ActionNotify(notifyEditor->text());
    case ActionBase::COLOR:
      return new ActionColor(colorEditor->color().name());
    case ActionBase::MARKASREAD:
      return new ActionMarkAsRead();
    default:
      kdWarning(5100) << "unknown action type in SingleActionWidget::getValue()" << endl;
      return 0;
  }
}

void SingleActionWidget::clear()
{
  if (scoreEditor) scoreEditor->setValue(0);
  if (notifyEditor) notifyEditor->clear();
  if (colorEditor) colorEditor->setCurrentItem(0);
  types->setCurrentItem(0);
  stack->raiseWidget(dummyLabel);
}

//============================================================================
//
// class ActionEditWidget (the widget to edit the actions of a rule)
//
//============================================================================
ActionEditWidget::ActionEditWidget(KScoringManager *m,TQWidget *p, const char *n)
  : KWidgetLister(1,8,p,n), manager(m)
{
  // create one initial widget
  addWidgetAtEnd();
}

ActionEditWidget::~ActionEditWidget()
{}

TQWidget* ActionEditWidget::createWidget( TQWidget *parent )
{
  return new SingleActionWidget(manager,parent);
}

void ActionEditWidget::slotEditRule(KScoringRule *rule)
{
  KScoringRule::ActionList l;
  if (rule) l = rule->getActions();
  if (!rule || l.count() == 0) {
    slotClear();
  } else {
    setNumberOfShownWidgetsTo(l.count());
    ActionBase *act = l.first();
    SingleActionWidget *saw = static_cast<SingleActionWidget*>(mWidgetList.first());
    while (act && saw) {
      saw->setAction(act);
      act = l.next();
      saw = static_cast<SingleActionWidget*>(mWidgetList.next());
    }
  }
}

void ActionEditWidget::updateRule(KScoringRule *rule)
{
  rule->cleanActions();
  for(TQWidget *w = mWidgetList.first(); w; w = mWidgetList.next()) {
    if (! w->isA("SingleActionWidget")) {
      kdWarning(5100) << "there is a widget in ActionEditWidget "
                      << "which isn't a SingleActionWidget" << endl;
    } else {
      SingleActionWidget *saw = dynamic_cast<SingleActionWidget*>(w);
	  if (saw)
	  {
	  	ActionBase *act = saw->createAction();
        if (act)
          rule->addAction(act);
      }
    }
  }
}

void ActionEditWidget::clearWidget(TQWidget *w)
{
  Q_ASSERT( w->isA("SingleActionWidget") );
  SingleActionWidget *sw = dynamic_cast<SingleActionWidget*>(w);
  if (sw)
    sw->clear();
}

//============================================================================
//
// class RuleEditWidget (the widget to edit one rule)
//
//============================================================================
RuleEditWidget::RuleEditWidget(KScoringManager *m,TQWidget *p, const char *n)
  : TQWidget(p,n), dirty(false), manager(m), oldRuleName(TQString::null)
{
  kdDebug(5100) << "RuleEditWidget::RuleEditWidget()" << endl;
  if ( !n ) setName( "RuleEditWidget" );
  TQVBoxLayout *topLayout = new TQVBoxLayout( this, 5, KDialog::spacingHint() );

  //------------- Name, Servers, Groups ---------------------
  TQGroupBox *groupB = new TQGroupBox(i18n("Properties"),this);
  topLayout->addWidget(groupB);
  TQGridLayout* groupL = new TQGridLayout(groupB, 6,2, 8,5);
  groupL->addRowSpacing(0, fontMetrics().lineSpacing()-4);

  // name
  ruleNameEdit = new KLineEdit( groupB, "ruleNameEdit" );
  groupL->addWidget( ruleNameEdit, 1, 1 );
  TQLabel *ruleNameLabel = new TQLabel(ruleNameEdit, i18n("&Name:"), groupB, "ruleNameLabel");
  groupL->addWidget( ruleNameLabel, 1, 0 );

  // groups
  groupsEdit = new KLineEdit( groupB, "groupsEdit" );
  groupL->addWidget( groupsEdit, 2, 1 );
  TQLabel *groupsLabel = new TQLabel(groupsEdit, i18n("&Groups:"), groupB, "groupsLabel");
  groupL->addWidget( groupsLabel, 2, 0 );

  TQPushButton *groupsBtn = new TQPushButton(i18n("A&dd Group"), groupB);
  connect(groupsBtn,TQT_SIGNAL(clicked()),TQT_SLOT(slotAddGroup()));
  groupL->addWidget( groupsBtn, 3, 0 );

  groupsBox = new KComboBox( false, groupB, "groupsBox" );
  groupsBox->setDuplicatesEnabled(false);
  groupsBox->insertStringList(manager->getGroups());
  groupsBox->setSizePolicy(TQSizePolicy(TQSizePolicy::Expanding, TQSizePolicy::Fixed));
  groupL->addWidget( groupsBox, 3, 1 );

  // expires
  expireCheck = new TQCheckBox(i18n("&Expire rule automatically"), groupB);
  groupL->addMultiCellWidget( expireCheck, 4,4, 0,1 );
  expireEdit = new KIntSpinBox(1,99999,1,30,10, groupB, "expireWidget");
  //Init suffix
  slotExpireEditChanged(30);
  connect(expireEdit, TQT_SIGNAL(valueChanged(int)), TQT_SLOT(slotExpireEditChanged(int)));
  groupL->addWidget( expireEdit, 5, 1 );
  expireLabel = new TQLabel(expireEdit, i18n("&Rule is valid for:"), groupB, "expireLabel");
  groupL->addWidget( expireLabel, 5, 0 );
  expireLabel->setEnabled(false);
  expireEdit->setEnabled(false);

  connect(expireCheck, TQT_SIGNAL(toggled(bool)), expireLabel, TQT_SLOT(setEnabled(bool)));
  connect(expireCheck, TQT_SIGNAL(toggled(bool)), expireEdit, TQT_SLOT(setEnabled(bool)));

  //------------- Conditions ---------------------
  TQGroupBox *groupConds = new TQGroupBox(i18n("Conditions"), this);
  topLayout->addWidget(groupConds);
  TQGridLayout *condL = new TQGridLayout(groupConds, 3,2, 8,5);

  condL->addRowSpacing(0, fontMetrics().lineSpacing()-4);

  TQButtonGroup *buttonGroup = new TQButtonGroup(groupConds);
  buttonGroup->hide();
  linkModeAnd = new TQRadioButton(i18n("Match a&ll conditions"), groupConds);
  buttonGroup->insert(linkModeAnd);
  condL->addWidget(linkModeAnd, 1,0);
  linkModeOr = new TQRadioButton(i18n("Matc&h any condition"), groupConds);
  buttonGroup->insert(linkModeOr);
  condL->addWidget(linkModeOr, 1,1);
  linkModeAnd->setChecked(true);

  condEditor = new ConditionEditWidget(manager,groupConds);
  condL->addMultiCellWidget(condEditor, 2,2, 0,1);
  connect(condEditor,TQT_SIGNAL(widgetRemoved()),this,TQT_SLOT(slotShrink()));

  //------------- Actions ---------------------
  TQGroupBox *groupActions = new TQGroupBox(i18n("Actions"), this);
  topLayout->addWidget(groupActions);
  TQBoxLayout *actionL = new TQVBoxLayout(groupActions,8,5);
  actionL->addSpacing(fontMetrics().lineSpacing()-4);
  actionEditor = new ActionEditWidget(manager,groupActions);
  actionL->addWidget(actionEditor);
  connect(actionEditor,TQT_SIGNAL(widgetRemoved()),this,TQT_SLOT(slotShrink()));

  topLayout->addStretch(1);

  kdDebug(5100) << "constructed RuleEditWidget" << endl;
}

RuleEditWidget::~RuleEditWidget()
{
}

void RuleEditWidget::slotEditRule(const TQString& ruleName)
{
  kdDebug(5100) << "RuleEditWidget::slotEditRule(" << ruleName << ")" << endl;
//   // first update the old rule if there is one
//   kdDebug(5100) << "let see if we have a rule with name " << oldRuleName << endl;
//   KScoringRule *rule;
//   if (!oldRuleName.isNull() && oldRuleName != ruleName) {
//     rule = manager->findRule(oldRuleName);
//     if (rule) {
//       kdDebug(5100) << "updating rule " << rule->getName() << endl;
//       updateRule(rule);
//     }
//   }

  KScoringRule* rule = manager->findRule(ruleName);
  if (!rule) {
    kdDebug(5100) << "no rule for ruleName " << ruleName << endl;
    clearContents();
    return;
  }
  oldRuleName = rule->getName();
  ruleNameEdit->setText(rule->getName());
  groupsEdit->setText(rule->getGroups().join(";"));

  bool b = rule->getExpireDate().isValid();
  expireCheck->setChecked(b);
  expireEdit->setEnabled(b);
  expireLabel->setEnabled(b);
  if (b)
    expireEdit->setValue(TQDate::currentDate().daysTo(rule->getExpireDate()));
  else
    expireEdit->setValue(30);
  if (rule->getLinkMode() == KScoringRule::AND) {
    linkModeAnd->setChecked(true);
  }
  else {
    linkModeOr->setChecked(true);
  }

  condEditor->slotEditRule(rule);
  actionEditor->slotEditRule(rule);

  kdDebug(5100) << "RuleEditWidget::slotEditRule() ready" << endl;
}

void RuleEditWidget::clearContents()
{
  ruleNameEdit->setText("");
  groupsEdit->setText("");
  expireCheck->setChecked(false);
  expireEdit->setValue(30);
  expireEdit->setEnabled(false);
  condEditor->slotEditRule(0);
  actionEditor->slotEditRule(0);
  oldRuleName = TQString::null;
}

void RuleEditWidget::updateRule(KScoringRule *rule)
{
  oldRuleName = TQString::null;
  TQString groups = groupsEdit->text();
  if (groups.isEmpty())
    rule->setGroups(TQStringList(".*"));
  else
    rule->setGroups(TQStringList::split(";",groups));
  bool b = expireCheck->isChecked();
  if (b)
    rule->setExpireDate(TQDate::currentDate().addDays(expireEdit->value()));
  else
    rule->setExpireDate(TQDate());
  actionEditor->updateRule(rule);
  rule->setLinkMode(linkModeAnd->isChecked()?KScoringRule::AND:KScoringRule::OR);
  condEditor->updateRule(rule);
  if (rule->getName() != ruleNameEdit->text())
    manager->setRuleName(rule,ruleNameEdit->text());
}

void RuleEditWidget::updateRule()
{
  KScoringRule *rule = manager->findRule(oldRuleName);
  if (rule) updateRule(rule);
}

void RuleEditWidget::slotAddGroup()
{
  TQString grp = groupsBox->currentText();
  if ( grp.isEmpty() )
      return;
  TQString txt = groupsEdit->text().stripWhiteSpace();
  if ( txt == ".*" || txt.isEmpty() ) groupsEdit->setText(grp);
  else groupsEdit->setText(txt + ";" + grp);
}

void RuleEditWidget::setDirty()
{
  kdDebug(5100) << "RuleEditWidget::setDirty()" << endl;
  if (dirty) return;
  dirty = true;
}

void RuleEditWidget::slotShrink()
{
  emit(shrink());
}

void RuleEditWidget::slotExpireEditChanged(int value)
{
  expireEdit->setSuffix(i18n(" day", " days", value));
}

//============================================================================
//
// class RuleListWidget (the widget for managing a list of rules)
//
//============================================================================
RuleListWidget::RuleListWidget(KScoringManager *m, bool standalone, TQWidget *p, const char *n)
  : TQWidget(p,n), alone(standalone), manager(m)
{
  kdDebug(5100) << "RuleListWidget::RuleListWidget()" << endl;
  if (!n) setName("RuleListWidget");
  TQVBoxLayout *topL = new TQVBoxLayout(this,standalone? 0:5,KDialog::spacingHint());
  ruleList = new KListBox(this);
  if (standalone) {
    connect(ruleList,TQT_SIGNAL(doubleClicked(TQListBoxItem*)),
            this,TQT_SLOT(slotEditRule(TQListBoxItem*)));
    connect(ruleList,TQT_SIGNAL(returnPressed(TQListBoxItem*)),
            this,TQT_SLOT(slotEditRule(TQListBoxItem*)));
  }
  connect(ruleList, TQT_SIGNAL(currentChanged(TQListBoxItem*)),
          this, TQT_SLOT(slotRuleSelected(TQListBoxItem*)));
  topL->addWidget(ruleList);

  TQHBoxLayout *btnL = new TQHBoxLayout( topL, KDialog::spacingHint() );
  mRuleUp = new TQPushButton( this );
  mRuleUp->setPixmap( BarIcon( "up", KIcon::SizeSmall ) );
  TQToolTip::add( mRuleUp, i18n("Move rule up") );
  btnL->addWidget( mRuleUp );
  connect( mRuleUp, TQT_SIGNAL( clicked() ), TQT_SLOT( slotRuleUp() ) );
  mRuleDown = new TQPushButton( this );
  mRuleDown->setPixmap( BarIcon( "down", KIcon::SizeSmall ) );
  TQToolTip::add( mRuleDown, i18n("Move rule down") );
  btnL->addWidget( mRuleDown );
  connect( mRuleDown, TQT_SIGNAL( clicked() ), TQT_SLOT( slotRuleDown() ) );

  btnL = new TQHBoxLayout( topL, KDialog::spacingHint() );
  editRule=0L;
  newRule = new TQPushButton(this);
  newRule->setPixmap( BarIcon( "filenew", KIcon::SizeSmall ) );
  TQToolTip::add(newRule,i18n("New rule")),
  btnL->addWidget(newRule);
  connect(newRule, TQT_SIGNAL(clicked()), this, TQT_SLOT(slotNewRule()));
  // if we're standalone, we need an additional edit button
  if (standalone) {
    editRule = new TQPushButton(this);
    editRule->setIconSet( BarIconSet("edit", KIcon::SizeSmall) );
    TQToolTip::add(editRule,i18n("Edit rule"));
    btnL->addWidget(editRule);
    connect(editRule,TQT_SIGNAL(clicked()),this,TQT_SLOT(slotEditRule()));
  }
  delRule = new TQPushButton(this);
  delRule->setIconSet( BarIconSet( "editdelete", KIcon::SizeSmall ) );
  TQToolTip::add(delRule,i18n("Remove rule"));
  btnL->addWidget(delRule);
  connect(delRule, TQT_SIGNAL(clicked()), this, TQT_SLOT(slotDelRule()));
  copyRule = new TQPushButton(this);
  copyRule->setIconSet(BarIconSet("editcopy", KIcon::SizeSmall));
  TQToolTip::add(copyRule,i18n("Copy rule"));
  btnL->addWidget(copyRule);
  connect(copyRule, TQT_SIGNAL(clicked()), this, TQT_SLOT(slotCopyRule()));

  // the group filter
  TQBoxLayout *filterL = new TQVBoxLayout(topL,KDialog::spacingHint());
  KComboBox *filterBox = new KComboBox(this);
  TQStringList l = m->getGroups();
  filterBox->insertItem(i18n("<all groups>"));
  filterBox->insertStringList(l);
  filterBox->setSizePolicy(TQSizePolicy(TQSizePolicy::Expanding, TQSizePolicy::Fixed));
  connect(filterBox,TQT_SIGNAL(activated(const TQString&)),
          this,TQT_SLOT(slotGroupFilter(const TQString&)));
  slotGroupFilter(i18n("<all groups>"));
  TQLabel *lab = new TQLabel(filterBox,i18n("Sho&w only rules for group:"),this);
  filterL->addWidget(lab);
  filterL->addWidget(filterBox);

  connect(manager,TQT_SIGNAL(changedRules()),
          this,TQT_SLOT(updateRuleList()));
  connect(manager,TQT_SIGNAL(changedRuleName(const TQString&,const TQString&)),
          this,TQT_SLOT(slotRuleNameChanged(const TQString&,const TQString&)));

  updateRuleList();
  updateButton();
}

RuleListWidget::~RuleListWidget()
{
}

void RuleListWidget::updateButton()
{
  bool state = ruleList->count() > 0;
  if(editRule)
    editRule->setEnabled(state);
  delRule->setEnabled(state);
  copyRule->setEnabled(state);

  TQListBoxItem *item = ruleList->item( ruleList->currentItem() );
  if ( item ) {
    mRuleUp->setEnabled( item->prev() != 0 );
    mRuleDown->setEnabled( item->next() != 0 );
  }
}

void RuleListWidget::updateRuleList()
{
  emit leavingRule();
  kdDebug(5100) << "RuleListWidget::updateRuleList()" << endl;
  TQString curr = ruleList->currentText();
  ruleList->clear();
  if (group == i18n("<all groups>")) {
    TQStringList l = manager->getRuleNames();
    ruleList->insertStringList(l);
  } else {
    KScoringManager::ScoringRuleList l = manager->getAllRules();
    for (KScoringRule* rule = l.first(); rule; rule = l.next() ) {
      if (rule->matchGroup(group)) ruleList->insertItem(rule->getName());
    }
  }
  int index = setCurrentItem(ruleList,curr);
  if (index <0) {
    ruleList->setCurrentItem(0);
    slotRuleSelected(ruleList->currentText());
  }
  else {
    slotRuleSelected(curr);
  }
}

void RuleListWidget::updateRuleList(const KScoringRule *rule)
{
  kdDebug(5100) << "RuleListWidget::updateRuleList(" << rule->getName() << ")" << endl;
  TQString name = rule->getName();
  updateRuleList();
  slotRuleSelected(name);
}

void RuleListWidget::slotRuleNameChanged(const TQString& oldName, const TQString& newName)
{
  int ind = ruleList->currentItem();
  for (uint i=0;i<ruleList->count();++i)
    if (ruleList->text(i) == oldName) {
      ruleList->changeItem(newName,i);
      ruleList->setCurrentItem(ind);
      return;
    }
}

void RuleListWidget::slotEditRule(const TQString& s)
{
  emit ruleEdited(s);
}

void RuleListWidget::slotEditRule()
{
  if (ruleList->currentItem() >= 0) {
    emit ruleEdited(ruleList->currentText());
  }
  else if (ruleList->count() == 0)
    emit ruleEdited(TQString::null);
}

void RuleListWidget::slotEditRule(TQListBoxItem* item)
{
  slotEditRule(item->text());
}

void RuleListWidget::slotGroupFilter(const TQString& s)
{
  group = s;
  updateRuleList();
}

void RuleListWidget::slotRuleSelected(const TQString& ruleName)
{
  emit leavingRule();
  kdDebug(5100) << "RuleListWidget::slotRuleSelected(" << ruleName << ")" << endl;
  if (ruleName != ruleList->currentText()) {
    setCurrentItem(ruleList,ruleName);
  }
  updateButton();
  emit ruleSelected(ruleName);
}

void RuleListWidget::slotRuleSelected(TQListBoxItem *item)
{
  if (!item) return;
  TQString ruleName = item->text();
  slotRuleSelected(ruleName);
}

void RuleListWidget::slotRuleSelected(int index)
{
  uint idx = index;
  if (idx >= ruleList->count()) return;
  TQString ruleName = ruleList->text(index);
  slotRuleSelected(ruleName);
}

void RuleListWidget::slotNewRule()
{
  emit leavingRule();
  KScoringRule *rule = manager->addRule();
  updateRuleList(rule);
  if (alone) slotEditRule(rule->getName());
  updateButton();
}

void RuleListWidget::slotDelRule()
{
  KScoringRule *rule = manager->findRule(ruleList->currentText());
  if (rule)
    manager->deleteRule(rule);
  // goto the next rule
  if (!alone) slotEditRule();
  updateButton();
}

void RuleListWidget::slotCopyRule()
{
  emit leavingRule();
  TQString ruleName = ruleList->currentText();
  KScoringRule *rule = manager->findRule(ruleName);
  if (rule) {
    KScoringRule *nrule = manager->copyRule(rule);
    updateRuleList(nrule);
    slotEditRule(nrule->getName());
  }
  updateButton();
}

void RuleListWidget::slotRuleUp()
{
  KScoringRule *rule = 0, *below = 0;
  TQListBoxItem *item = ruleList->item( ruleList->currentItem() );
  if ( item ) {
    rule = manager->findRule( item->text() );
    item = item->prev();
    if ( item )
      below = manager->findRule( item->text() );
  }
  if ( rule && below )
    manager->moveRuleAbove( rule, below );
  updateRuleList();
  updateButton();
}

void RuleListWidget::slotRuleDown()
{
  KScoringRule *rule = 0, *above = 0;
  TQListBoxItem *item = ruleList->item( ruleList->currentItem() );
  if ( item ) {
    rule = manager->findRule( item->text() );
    item = item->next();
    if ( item )
      above = manager->findRule( item->text() );
  }
  if ( rule && above )
    manager->moveRuleBelow( rule, above );
  updateRuleList();
  updateButton();
}

//============================================================================
//
// class KScoringEditor (the score edit dialog)
//
//============================================================================
KScoringEditor* KScoringEditor::scoreEditor = 0;

KScoringEditor::KScoringEditor(KScoringManager* m,
                               TQWidget *parent, const char *name)
  : KDialogBase(parent,name,false,i18n("Rule Editor"),Ok|Apply|Cancel,Ok,true), manager(m)
{
  manager->pushRuleList();
  if (!scoreEditor) scoreEditor = this;
  kdDebug(5100) << "KScoringEditor::KScoringEditor()" << endl;
  if (!name) setName("KScoringEditor");
  // the left side gives an overview about all rules, the right side
  // shows a detailed view of an selected rule
  TQWidget *w = new TQWidget(this);
  setMainWidget(w);
  TQHBoxLayout *hbl = new TQHBoxLayout(w,0,spacingHint());
  ruleLister = new RuleListWidget(manager,false,w);
  hbl->addWidget(ruleLister);
  ruleEditor = new RuleEditWidget(manager,w);
  hbl->addWidget(ruleEditor);
  connect(ruleLister,TQT_SIGNAL(ruleSelected(const TQString&)),
          ruleEditor, TQT_SLOT(slotEditRule(const TQString&)));
  connect(ruleLister, TQT_SIGNAL(leavingRule()),
          ruleEditor, TQT_SLOT(updateRule()));
  connect(ruleEditor, TQT_SIGNAL(shrink()), TQT_SLOT(slotShrink()));
  connect(this,TQT_SIGNAL(finished()),TQT_SLOT(slotFinished()));
  ruleLister->slotRuleSelected(0);
  resize(550, sizeHint().height());
}

void KScoringEditor::setDirty()
{
  TQPushButton *applyBtn = actionButton(Apply);
  applyBtn->setEnabled(true);
}

KScoringEditor::~KScoringEditor()
{
  scoreEditor = 0;
}

KScoringEditor* KScoringEditor::createEditor(KScoringManager* m,
                                             TQWidget *parent, const char *name)
{
  if (scoreEditor) return scoreEditor;
  else return new KScoringEditor(m,parent,name);
}

void KScoringEditor::setRule(KScoringRule* r)
{
  kdDebug(5100) << "KScoringEditor::setRule(" << r->getName() << ")" << endl;
  TQString ruleName = r->getName();
  ruleLister->slotRuleSelected(ruleName);
}

void KScoringEditor::slotShrink()
{
  TQTimer::singleShot(5, this, TQT_SLOT(slotDoShrink()));
}

void KScoringEditor::slotDoShrink()
{
  updateGeometry();
  TQApplication::sendPostedEvents();
  resize(width(),sizeHint().height());
}

void KScoringEditor::slotApply()
{
  TQString ruleName = ruleLister->currentRule();
  KScoringRule *rule = manager->findRule(ruleName);
  if (rule) {
    ruleEditor->updateRule(rule);
    ruleLister->updateRuleList(rule);
  }
  manager->removeTOS();
  manager->pushRuleList();
}

void KScoringEditor::slotOk()
{
  slotApply();
  manager->removeTOS();
  KDialogBase::slotOk();
  manager->editorReady();
}

void KScoringEditor::slotCancel()
{
  manager->popRuleList();
  KDialogBase::slotCancel();
}

void KScoringEditor::slotFinished()
{
  delayedDestruct();
}

//============================================================================
//
// class KScoringEditorWidgetDialog (a dialog for the KScoringEditorWidget)
//
//============================================================================
KScoringEditorWidgetDialog::KScoringEditorWidgetDialog(KScoringManager *m, const TQString& r, TQWidget *p, const char *n)
  : KDialogBase(p,n,true,i18n("Edit Rule"),
                KDialogBase::Ok|KDialogBase::Apply|KDialogBase::Close,
                KDialogBase::Ok,true),
    manager(m), ruleName(r)
{
  TQFrame *f = makeMainWidget();
  TQBoxLayout *topL = new TQVBoxLayout(f);
  ruleEditor = new RuleEditWidget(manager,f);
  connect(ruleEditor, TQT_SIGNAL(shrink()), TQT_SLOT(slotShrink()));
  topL->addWidget(ruleEditor);
  ruleEditor->slotEditRule(ruleName);
  resize(0,0);
}

void KScoringEditorWidgetDialog::slotApply()
{
  KScoringRule *rule = manager->findRule(ruleName);
  if (rule) {
    ruleEditor->updateRule(rule);
    ruleName = rule->getName();
  }
}

void KScoringEditorWidgetDialog::slotOk()
{
  slotApply();
  KDialogBase::slotOk();
}

void KScoringEditorWidgetDialog::slotShrink()
{
  TQTimer::singleShot(5, this, TQT_SLOT(slotDoShrink()));
}

void KScoringEditorWidgetDialog::slotDoShrink()
{
  updateGeometry();
  TQApplication::sendPostedEvents();
  resize(width(),sizeHint().height());
}

//============================================================================
//
// class KScoringEditorWidget (a reusable widget for config dialog...)
//
//============================================================================
KScoringEditorWidget::KScoringEditorWidget(KScoringManager *m,TQWidget *p, const char *n)
  : TQWidget(p,n), manager(m)
{
  TQBoxLayout *topL = new TQVBoxLayout(this);
  ruleLister = new RuleListWidget(manager,true,this);
  topL->addWidget(ruleLister);
  connect(ruleLister,TQT_SIGNAL(ruleEdited(const TQString&)),
          this,TQT_SLOT(slotRuleEdited(const TQString &)));
}

KScoringEditorWidget::~KScoringEditorWidget()
{
  manager->editorReady();
}

void KScoringEditorWidget::slotRuleEdited(const TQString& ruleName)
{
  KScoringEditorWidgetDialog dlg(manager,ruleName,this);
  dlg.exec();
  ruleLister->updateRuleList();
}

#include "kscoringeditor.moc"
