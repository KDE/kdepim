/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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

#ifndef BALOODEBUGWIDGET_H
#define BALOODEBUGWIDGET_H

#include <QWidget>
#include "pimcommon_export.h"
#include "baloodebugsearchpathcombobox.h"
#include <Akonadi/Item>
class KLineEdit;
class QPushButton;
namespace PimCommon {
class PlainTextEditorWidget;
class PIMCOMMON_EXPORT BalooDebugWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BalooDebugWidget(QWidget *parent = 0);
    ~BalooDebugWidget();

    void setAkonadiId(Akonadi::Item::Id akonadiId);
    void setSearchType(BalooDebugSearchPathComboBox::SearchType type);
    void doSearch();
private Q_SLOTS:
    void slotSearchLineTextChanged(const QString &text);
    void slotSearch();
    void slotResult(const QString &result);
    void slotError(const QString &errorStr);

private:
    PimCommon::PlainTextEditorWidget *mPlainTextEditor;
    PimCommon::BalooDebugSearchPathComboBox *mSearchPathComboBox;
    KLineEdit *mLineEdit;
    QPushButton *mSearchButton;
};
}


#endif // BALOODEBUGWIDGET_H

