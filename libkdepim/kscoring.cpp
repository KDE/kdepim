/*
    kscoring.cpp

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
#ifdef KDE_USE_FINAL
#undef QT_NO_ASCII_CAST
#endif

#undef QT_NO_COMPAT

#include <iostream>

#include <tqfile.h>
#include <tqdom.h>
#include <tqlayout.h>
#include <tqlabel.h>
#include <tqcheckbox.h>
#include <tqtextview.h>

#include <klocale.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kinputdialog.h>

#include "kscoring.h"
#include "kscoringeditor.h"


//----------------------------------------------------------------------------
// a small function to encode attribute values, code stolen from QDom
static TQString toXml(const TQString& str)
{
  TQString tmp(str);
  uint len = tmp.length();
  uint i = 0;
  while ( i < len ) {
    if (tmp[(int)i] == '<') {
      tmp.replace(i, 1, "&lt;");
      len += 3;
      i += 4;
    } else if (tmp[(int)i] == '"') {
      tmp.replace(i, 1, "&quot;");
      len += 5;
      i += 6;
    } else if (tmp[(int)i] == '&') {
       tmp.replace(i, 1, "&amp;");
       len += 4;
       i += 5;
    } else if (tmp[(int)i] == '>') {
       tmp.replace(i, 1, "&gt;");
       len += 3;
       i += 4;
    } else {
       ++i;
    }
  }

  return tmp;
}


// small dialog to display the messages from NotifyAction
NotifyDialog* NotifyDialog::me = 0;
NotifyDialog::NotesMap NotifyDialog::dict;

NotifyDialog::NotifyDialog(TQWidget* p)
  : KDialogBase(p,"notify action dialog",true,"Notify Message",Close,Close,true)
{
  TQFrame *f = makeMainWidget();
  TQVBoxLayout *topL = new TQVBoxLayout(f);
  note = new TQLabel(f);
  note->setTextFormat(RichText);
  topL->addWidget(note);
  TQCheckBox *check = new TQCheckBox(i18n("Do not show this message again"),f);
  check->setChecked(true);
  topL->addWidget(check);
  connect(check,TQT_SIGNAL(toggled(bool)),TQT_SLOT(slotShowAgainToggled(bool)));
}

void NotifyDialog::slotShowAgainToggled(bool flag)
{
  dict.replace(msg,!flag);
  kdDebug(5100) << "note \"" << note << "\" will popup again: " << flag << endl;
}

void NotifyDialog::display(ScorableArticle& a, const TQString& s)
{
  kdDebug(5100) << "displaying message" << endl;
  if (!me) me = new NotifyDialog();
  me->msg = s;

  NotesMap::Iterator i = dict.find(s);
  if (i == dict.end() || i.data()) {
    TQString msg = i18n("Article\n<b>%1</b><br><b>%2</b><br>caused the following"
                       " note to appear:<br>%3").
                  arg(a.from()).
                  arg(a.subject()).
                  arg(s);
    me->note->setText(msg);
    if ( i == dict.end() ) i = dict.replace(s,false);
    me->adjustSize();
    me->exec();
  }
}


//----------------------------------------------------------------------------
ScorableArticle::~ScorableArticle()
{
}

void ScorableArticle::displayMessage(const TQString& note)
{
  NotifyDialog::display(*this,note);
}

//----------------------------------------------------------------------------
ScorableGroup::~ScorableGroup()
{
}

// the base class for all actions
ActionBase::ActionBase()
{
  kdDebug(5100) << "new Action " << this << endl;
}

ActionBase::~ActionBase()
{
  kdDebug(5100) << "delete Action " << this << endl;
}


TQStringList ActionBase::userNames()
{
  TQStringList l;
  l << userName(SETSCORE);
  l << userName(NOTIFY);
  l << userName(COLOR);
  l << userName(MARKASREAD);
  return l;
}

ActionBase* ActionBase::factory(int type, const TQString &value)
{
  switch (type) {
    case SETSCORE: return new ActionSetScore(value);
    case NOTIFY:   return new ActionNotify(value);
    case COLOR:    return new ActionColor(value);
    case MARKASREAD: return new ActionMarkAsRead();
  default:
    kdWarning(5100) << "unknown type " << type << " in ActionBase::factory()" << endl;
    return 0;
  }
}

TQString ActionBase::userName(int type)
{
  switch (type) {
    case SETSCORE: return i18n("Adjust Score");
    case NOTIFY:   return i18n("Display Message");
    case COLOR:    return i18n("Colorize Header");
    case MARKASREAD: return i18n("Mark As Read");
  default:
    kdWarning(5100) << "unknown type " << type << " in ActionBase::userName()" << endl;
    return 0;
  }
}

int ActionBase::getTypeForName(const TQString& name)
{
  if (name == "SETSCORE") return SETSCORE;
  else if (name == "NOTIFY") return NOTIFY;
  else if (name == "COLOR") return COLOR;
  else if (name == "MARKASREAD") return MARKASREAD;
  else {
    kdWarning(5100) << "unknown type string " << name
                    << " in ActionBase::getTypeForName()" << endl;
    return -1;
  }
}

int ActionBase::getTypeForUserName(const TQString& name)
{
  if (name == userName(SETSCORE)) return SETSCORE;
  else if (name == userName(NOTIFY)) return NOTIFY;
  else if (name == userName(COLOR)) return COLOR;
  else if ( name == userName(MARKASREAD) ) return MARKASREAD;
  else {
    kdWarning(5100) << "unknown type string " << name
                    << " in ActionBase::getTypeForUserName()" << endl;
    return -1;
  }
}

// the set score action
ActionSetScore::ActionSetScore(short v)
  : val(v)
{
}

ActionSetScore::ActionSetScore(const TQString& s)
{
  val = s.toShort();
}

ActionSetScore::ActionSetScore(const ActionSetScore& as)
  : ActionBase(),
    val(as.val)
{
}

ActionSetScore::~ActionSetScore()
{
}

TQString ActionSetScore::toString() const
{
  TQString a;
  a += "<Action type=\"SETSCORE\" value=\"" + TQString::number(val) + "\" />";
  return a;
}

void ActionSetScore::apply(ScorableArticle& a) const
{
  a.addScore(val);
}

ActionSetScore* ActionSetScore::clone() const
{
  return new ActionSetScore(*this);
}

// the color action
ActionColor::ActionColor(const TQColor& c)
  : ActionBase(), color(c)
{
}

ActionColor::ActionColor(const TQString& s)
  : ActionBase()
{
  setValue(s);
}

ActionColor::ActionColor(const ActionColor& a)
  : ActionBase(), color(a.color)
{
}

ActionColor::~ActionColor()
{}

TQString ActionColor::toString() const
{
  TQString a;
  a += "<Action type=\"COLOR\" value=\"" + toXml(color.name()) + "\" />";
  return a;
}

void ActionColor::apply(ScorableArticle& a) const
{
  a.changeColor(color);
}

ActionColor* ActionColor::clone() const
{
  return new ActionColor(*this);
}


// the notify action
ActionNotify::ActionNotify(const TQString& s)
{
  note = s;
}

ActionNotify::ActionNotify(const ActionNotify& an)
  : ActionBase()
{
  note = an.note;
}

TQString ActionNotify::toString() const
{
  return "<Action type=\"NOTIFY\" value=\"" + toXml(note) + "\" />";
}

void ActionNotify::apply(ScorableArticle& a) const
{
  a.displayMessage(note);
}

ActionNotify* ActionNotify::clone() const
{
  return new ActionNotify(*this);
}


// mark as read action
ActionMarkAsRead::ActionMarkAsRead() :
  ActionBase()
{
}

ActionMarkAsRead::ActionMarkAsRead( const ActionMarkAsRead &action ) :
  ActionBase()
{
  Q_UNUSED( action );
}

TQString ActionMarkAsRead::toString() const
{
  return "<Action type=\"MARKASREAD\"/>";
}

void ActionMarkAsRead::apply( ScorableArticle &article ) const
{
  article.markAsRead();
}

ActionMarkAsRead* ActionMarkAsRead::clone() const
{
  return new ActionMarkAsRead(*this);
}

//----------------------------------------------------------------------------
NotifyCollection::NotifyCollection()
{
  notifyList.setAutoDelete(true);
}

NotifyCollection::~NotifyCollection()
{
}

void NotifyCollection::addNote(const ScorableArticle& a, const TQString& note)
{
  article_list *l = notifyList.find(note);
  if (!l) {
    notifyList.insert(note,new article_list);
    l = notifyList.find(note);
  }
  article_info i;
  i.from = a.from();
  i.subject = a.subject();
  l->append(i);
}

TQString NotifyCollection::collection() const
{
  TQString notifyCollection = i18n("<h1>List of collected notes</h1>");
  notifyCollection += "<p><ul>";
  // first look thru the notes and create one string
  TQDictIterator<article_list> it(notifyList);
  for(;it.current();++it) {
    const TQString& note = it.currentKey();
    notifyCollection += "<li>" + note + "<ul>";
    article_list* alist = it.current();
    article_list::Iterator ait;
    for(ait = alist->begin(); ait != alist->end(); ++ait) {
      notifyCollection += "<li><b>From: </b>" + (*ait).from + "<br>";
      notifyCollection += "<b>Subject: </b>" + (*ait).subject;
    }
    notifyCollection += "</ul>";
  }
  notifyCollection += "</ul>";

  return notifyCollection;
}

void NotifyCollection::displayCollection(TQWidget *p) const
{
  //KMessageBox::information(p,collection(),i18n("Collected Notes"));
  KDialogBase *dlg = new KDialogBase( p, 0, false, i18n("Collected Notes"),
                                      KDialogBase::Close, KDialogBase::Close );
  TQTextView *text = new TQTextView(dlg);
  text->setText(collection());
  dlg->setMainWidget(text);
  dlg->setMinimumWidth(300);
  dlg->setMinimumHeight(300);
  dlg->show();
}

//----------------------------------------------------------------------------
KScoringExpression::KScoringExpression(const TQString& h, const TQString& t, const TQString& n, const TQString& ng)
  : header(h), expr_str(n)
{
  if (t == "MATCH" ) {
    cond = MATCH;
    expr.setPattern(expr_str);
    expr.setCaseSensitive(false);
  }
  else if ( t == "MATCHCS" ) {
    cond = MATCHCS;
    expr.setPattern( expr_str );
    expr.setCaseSensitive( true );
  }
  else if (t == "CONTAINS" ) cond = CONTAINS;
  else if (t == "EQUALS" ) cond = EQUALS;
  else if (t == "GREATER") {
    cond = GREATER;
    expr_int = expr_str.toInt();
  }
  else if (t == "SMALLER") {
    cond = SMALLER;
    expr_int = expr_str.toInt();
  }
  else {
    kdDebug(5100) << "unknown match type in new expression" << endl;
  }

  neg = ng.toInt();
  c_header = header.latin1();

  kdDebug(5100) << "new expr: " << c_header << "  " << t << "  "
                << expr_str << "  " << neg << endl;
}

// static
int KScoringExpression::getConditionForName(const TQString& s)
{
  if (s == getNameForCondition(CONTAINS)) return CONTAINS;
  else if (s == getNameForCondition(MATCH)) return MATCH;
  else if (s == getNameForCondition(MATCHCS)) return MATCHCS;
  else if (s == getNameForCondition(EQUALS)) return EQUALS;
  else if (s == getNameForCondition(SMALLER)) return SMALLER;
  else if (s == getNameForCondition(GREATER)) return GREATER;
  else {
    kdWarning(5100) << "unknown condition name " << s
                    << " in KScoringExpression::getConditionForName()" << endl;
    return -1;
  }
}

// static
TQString KScoringExpression::getNameForCondition(int cond)
{
  switch (cond) {
  case CONTAINS: return i18n("Contains Substring");
  case MATCH: return i18n("Matches Regular Expression");
  case MATCHCS: return i18n("Matches Regular Expression (Case Sensitive)");
  case EQUALS: return i18n("Is Exactly the Same As");
  case SMALLER: return i18n("Less Than");
  case GREATER: return i18n("Greater Than");
  default:
    kdWarning(5100) << "unknown condition " << cond
                    << " in KScoringExpression::getNameForCondition()" << endl;
    return "";
  }
}

// static
TQStringList KScoringExpression::conditionNames()
{
  TQStringList l;
  l << getNameForCondition(CONTAINS);
  l << getNameForCondition(MATCH);
  l << getNameForCondition(MATCHCS);
  l << getNameForCondition(EQUALS);
  l << getNameForCondition(SMALLER);
  l << getNameForCondition(GREATER);
  return l;
}

// static
TQStringList KScoringExpression::headerNames()
{
  TQStringList l;
  l.append("From");
  l.append("Message-ID");
  l.append("Subject");
  l.append("Date");
  l.append("References");
  l.append("NNTP-Posting-Host");
  l.append("Bytes");
  l.append("Lines");
  l.append("Xref");
  return l;
}

KScoringExpression::~KScoringExpression()
{
}

bool KScoringExpression::match(ScorableArticle& a) const
{
  //kdDebug(5100) << "matching against header " << c_header << endl;
  bool res = true;
  TQString head;

  if (header == "From")
    head = a.from();
  else if (header == "Subject")
    head = a.subject();
  else
    head = a.getHeaderByType(c_header);

  if (!head.isEmpty()) {
    switch (cond) {
    case EQUALS:
      res = (head.lower() == expr_str.lower());
      break;
    case CONTAINS:
      res = (head.lower().find(expr_str.lower()) >= 0);
      break;
    case MATCH:
    case MATCHCS:
      res = (expr.search(head)!=-1);
      break;
    case GREATER:
      res = (head.toInt() > expr_int);
      break;
    case SMALLER:
      res = (head.toInt() < expr_int);
      break;
    default:
      kdDebug(5100) << "unknown match" << endl;
      res = false;
    }
  }
  else res = false;
//  kdDebug(5100) << "matching returns " << res << endl;
  return (neg)?!res:res;
}

void KScoringExpression::write(TQTextStream& st) const
{
  st << toString();
}

TQString KScoringExpression::toString() const
{
//   kdDebug(5100) << "KScoringExpression::toString() starts" << endl;
//   kdDebug(5100) << "header is " << header << endl;
//   kdDebug(5100) << "expr is " << expr_str << endl;
//   kdDebug(5100) << "neg is " << neg << endl;
//   kdDebug(5100) << "type is " << getType() << endl;
  TQString e;
  e += "<Expression neg=\"" + TQString::number(neg?1:0)
    + "\" header=\"" + header
    + "\" type=\"" + getTypeString()
    + "\" expr=\"" + toXml(expr_str)
    + "\" />";
//   kdDebug(5100) << "KScoringExpression::toString() finished" << endl;
  return e;
}

TQString KScoringExpression::getTypeString() const
{
  return KScoringExpression::getTypeString(cond);
}

TQString KScoringExpression::getTypeString(int cond)
{
  switch (cond) {
  case CONTAINS: return "CONTAINS";
  case MATCH: return "MATCH";
  case MATCHCS: return "MATCHCS";
  case EQUALS: return "EQUALS";
  case SMALLER: return "SMALLER";
  case GREATER: return "GREATER";
  default:
    kdWarning(5100) << "unknown cond " << cond << " in KScoringExpression::getTypeString()" << endl;
    return "";
  }
}

int  KScoringExpression::getType() const
{
  return cond;
}

//----------------------------------------------------------------------------
KScoringRule::KScoringRule(const TQString& n )
  : name(n), link(AND)
{
  expressions.setAutoDelete(true);
  actions.setAutoDelete(true);
}

KScoringRule::KScoringRule(const KScoringRule& r)
{
  kdDebug(5100) << "copying rule " << r.getName() << endl;
  name = r.getName();
  expressions.setAutoDelete(true);
  actions.setAutoDelete(true);
  // copy expressions
  expressions.clear();
  const ScoreExprList& rexpr = r.expressions;
  TQPtrListIterator<KScoringExpression> it(rexpr);
  for ( ; it.current(); ++it ) {
    KScoringExpression *t = new KScoringExpression(**it);
    expressions.append(t);
  }
  // copy actions
  actions.clear();
  const ActionList& ract = r.actions;
  TQPtrListIterator<ActionBase> ait(ract);
  for ( ; ait.current(); ++ait ) {
    ActionBase *t = *ait;
    actions.append(t->clone());
  }
  // copy groups, servers, linkmode and expires
  groups = r.groups;
  expires = r.expires;
  link = r.link;
}

KScoringRule::~KScoringRule()
{
  cleanExpressions();
  cleanActions();
}

void KScoringRule::cleanExpressions()
{
  // the expressions is setAutoDelete(true)
  expressions.clear();
}

void KScoringRule::cleanActions()
{
  // the actions is setAutoDelete(true)
  actions.clear();
}

void KScoringRule::addExpression( KScoringExpression* expr)
{
  kdDebug(5100) << "KScoringRule::addExpression" << endl;
  expressions.append(expr);
}

void KScoringRule::addAction(int type, const TQString& val)
{
  ActionBase *action = ActionBase::factory(type,val);
  addAction(action);
}

void KScoringRule::addAction(ActionBase* a)
{
  kdDebug(5100) << "KScoringRule::addAction() " << a->toString() << endl;
  actions.append(a);
}

void KScoringRule::setLinkMode(const TQString& l)
{
  if (l == "OR") link = OR;
  else link = AND;
}

void KScoringRule::setExpire(const TQString& e)
{
  if (e != "never") {
    TQStringList l = TQStringList::split("-",e);
    Q_ASSERT( l.count() == 3 );
    expires.setYMD( (*(l.at(0))).toInt(),
        (*(l.at(1))).toInt(),
        (*(l.at(2))).toInt());
  }
  kdDebug(5100) << "Rule " << getName() << " expires at " << getExpireDateString() << endl;
}

bool KScoringRule::matchGroup(const TQString& group) const
{
  for(GroupList::ConstIterator i=groups.begin(); i!=groups.end();++i) {
    TQRegExp e(*i);
    if (e.search(group, 0) != -1 &&
	(uint)e.matchedLength() == group.length())
        return true;
  }
  return false;
}

void KScoringRule::applyAction(ScorableArticle& a) const
{
  TQPtrListIterator<ActionBase> it(actions);
  for(; it.current(); ++it) {
    it.current()->apply(a);
  }
}

void KScoringRule::applyRule(ScorableArticle& a) const
{
  // kdDebug(5100) << "checking rule " << name << endl;
  // kdDebug(5100)  << " for article from "
  //              << a->from()->asUnicodeString()
  //              << endl;
  bool oper_and = (link == AND);
  bool res = true;
  TQPtrListIterator<KScoringExpression> it(expressions);
  //kdDebug(5100) << "checking " << expressions.count() << " expressions" << endl;
  for (; it.current(); ++it) {
    Q_ASSERT( it.current() );
    res = it.current()->match(a);
    if (!res && oper_and) return;
    else if (res && !oper_and) break;
  }
  if (res) applyAction(a);
}

void KScoringRule::applyRule(ScorableArticle& a /*, const TQString& s*/, const TQString& g) const
{
  // check if one of the groups match
  for (TQStringList::ConstIterator i = groups.begin(); i != groups.end(); ++i) {
    if (TQRegExp(*i).search(g) != -1) {
      applyRule(a);
      return;
    }
  }
}

