/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2005 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifndef KNFILTERCONFIGWIDGET_H
#define KNFILTERCONFIGWIDGET_H

#include <QTabWidget>

namespace KNode {
  class RangeFilterWidget;
  class SearchDialog;
  class StatusFilterWidget;
  class StringFilterWidget;
}

/** Filter configuration widget. */
class KNFilterConfigWidget : public QTabWidget
{
  Q_OBJECT

  friend class KNFilterDialog;
  friend class KNode::SearchDialog;

  public:
    explicit KNFilterConfigWidget( QWidget *parent = 0 );
    ~KNFilterConfigWidget();

    void reset();

    /// useablity hack for the search dialog
    void setStartFocus();

  protected:
    KNode::StatusFilterWidget *status;
    KNode::StringFilterWidget *subject;
    KNode::StringFilterWidget *from;
    KNode::StringFilterWidget *messageId;
    KNode::StringFilterWidget *references;
    KNode::RangeFilterWidget *age;
    KNode::RangeFilterWidget *lines;
    KNode::RangeFilterWidget *score;
};

#endif
