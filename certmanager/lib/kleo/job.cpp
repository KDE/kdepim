/*  -*- mode: C++; c-file-style: "gnu" -*-
    job.cpp

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License,
    version 2, as published by the Free Software Foundation.

    Libkleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "job.h"

#include "keylistjob.h"
#include "encryptjob.h"
#include "decryptjob.h"
#include "signjob.h"
#include "verifydetachedjob.h"
#include "verifyopaquejob.h"
#include "keygenerationjob.h"
#include "importjob.h"
#include "exportjob.h"
#include "downloadjob.h"
#include "deletejob.h"

Kleo::Job::Job( QObject * parent, const char * name )
  : QObject( parent, name )
{

}

Kleo::Job::~Job() {

}




#define make_job_subclass(x) \
  Kleo::x##Job::x##Job( QObject * parent, const char * name ) : Job( parent, name ) {} \
  Kleo::x##Job::~x##Job() {}

make_job_subclass(KeyList)
make_job_subclass(Encrypt)
make_job_subclass(Decrypt)
make_job_subclass(Sign)
make_job_subclass(VerifyDetached)
make_job_subclass(VerifyOpaque)
make_job_subclass(KeyGeneration)
make_job_subclass(Import)
make_job_subclass(Export)
make_job_subclass(Download)
make_job_subclass(Delete)

#undef make_job_subclass

#include "job.moc"

#include "keylistjob.moc"
#include "encryptjob.moc"
#include "decryptjob.moc"
#include "signjob.moc"
#include "verifydetachedjob.moc"
#include "verifyopaquejob.moc"
#include "keygenerationjob.moc"
#include "importjob.moc"
#include "exportjob.moc"
#include "downloadjob.moc"
#include "deletejob.moc"