void KScoringRule::write(TQTextStream& s) const
{
  s << toString();
}

TQString KScoringRule::toString() const
{
  //kdDebug(5100) << "KScoringRule::toString() starts" << endl;
  TQString r;
  r += "<Rule name=\"" + toXml(name) + "\" linkmode=\"" + getLinkModeName();
  r += "\" expires=\"" + getExpireDateString() + "\">";
  //kdDebug(5100) << "building grouplist..." << endl;
  for(GroupList::ConstIterator i=groups.begin();i!=groups.end();++i) {
    r += "<Group name=\"" + toXml(*i) + "\" />";
  }
  //kdDebug(5100) << "building expressionlist..." << endl;
  TQPtrListIterator<KScoringExpression> eit(expressions);
  for (; eit.current(); ++eit) {
    r += eit.current()->toString();
  }
  //kdDebug(5100) << "building actionlist..." << endl;
  TQPtrListIterator<ActionBase> ait(actions);
  for (; ait.current(); ++ait) {
    r += ait.current()->toString();
  }
  r += "</Rule>";
  //kdDebug(5100) << "KScoringRule::toString() finished" << endl;
  return r;
}

TQString KScoringRule::getLinkModeName() const
{
  switch (link) {
  case AND: return "AND";
  case OR: return "OR";
  default: return "AND";
  }
}

