/* -*- C++ -*-
   This file implements the base class for kab´s looks..

   the KDE addressbook

   $ Author: Mirko Boehm $
   $ Copyright: (C) 1996-2001, Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org $
   $ License: GPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Troll Tech, Norway. $

   $Revision$
*/

#include "look_basic.h"


KABBasicLook::KABBasicLook(QWidget* parent, const char* name)
    : QWidget(parent, name),
      m_ro(false)
{
}

void KABBasicLook::setReadonly(bool state)
{

    m_ro=state;
}

bool KABBasicLook::readonly() const
{
    return m_ro;
}

void KABBasicLook::setEntry(const KABC::Addressee& e)
{
  current=e;
  repaint(false);
}

KABC::Addressee KABBasicLook::entry()
{
    return current;
}

void KABBasicLook::configure(KConfig*)
{
}

KABLookFactory::KABLookFactory(QWidget* parent_, const char* name_)
    : parent(parent_), name(name_)
{
}

KABLookFactory::~KABLookFactory()
{
}

#include "look_basic.moc"
