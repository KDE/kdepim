/*
    kscoring.h

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

#ifndef KSCORING_H
#define KSCORING_H

#include <unistd.h>

#include <tqglobal.h>
#include <tqptrlist.h>
#include <tqptrstack.h>
#include <tqregexp.h>

#include <tqobject.h>
#include <tqstring.h>
#include <tqstringlist.h>
#include <tqdatetime.h>
#include <tqcolor.h>
#include <tqtable.h>
#include <tqmap.h>
#include <tqdict.h>

#include <kdialogbase.h>
#include <klineedit.h>
#include <knuminput.h>

#include <kdepimmacros.h>

class QDomNode;
class QDomDocument;
class QDomElement;
class QTextStream;
class QLabel;


/**
  The following classes ScorableArticle, ScorableGroup define
  the interface for the scoring. Any application using this mechanism should
  create its own subclasses of this classes. The scoring itself will be handled
  by the ScoringManager class.
 */

//----------------------------------------------------------------------------
class KDE_EXPORT ScorableGroup
{
public:
  virtual ~ScorableGroup();
};

class KDE_EXPORT ScorableArticle
{
public:
  virtual ~ScorableArticle();

  virtual void addScore(short) {}
  virtual void displayMessage(const TQString&);
  virtual void changeColor(const TQColor&) {}
  virtual void markAsRead() {}
  virtual TQString from() const = 0;
  virtual TQString subject() const = 0;
  virtual TQString getHeaderByType(const TQString&) const = 0;
  //virtual ScorableGroup group() const =0;
};


//----------------------------------------------------------------------------
/**
  Base class for other Action classes.
 */
class KDE_EXPORT ActionBase {
 public:
  ActionBase();
  virtual ~ActionBase();
  virtual TQString toString() const =0;
  virtual void apply(ScorableArticle&) const =0;
  virtual ActionBase* clone() const =0;
  virtual int getType() const =0;
  virtual TQString getValueString() const { return TQString(); }
  virtual void setValue(const TQString&) {}
  static ActionBase* factory(int type, const TQString &value);
  static TQStringList userNames();
  static TQString userName(int type);
  static int getTypeForName(const TQString& name);
  static int getTypeForUserName(const TQString& name);
  TQString userName() { return userName(getType()); }
  enum ActionTypes { SETSCORE, NOTIFY, COLOR, MARKASREAD };
};

class KDE_EXPORT ActionColor : public ActionBase {
public:
  ActionColor(const TQColor&);
  ActionColor(const TQString&);
  ActionColor(const ActionColor&);
  virtual ~ActionColor();
  virtual TQString toString() const;
  virtual int getType() const { return COLOR; }
  virtual TQString getValueString() const { return color.name(); }
  virtual void setValue(const TQString& s) { color.setNamedColor(s); }
  void setValue(const TQColor& c) { color = c; }
  TQColor value() const { return color; }
  virtual void apply(ScorableArticle&) const;
  virtual ActionColor* clone() const;
private:
  TQColor color;
};

class KDE_EXPORT ActionSetScore : public ActionBase {
 public:
  ActionSetScore(short);
  ActionSetScore(const ActionSetScore&);
  ActionSetScore(const TQString&);
  virtual ~ActionSetScore();
  virtual TQString toString() const;
  virtual int getType() const { return SETSCORE; }
  virtual TQString getValueString() const { return TQString::number(val); }
  virtual void setValue(const TQString& s) { val = s.toShort(); }
  void setValue(short v) { val = v; }
  short value() const { return val; }
  virtual void apply(ScorableArticle&) const;
  virtual ActionSetScore* clone() const;
 private:
  short val;
};

class KDE_EXPORT ActionNotify : public ActionBase {
 public:
  ActionNotify(const TQString&);
  ActionNotify(const ActionNotify&);
  virtual ~ActionNotify() {}
  virtual TQString toString() const;
  virtual int getType() const { return NOTIFY; }
  virtual TQString getValueString() const { return note; }
  virtual void setValue(const TQString& s) { note = s; }
  virtual void apply(ScorableArticle&) const;
  virtual ActionNotify* clone() const;
 private:
  TQString note;
};