TQString KScoringRule::getExpireDateString() const
{
  if (expires.isNull()) return "never";
  else {
    return TQString::number(expires.year()) + TQString("-")
      + TQString::number(expires.month()) + TQString("-")
      + TQString::number(expires.day());
  }
}

bool KScoringRule::isExpired() const
{
  return (expires.isValid() && (expires < TQDate::currentDate()));
}



//----------------------------------------------------------------------------
KScoringManager::KScoringManager(const TQString& appName)
  :  cacheValid(false)//, _s(0)
{
  allRules.setAutoDelete(true);
  // determine filename of the scorefile
  if(appName.isEmpty())
    mFilename = KGlobal::dirs()->saveLocation("appdata") + "/scorefile";
  else
    mFilename = KGlobal::dirs()->saveLocation("data") + "/" + appName + "/scorefile";
  // open the score file
  load();
}


KScoringManager::~KScoringManager()
{
}

void KScoringManager::load()
{
  TQDomDocument sdoc("Scorefile");
  TQFile f( mFilename );
  if ( !f.open( IO_ReadOnly ) )
    return;
  if ( !sdoc.setContent( &f ) ) {
    f.close();
    kdDebug(5100) << "loading the scorefile failed" << endl;
    return;
  }
  f.close();
  kdDebug(5100) << "loaded the scorefile, creating internal representation" << endl;
  allRules.clear();
  createInternalFromXML(sdoc);
  expireRules();
  kdDebug(5100) << "ready, got " << allRules.count() << " rules" << endl;
}

