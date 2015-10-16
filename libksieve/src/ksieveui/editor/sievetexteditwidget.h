/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#ifndef SIEVETEXTEDITWIDGET_H
#define SIEVETEXTEDITWIDGET_H

#include <QWidget>
#include "ksieveui_export.h"
namespace KPIMTextEdit
{
class SlideContainer;
class PlainTextEditFindBar;
}

namespace KSieveUi
{
class SieveTextEdit;
class SieveTextEditWidgetPrivate;
class KSIEVEUI_EXPORT SieveTextEditWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SieveTextEditWidget(QWidget *parent = Q_NULLPTR);
    SieveTextEditWidget(KSieveUi::SieveTextEdit *customTextEdit, QWidget *parent);
    ~SieveTextEditWidget();
    void setReadOnly(bool readOnly);

    KSieveUi::SieveTextEdit *textEdit() const;

public Q_SLOTS:
    void slotReplace();
    void slotFind();

private:
    void initialize(KSieveUi::SieveTextEdit *custom = Q_NULLPTR);
    SieveTextEditWidgetPrivate *const d;
    KSieveUi::SieveTextEdit *mTextEdit;
    KPIMTextEdit::SlideContainer *mSliderContainer;
    KPIMTextEdit::PlainTextEditFindBar *mFindBar;
};
}
#endif // SIEVETEXTEDITWIDGET_H
