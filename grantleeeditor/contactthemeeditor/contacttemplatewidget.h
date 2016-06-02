/*
   Copyright (C) 2013-2016 Montel Laurent <montel@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef CONTACTTEMPLATEWIDGET_H
#define CONTACTTEMPLATEWIDGET_H

#include <QWidget>
#include "PimCommon/TemplateListWidget"

class ContactTemplateListWidget : public PimCommon::TemplateListWidget
{
    Q_OBJECT
public:
    explicit ContactTemplateListWidget(const QString &configName, QWidget *parent = Q_NULLPTR);
    ~ContactTemplateListWidget();

    QVector<PimCommon::defaultTemplate> defaultTemplates() Q_DECL_OVERRIDE;
};

class ContactTemplateWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ContactTemplateWidget(const QString &title, QWidget *parent = Q_NULLPTR);
    ~ContactTemplateWidget();

Q_SIGNALS:
    void insertTemplate(const QString &);

private:
    ContactTemplateListWidget *mListTemplate;
};

#endif // CONTACTTEMPLATEWIDGET_H
