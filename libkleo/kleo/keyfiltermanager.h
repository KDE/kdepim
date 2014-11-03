/*
    keyfiltermanager.h

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004 Klarälvdalens Datakonsult AB

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

#ifndef __KLEO_KEYFILTERMANAGER_H__
#define __KLEO_KEYFILTERMANAGER_H__

#include "kleo_export.h"
#include <QtCore/QObject>

#include <boost/shared_ptr.hpp>

#include <kleo/keyfilter.h>

#include <vector>

namespace GpgME
{
class Key;
}

class QAbstractItemModel;
class QModelIndex;
class QFont;
class QColor;
class QIcon;

namespace Kleo
{

class KLEO_EXPORT KeyFilterManager : public QObject
{
    Q_OBJECT
protected:
    explicit KeyFilterManager(QObject *parent = 0);
    ~KeyFilterManager();

public:
    static KeyFilterManager *instance();

    const boost::shared_ptr<KeyFilter> &filterMatching(const GpgME::Key &key, KeyFilter::MatchContexts contexts) const;
    std::vector< boost::shared_ptr<KeyFilter> > filtersMatching(const GpgME::Key &key, KeyFilter::MatchContexts contexts) const;

    QAbstractItemModel *model() const;

    const boost::shared_ptr<KeyFilter> &keyFilterByID(const QString &id) const;
    const boost::shared_ptr<KeyFilter> &fromModelIndex(const QModelIndex &mi) const;
    QModelIndex toModelIndex(const boost::shared_ptr<KeyFilter> &kf) const;

    void reload();

    QFont font(const GpgME::Key &key, const QFont &baseFont) const;
    QColor bgColor(const GpgME::Key &key) const;
    QColor fgColor(const GpgME::Key &key) const;
    QIcon icon(const GpgME::Key &key) const;

    class Private;
private:
    Private *d;
    static KeyFilterManager *mSelf;
};

}

#endif // __KLEO_KEYFILTERMANAGER_H__
