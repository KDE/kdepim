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


#ifndef GRAMMARCONFIGUREWIDGET_H
#define GRAMMARCONFIGUREWIDGET_H

#include "grammar_export.h"
#include <QWidget>
class KConfig;
namespace Grammar
{
class GrammarConfigureWidgetPrivate;
class GRAMMAR_EXPORT GrammarConfigureWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GrammarConfigureWidget(KConfig *config, QWidget *parent=0);
    ~GrammarConfigureWidget();

    void setDefault();
    void setLanguage(const QString &lang);
    void save();

private:
    friend class GrammarConfigureWidgetPrivate;
    GrammarConfigureWidgetPrivate * const d;
};
}

#endif // GRAMMARCONFIGUREWIDGET_H
