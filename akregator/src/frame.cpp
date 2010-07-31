/*
    This file is part of Akregator.

    Copyright (C) 2004 Sashmit Bhaduri <smt@vfemail.net>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <tqregexp.h>
#include <tqstylesheet.h>

#include <kactioncollection.h>
#include <kdebug.h>
#include <klocale.h>
#include <kparts/browserextension.h>
#include <kparts/part.h>

#include <libkdepim/progressmanager.h>

#include "frame.h"

namespace Akregator {

Frame::Frame(TQObject * parent, KParts::ReadOnlyPart *p, TQWidget *visWidget, const TQString& tit, bool watchSignals)
   :TQObject(parent, "aKregatorFrame")
{
    m_autoDeletePart = false;
    m_part=p;
    m_widget=visWidget;
    m_title=tit;
    m_state=Idle;
    m_progress=-1;
    m_progressItem=0;

    if (watchSignals) // e.g, articles tab has no part
    {
        connect(m_part, TQT_SIGNAL(setWindowCaption (const TQString &)), this, TQT_SLOT(setCaption (const TQString &)));
        connect(m_part, TQT_SIGNAL(setStatusBarText (const TQString &)), this, TQT_SLOT(setStatusText (const TQString &)));

        KParts::BrowserExtension *ext=KParts::BrowserExtension::childObject( p );
        if (ext)
            connect( ext, TQT_SIGNAL(loadingProgress(int)), this, TQT_SLOT(setProgress(int)) );

        connect(p, TQT_SIGNAL(started(KIO::Job*)), this, TQT_SLOT(setStarted()));
        connect(p, TQT_SIGNAL(completed()), this, TQT_SLOT(setCompleted()));
        connect(p, TQT_SIGNAL(canceled(const TQString &)), this, TQT_SLOT(setCanceled(const TQString&)));
        connect(p, TQT_SIGNAL(completed(bool)), this, TQT_SLOT(setCompleted()));

/*        KActionCollection *coll=p->actionCollection();
        if (coll)
        {
            connect( coll, TQT_SIGNAL( actionStatusText( const TQString & ) ),
             this, TQT_SLOT( slotActionStatusText( const TQString & ) ) );
            connect( coll, TQT_SIGNAL( clearStatusText() ),
             this, TQT_SLOT( slotClearStatusText() ) );
        }
*/
    }
}

void Frame::setAutoDeletePart(bool autoDelete)
{
    m_autoDeletePart = autoDelete;
}

Frame::~Frame()
{
    if(m_progressItem) 
    {
        m_progressItem->setComplete();
    }
    if (m_autoDeletePart)
        m_part->deleteLater();
}

int Frame::state() const
{
    return m_state;
}

KParts::ReadOnlyPart *Frame::part() const
{
    return m_part;
}

TQWidget *Frame::widget() const
{
    return m_widget;
}

void Frame::setTitle(const TQString &s)
{
    if (m_title != s)
    {
        m_title = s;
        emit titleChanged(this, s);
    }
}

void Frame::setCaption(const TQString &s)
{
    if(m_progressItem) m_progressItem->setLabel(s);
    m_caption=s;
    emit captionChanged(s);
}

void Frame::setStatusText(const TQString &s)
{
    m_statusText=s;
    m_statusText.replace(TQRegExp("<[^>]*>"), "");
    emit statusText(m_statusText);
}

void Frame::setProgress(int a)
{
    if(m_progressItem) {
        m_progressItem->setProgress((int)a);
    }
    m_progress=a;
    emit loadingProgress(a);
}

void Frame::setState(int a)
{
    m_state=a;
    
    switch (m_state)
    {
        case Frame::Started:
            emit started();
            break;
        case Frame::Canceled:
            emit canceled(TQString::null);
            break;
        case Frame::Idle:
        case Frame::Completed:
        default:
            emit completed();
    }}



const TQString& Frame::title() const
{
    return m_title;
}

const TQString& Frame::caption() const
{
    return m_caption;
}

const TQString& Frame::statusText() const
{
    return m_statusText;
}

void Frame::setStarted()
{
    if(m_progressId.isNull() || m_progressId.isEmpty()) m_progressId = KPIM::ProgressManager::getUniqueID();
    m_progressItem = KPIM::ProgressManager::createProgressItem(m_progressId, TQStyleSheet::escape( title() ), TQString::null, false);
    m_progressItem->setStatus(i18n("Loading..."));
    //connect(m_progressItem, TQT_SIGNAL(progressItemCanceled(KPIM::ProgressItem*)), TQT_SLOT(slotAbortFetch()));
    m_state=Started;
    emit started();
}

void Frame::setCanceled(const TQString &s)
{
    if(m_progressItem) {
        m_progressItem->setStatus(i18n("Loading canceled"));
        m_progressItem->setComplete();
        m_progressItem = 0;
    }
    m_state=Canceled;
    emit canceled(s);
}

void Frame::setCompleted()
{
    if(m_progressItem) {
        m_progressItem->setStatus(i18n("Loading completed"));
        m_progressItem->setComplete();
        m_progressItem = 0;
    }
    m_state=Completed;
    emit completed();
}

int Frame::progress() const
{
    return m_progress;
}

} // namespace Akregator

#include "frame.moc"
