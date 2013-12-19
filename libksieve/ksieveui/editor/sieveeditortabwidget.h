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


#ifndef SIEVEEDITORTABWIDGET_H
#define SIEVEEDITORTABWIDGET_H

#include <KTabWidget>

namespace KSieveUi {
class SieveEditorHelpHtmlWidget;
class SieveEditorTabWidget : public KTabWidget
{
    Q_OBJECT
public:
    explicit SieveEditorTabWidget(QWidget *parent=0);
    ~SieveEditorTabWidget();

public slots:
    void slotAddHelpPage(const QString &url);

protected:
    void tabRemoved(int index);
    void tabInserted(int index);

private slots:
    void slotTitleChanged(KSieveUi::SieveEditorHelpHtmlWidget *widget, const QString &title);
    void slotTabCloseRequested(int index);
};
}

#endif // SIEVEEDITORTABWIDGET_H
