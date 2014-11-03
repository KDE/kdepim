/* -*- mode: c++; c-basic-offset:4 -*-
    utils/headerview.cpp

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

#include <config-kleopatra.h>

#include "headerview.h"

#include <algorithm>
#include <numeric>
#include <cassert>

#include <QDebug>

//#define ENABLE_HEADERVIEW_DEBUG

#ifdef ENABLE_HEADERVIEW_DEBUG
# define hvDebug qDebug
#else
# define hvDebug if ( true ) {} else qDebug
#endif

using namespace Kleo;

static std::vector<int> section_sizes(const QHeaderView *view)
{
    assert(view);
    std::vector<int> result;
    result.reserve(view->count());
    for (int i = 0, end = view->count() ; i != end ; ++i) {
        result.push_back(view->sectionSize(i));
    }
    return result;
}

static void apply_section_sizes(QHeaderView *view, const std::vector<int> &newSizes)
{
    assert(view);
    for (unsigned int i = 0, end = newSizes.size() ; i != end ; ++i) {
        view->resizeSection(i, newSizes[i]);
    }
}

namespace
{

template <typename T_container>
inline typename T_container::value_type lookup(const T_container &c, unsigned int i, const typename T_container::value_type &defaultValue)
{
    return i < c.size() ? c[i] : defaultValue ;
}

}

template <typename T, typename A>
QDebug operator<<(QDebug debug, const std::vector<T, A> &v)
{
    debug.nospace() << "std::vector(";
    for (typename std::vector<T, A>::size_type i = 0; i < v.size(); ++i) {
        if (i) {
            debug << ", ";
        }
        debug << v[i];
    }
    debug << ")";
    return debug.space();
}

class HeaderView::Private
{
    friend class ::Kleo::HeaderView;
    HeaderView *const q;
public:
    Private(HeaderView *qq)
        : q(qq),
          mousePressed(false),
          modes(),
          sizes()
    {
        connect(q, SIGNAL(sectionCountChanged(int,int)), q, SLOT(_klhv_slotSectionCountChanged(int,int)));
        connect(q, SIGNAL(sectionResized(int,int,int)), q, SLOT(_klhv_slotSectionResized(int,int,int)));
    }

    void _klhv_slotSectionCountChanged(int oldCount, int newCount)
    {
        if (newCount == oldCount) {
            return;
        }
        hvDebug() << oldCount << "->" << newCount;
        if (newCount < oldCount) {
            return;
        }
        ensureNumSections(newCount);
        for (unsigned int i = 0, end = std::min<unsigned int>(newCount, modes.size()) ; i < end ; ++i) {
            q->QHeaderView::setResizeMode(i, modes[i]);
        }
        apply_section_sizes(q, sizes);
    }

    void _klhv_slotSectionResized(int idx, int oldSize, int newSize)
    {
        hvDebug() << idx << ':' << oldSize << "->" << newSize;
        ensureNumSections(idx + 1);
        sizes[idx] = newSize;
    }

    void ensureNumSections(unsigned int num)
    {
        if (num > modes.size()) {
            modes.resize(num, QHeaderView::Interactive);
        }
        if (num > sizes.size()) {
            sizes.resize(num, q->defaultSectionSize());
        }
    }

    bool mousePressed : 1;
    std::vector<QHeaderView::ResizeMode> modes;
    std::vector<int> sizes;
};

HeaderView::HeaderView(Qt::Orientation o, QWidget *p)
    : QHeaderView(o, p), d(new Private(this))
{

}

HeaderView::~HeaderView() {}

#if 0
static std::vector<int> calculate_section_sizes(const std::vector<int> &oldSizes, int newLength, const std::vector<QHeaderView::ResizeMode> &modes, int minSize)
{

    if (oldSizes.empty()) {
        hvDebug() << "no existing sizes";
        return std::vector<int>();
    }

    int oldLength = 0, fixedLength = 0, stretchLength = 0;
    int numStretchSections = 0;
    for (unsigned int i = 0, end = oldSizes.size() ; i != end ; ++i) {
        oldLength += oldSizes[i];
        if (lookup(modes, i, QHeaderView::Fixed) == QHeaderView::Stretch) {
            stretchLength += oldSizes[i];
            ++numStretchSections;
        } else {
            fixedLength += oldSizes[i];
        }
    }

    if (oldLength <= 0) {
        hvDebug() << "no existing lengths - returning equidistant sizes";
        return std::vector<int>(oldSizes.size(), newLength / oldSizes.size());
    }

    const int stretchableSpace = std::max(newLength - fixedLength, 0);

    std::vector<int> newSizes;
    newSizes.reserve(oldSizes.size());
    for (unsigned int i = 0, end = oldSizes.size() ; i != end ; ++i)
        newSizes.push_back(std::max(minSize,
                                    lookup(modes, i, QHeaderView::Fixed) == QHeaderView::Stretch
                                    ? stretchLength ? stretchableSpace * oldSizes[i] / stretchLength : stretchableSpace / numStretchSections
                                    : oldSizes[i]));

    hvDebug() << "\noldSizes = " << oldSizes << "/" << oldLength
              << "\nnewSizes = " << newSizes << "/" << newLength;

    return newSizes;
}
#endif

void HeaderView::setSectionSizes(const std::vector<int> &sizes)
{
    hvDebug() << sizes;
    d->ensureNumSections(sizes.size());
    d->sizes = sizes;
    apply_section_sizes(this, sizes);
    hvDebug() << "->" << sectionSizes();
}

std::vector<int> HeaderView::sectionSizes() const
{
    return section_sizes(this);
}

void HeaderView::setSectionResizeMode(unsigned int section, ResizeMode mode)
{
    d->ensureNumSections(section + 1);
    d->modes[section] = mode;
    if (section < static_cast<unsigned int>(count())) {
        QHeaderView::setResizeMode(section, mode);
    }
}

#if 0
void HeaderView::setModel(QAbstractItemModel *model)
{

    hvDebug() << "before" << section_sizes(this);

    QHeaderView::setModel(model);

    hvDebug() << "after " << section_sizes(this);

}

void HeaderView::setRootIndex(const QModelIndex &idx)
{
    hvDebug() << "before" << section_sizes(this);
    QHeaderView::setRootIndex(idx);
    hvDebug() << "after " << section_sizes(this);
}

void HeaderView::mousePressEvent(QMouseEvent *e)
{
    d->mousePressed = true;
    QHeaderView::mousePressEvent(e);
}

void HeaderView::mouseReleaseEvent(QMouseEvent *e)
{
    d->mousePressed = false;
    QHeaderView::mouseReleaseEvent(e);
}

void HeaderView::updateGeometries()
{

    const std::vector<int> oldSizes = d->mousePressed ? section_sizes(this) : d->sizes ;

    hvDebug() << "before" << section_sizes(this) << '(' << d->sizes << ')';

    QHeaderView::updateGeometries();

    hvDebug() << "after " << section_sizes(this);

    const std::vector<int> newSizes = calculate_section_sizes(oldSizes, width(), d->modes, minimumSectionSize());
    d->sizes = newSizes;

    apply_section_sizes(this, newSizes);
}
#endif

#include "moc_headerview.cpp"
