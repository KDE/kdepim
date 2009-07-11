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

#ifndef MESSAGECOMPOSER_MAINTEXTJOB_H
#define MESSAGECOMPOSER_MAINTEXTJOB_H

#include "job.h"
#include "messagecomposer_export.h"

namespace MessageComposer {

class MainTextJobPrivate;
class TextPart;

class MESSAGECOMPOSER_EXPORT MainTextJob : public Job
{
  Q_OBJECT

  public:
    explicit MainTextJob( TextPart *textPart = 0, QObject *parent = 0 );
    virtual ~MainTextJob();

    TextPart *textPart() const;
    void setTextPart( TextPart *part );

  protected Q_SLOTS:
    virtual void doStart();
    virtual void process();

  private:
    Q_DECLARE_PRIVATE( MainTextJob )
};

} // namespace MessageComposer

#endif
