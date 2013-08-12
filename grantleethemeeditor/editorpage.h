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

#ifndef GRANTLEEEDITOREDITORPAGE_H
#define GRANTLEEEDITOREDITORPAGE_H

#include <QWidget>
#include "grantleethemeeditor_export.h"

namespace GrantleeThemeEditor {
class GRANTLEETHEMEEDITOR_EXPORT EditorPage : public QWidget
{
    Q_OBJECT
public:
    enum PageType {
        MainPage = 0,
        ExtraPage = 1
    };
    explicit EditorPage(GrantleeThemeEditor::EditorPage::PageType type, QWidget *parent=0);
    ~EditorPage();

Q_SIGNALS:
    void needUpdateViewer();
    void changed();

protected:
    PageType mType;
};
}

#endif // GRANTLEEEDITOREDITOREDITORPAGE_H
