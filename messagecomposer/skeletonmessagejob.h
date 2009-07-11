/*
  Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>
  Based on ideas by Stephen Kelly.

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

#ifndef MESSAGECOMPOSER_SKELETONMESSAGEJOB_H
#define MESSAGECOMPOSER_SKELETONMESSAGEJOB_H

#include "job.h"
#include "messagecomposer_export.h"

namespace KMime {
  class Message;
}

namespace MessageComposer {

class SkeletonMessageJobPrivate;
class InfoPart;

/**
  A message containing only the headers...
*/
class MESSAGECOMPOSER_EXPORT SkeletonMessageJob : public Job
{
  Q_OBJECT

  public:
    explicit SkeletonMessageJob( InfoPart *infoPart = 0, QObject *parent = 0 );
    virtual ~SkeletonMessageJob();

    InfoPart *infoPart() const;
    void setInfoPart( InfoPart *part );

    KMime::Message *message() const;
    // TODO I think there is a way in C++ to make content() private, even if it's public
    // in the base class.

  protected Q_SLOTS:
    virtual void process();

  private:
    Q_DECLARE_PRIVATE( SkeletonMessageJob )
};

} // namespace MessageComposer

#endif