class KDE_EXPORT ActionMarkAsRead : public ActionBase {
  public:
    ActionMarkAsRead();
    ActionMarkAsRead( const ActionMarkAsRead& );
    virtual ~ActionMarkAsRead() {}
    virtual TQString toString() const;
    virtual int getType() const { return MARKASREAD; }
    virtual void apply( ScorableArticle &article ) const;
    virtual ActionMarkAsRead* clone() const;
};

class KDE_EXPORT NotifyCollection
{
public:
  NotifyCollection();
  ~NotifyCollection();
  void addNote(const ScorableArticle&, const TQString&);
  TQString collection() const;
  void displayCollection(TQWidget *p=0) const;
private:
  struct article_info {
    TQString from;
    TQString subject;
  };
  typedef TQValueList<article_info> article_list;
  typedef TQDict<article_list> note_list;
  note_list notifyList;
};


//----------------------------------------------------------------------------
class KDE_EXPORT KScoringExpression
{
  friend class KScoringRule;
 public:
  enum Condition { CONTAINS, MATCH, EQUALS, SMALLER, GREATER, MATCHCS };

  KScoringExpression(const TQString&,const TQString&,const TQString&, const TQString&);
  ~KScoringExpression();

  bool match(ScorableArticle& a) const ;
  TQString getTypeString() const;
  static TQString getTypeString(int);
  int getType() const;
  TQString toString() const;
  void write(TQTextStream& ) const;

  bool isNeg() const { return neg; }
  Condition getCondition() const { return cond; }
  TQString getExpression() const { return expr_str; }
  TQString getHeader() const { return header; }
  static TQStringList conditionNames();
  static TQStringList headerNames();
  static int getConditionForName(const TQString&);
  static TQString getNameForCondition(int);
 private:
  bool neg;
  TQString header;
  const char* c_header;
  Condition cond;
  TQRegExp expr;
  TQString expr_str;
  int expr_int;
};

//----------------------------------------------------------------------------
class KDE_EXPORT KScoringRule
{
  friend class KScoringManager;
 public:
  KScoringRule(const TQString& name);
  KScoringRule(const KScoringRule& r);
  ~KScoringRule();

  typedef TQPtrList<KScoringExpression> ScoreExprList;
  typedef TQPtrList<ActionBase> ActionList;
  typedef TQStringList GroupList;
  enum LinkMode { AND, OR };

  TQString getName() const { return name; }
  TQStringList getGroups() const { return groups; }
  void setGroups(const TQStringList &l) { groups = l; }
  LinkMode getLinkMode() const { return link; }
  TQString getLinkModeName() const;
  TQString getExpireDateString() const;
  TQDate getExpireDate() const { return expires; }
  void setExpireDate(const TQDate &d) { expires = d; }
  bool isExpired() const;
  ScoreExprList getExpressions() const { return expressions; }
  ActionList getActions() const { return actions; }
  void cleanExpressions();
  void cleanActions();

  bool matchGroup(const TQString& group) const ;
  void applyRule(ScorableArticle& a) const;
  void applyRule(ScorableArticle& a, const TQString& group) const;
  void applyAction(ScorableArticle& a) const;

  void setLinkMode(const TQString& link);
  void setLinkMode(LinkMode m) { link = m; }
  void setExpire(const TQString& exp);
  void addExpression( KScoringExpression* );
  void addGroup( const TQString& group) { groups.append(group); }
  //void addServer(const TQString& server) { servers.append(server); }
  void addAction(int, const TQString& );
  void addAction(ActionBase*);

  void updateXML(TQDomElement& e, TQDomDocument& d);
  TQString toString() const;

  // writes the rule in XML format into the textstream
  void write(TQTextStream& ) const;
protected:
  //! assert that the name is unique
  void setName(const TQString &n) { name = n; }
private:
  TQString name;
  GroupList groups;
  //ServerList servers;
  LinkMode link;
  ScoreExprList expressions;
  ActionList actions;
  TQDate expires;
};

/** this helper class implements a stack for lists of lists of rules.
    With the help of this class its very easy for the KScoringManager
    to temporary drop lists of rules and restore them afterwards
*/
class KDE_EXPORT RuleStack
{
public:
  RuleStack();
  ~RuleStack();
  //! puts the list on the stack, doesn't change the list
  void push(TQPtrList<KScoringRule>&);
  //! clears the argument list and copy the content of the TOS into it
  //! after that the TOS gets dropped
  void pop(TQPtrList<KScoringRule>&);
  //! like pop but without dropping the TOS
  void top(TQPtrList<KScoringRule>&);
  //! drops the TOS
  void drop();
private:
  TQPtrStack< TQPtrList<KScoringRule> > stack;
};

