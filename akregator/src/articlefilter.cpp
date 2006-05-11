/*
 * articlefilter.cpp
 *
 * Copyright (c) 2004, 2005 Frerich Raabe <raabe@kde.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "articlefilter.h"
#include "article.h"
#include "shared.h"

#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kurl.h>

#include <qregexp.h>

namespace Akregator {
namespace Filters {

QString Criterion::subjectToString(Subject subj)
{
    switch (subj)
    {
        case Title:
            return QString::fromLatin1("Title");
        case Link:
            return QString::fromLatin1("Link");
        case Author:
            return QString::fromLatin1("Author");
        case Description:
            return QString::fromLatin1("Description");
        case Status:
            return QString::fromLatin1("Status");
        case KeepFlag:
            return QString::fromLatin1("KeepFlag");
        default: // should never happen (TM)
            return QString::fromLatin1("Description");
    }
}

Criterion::Subject Criterion::stringToSubject(const QString& subjStr)
{
    if (subjStr == QString::fromLatin1("Title"))
        return Title;
    else if (subjStr == QString::fromLatin1("Link"))
        return Link;
    else if (subjStr == QString::fromLatin1("Description"))
        return Description;
    else if (subjStr == QString::fromLatin1("Author"))
        return Author;
    else if (subjStr == QString::fromLatin1("Status"))
        return Status;
    else if (subjStr == QString::fromLatin1("KeepFlag"))
        return KeepFlag;

    // hopefully never reached
    return Description;
}

QString Criterion::predicateToString(Predicate pred)
{
    switch (pred)
    {
        case Contains:
            return QString::fromLatin1("Contains");
        case Equals:
            return QString::fromLatin1("Equals");
        case Matches:
            return QString::fromLatin1("Matches");
        case Negation:
            return QString::fromLatin1("Negation");
        default:// hopefully never reached
            return QString::fromLatin1("Contains");
    }
}

Criterion::Predicate Criterion::stringToPredicate(const QString& predStr)
{
    if (predStr == QString::fromLatin1("Contains"))
        return Contains;
    else if (predStr == QString::fromLatin1("Equals"))
        return Equals;
    else if (predStr == QString::fromLatin1("Matches"))
        return Matches;
    else if (predStr == QString::fromLatin1("Negation"))
        return Negation;
    
    // hopefully never reached
    return Contains;
}

Criterion::Criterion()
{
}

Criterion::Criterion( Subject subject, Predicate predicate, const QVariant &object )
    : m_subject( subject )
    , m_predicate( predicate )
    , m_object( object )
{

}

void Criterion::writeConfig(KConfig* config) const
{
    config->writeEntry(QString::fromLatin1("subject"), subjectToString(m_subject));

    config->writeEntry(QString::fromLatin1("predicate"), predicateToString(m_predicate));

    config->writeEntry(QString::fromLatin1("objectType"), QString(m_object.typeName()));

    config->writeEntry(QString::fromLatin1("objectValue"), m_object);
}

void Criterion::readConfig(KConfig* config)
{
    m_subject = stringToSubject(config->readEntry(QString::fromLatin1("subject")));
    m_predicate = stringToPredicate(config->readEntry(QString::fromLatin1("predicate")));
    QVariant::Type type = QVariant::nameToType(config->readEntry(QString::fromLatin1("objType")).ascii());

    if (type != QVariant::Invalid)
    {
        m_object = config->readPropertyEntry(QString::fromLatin1("objectValue"), type);
    }
}

bool Criterion::satisfiedBy( const Article &article ) const
{
    QVariant concreteSubject;

    switch ( m_subject ) {
        case Title:
            concreteSubject = QVariant(article.title());
            break;
        case Description:
            concreteSubject = QVariant(article.description());
            break;
        case Author:
            concreteSubject = QVariant(article.author());
            break;
        case Link:
            // ### Maybe use prettyURL here?
            concreteSubject = QVariant(article.link().url());
            break;
        case Status:
            concreteSubject = QVariant(article.status());
            break;
        case KeepFlag:
            concreteSubject = QVariant(article.keep());   
        default:
            break;
    }

    bool satisfied = false;

    const Predicate predicateType = static_cast<Predicate>( m_predicate & ~Negation );
	QString subjectType=concreteSubject.typeName();

    switch ( predicateType ) {
        case Contains:
            satisfied = concreteSubject.toString().find( m_object.toString(), 0, false ) != -1;
            break;
        case Equals:
            if (subjectType=="int")
                satisfied = concreteSubject.toInt() == m_object.toInt();
            else
                satisfied = concreteSubject.toString() == m_object.toString();
            break;
        case Matches:
            satisfied = QRegExp( m_object.toString() ).search( concreteSubject.toString() ) != -1;
            break;
        default:
            kdDebug() << "Internal inconsistency; predicateType should never be Negation" << endl;
            break;
    }

    if ( m_predicate & Negation ) {
        satisfied = !satisfied;
    }

    return satisfied;
}

Criterion::Subject Criterion::subject() const
{
    return m_subject;
}

Criterion::Predicate Criterion::predicate() const
{
    return m_predicate;
}

QVariant Criterion::object() const
{
    return m_object;
}

ArticleMatcher::ArticleMatcher()
    : m_association( None )
{
}

ArticleMatcher::~ArticleMatcher()
{
}

bool ArticleMatcher::matchesAll() const
{
    return m_criteria.isEmpty();
}

ArticleMatcher* ArticleMatcher::clone() const
{
    return new ArticleMatcher(*this);
}

ArticleMatcher::ArticleMatcher( const QValueList<Criterion> &criteria, Association assoc)
    : m_criteria( criteria )
    , m_association( assoc )
{
}

ArticleMatcher& ArticleMatcher::operator=(const ArticleMatcher& other)
{
    m_association = other.m_association;
    m_criteria = other.m_criteria;
    return *this;
}

ArticleMatcher::ArticleMatcher(const ArticleMatcher& other) : AbstractMatcher(other)
{
    *this = other;
}

bool ArticleMatcher::matches( const Article &a ) const
{
    switch ( m_association ) {
        case LogicalOr:
            return anyCriterionMatches( a );
        case LogicalAnd:
            return allCriteriaMatch( a );
        default:
            break;
    }
    return true;
}

void ArticleMatcher::writeConfig(KConfig* config) const
{
    config->writeEntry(QString::fromLatin1("matcherAssociation"), associationToString(m_association));
    
    config->writeEntry(QString::fromLatin1("matcherCriteriaCount"), m_criteria.count());

    int index = 0;

    for (QValueList<Criterion>::ConstIterator it = m_criteria.begin(); it != m_criteria.end(); ++it)
    {
        config->setGroup(config->group()+QString::fromLatin1("_Criterion")+QString::number(index));
        (*it).writeConfig(config);
        ++index;
    }
}

void ArticleMatcher::readConfig(KConfig* config)
{
    m_criteria.clear();
    m_association = stringToAssociation(config->readEntry(QString::fromLatin1("matcherAssociation")));

    int count =  config->readNumEntry(QString::fromLatin1("matcherCriteriaCount"), 0);
    
    for (int i = 0; i < count; ++i)
    {
        Criterion c;
        config->setGroup(config->group()+QString::fromLatin1("_Criterion")+QString::number(i));
        c.readConfig(config);
        m_criteria.append(c);
    }
}

bool ArticleMatcher::operator==(const AbstractMatcher& other) const
{
    AbstractMatcher* ptr = const_cast<AbstractMatcher*>(&other);
    ArticleMatcher* o = dynamic_cast<ArticleMatcher*>(ptr);
    if (!o)
        return false;
    else
        return m_association == o->m_association && m_criteria == o->m_criteria;
}
bool ArticleMatcher::operator!=(const AbstractMatcher& other) const
{
    return !(*this == other);
}

bool ArticleMatcher::anyCriterionMatches( const Article &a ) const
{
    if (m_criteria.count()==0)
        return true;
    QValueList<Criterion>::ConstIterator it = m_criteria.begin();
    QValueList<Criterion>::ConstIterator end = m_criteria.end();
    for ( ; it != end; ++it ) {
        if ( ( *it ).satisfiedBy( a ) ) {
            return true;
        }
    }
    return false;
}

bool ArticleMatcher::allCriteriaMatch( const Article &a ) const
{
    if (m_criteria.count()==0)
        return true;
    QValueList<Criterion>::ConstIterator it = m_criteria.begin();
    QValueList<Criterion>::ConstIterator end = m_criteria.end();
    for ( ; it != end; ++it ) {
        if ( !( *it ).satisfiedBy( a ) ) {
            return false;
        }
    }
    return true;
}

ArticleMatcher::Association ArticleMatcher::stringToAssociation(const QString& assocStr)
{
    if (assocStr == QString::fromLatin1("LogicalAnd"))
        return LogicalAnd;
    else if (assocStr == QString::fromLatin1("LogicalOr"))
        return LogicalOr;
    else
        return None;
}

QString ArticleMatcher::associationToString(Association association)
{
    switch (association)
    {
        case LogicalAnd:
            return QString::fromLatin1("LogicalAnd");
        case LogicalOr:
            return QString::fromLatin1("LogicalOr");
        default:
            return QString::fromLatin1("None");
    }
}


class TagMatcher::TagMatcherPrivate
{
    public:
    QString tagID;
    bool operator==(const TagMatcherPrivate& other) const
    {
        return tagID == other.tagID;
    }
};

TagMatcher::TagMatcher(const QString& tagID) : d(new TagMatcherPrivate)
{
    d->tagID = tagID;
}

TagMatcher::TagMatcher() : d(new TagMatcherPrivate)
{
}

TagMatcher::~TagMatcher()
{
    delete d;
    d = 0;
}

bool TagMatcher::matches(const Article& article) const
{
    return article.hasTag(d->tagID);
}

TagMatcher* TagMatcher::clone() const
{
    return new TagMatcher(*this);
}


TagMatcher::TagMatcher(const TagMatcher& other) : AbstractMatcher(other), d(0)
{
    *this = other;
}

void TagMatcher::writeConfig(KConfig* config) const
{
    config->writeEntry(QString::fromLatin1("matcherType"), QString::fromLatin1("TagMatcher"));
    config->writeEntry(QString::fromLatin1("matcherParams"), d->tagID);
}

void TagMatcher::readConfig(KConfig* config)
{
    d->tagID = config->readEntry(QString::fromLatin1("matcherParams"));
}

bool TagMatcher::operator==(const AbstractMatcher& other) const
{
    AbstractMatcher* ptr = const_cast<AbstractMatcher*>(&other);
    TagMatcher* tagFilter = dynamic_cast<TagMatcher*>(ptr);
    return tagFilter ? *d == *(tagFilter->d) : false;
}

bool TagMatcher::operator!=(const AbstractMatcher &other) const
{
    return !(*this == other);
}

TagMatcher& TagMatcher::operator=(const TagMatcher& other)
{
    d = new TagMatcherPrivate;
    *d = *(other.d);
    return *this;
}

void DeleteAction::exec(Article& article)
{
    if (!article.isNull())
        article.setDeleted();
}

SetStatusAction::SetStatusAction(int status) : m_status(status)
{
}
        
void SetStatusAction::exec(Article& article)
{
    if (!article.isNull())
        article.setStatus(m_status);
}

int SetStatusAction::status() const
{
    return m_status;
}

void SetStatusAction::setStatus(int status)
{
    m_status = status;
}

void SetStatusAction::writeConfig(KConfig* config) const
{
    config->writeEntry(QString::fromLatin1("actionType"), QString::fromLatin1("SetStatusAction"));
    config->writeEntry(QString::fromLatin1("actionParams"), m_status);
}

void SetStatusAction::readConfig(KConfig* config)
{
    m_status = config->readNumEntry(QString::fromLatin1("actionParams"), Article::Read);
}

bool SetStatusAction::operator==(const AbstractAction& other)
{
    AbstractAction* ptr = const_cast<AbstractAction*>(&other);
    SetStatusAction* o = dynamic_cast<SetStatusAction*>(ptr);
    if (!o)
        return false;
    else
        return m_status == o->m_status;
}

 
AssignTagAction::AssignTagAction(const QString& tagID) : m_tagID(tagID)
{
}

void AssignTagAction::exec(Article& article)
{
    if (!article.isNull())
        article.addTag(m_tagID);
}

class ArticleFilter::ArticleFilterPrivate : public Shared
{
    public:
    AbstractAction* action;
    AbstractMatcher* matcher;
    QString name;
    int id;
    
};

ArticleFilter::ArticleFilter() : d(new ArticleFilterPrivate)
{
    d->id = KApplication::random();
    d->action = 0;
    d->matcher = 0;
}

ArticleFilter::ArticleFilter(const AbstractMatcher& matcher, const AbstractAction& action) : d(new ArticleFilterPrivate)
{
    d->id = KApplication::random();
    d->matcher = matcher.clone();
    d->action = action.clone();
}

ArticleFilter::ArticleFilter(const ArticleFilter& other)
{
    *this = other;
}

ArticleFilter::~ArticleFilter()
{
    if (d->deref())
    {
        delete d->action;
        delete d->matcher;
        delete d;
        d = 0;
    }
    
}

AbstractMatcher* ArticleFilter::matcher() const
{
    return d->matcher;
}

AbstractAction* ArticleFilter::action() const
{
    return d->action;
}

void ArticleFilter::setMatcher(const AbstractMatcher& matcher)
{
    delete d->matcher;
    d->matcher = matcher.clone();
}

void ArticleFilter::setAction(const AbstractAction& action)
{
    delete d->action;
    d->action = action.clone();
}

ArticleFilter& ArticleFilter::operator=(const ArticleFilter& other)
{
    if (this != &other)
    {
        other.d->ref();
        if (d && d->deref())
            delete d;
        d = other.d;
    }
    return *this;
}

int ArticleFilter::id() const 
{
    return d->id;
}

bool ArticleFilter::operator==(const ArticleFilter& other) const
{
    return *(d->matcher) == *(other.d->matcher) && *(d->action) == *(other.d->action) && d->name == other.d->name;
}

void ArticleFilterList::writeConfig(KConfig* config) const
{
    config->setGroup(QString::fromLatin1("Filters"));
    config->writeEntry(QString::fromLatin1("count"), count());
    int index = 0;
    for (ArticleFilterList::ConstIterator it = begin(); it != end(); ++it)
    {
        config->setGroup(QString::fromLatin1("Filters_")+QString::number(index));
        (*it).writeConfig(config);
        ++index;
    }
}

void ArticleFilterList::readConfig(KConfig* config)
{
    clear();
    config->setGroup(QString::fromLatin1("Filters"));
    int count = config->readNumEntry(QString::fromLatin1("count"), 0);
    for (int i = 0; i < count; ++i)
    {
        config->setGroup(QString::fromLatin1("Filters_")+QString::number(i));
        ArticleFilter filter;
        filter.readConfig(config);
        append(filter);
    }
}


void AssignTagAction::readConfig(KConfig* config)
{
    m_tagID = config->readEntry(QString::fromLatin1("actionParams"));
}

void AssignTagAction::writeConfig(KConfig* config) const
{
    config->writeEntry(QString::fromLatin1("actionType"), QString::fromLatin1("AssignTagAction"));
    config->writeEntry(QString::fromLatin1("actionParams"), m_tagID);
}

bool AssignTagAction::operator==(const AbstractAction& other)
{
    AbstractAction* ptr = const_cast<AbstractAction*>(&other);
    AssignTagAction* o = dynamic_cast<AssignTagAction*>(ptr);
    if (!o)
        return false;
    else
        return m_tagID == o->m_tagID;
}

const QString& AssignTagAction::tagID() const
{
    return m_tagID;
}

void AssignTagAction::setTagID(const QString& tagID)
{
    m_tagID = tagID;
}

void DeleteAction::readConfig(KConfig* /*config*/)
{
}

