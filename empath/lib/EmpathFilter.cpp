/*
    Empath - Mailer for KDE

    Copyright 1999, 2000
        Rik Hemsley <rik@kde.org>
        Wilco Greven <j.w.greven@student.utwente.nl>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


// KDE includes
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>

// Local includes
#include "EmpathMatcher.h"
#include "EmpathFilter.h"
#include "EmpathDefines.h"

EmpathFilter::EmpathFilter(const QString & name)
    :    priority_(0),
        fEventHandler_(0),
        name_(name)
{
    // Empty.
}

EmpathFilter::~EmpathFilter()
{
    // Empty.
}

    void
EmpathFilter::save()
{
    KConfig * config = KGlobal::config();

    config->setGroup(QString::fromUtf8("Filter_") + name_);

    config->writeEntry(QString::fromUtf8("MatchExpressions"), matchExprs_.count());
    config->writeEntry(QString::fromUtf8("SourceFolder"),    url_.asString());
    config->writeEntry(QString::fromUtf8("Priority"),  priority_);

    EmpathMatcherListIterator it(matchExprs_);

    int c = 0;

    for (; it.current() ; ++it)
        it.current()->save(name_, c++);

    if (fEventHandler_ != 0)
        fEventHandler_->save(name_);

    config->sync();
}

    void
EmpathFilter::load()
{
    KConfig * config = KGlobal::config();

    config->setGroup(QString::fromUtf8("Filter_") + name_);

    url_ = config->readEntry(QString::fromUtf8("SourceFolder"));

    Q_UINT32 numMatchExprs =
        config->readUnsignedNumEntry(QString::fromUtf8("MatchExpressions"));

    for (Q_UINT32 i = 0 ; i < numMatchExprs ; ++i)
        loadMatchExpr(i);

    loadEventHandler();

    priority_ =
        config->readUnsignedNumEntry(QString::fromUtf8("Priority"), 100);
}

    void
EmpathFilter::loadMatchExpr(Q_UINT32 matchExprID)
{
    EmpathMatcher * matcher = new EmpathMatcher;
    matcher->load(name_, matchExprID);
    matchExprs_.append(matcher);
}

    void
EmpathFilter::loadEventHandler()
{
    EmpathFilterEventHandler * handler = new EmpathFilterEventHandler;

    if (handler->load(name_)) {

        fEventHandler_ = handler;

    } else {

        fEventHandler_ = 0;
        delete handler;
    }
}

    void
EmpathFilter::setEventHandler(EmpathFilterEventHandler * handler)
{
    delete fEventHandler_;

    fEventHandler_ = handler;
}

    EmpathFilterEventHandler *
EmpathFilter::eventHandler()
{
    return fEventHandler_;
}

    EmpathURL
EmpathFilter::url() const
{
    return url_;
}

    void
EmpathFilter::setURL(const EmpathURL & url)
{
    url_ = url;
}

    QString
EmpathFilter::description() const
{
    if (fEventHandler_ == 0)
        return i18n("This filter does nothing");

    QString desc;

    desc += i18n("When new mail arrives in");
    desc += QString::fromUtf8(" ");
    desc += url_.asString();
    desc += QString::fromUtf8(", ");
    desc += actionDescription();

    return desc;
}

    QString
EmpathFilter::actionDescription() const
{
    if (fEventHandler_ == 0)
        return i18n("No action defined");
    else
        return fEventHandler_->description();
}

    QList<EmpathMatcher> *
EmpathFilter::matchExprList()
{
    return &matchExprs_;
}

    void
EmpathFilter::filter(const EmpathURL & id)
{
    if (fEventHandler_ == 0) {
        empathDebug(QString::fromUtf8("Event handler not defined"));
        return;
    }

    if (!match(id))
        return;

    fEventHandler_->handleMessage(id);
}

    bool
EmpathFilter::match(const EmpathURL & id)
{
    EmpathMatcherListIterator it(matchExprs_);

    for (; it.current(); ++it)
        if (it.current()->match(id))
            return true;

    return false;
}

// vim:ts=4:sw=4:tw=78