void KScoringManager::save()
{
  kdDebug(5100) << "KScoringManager::save() starts" << endl;
  TQFile f( mFilename );
  if ( !f.open( IO_WriteOnly ) )
    return;
  TQTextStream stream(&f);
  stream.setEncoding(TQTextStream::Unicode);
  kdDebug(5100) << "KScoringManager::save() creating xml" << endl;
  createXMLfromInternal().save(stream,2);
  kdDebug(5100) << "KScoringManager::save() finished" << endl;
}

TQDomDocument KScoringManager::createXMLfromInternal()
{
  // I was'nt able to create a TQDomDocument in memory:(
  // so I write the content into a string, which is really stupid
  TQDomDocument sdoc("Scorefile");
  TQString ss; // scorestring
  ss += "<?xml version = '1.0'?><!DOCTYPE Scorefile >";
  ss += toString();
  ss += "</Scorefile>\n";
  kdDebug(5100) << "KScoringManager::createXMLfromInternal():" << endl << ss << endl;
  sdoc.setContent(ss);
  return sdoc;
}

TQString KScoringManager::toString() const
{
  TQString s;
  s += "<Scorefile>\n";
  TQPtrListIterator<KScoringRule> it(allRules);
  for( ; it.current(); ++it) {
    s += it.current()->toString();
  }
  return s;
}

