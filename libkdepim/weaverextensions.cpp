/* -*- C++ -*-

   This file implements the Weaver Extensions basics.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 2004, Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Troll Tech, Norway. $

*/

#include "weaverextensions.h"
#include "weaver.h"

namespace KPIM {
namespace ThreadWeaver {

    WeaverExtension::WeaverExtension ( TQObject *parent, const char *name)
        : TQObject (parent, name)
    {
    }

    void WeaverExtension::attach (Weaver *w)
    {
        connect (w, TQT_SIGNAL (threadCreated (Thread *) ),
                 TQT_SLOT (threadCreated (Thread *) ) );
        connect (w, TQT_SIGNAL (threadDestroyed (Thread *) ),
                 TQT_SLOT (threadDestroyed (Thread *) ) );
        connect (w, TQT_SIGNAL (threadBusy (Thread *) ),
                 TQT_SLOT (threadBusy (Thread *) ) );
        connect (w, TQT_SIGNAL (threadSuspended (Thread *) ),
                 TQT_SLOT (threadSuspended (Thread *) ) );
    }

    WeaverExtension::~WeaverExtension()
    {
    }

    void WeaverExtension::threadCreated (Thread *)
    {
    }

    void WeaverExtension::threadDestroyed (Thread *)
    {
    }

    void WeaverExtension::threadBusy (Thread *)
    {
    }

    void WeaverExtension::threadSuspended (Thread *)
    {
    }

}
}

#include "weaverextensions.moc"