//----------------------------------------------------------------------------
// Manages the score rules.
class KDE_EXPORT KScoringManager : public QObject
{
  Q_OBJECT

 public:
  //* this is the container for all rules
  typedef TQPtrList<KScoringRule> ScoringRuleList;

  KScoringManager(const TQString& appName = TQString::null);
  virtual ~KScoringManager();

  //* returns a list of all available groups, must be overridden
  virtual TQStringList getGroups() const =0;

  //! returns a list of common (or available) headers
  //! defaults to returning { Subject, From, Message-ID, Date }
  virtual TQStringList getDefaultHeaders() const;

  //* setting current server and group and calling applyRules(ScorableArticle&)
  void applyRules(ScorableArticle& article, const TQString& group/*, const TQString& server*/);
  //* assuming a properly set group
  void applyRules(ScorableArticle&);
  //* same as above
  void applyRules(ScorableGroup* group);

  //* pushes the current rule list onto a stack
  void pushRuleList();
  //* restores the current rule list from list stored on a stack
  //* by a previous call to pushRuleList (this implicitly deletes the
  //* current rule list)
  void popRuleList();
  //* removes the TOS from the stack of rule lists
  void removeTOS();

  KScoringRule* addRule(KScoringRule *);
  KScoringRule* addRule(const ScorableArticle&, TQString group, short =0);
  KScoringRule* addRule();
  void cancelNewRule(KScoringRule *);
  void deleteRule(KScoringRule *);
  void editRule(KScoringRule *e, TQWidget *w=0);
  KScoringRule* copyRule(KScoringRule *);
  void moveRuleAbove( KScoringRule *above, KScoringRule *below );
  void moveRuleBelow( KScoringRule *below, KScoringRule *above );
  void setGroup(const TQString& g);
  // has to be called after setGroup() or initCache()
  bool hasRulesForCurrentGroup();
  TQString findUniqueName() const;

  /** called from an editor whenever it finishes editing the rule base,
      causes the finishedEditing signal to be emitted */
  void editorReady();

  ScoringRuleList getAllRules() const { return allRules; }
  KScoringRule *findRule(const TQString&);
  TQStringList getRuleNames();
  void setRuleName(KScoringRule *, const TQString&);
  int getRuleCount() const { return allRules.count(); }
  TQString toString() const;

  bool setCacheValid(bool v);
  bool isCacheValid() { return cacheValid; }
  void initCache(const TQString& group/*, const TQString& server*/);

  void load();
  void save();

  //--------------- Properties
  virtual bool canScores() const { return true; }
  virtual bool canNotes() const { return true; }
  virtual bool canColors() const { return false; }
  virtual bool canMarkAsRead() const { return false; }
  virtual bool hasFeature(int);

 signals:
  void changedRules();
  void changedRuleName(const TQString& oldName, const TQString& newName);
  void finishedEditing();

 private:
  void addRuleInternal(KScoringRule *e);
  void expireRules();

  TQDomDocument createXMLfromInternal();
  void createInternalFromXML(TQDomNode);

  // list of all Rules
  ScoringRuleList allRules;

  // the stack for temporary storing rule lists
  RuleStack stack;

  // for the cache
  bool cacheValid;
  // current rule set, ie the cache
  ScoringRuleList ruleList;
  //TQString server;
  TQString group;

  //ScorableServer* _s;

  // filename of the scorefile
  TQString mFilename;
};


//----------------------------------------------------------------------------
class KDE_EXPORT NotifyDialog : public KDialogBase
{
  Q_OBJECT
public:
  static void display(ScorableArticle&,const TQString&);
protected slots:
  void slotShowAgainToggled(bool);
private:
  NotifyDialog(TQWidget* p =0);
  static NotifyDialog *me;

  TQLabel *note;
  TQString msg;
  typedef TQMap<TQString,bool> NotesMap;
  static NotesMap dict;
};


#endif
