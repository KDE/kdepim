/*
    Twister - PIM app for KDE

    Copyright 2000
        Rik Hemsley <rik@kde.org>
    
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

// Qt includes
#include <qvaluelist.h>
#include <qlayout.h>
#include <qsplitter.h>
#include <qlabel.h>

// KDE includes
#include <kconfig.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kglobal.h>
#include <kaction.h>
#include <kinstance.h>

// Local includes
#include "TwisterBrowser.h"
#include "TwisterSideBar.h"
#include "TwisterPartHolder.h"
#include "Twister.h"

extern "C"
{
    void *init_libTwisterBrowser()
    {
        return new TwisterBrowserPartFactory;
    }
}

KInstance * TwisterBrowserPartFactory::instance_ = 0L;

TwisterBrowserPartFactory::TwisterBrowserPartFactory()
{
    // Empty.
}

TwisterBrowserPartFactory::~TwisterBrowserPartFactory()
{
    delete instance_;
    instance_ = 0L;
}

    QObject *
TwisterBrowserPartFactory::create(
    QObject * parent,
    const char * name,
    const char *,
    const QStringList &
)
{
    QObject * o = new TwisterBrowserPart((QWidget *)parent, name);
    emit objectCreated(o);
    return o;
}

    KInstance *
TwisterBrowserPartFactory::instance()
{
    if (0 == instance_)
        instance_ = new KInstance("TwisterBrowser");

    return instance_;
}

// -------------------------------------------------------------------------

TwisterBrowserPart::TwisterBrowserPart(
    QWidget * parent,
    const char * name
)
    :   KParts::ReadWritePart(parent, name)
{
    setInstance(TwisterBrowserPartFactory::instance());

    widget_ = new TwisterBrowser(parent);
    widget_->setFocusPolicy(QWidget::StrongFocus);
    setWidget(widget_);
    setXMLFile("TwisterBrowser.rc");
    enableAllActions(false);
}

TwisterBrowserPart::~TwisterBrowserPart()
{
    // Empty.
}

    void
TwisterBrowserPart::enableAllActions(bool)
{
    // STUB
}

    void
TwisterBrowserPart::_initActions()
{
}

// -------------------------------------------------------------------------


TwisterBrowser::TwisterBrowser(QWidget * parent)
    : QWidget(parent, "TwisterBrowser")
{
    QSplitter * hSplit = new QSplitter(this, "hSplit");

    TwisterSideBar * sidebar = new TwisterSideBar(hSplit);
    TwisterPartHolder * partHolder = new TwisterPartHolder(hSplit);

    QObject::connect(
        sidebar,    SIGNAL(switchWidget(const QString &)),
        partHolder, SLOT(s_switchWidget(const QString &)));

    _connectUp();

    (new QVBoxLayout(this))->addWidget(hSplit);
}

TwisterBrowser::~TwisterBrowser()
{
    // Empty.
}

    void
TwisterBrowser::_connectUp()
{
}


// vim:ts=4:sw=4:tw=78
