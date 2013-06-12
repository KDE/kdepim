/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>

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

#ifndef PAGECOLORBACKGROUNDWIDGET_H
#define PAGECOLORBACKGROUNDWIDGET_H

#include <QWidget>
class KUrl;

namespace Ui {
class PageColorBackgroundWidget;
}

namespace ComposerEditorNG
{
class PageColorBackgroundWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit PageColorBackgroundWidget(QWidget *parent = 0);
    ~PageColorBackgroundWidget();
    
    QColor pageBackgroundColor() const;
    void setPageBackgroundColor(const QColor &);

    QColor textColor() const;
    void setTextColor(const QColor &);

    QColor linkColor() const;
    void setLinkColor(const QColor &col);

    QColor activeLinkColor() const;
    void setActiveLinkColor(const QColor &col);

    QColor visitedLinkColor() const;
    void setVisitedLinkColor(const QColor &col);


    void setUseDefaultColor(bool b);
    bool useDefaultColor() const;

    KUrl backgroundImageUrl() const;
    void setBackgroundImageUrl(const KUrl& url);

private:
    Ui::PageColorBackgroundWidget *ui;
};
}

#endif // PAGECOLORBACKGROUNDWIDGET_H
