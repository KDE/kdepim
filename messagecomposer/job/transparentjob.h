/*
  Copyright (C) 2009 Klaralvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
  Copyright (c) 2009 Leo Franchi <lfranchi@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
*/

#ifndef MESSAGECOMPOSER_TRANSPARENTJOB_H
#define MESSAGECOMPOSER_TRANSPARENTJOB_H


#include "contentjobbase.h"
#include "messagecomposer_export.h"



namespace MessageComposer {

class TransparentJobPrivate;

/**
  A job that just wraps some KMime::Content into a job object
  for use as a subjob in another job.
 */
class MESSAGECOMPOSER_EXPORT TransparentJob : public MessageComposer::ContentJobBase
{
  Q_OBJECT

  public:
    explicit TransparentJob( QObject *parent = 0 );
    virtual ~TransparentJob();

    void setContent( KMime::Content* content );
    void process();
    
  private:
    Q_DECLARE_PRIVATE( TransparentJob )
};

}

#endif
