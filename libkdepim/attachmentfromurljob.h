/*
  Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

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

#ifndef KDEPIM_ATTACHMENTFROMURLJOB_H
#define KDEPIM_ATTACHMENTFROMURLJOB_H

#include "attachmentloadjob.h"
#include "kdepim_export.h"

#include <KDE/KUrl>

namespace KPIM {

/**
*/
class KDEPIM_EXPORT AttachmentFromUrlJob : public AttachmentLoadJob
{
  Q_OBJECT

  public:
    explicit AttachmentFromUrlJob( const KUrl &url = KUrl(), QObject *parent = 0 );
    virtual ~AttachmentFromUrlJob();

    KUrl url() const;
    void setUrl( const KUrl &url );
    qint64 maximumAllowedSize() const;
    void setMaximumAllowedSize( qint64 size );

  protected slots:
    virtual void doStart();

  private:
    class Private;
    friend class Private;
    Private *const d;

    Q_PRIVATE_SLOT( d, void transferJobData( KIO::Job*, QByteArray ) )
    Q_PRIVATE_SLOT( d, void transferJobResult( KJob* ) )
};

} // namespace KPIM

#endif
