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

// Qt includes
#include <qvaluelist.h>
#include <qlayout.h>
#include <qsplitter.h>

// KDE includes
#include <kconfig.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kglobal.h>
#include <kaction.h>
#include <kinstance.h>

// Local includes
#include "EmpathBrowser.h"
#include "EmpathCustomEvents.h"
#include "Empath.h"

extern "C"
{
    void *init_libEmpathBrowser()
    {
        return new EmpathBrowserPartFactory;
    }
}

KInstance * EmpathBrowserPartFactory::instance_ = 0L;

EmpathBrowserPartFactory::EmpathBrowserPartFactory()
{
    // Empty.
}

EmpathBrowserPartFactory::~EmpathBrowserPartFactory()
{
    delete instance_;
    instance_ = 0L;
}

    QObject *
EmpathBrowserPartFactory::create(
    QObject * parent,
    const char * name,
    const char *,
    const QStringList &
)
{
    QObject * o = new EmpathBrowserPart((QWidget *)parent, name);
    emit objectCreated(o);
    return o;
}

    KInstance *
EmpathBrowserPartFactory::instance()
{
    if (0 == instance_)
        instance_ = new KInstance("EmpathBrowser");

    return instance_;
}

// -------------------------------------------------------------------------

EmpathBrowserPart::EmpathBrowserPart(
    QWidget * parent,
    const char * name
)
    :   KParts::ReadWritePart(parent, name)
{
    setInstance(EmpathBrowserPartFactory::instance());

    widget_ = new EmpathBrowser(parent);
    widget_->setFocusPolicy(QWidget::StrongFocus);
    setWidget(widget_);

    insertChildClient(widget_->folderListPart_);
    insertChildClient(widget_->messageListPart_);
    insertChildClient(widget_->messageViewPart_);

    setXMLFile("EmpathBrowser.rc");

    connect(
        this,                       SIGNAL(showFolder(const EmpathURL &)),
        widget_->messageListPart_,  SLOT(s_showFolder(const EmpathURL &)));

    connect(
        this,                       SIGNAL(showFolder(const EmpathURL &)),
        widget_->folderListPart_,   SLOT(s_setActiveFolder(const EmpathURL &)));
}

EmpathBrowserPart::~EmpathBrowserPart()
{
    // Empty.
}

    void
EmpathBrowserPart::s_showFolder(const EmpathURL & url)
{
    emit(showFolder(url));
}

// -------------------------------------------------------------------------


EmpathBrowser::EmpathBrowser(QWidget * parent)
    : QWidget(parent, "Browser")
{
    QSplitter * hSplit = new QSplitter(this, "hSplit");

    KLibFactory * folderListFactory =
        KLibLoader::self()->factory("libEmpathFolderListWidget");

    if (0 != folderListFactory) {

        folderListPart_ =
            static_cast<KParts::ReadWritePart *>
                (
                    folderListFactory->create(
                        hSplit,
                        "folder list part",
                        "KParts::ReadWritePart"
                    )
                );

    } else {
        
        empathDebug("Argh. Can't load a folder list part.");
        return;
    }

    QSplitter * vSplit = new QSplitter(Qt::Vertical, hSplit, "vSplit");

    KLibFactory * messageListFactory =
        KLibLoader::self()->factory("libEmpathMessageListWidget");
    
    KLibFactory * messageViewFactory =
        KLibLoader::self()->factory("libEmpathMessageViewWidget");


    if (0 != messageListFactory) {

        messageListPart_ =
            static_cast<KParts::ReadWritePart *>
                (
                    messageListFactory->create(
                        vSplit,
                        "message list part",
                        "KParts::ReadWritePart"
                    )
                );

    } else {
        
        empathDebug("Argh. Can't load a message list part.");
        return;
    }

    if (0 != messageViewFactory) {

        messageViewPart_ =
            static_cast<KParts::ReadOnlyPart *>
            (
                messageViewFactory->create(
                    vSplit,
                    "message view part",
                    "KParts::ReadOnlyPart"
                )
            );

    } else {

        empathDebug("Argh. Can't load a message viewing part.");
        return;
    }

    connect(
        folderListPart_,  SIGNAL(showFolder(const EmpathURL &)),
        messageListPart_, SLOT(s_showFolder(const EmpathURL &)));

    connect(
        messageListPart_, SIGNAL(showMessage(const EmpathURL &)),
        messageViewPart_, SLOT(s_showMessage(const EmpathURL &)));

    folderListPart_->widget()->setFocus();

    (new QVBoxLayout(this))->addWidget(hSplit);
}

EmpathBrowser::~EmpathBrowser()
{
    // Empty.
}

// vim:ts=4:sw=4:tw=78
