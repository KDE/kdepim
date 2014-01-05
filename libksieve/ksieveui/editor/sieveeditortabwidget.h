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

public Q_SLOTS:
    void slotAddHelpPage(const QString &variableName, const QString &url);

protected:
    void tabRemoved(int index);
    void tabInserted(int index);

private Q_SLOTS:
    void slotTitleChanged(KSieveUi::SieveEditorHelpHtmlWidget *widget, const QString &title);
    void slotTabCloseRequested(int index);
    void slotProgressIndicatorPixmapChanged(KSieveUi::SieveEditorHelpHtmlWidget *widget, const QPixmap &pixmap);
    void slotLoadFinished(KSieveUi::SieveEditorHelpHtmlWidget *widget, bool success);
};
}

#endif // SIEVEEDITORTABWIDGET_H
