/*
    Copyright (c) 2005 by Volker Krause <volker.krause@rwth-aachen.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifndef KNODE_MAILSENDJOB_H
#define KNODE_MAILSENDJOB_H

#include "knjobdata.h"

class KJob;

namespace KNode {

/** Sends a mail to a SMTP server. */
class MailSendJob : public KNJobData
{
  Q_OBJECT
  public:
    MailSendJob( KNJobConsumer *c, KNServerInfo *a, KNJobItem *i );

    virtual void execute();

  private slots:
    void slotResult( KJob *job );
};

}
#endif
