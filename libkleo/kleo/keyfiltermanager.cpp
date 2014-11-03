/*
    keyfiltermanager.cpp

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

#include "keyfiltermanager.h"
#include "kconfigbasedkeyfilter.h"

#include "cryptobackendfactory.h"
#include "stl_util.h"

#include <kconfig.h>
#include <kconfiggroup.h>
#include <KLocalizedString>
#include <QIcon>
#include <QDebug>

#include <QCoreApplication>
#include <QRegExp>
#include <QStringList>
#include <QAbstractListModel>
#include <QModelIndex>

#include <boost/bind.hpp>
#include <boost/iterator/filter_iterator.hpp>

#include <algorithm>
#include <vector>
#include <climits>

using namespace Kleo;
using namespace boost;
using namespace GpgME;

namespace
{

class Model : public QAbstractListModel
{
    KeyFilterManager::Private *m_keyFilterManagerPrivate;
public:
    explicit Model(KeyFilterManager::Private *p)
        : QAbstractListModel(0), m_keyFilterManagerPrivate(p) {}

    /* reimp */ int rowCount(const QModelIndex &) const;
    /* reimp */ QVariant data(const QModelIndex &idx, int role) const;
    /* upgrade to public */ using QAbstractListModel::reset;
};

class AllCertificatesKeyFilter : public KeyFilterImplBase
{
public:
    AllCertificatesKeyFilter()
        : KeyFilterImplBase()
    {
        mSpecificity = UINT_MAX; // overly high for ordering
        mName = i18n("All Certificates");
        mId = QLatin1String("all-certificates");
        mMatchContexts = Filtering;
    }
};

class MyCertificatesKeyFilter : public KeyFilterImplBase
{
public:
    MyCertificatesKeyFilter()
        : KeyFilterImplBase()
    {
        mHasSecret = Set;
        mSpecificity = UINT_MAX - 1; // overly high for ordering

        mName = i18n("My Certificates");
        mId = QLatin1String("my-certificates");
        mMatchContexts = AnyMatchContext;
        mBold = true;
    }
};

class TrustedCertificatesKeyFilter : public KeyFilterImplBase
{
public:
    TrustedCertificatesKeyFilter()
        : KeyFilterImplBase()
    {
        mRevoked = NotSet;
        mValidity = IsAtLeast;
        mValidityReferenceLevel = UserID::Marginal; // Full?
        mSpecificity = UINT_MAX - 2; // overly high for ordering

        mName = i18n("Trusted Certificates");
        mId = QLatin1String("trusted-certificates");
        mMatchContexts = Filtering;
    }
};

class OtherCertificatesKeyFilter : public KeyFilterImplBase
{
public:
    OtherCertificatesKeyFilter()
        : KeyFilterImplBase()
    {
        mHasSecret = NotSet;
        mValidity = IsAtMost;
        mValidityReferenceLevel = UserID::Never;
        mSpecificity = UINT_MAX - 3; // overly high for ordering

        mName = i18n("Other Certificates");
        mId = QLatin1String("other-certificates");
        mMatchContexts = Filtering;
    }
};
}

static std::vector< shared_ptr<KeyFilter> > defaultFilters()
{
    std::vector<shared_ptr<KeyFilter> > result;
    result.reserve(3);
    result.push_back(shared_ptr<KeyFilter>(new MyCertificatesKeyFilter));
    result.push_back(shared_ptr<KeyFilter>(new TrustedCertificatesKeyFilter));
    result.push_back(shared_ptr<KeyFilter>(new OtherCertificatesKeyFilter));
    result.push_back(shared_ptr<KeyFilter>(new AllCertificatesKeyFilter));
    return result;
}

class KeyFilterManager::Private
{
public:
    Private() : filters(), model(this) {}
    void clear()
    {
        filters.clear();
        model.reset();
    }

    std::vector< shared_ptr<KeyFilter> > filters;
    Model model;
};

KeyFilterManager *KeyFilterManager::mSelf = 0;

KeyFilterManager::KeyFilterManager(QObject *parent)
    : QObject(parent), d(new Private)
{
    mSelf = this;
    // ### DF: doesn't a KStaticDeleter work more reliably?
    if (QCoreApplication *app = QCoreApplication::instance()) {
        connect(app, SIGNAL(aboutToQuit()), SLOT(deleteLater()));
    }
    reload();
}

KeyFilterManager::~KeyFilterManager()
{
    mSelf = 0;
    if (d) {
        d->clear();
    }
    delete d; d = 0;
}

KeyFilterManager *KeyFilterManager::instance()
{
    if (!mSelf) {
        mSelf = new KeyFilterManager();
    }
    return mSelf;
}

const shared_ptr<KeyFilter> &KeyFilterManager::filterMatching(const Key &key, KeyFilter::MatchContexts contexts) const
{
    const std::vector< shared_ptr<KeyFilter> >::const_iterator it
        = std::find_if(d->filters.begin(), d->filters.end(),
                       bind(&KeyFilter::matches, _1, cref(key), contexts));
    if (it != d->filters.end()) {
        return *it;
    }
    static const shared_ptr<KeyFilter> null;
    return null;
}

std::vector< shared_ptr<KeyFilter> > KeyFilterManager::filtersMatching(const Key &key, KeyFilter::MatchContexts contexts) const
{
    std::vector< shared_ptr<KeyFilter> > result;
    result.reserve(d->filters.size());
    std::remove_copy_if(d->filters.begin(), d->filters.end(),
                        std::back_inserter(result),
                        !bind(&KeyFilter::matches, _1, cref(key), contexts));
    return result;
}

