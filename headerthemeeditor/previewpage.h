/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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


#ifndef PREVIEWPAGE_H
#define PREVIEWPAGE_H

#include <QWidget>
class KPushButton;

namespace MessageViewer {
class Viewer;
}

class PreviewPage : public QWidget
{
    Q_OBJECT
public:
    explicit PreviewPage(QWidget *parent = 0);
    ~PreviewPage();

public Q_SLOTS:
    void slotUpdateViewer();

private:
    MessageViewer::Viewer *mViewer;
    KPushButton *mUpdate;
};

#endif // PREVIEWPAGE_H
