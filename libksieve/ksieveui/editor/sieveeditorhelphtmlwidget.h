/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#ifndef SIEVEEDITORHELPHTMLWIDGET_H
#define SIEVEEDITORHELPHTMLWIDGET_H

#include <QWidget>

namespace KSieveUi {
class SieveEditorWebView;
class SieveEditorLoadProgressIndicator;
class SieveEditorHelpHtmlWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SieveEditorHelpHtmlWidget(QWidget *parent=0);
    ~SieveEditorHelpHtmlWidget();

    void setHelp(const QString &variableName, const QString &url);
    QString variableName() const;

Q_SIGNALS:
    void titleChanged(KSieveUi::SieveEditorHelpHtmlWidget *widget, const QString &title);
    void progressIndicatorPixmapChanged(KSieveUi::SieveEditorHelpHtmlWidget *widget, const QPixmap &);
    void loadFinished(KSieveUi::SieveEditorHelpHtmlWidget *widget, bool success);

private slots:
    void slotTitleChanged(const QString &);
    void slotFinished(bool b);
    void slotLoadStarted();
    void slotPixmapChanged(const QPixmap &pixmap);
    void slotLoadFinished(bool success);
private:
    QString mVariableName;
    SieveEditorWebView *mWebView;
    SieveEditorLoadProgressIndicator *mProgressIndicator;
};
}

#endif // SIEVEEDITORHELPHTMLWIDGET_H