void DeleteAction::writeConfig(KConfig* config) const
{
    config->writeEntry(QString::fromLatin1("actionType"), QString::fromLatin1("DeleteAction"));
}

bool DeleteAction::operator==(const AbstractAction& other)
{
    AbstractAction* ptr = const_cast<AbstractAction*>(&other);
    DeleteAction* o = dynamic_cast<DeleteAction*>(ptr);
    return o != 0;
}

void ArticleFilter::readConfig(KConfig* config)
{
    delete d->matcher;
    d->matcher = 0;
    delete d->action;
    d->action = 0;

    d->name = config->readEntry(QString::fromLatin1("name"));
    d->id = config->readNumEntry(QString::fromLatin1("id"), 0);

    QString matcherType = config->readEntry(QString::fromLatin1("matcherType"));

    if (matcherType == QString::fromLatin1("TagMatcher"))
        d->matcher = new TagMatcher();
    else if (matcherType == QString::fromLatin1("ArticleMatcher"))
        d->matcher = new ArticleMatcher();

    if (d->matcher)
        d->matcher->readConfig(config);


        QString actionType = config->readEntry(QString::fromLatin1("actionType"));

    if (actionType == QString::fromLatin1("AssignTagAction"))
        d->action = new AssignTagAction();
    else if (actionType == QString::fromLatin1("DeleteAction"))
        d->action = new DeleteAction();
    else if (actionType == QString::fromLatin1("SetStatusAction"))
        d->action = new SetStatusAction();

    if (d->action)
        d->action->readConfig(config);
}

void ArticleFilter::writeConfig(KConfig* config) const
{
    config->writeEntry(QString::fromLatin1("name"), d->name);
    config->writeEntry(QString::fromLatin1("id"), d->id);
    d->matcher->writeConfig(config);
    d->action->writeConfig(config);
}

void ArticleFilter::setName(const QString& name)
{
    d->name = name;
}

const QString& ArticleFilter::name() const
{
    return d->name;
}

void ArticleFilter::applyTo(Article& article) const
{
    if (d->matcher && d->action && d->matcher->matches(article))
        d->action->exec(article);
}
} //namespace Filters
} //namespace Akregator
