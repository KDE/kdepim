/*
  Copyright (c) 2013, 2014 Montel Laurent <montel.org>

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


#ifndef CUSTOMTREEVIEW_H
#define CUSTOMTREEVIEW_H

#include <QTreeWidget>
#include "pimcommon_export.h"
class QPaintEvent;
namespace PimCommon {
class PIMCOMMON_EXPORT CustomTreeView : public QTreeWidget
{
    Q_OBJECT
public:
    explicit CustomTreeView(QWidget *parent = 0);
    ~CustomTreeView();

    void setDefaultText(const QString &text);

    bool showDefaultText() const;
    void setShowDefaultText(bool b);

private Q_SLOTS:
    void slotGeneralPaletteChanged();
    void slotGeneralFontChanged();

protected:
    void paintEvent( QPaintEvent *event );

protected:
    bool mShowDefaultText;

private:
    QColor mTextColor;
    QString mDefaultText;
};
}

#endif
