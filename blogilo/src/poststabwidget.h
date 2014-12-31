/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#ifndef POSTSTABWIDGET_H
#define POSTSTABWIDGET_H

#include <QTabWidget>
class QToolButton;

class PostsTabWidget : public QTabWidget
{
    Q_OBJECT
public:
    explicit PostsTabWidget(QWidget *parent);
    ~PostsTabWidget();

Q_SIGNALS:
    void createNewPost();
    void closeTabClicked();
    void tabRemoveAllExclude(int);

private Q_SLOTS:
    void slotTabContextMenuRequest(const QPoint &);

private:
    QToolButton *mNewTabButton;
    QToolButton *mCloseTabButton;
};

#endif // POSTSTABWIDGET_H
