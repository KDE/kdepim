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
#include "EmpathView.h"
#include "EmpathCustomEvents.h"
#include "Empath.h"
#include "EmpathMessageListWidget.h"
#include "EmpathFolderListWidget.h"

extern "C"
{
    void *init_libEmpathView()
    {
        return new EmpathViewPartFactory;
    }
}

KInstance * EmpathViewPartFactory::instance_ = 0L;

EmpathViewPartFactory::EmpathViewPartFactory()
{
    // Empty.
}

EmpathViewPartFactory::~EmpathViewPartFactory()
{
    delete instance_;
    instance_ = 0L;
}

    QObject *
EmpathViewPartFactory::create(
    QObject * parent,
    const char * name,
    const char *,
    const QStringList &
)
{
    QObject * o = new EmpathViewPart((QWidget *)parent, name);
    emit objectCreated(o);
    return o;
}

    KInstance *
EmpathViewPartFactory::instance()
{
    if (0 == instance_)
        instance_ = new KInstance("EmpathView");

    return instance_;
}

// -------------------------------------------------------------------------

EmpathViewPart::EmpathViewPart(
    QWidget * parent,
    const char * name
)
    :   KParts::ReadWritePart(parent, name)
{
    setInstance(EmpathViewPartFactory::instance());

    widget_ = new EmpathView(parent);
    widget_->setFocusPolicy(QWidget::StrongFocus);
    setWidget(widget_);

    insertChildClient(widget_->messageViewPart_);

    setXMLFile("EmpathView.rc");

    connect(
        this,                       SIGNAL(showFolder(const EmpathURL &)),
        widget_->messageListWidget_,  SLOT(s_showFolder(const EmpathURL &)));

//    connect(
//        this,                       SIGNAL(showFolder(const EmpathURL &)),
//        widget_->folderListWidget_, SLOT(s_setActiveFolder(const EmpathURL &)));

    connect(
        widget_->messageListWidget_,SIGNAL(messageActivated(const EmpathURL &)),
        widget_->messageViewPart_,  SLOT(s_changeView(const EmpathURL &)));
}

EmpathViewPart::~EmpathViewPart()
{
    // Empty.
}

    void
EmpathViewPart::s_showFolder(const EmpathURL & url)
{
    empathDebug("showFolder(" + url.asString() + ")");
    emit(showFolder(url));
}

// -------------------------------------------------------------------------


EmpathView::EmpathView(QWidget * parent)
    : QWidget(parent, "View")
{
//    QSplitter * hSplit = new QSplitter(this, "hSplit");

//    folderListWidget_ = new EmpathFolderListWidget(hSplit);

//    QSplitter * vSplit = new QSplitter(Qt::Vertical, hSplit, "vSplit");

    QSplitter * vSplit = new QSplitter(Qt::Vertical, this, "vSplit");

    KLibFactory * messageViewFactory =
        KLibLoader::self()->factory("libEmpathMessageViewWidget");

    messageListWidget_ = new EmpathMessageListWidget(vSplit);

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

//    connect(
//        folderListWidget_,  SIGNAL(showFolder(const EmpathURL &)),
//        messageListWidget_, SLOT(s_showFolder(const EmpathURL &)));

    connect(
        messageListWidget_, SIGNAL(showMessage(const EmpathURL &)),
        messageViewPart_,   SLOT(s_showMessage(const EmpathURL &)));

//    folderListWidget_->setFocus();
    messageListWidget_->setFocus();

//    (new QVBoxLayout(this))->addWidget(hSplit);
    (new QVBoxLayout(this))->addWidget(vSplit);
}

EmpathView::~EmpathView()
{
    // Empty.
}

// vim:ts=4:sw=4:tw=78
