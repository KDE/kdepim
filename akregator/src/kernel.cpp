/*
    This file is part of Akregator.

    Copyright (C) 2005 Frank Osterfeld <frank.osterfeld at kdemail.net>

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

#include "kernel.h"

#include <kstaticdeleter.h>

#include "fetchqueue.h"
#include "tagset.h"

namespace Akregator
{

Kernel* Kernel::m_self = 0;

static KStaticDeleter<Kernel> kernelsd;

Kernel* Kernel::self()
{
    if (!m_self)
        m_self = kernelsd.setObject(m_self, new Kernel);
    return m_self;
}

Kernel::Kernel()
{
    m_fetchQueue = new FetchQueue();
    m_tagSet = new TagSet();
    m_storage = 0;
    m_feedList = 0;
}
 
Kernel::~Kernel()
{
    delete m_fetchQueue;
}
         

}
