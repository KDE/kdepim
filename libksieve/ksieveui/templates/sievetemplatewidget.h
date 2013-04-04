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

#ifndef SIEVETEMPLATEWIDGET_H
#define SIEVETEMPLATEWIDGET_H

#include <QWidget>
#include <QListWidget>

#include "sievedefaulttemplate.h"

namespace KSieveUi {
class SieveTemplateListWidgetPrivate;
class SieveTemplateListWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit SieveTemplateListWidget(QWidget *parent = 0);
    ~SieveTemplateListWidget();

    virtual QList<KSieveUi::SieveDefaultTemplate::defaultTemplate> defaultTemplates();
protected:
    QStringList mimeTypes() const;
    QMimeData *mimeData( const QList<QListWidgetItem *> items ) const;

Q_SIGNALS:
    void insertTemplate(const QString &);

private:
    friend class SieveTemplateListWidgetPrivate;
    SieveTemplateListWidgetPrivate * const d;
    Q_PRIVATE_SLOT( d, void slotAdd() )
    Q_PRIVATE_SLOT( d, void slotRemove() )
    Q_PRIVATE_SLOT( d, void slotModify() )
    Q_PRIVATE_SLOT( d, void slotInsertTemplate() )
    Q_PRIVATE_SLOT( d, void slotContextMenu(const QPoint &pos) )

    enum SieveTemplateData {
        SieveText = Qt::UserRole + 1,
        DefaultTemplate = Qt::UserRole + 2
    };
};

class SieveTemplateWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SieveTemplateWidget(const QString &title, QWidget *parent = 0);
    ~SieveTemplateWidget();
Q_SIGNALS:
    void insertTemplate(const QString &);
private:
    SieveTemplateListWidget *mListTemplate;
};
}

#endif // SIEVETEMPLATEWIDGET_H