namespace
{
struct ByDecreasingSpecificity : std::binary_function<shared_ptr<KeyFilter>, shared_ptr<KeyFilter>, bool> {
    bool operator()(const shared_ptr<KeyFilter> &lhs, const shared_ptr<KeyFilter> &rhs) const
    {
        return lhs->specificity() > rhs->specificity();
    }
};
}

void KeyFilterManager::reload()
{
    d->clear();

    d->filters = defaultFilters();

    if (KConfig *config = CryptoBackendFactory::instance()->configObject()) {
        const QStringList groups = config->groupList().filter(QRegExp(QLatin1String("^Key Filter #\\d+$")));
        for (QStringList::const_iterator it = groups.begin() ; it != groups.end() ; ++it) {
            const KConfigGroup cfg(config, *it);
            d->filters.push_back(shared_ptr<KeyFilter>(new KConfigBasedKeyFilter(cfg)));
        }
    }
    std::stable_sort(d->filters.begin(), d->filters.end(), ByDecreasingSpecificity());
    qDebug() << "final filter count is" << d->filters.size();
}

QAbstractItemModel *KeyFilterManager::model() const
{
    return &d->model;
}

const shared_ptr<KeyFilter> &KeyFilterManager::keyFilterByID(const QString &id) const
{
    const std::vector< shared_ptr<KeyFilter> >::const_iterator it
        = std::find_if(d->filters.begin(), d->filters.end(),
                       bind(&KeyFilter::id, _1) == id);
    if (it != d->filters.end()) {
        return *it;
    }
    static const shared_ptr<KeyFilter> null;
    return null;
}

const shared_ptr<KeyFilter> &KeyFilterManager::fromModelIndex(const QModelIndex &idx) const
{
    if (!idx.isValid() || idx.model() != &d->model || idx.row() < 0 ||
            static_cast<unsigned>(idx.row()) >= d->filters.size()) {
        static const shared_ptr<KeyFilter> null;
        return null;
    }
    return d->filters[idx.row()];
}

QModelIndex KeyFilterManager::toModelIndex(const shared_ptr<KeyFilter> &kf) const
{
    if (!kf) {
        return QModelIndex();
    }
    const std::pair <
    std::vector<shared_ptr<KeyFilter> >::const_iterator,
        std::vector<shared_ptr<KeyFilter> >::const_iterator
        > pair = std::equal_range(d->filters.begin(), d->filters.end(), kf, ByDecreasingSpecificity());
    const std::vector<shared_ptr<KeyFilter> >::const_iterator it
        = std::find(pair.first, pair.second, kf);
    if (it != pair.second) {
        return d->model.index(it - d->filters.begin());
    } else {
        return QModelIndex();
    }
}

int Model::rowCount(const QModelIndex &) const
{
    return m_keyFilterManagerPrivate->filters.size();
}

QVariant Model::data(const QModelIndex &idx, int role) const
{
    if ((role != Qt::DisplayRole && role != Qt::EditRole &&
            role != Qt::ToolTipRole && role != Qt::DecorationRole) ||
            !idx.isValid() || idx.model() != this ||
            idx.row() < 0 || static_cast<unsigned>(idx.row()) >  m_keyFilterManagerPrivate->filters.size()) {
        return QVariant();
    }
    if (role == Qt::DecorationRole) {
        return m_keyFilterManagerPrivate->filters[idx.row()]->icon();
    } else {
        return m_keyFilterManagerPrivate->filters[idx.row()]->name();
    }
}

namespace
{
template <typename C, typename P1, typename P2>
typename C::const_iterator find_if_and(const C &c, P1 p1, P2 p2)
{
    return std::find_if(boost::make_filter_iterator(p1, boost::begin(c), boost::end(c)),
                        boost::make_filter_iterator(p1, boost::end(c), boost::end(c)),
                        p2).base();
}
}

QFont KeyFilterManager::font(const Key &key, const QFont &baseFont) const
{
    return kdtools::accumulate_transform_if(d->filters, mem_fn(&KeyFilter::fontDesription),
                                            bind(&KeyFilter::matches, _1, key, KeyFilter::Appearance),
                                            KeyFilter::FontDescription(),
                                            bind(&KeyFilter::FontDescription::resolve, _1, _2)).font(baseFont);
}

static QColor get_color(const std::vector< shared_ptr<KeyFilter> > &filters, const Key &key, QColor(KeyFilter::*fun)() const)
{
    const std::vector< shared_ptr<KeyFilter> >::const_iterator it
        = find_if_and(filters,
                      bind(&KeyFilter::matches, _1, key, KeyFilter::Appearance),
                      bind(&QColor::isValid, bind(fun, _1)));
    if (it == filters.end()) {
        return QColor();
    } else {
        return (it->get()->*fun)();
    }
}

static QString get_string(const std::vector< shared_ptr<KeyFilter> > &filters, const Key &key, QString(KeyFilter::*fun)() const)
{
    const std::vector< shared_ptr<KeyFilter> >::const_iterator it
        = find_if_and(filters,
                      bind(&KeyFilter::matches, _1, key, KeyFilter::Appearance),
                      !bind(&QString::isEmpty, bind(fun, _1)));
    if (it == filters.end()) {
        return QString();
    } else {
        return (*it)->icon();
    }
}

QColor KeyFilterManager::bgColor(const Key &key) const
{
    return get_color(d->filters, key, &KeyFilter::bgColor);
}

QColor KeyFilterManager::fgColor(const Key &key) const
{
    return get_color(d->filters, key, &KeyFilter::fgColor);
}

QIcon KeyFilterManager::icon(const Key &key) const
{
    const QString icon = get_string(d->filters, key, &KeyFilter::icon);
    return icon.isEmpty() ? QIcon() : QIcon::fromTheme(icon) ;
}

