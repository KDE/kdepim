/*
  Copyright (c) 2005 Rafal Rzepecki <divide@users.sourceforge.net>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "categoryhierarchyreader.h"
#include "categoryconfig.h"

#include <KComboBox>

#include <QTreeWidget>

using namespace CalendarSupport;

inline QString &quote(QString &string)
{
    Q_ASSERT(CategoryConfig::categorySeparator != QLatin1String("@"));
    return string.replace(QLatin1Char('@'), QLatin1String("@0")).replace(QLatin1Char('\\') +
            CategoryConfig::categorySeparator,
            QLatin1String("@1"));
}

inline QStringList &unquote(QStringList &strings)
{
    return
        strings.replaceInStrings(QLatin1String("@1"), CategoryConfig::categorySeparator).
        replaceInStrings(QLatin1String("@0"), QLatin1String("@"));
}

QStringList CategoryHierarchyReader::path(QString string)
{
    QStringList _path =
        quote(string).split(CategoryConfig::categorySeparator, QString::SkipEmptyParts);
    return unquote(_path);
}

void CategoryHierarchyReader::read(QStringList categories)
{
    clear();
    QStringList::Iterator it;

    // case insensitive sort
    QMap<QString, QString> map;
    foreach (const QString &str, categories) {
        map.insert(str.toLower(), str);
    }

    categories = map.values();

    QStringList last_path;
    for (it = categories.begin(); it != categories.end(); ++it) {
        QStringList _path = path(*it);

        // we need to figure out where last item and the new one differ
        QStringList::Iterator jt, kt;
        int split_level = 0;
        QStringList new_path = _path; // save it for later
        for (jt = _path.begin(), kt = last_path.begin();
                jt != _path.end() && kt != last_path.end(); ++jt, ++kt) {
            if (*jt == *kt) {
                split_level++;
            } else {
                break; // now we have first non_equal component in the iterators
            }
        }

        // make a path relative to the shared ancestor
        if (jt != _path.begin()) {
            _path.erase(_path.begin(), jt);
        }
        last_path = new_path;

        if (_path.isEmpty()) {
            // something is wrong, we already have this node
            continue;
        }

        // find that ancestor
        while (split_level < depth()) {
            goUp();
        }
        Q_ASSERT(split_level == depth());

        // make the node and any non-existent ancestors
        while (!_path.isEmpty()) {
            addChild(_path.first(), QVariant(*it));
            _path.pop_front();
        }
    }
}

void CategoryHierarchyReaderQComboBox::clear()
{
    mBox->clear();
}

void CategoryHierarchyReaderQComboBox::goUp()
{
    mCurrentDepth--;
}

void CategoryHierarchyReaderQComboBox::addChild(const QString &label, const QVariant &userData)
{
    QString spaces;
    spaces.fill(QLatin1Char(' '), 2 * mCurrentDepth);
    mBox->addItem(spaces + label, userData);
    mCurrentDepth++;
}

int CategoryHierarchyReaderQComboBox::depth() const
{
    return mCurrentDepth;
}

#ifndef QT_NO_TREEWIDGET

void CategoryHierarchyReaderQTreeWidget::clear()
{
    mTree->clear();
}

void CategoryHierarchyReaderQTreeWidget::goUp()
{
    Q_ASSERT(mItem);
    mItem = mItem->parent();
    --mCurrentDepth;
}

void CategoryHierarchyReaderQTreeWidget::addChild(const QString &label, const QVariant &userData)
{
    Q_UNUSED(userData);

    if (mItem) {
        mItem = new QTreeWidgetItem(mItem, QStringList() << label);
    } else {
        mItem = new QTreeWidgetItem(mTree, QStringList() << label);
    }

    mItem->setExpanded(true);
    ++mCurrentDepth;
}

int CategoryHierarchyReaderQTreeWidget::depth() const
{
    return mCurrentDepth;
}
#endif