void KScoringManager::expireRules()
{
  for ( KScoringRule *cR = allRules.first(); cR; cR=allRules.next()) {
    if (cR->isExpired()) {
      kdDebug(5100) << "Rule " << cR->getName() << " is expired, deleting it" << endl;
      allRules.remove();
    }
  }
}

void KScoringManager::createInternalFromXML(TQDomNode n)
{
  static KScoringRule *cR = 0; // the currentRule
  // the XML file was parsed and now we simply traverse the resulting tree
  if ( !n.isNull() ) {
    kdDebug(5100) << "inspecting node of type " << n.nodeType()
                  << " named " << n.toElement().tagName() << endl;

    switch (n.nodeType()) {
    case TQDomNode::DocumentNode: {
      // the document itself
      break;
    }
    case TQDomNode::ElementNode: {
      // Server, Newsgroup, Rule, Expression, Action
      TQDomElement e = n.toElement();
      //kdDebug(5100) << "The name of the element is "
      //<< e.tagName().latin1() << endl;
      TQString s = e.tagName();
      if (s == "Rule") {
        cR = new KScoringRule(e.attribute("name"));
        cR->setLinkMode(e.attribute("linkmode"));
        cR->setExpire(e.attribute("expires"));
        addRuleInternal(cR);
      }
      else if (s == "Group") {
        Q_CHECK_PTR(cR);
        cR->addGroup( e.attribute("name") );
      }
      else if (s == "Expression") {
        cR->addExpression(new KScoringExpression(e.attribute("header"),
                                                 e.attribute("type"),
                                                 e.attribute("expr"),
                                                 e.attribute("neg")));
      }
      else if (s == "Action") {
        Q_CHECK_PTR(cR);
        cR->addAction(ActionBase::getTypeForName(e.attribute("type")),
                      e.attribute("value"));
      }
      break;
    }
    default: // kdDebug(5100) << "unknown DomNode::type" << endl;
      ;
    }
    TQDomNodeList nodelist = n.childNodes();
    unsigned cnt = nodelist.count();
    //kdDebug(5100) << "recursive checking " << cnt << " nodes" << endl;
    for (unsigned i=0;i<cnt;++i)
      createInternalFromXML(nodelist.item(i));
  }
}

