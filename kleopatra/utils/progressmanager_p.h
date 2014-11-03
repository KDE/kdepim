/* -*- mode: c++; c-basic-offset:4 -*-
    utils/progressmanager_p.h

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2008 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

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

#ifndef __KLEOPATRA_UTILS_PROGRESSMANAGER_P_H_
#define __KLEOPATRA_UTILS_PROGRESSMANAGER_P_H_

#include "utils/progressmanager.h"

#include <libkdepim/progresswidget/progressmanager.h>

#include <kleo/job.h>

#include <QPointer>

namespace
{

class Mapper : QObject
{
    Q_OBJECT
public:
    Mapper(KPIM::ProgressItem *item, Kleo::Job *job)
        : QObject(item), m_item(item), m_job(job)
    {

        connect(item, SIGNAL(progressItemCanceled(KPIM::ProgressItem *)),
                job,  SLOT(slotCancel()));

        connect(job,  SIGNAL(done()),
                this, SLOT(slotDone()));
        connect(job,  SIGNAL(progress(QString, int, int)),
                this, SLOT(slotProgress(QString, int, int)));
    }

private Q_SLOTS:
    void slotDone()
    {
        if (m_item) {
            m_item->setComplete();
        }
        m_item = 0;
    }

    void slotProgress(const QString &what, int current, int total)
    {
        if (!m_item) {
            return;
        }
        m_item->setStatus(what);
        m_item->setProgress(total ? current / total : 0);
    }

private:
    QPointer<KPIM::ProgressItem> m_item;
    QPointer<Kleo::Job> m_job;
};

}

#endif // __KLEOPATRA_UTILS_PROGRESSMANAGER_P_H_
