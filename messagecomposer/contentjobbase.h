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

#ifndef MESSAGECOMPOSER_CONTENTJOBBASE_H
#define MESSAGECOMPOSER_CONTENTJOBBASE_H

#include "jobbase.h"
#include "messagecomposer_export.h"

namespace KMime {
  class Content;
}

namespace MessageComposer {

class ContentJobBasePrivate;

class MESSAGECOMPOSER_EXPORT ContentJobBase : public JobBase
{
  Q_OBJECT

  public:
    explicit ContentJobBase( QObject *parent = 0 );
    virtual ~ContentJobBase();

    /**
      Starts processing this ContentJobBase asynchronously.  
      This processes all children in order first, then calls process().
      Emits finished() after all processing is done, and the
      content is reachable through content().
    */
    virtual void start();

    /**
      Get the resulting KMime::Content that the ContentJobBase has generated.
      Jobs never delete their content.
    */
    KMime::Content *content() const;

    /**
      This is meant to be used instead of KCompositeJob::addSubjob(), making
      it possible to add subjobs from the outside.
      Transfers ownership of the @p job to this object.
    */
    bool appendSubjob( ContentJobBase *job );

    /**
      Set some extra content to be saved with the job, and available
        later, for example, in slot handling result of job.
      Job does not take care of deleting extra content.
      */
    void setExtraContent( KMime::Content* extra );

    /**
      Get extra content that was previously added.
     */
    KMime::Content* extraContent() const;

  protected:
    ContentJobBase( ContentJobBasePrivate &dd, QObject *parent );

    /** Use appendSubjob() instead. */
    virtual bool addSubjob( KJob *job );

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
    Q_DECLARE_PRIVATE( ContentJobBase )
};

} // namespace MessageComposer

#endif