KScoringRule* KScoringManager::addRule(const ScorableArticle& a, TQString group, short score)
{
  KScoringRule *rule = new KScoringRule(findUniqueName());
  rule->addGroup( group );
  rule->addExpression(
    new KScoringExpression("From","CONTAINS",
                            a.from(),"0"));
  if (score) rule->addAction(new ActionSetScore(score));
  rule->setExpireDate(TQDate::currentDate().addDays(30));
  addRule(rule);
  KScoringEditor *edit = KScoringEditor::createEditor(this);
  edit->setRule(rule);
  edit->show();
  setCacheValid(false);
  return rule;
}

KScoringRule* KScoringManager::addRule(KScoringRule* expr)
{
  int i = allRules.findRef(expr);
  if (i == -1) {
    // only add a rule we don't know
    addRuleInternal(expr);
  }
  else {
    emit changedRules();
  }
  return expr;
}

KScoringRule* KScoringManager::addRule()
{
  KScoringRule *rule = new KScoringRule(findUniqueName());
  addRule(rule);
  return rule;
}

void KScoringManager::addRuleInternal(KScoringRule *e)
{
  allRules.append(e);
  setCacheValid(false);
  emit changedRules();
  kdDebug(5100) << "KScoringManager::addRuleInternal " << e->getName() << endl;
}

