/*
    qgpgmelistallkeysjob.h

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004,2008 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    Libkleopatra is distributed in the hope that it will be useful,
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

#ifndef __KLEO_QGPGMELISTALLKEYSJOB_H__
#define __KLEO_QGPGMELISTALLKEYSJOB_H__

#include "kleo/listallkeysjob.h"

#include "threadedjobmixin.h"

#include <gpgme++/keylistresult.h>
#include <gpgme++/key.h>

namespace Kleo
{

class QGpgMEListAllKeysJob
#ifdef Q_MOC_RUN
    : public ListAllKeysJob
#else
    : public _detail::ThreadedJobMixin<ListAllKeysJob, boost::tuple<GpgME::KeyListResult, std::vector<GpgME::Key>, std::vector<GpgME::Key>, QString, GpgME::Error> >
#endif
{
    Q_OBJECT
#ifdef Q_MOC_RUN
public Q_SLOTS:
    void slotFinished();
#endif
public:
    explicit QGpgMEListAllKeysJob(GpgME::Context *context);
    ~QGpgMEListAllKeysJob();

    /*! \reimp from ListAllKeysJob */
    GpgME::Error start(bool mergeKeys);

    /*! \reimp from ListAllKeysJob */
    GpgME::KeyListResult exec(std::vector<GpgME::Key> &pub, std::vector<GpgME::Key> &sec, bool mergeKeys);

    /*! \reimp from Job */
    void showErrorDialog(QWidget *parent, const QString &caption) const;

    /*! \reimp from ThreadedJobMixin */
    void resultHook(const result_type &result);

private:
    GpgME::KeyListResult mResult;
};

}

#endif // __KLEO_QGPGMELISTALLKEYSJOB_H__
