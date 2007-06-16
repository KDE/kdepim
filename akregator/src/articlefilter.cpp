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
#include "types.h"

#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <krandom.h>
#include <kurl.h>

#include <QList>
#include <QRegExp>


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
    m_subject = stringToSubject(config->readEntry(QString::fromLatin1("subject"), QString()));
    m_predicate = stringToPredicate(config->readEntry(QString::fromLatin1("predicate"), QString()));
    QVariant::Type type = QVariant::nameToType(config->readEntry(QString::fromLatin1("objType"), QString()).toAscii());

    if (type != QVariant::Invalid)
    {
        m_object = config->readEntry(QString::fromLatin1("objectValue"), QVariant(type) );
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
        case Link:
            // ### Maybe use prettyUrl here?
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
            satisfied = concreteSubject.toString().indexOf( m_object.toString(), 0, Qt::CaseInsensitive ) != -1;
            break;
        case Equals:
            if (subjectType=="int")
                satisfied = concreteSubject.toInt() == m_object.toInt();
            else
                satisfied = concreteSubject.toString() == m_object.toString();
            break;
        case Matches:
            satisfied = QRegExp( m_object.toString() ).indexIn( concreteSubject.toString() ) != -1;
            break;
        default:
            kDebug() << "Internal inconsistency; predicateType should never be Negation" << endl;
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

ArticleMatcher* ArticleMatcher::clone() const
{
    return new ArticleMatcher(*this);
}

ArticleMatcher::ArticleMatcher( const QList<Criterion> &criteria, Association assoc)
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

    for (QList<Criterion>::ConstIterator it = m_criteria.begin(); it != m_criteria.end(); ++it)
    {
        config->setGroup(config->group()+QString::fromLatin1("_Criterion")+QString::number(index));
        (*it).writeConfig(config);
        ++index;
    }
}

void ArticleMatcher::readConfig(KConfig* config)
{
    m_criteria.clear();
    m_association = stringToAssociation(config->readEntry(QString::fromLatin1("matcherAssociation"), QString()));

    int count =  config->readEntry(QString::fromLatin1("matcherCriteriaCount"), 0);
    
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
    QList<Criterion>::ConstIterator it = m_criteria.begin();
    QList<Criterion>::ConstIterator end = m_criteria.end();
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
    QList<Criterion>::ConstIterator it = m_criteria.begin();
    QList<Criterion>::ConstIterator end = m_criteria.end();
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


} //namespace Filters
} //namespace Akregator