void KScoringManager::cancelNewRule(KScoringRule *r)
{
  // if e was'nt previously added to the list of rules, we delete it
  int i = allRules.findRef(r);
  if (i == -1) {
    kdDebug(5100) << "deleting rule " << r->getName() << endl;
    deleteRule(r);
  }
  else {
    kdDebug(5100) << "rule " << r->getName() << " not deleted" << endl;
  }
}

void KScoringManager::setRuleName(KScoringRule *r, const TQString& s)
{
  bool cont = true;
  TQString text = s;
  TQString oldName = r->getName();
  while (cont) {
    cont = false;
    TQPtrListIterator<KScoringRule> it(allRules);
    for (; it.current(); ++it) {
      if ( it.current() != r && it.current()->getName() == text ) {
        kdDebug(5100) << "rule name " << text << " is not unique" << endl;
	text = KInputDialog::getText(i18n("Choose Another Rule Name"),
			i18n("The rule name is already assigned, please choose another name:"),
			text);
        cont = true;
        break;
      }
    }
  }
  if (text != oldName) {
    r->setName(text);
    emit changedRuleName(oldName,text);
  }
}

void KScoringManager::deleteRule(KScoringRule *r)
{
  int i = allRules.findRef(r);
  if (i != -1) {
    allRules.remove();
    emit changedRules();
  }
}

void KScoringManager::editRule(KScoringRule *e, TQWidget *w)
{
  KScoringEditor *edit = KScoringEditor::createEditor(this, w);
  edit->setRule(e);
  edit->show();
  delete edit;
}

void KScoringManager::moveRuleAbove( KScoringRule *above, KScoringRule *below )
{
  int aindex = allRules.findRef( above );
  int bindex = allRules.findRef( below );
  if ( aindex <= 0 || bindex < 0 )
    return;
  if ( aindex < bindex )
    --bindex;
  allRules.take( aindex );
  allRules.insert( bindex, above );
}

void KScoringManager::moveRuleBelow( KScoringRule *below, KScoringRule *above )
{
  int bindex = allRules.findRef( below );
  int aindex = allRules.findRef( above );
  if ( bindex < 0 || bindex >= (int)allRules.count() - 1 || aindex < 0 )
    return;
  if ( bindex < aindex )
    --aindex;
  allRules.take( bindex );
  allRules.insert( aindex + 1, below );
}

void KScoringManager::editorReady()
{
  kdDebug(5100) << "emitting signal finishedEditing" << endl;
  save();
  emit finishedEditing();
}

KScoringRule* KScoringManager::copyRule(KScoringRule *r)
{
  KScoringRule *rule = new KScoringRule(*r);
  rule->setName(findUniqueName());
  addRuleInternal(rule);
  return rule;
}

