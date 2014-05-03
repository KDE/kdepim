/*
    Copyright (c) 2014 Jonathan Marten <jjm@keelhaul.me.uk>

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

#ifndef CATEGORYSELECTWIDGET_H
#define CATEGORYSELECTWIDGET_H

#include <qwidget.h>
#include <AkonadiCore/tag.h>

class CategorySelectWidgetPrivate;

/**
 * @short A widget to specify a category (tag) filter.
 *
 * @since 4.14
 * @author Jonathan Marten
 **/

class CategorySelectWidget : public QWidget
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(CategorySelectWidget);

public:
    /**
     * Constructor.
     *
     * @param parent The parent widget
     **/
    explicit CategorySelectWidget(QWidget *parent = 0);

    /**
     * Destructor.
     **/
    virtual ~CategorySelectWidget();

    /**
     * Special @c Akonadi::Tag::Id values for filtering.
     **/
    enum FilterTag
    {
        FilterAll = -2,					/**< All items */
        FilterUntagged = -3,				/**< Untagged items */
        FilterGroups = -4				/**< Contact groups */
    };

    /**
     * Get the current tag filter list.
     *
     * @return The filter list, as would be sent by @c filterChanged()
     *
     * @see filterChanged
     **/
    QList<Akonadi::Tag::Id> filterTags() const;

signals:
    /**
     * The tag filter selection has changed.
     *
     * @param idList A list of @c Akonadi::Tag::Id's of the tags which
     * are to be included in the filter.
     *
     * @see CategorySelectModel::filterChanged
     **/
    void filterChanged(const QList<Akonadi::Tag::Id> &idList);

private:
    CategorySelectWidgetPrivate * const d_ptr;
};

#endif							// CATEGORYSELECTWIDGET_H
