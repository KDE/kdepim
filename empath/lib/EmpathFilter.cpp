/*
    Empath - Mailer for KDE
    
    Copyright (C) 1998, 1999 Rik Hemsley rik@kde.org
    
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

#ifdef __GNUG__
# pragma implementation "EmpathFilter.h"
#endif

// KDE includes
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>

// Local includes
#include "EmpathMatcher.h"
#include "EmpathFilter.h"
#include "EmpathDefines.h"
#include "EmpathConfig.h"

EmpathFilter::EmpathFilter(const QString & name)
    :    priority_(0),
        fEventHandler_(0),
        name_(name)
{
    empathDebug("ctor");
}

EmpathFilter::~EmpathFilter()
{
    empathDebug("dtor");
}

    void
EmpathFilter::save()
{
    empathDebug("save() called");
    
    KConfig * config = KGlobal::config();
    config->setGroup(EmpathConfig::GROUP_FILTER + name_);
    
    config->writeEntry(EmpathConfig::KEY_NUM_MATCH_EXPRS_FOR_FILTER,
        matchExprs_.count());
    
    empathDebug("Saving url name == \"" + url_.asString() + "\"");

    config->writeEntry(EmpathConfig::KEY_FILTER_FOLDER, url_.asString());
    
    empathDebug("Matchers to save: " + QString().setNum(matchExprs_.count()));
    
    empathDebug("Saving priority == " + QString().setNum(priority_));
    config->writeEntry(EmpathConfig::KEY_FILTER_PRIORITY, priority_);
    
    EmpathMatcherListIterator it(matchExprs_);
    
    int c = 0;
    
    for (; it.current() ; ++it)
        it.current()->save(name_, c++);

    empathDebug("Saving event handler");
    if (fEventHandler_ != 0)
        fEventHandler_->save(name_);
    
    config->sync();
}

    void
EmpathFilter::load()
{
    KConfig * config = KGlobal::config();
    config->setGroup(EmpathConfig::GROUP_FILTER + name_);
    
    url_ = config->readEntry(EmpathConfig::KEY_FILTER_FOLDER);
    empathDebug("My url is \"" + url_.asString() + "\"");
    
    Q_UINT32 numMatchExprs =
        config->readUnsignedNumEntry(
            EmpathConfig::KEY_NUM_MATCH_EXPRS_FOR_FILTER);
    
    empathDebug("There are " + QString().setNum(numMatchExprs) +
        " match expressions that act for this filter");

    for (Q_UINT32 i = 0 ; i < numMatchExprs ; ++i)
        loadMatchExpr(i);

    empathDebug("Loading event handler");
    loadEventHandler();
    
    priority_ =
        config->readUnsignedNumEntry(EmpathConfig::KEY_FILTER_PRIORITY, 100);
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
    empathDebug("url() called");
    return url_;
}

    void
EmpathFilter::setURL(const EmpathURL & url)
{
    empathDebug("setFolder(" + url.asString() + ") called");
    url_ = url;
}

    QString
EmpathFilter::description() const
{
    if (fEventHandler_ == 0)
        return i18n("This filter does nothing");

    QString desc;
    
    desc += i18n("When new mail arrives in");
    desc += " ";
    desc += url_.asString();
    desc += ", ";
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
    empathDebug("filter() called");
    
    if (fEventHandler_ == 0) {
        empathDebug("I have no event handler (action) defined");
        return;
    }
    
    if (!match(id)) {
        empathDebug("Didn't match this message");
        return;
    }
    
    fEventHandler_->handleMessage(id);
}

    bool
EmpathFilter::match(const EmpathURL & id)
{
    empathDebug("match(" + QString(id.asString()) + ") called");
    
    empathDebug("There are " + QString().setNum(matchExprs_.count()) +
        " match expressions to try");

    EmpathMatcherListIterator it(matchExprs_);
    
    for (; it.current(); ++it)
        if (it.current()->match(id)) {
            empathDebug("Matched message !");
            return true;
        }
    
    return false;
}

// vim:ts=4:sw=4:tw=78