void KScoringManager::applyRules(ScorableGroup* )
{
  kdWarning(5100) << "KScoringManager::applyRules(ScorableGroup* ) isn't implemented" << endl;
}

void KScoringManager::applyRules(ScorableArticle& article, const TQString& group)
{
  setGroup(group);
  applyRules(article);
}

void KScoringManager::applyRules(ScorableArticle& a)
{
  TQPtrListIterator<KScoringRule> it(isCacheValid()? ruleList : allRules);
  for( ; it.current(); ++it) {
    it.current()->applyRule(a);
  }
}

void KScoringManager::initCache(const TQString& g)
{
  group = g;
  ruleList.clear();
  TQPtrListIterator<KScoringRule> it(allRules);
  for (; it.current(); ++it) {
    if ( it.current()->matchGroup(group) ) {
      ruleList.append(it.current());
    }
  }
  kdDebug(5100) << "created cache for group " << group
                << " with " << ruleList.count() << " rules" << endl;
  setCacheValid(true);
}

void KScoringManager::setGroup(const TQString& g)
{
  if (group != g) initCache(g);
}

bool KScoringManager::hasRulesForCurrentGroup()
{
  return ruleList.count() != 0;
}


TQStringList KScoringManager::getRuleNames()
{
  TQStringList l;
  TQPtrListIterator<KScoringRule> it(allRules);
  for( ; it.current(); ++it) {
    l << it.current()->getName();
  }
  return l;
}

KScoringRule* KScoringManager::findRule(const TQString& ruleName)
{
  TQPtrListIterator<KScoringRule> it(allRules);
  for (; it.current(); ++it) {
    if ( it.current()->getName() == ruleName ) {
      return it;
    }
  }
  return 0;
}

bool KScoringManager::setCacheValid(bool v)
{
  bool res = cacheValid;
  cacheValid = v;
  return res;
}

TQString KScoringManager::findUniqueName() const
{
  int nr = 0;
  TQString ret;
  bool duplicated=false;

  while (nr < 99999999) {
    nr++;
    ret = i18n("rule %1").arg(nr);

    duplicated=false;
    TQPtrListIterator<KScoringRule> it(allRules);
    for( ; it.current(); ++it) {
      if (it.current()->getName() == ret) {
        duplicated = true;
        break;
      }
    }

    if (!duplicated)
      return ret;
  }

  return ret;
}

bool KScoringManager::hasFeature(int p)
{
  switch (p) {
    case ActionBase::SETSCORE: return canScores();
    case ActionBase::NOTIFY: return canNotes();
    case ActionBase::COLOR: return canColors();
    case ActionBase::MARKASREAD: return canMarkAsRead();
    default: return false;
  }
}

TQStringList KScoringManager::getDefaultHeaders() const
{
  TQStringList l;
  l.append("Subject");
  l.append("From");
  l.append("Date");
  l.append("Message-ID");
  return l;
}

void KScoringManager::pushRuleList()
{
  stack.push(allRules);
}

void KScoringManager::popRuleList()
{
  stack.pop(allRules);
}

void KScoringManager::removeTOS()
{
  stack.drop();
}

RuleStack::RuleStack()
{
}

RuleStack::~RuleStack()
{}

void RuleStack::push(TQPtrList<KScoringRule>& l)
{
  kdDebug(5100) << "RuleStack::push pushing list with " << l.count() << " rules" << endl;
  KScoringManager::ScoringRuleList *l1 = new KScoringManager::ScoringRuleList;
  for ( KScoringRule *r=l.first(); r != 0; r=l.next() ) {
    l1->append(new KScoringRule(*r));
  }
  stack.push(l1);
  kdDebug(5100) << "now there are " << stack.count() << " lists on the stack" << endl;
}

void RuleStack::pop(TQPtrList<KScoringRule>& l)
{
  top(l);
  drop();
  kdDebug(5100) << "RuleStack::pop pops list with " << l.count() << " rules" << endl;
  kdDebug(5100) << "now there are " << stack.count() << " lists on the stack" << endl;
}

void RuleStack::top(TQPtrList<KScoringRule>& l)
{
  l.clear();
  KScoringManager::ScoringRuleList *l1 = stack.top();
  l = *l1;
}

void RuleStack::drop()
{
  kdDebug(5100) << "drop: now there are " << stack.count() << " lists on the stack" << endl;
  stack.remove();
}


#include "kscoring.moc"
