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

#ifndef MESSAGECOMPOSER_JOB_H
#define MESSAGECOMPOSER_JOB_H

#include "messagecomposer_export.h"

#include <QtCore/QList>

#include <KDE/KCompositeJob>

namespace KMime {
  class Content;
}

namespace MessageComposer {

class Composer;
class JobPrivate;

class MESSAGECOMPOSER_EXPORT Job : public KCompositeJob
{
  Q_OBJECT

  public:
    typedef QList<Job*> List;

    enum Error
    {
      BugError = UserDefinedError + 1,
      IncompleteError,
      UserCancelledError,
      UserError = UserDefinedError + 42
    };

    explicit Job( QObject *parent = 0 );
    virtual ~Job();

    /**
      Starts processing this Job asynchronously.  
      This processes all children in order first, then calls process().
      Emits finished() after all processing is done, and the
      content is reachable through content().
    */
    virtual void start();

    /**
      Get the resulting KMime::Content that the Job has generated.
      Jobs never delete their content.
    */
    KMime::Content *content() const;

  protected:
    JobPrivate *const d_ptr;
    Job( JobPrivate &dd, QObject *parent );

  protected Q_SLOTS:
    /**
      Reimplement to do additional stuff before processing children, such as
      adding more subjobs.  Remember to call the base implementation.
    */
    virtual void doStart();

    /**
      This is called after all the children have been processed.
      (You must use their resulting contents, or delete them.)
      Reimplement in subclasses to process concrete content.  Call
      emitResult() when finished.
    */
    virtual void process() = 0;

    /* reimpl */
    virtual void slotResult( KJob *job );

  private:
    friend class Composer;
    void setComposer( Composer *composer );

    Q_DECLARE_PRIVATE( Job )
};

} // namespace MessageComposer

#endif
