/*
  Copyright (c) 2014 Christian Mollekopf <mollekopf@kolabsys.com>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef CALENDARSUPPORT_TAGWIDGETS_H
#define CALENDARSUPPORT_TAGWIDGETS_H

#include "calendarsupport_export.h"
#include <Akonadi/Tag>
#include <QWidget>

class KJob;
namespace Akonadi {
  class TagWidget;
}

namespace CalendarSupport {

class CALENDARSUPPORT_EXPORT TagWidget: public QWidget
{
    Q_OBJECT
public:
    explicit TagWidget(QWidget* parent = 0);
    void setSelection(const QStringList &);
    QStringList selection() const;
signals:
    void selectionChanged(const QStringList &);

private slots:
    void onTagCreated(KJob*);
    void onSelectionChanged(const Akonadi::Tag::List &);

private:
    Akonadi::TagWidget *mTagWidget;
    Akonadi::Tag::List mTagList;
    QStringList mCachedTagNames;
};

}

#endif
