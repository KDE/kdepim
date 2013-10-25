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

#ifndef TEMPLATELISTWIDGET_H
#define TEMPLATELISTWIDGET_H

#include "pimcommon/pimcommon_export.h"
#include <QListWidget>

namespace PimCommon {
class TemplateListWidgetPrivate;

struct defaultTemplate {
    QString name;
    QString text;
};

class PIMCOMMON_EXPORT TemplateListWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit TemplateListWidget(const QString &configName, QWidget *parent = 0);
    ~TemplateListWidget();

    //Need to load template in specific class to allow to use correct defaultTemplates function
    void loadTemplates();

    virtual QList<PimCommon::defaultTemplate> defaultTemplates();
    virtual bool addNewTemplate(QString &templateName, QString &templateScript);
    virtual bool modifyTemplate(QString &templateName, QString &templateScript, bool defaultTemplate);

protected:
    QStringList mimeTypes() const;
    QMimeData *mimeData( const QList<QListWidgetItem *> items ) const;

    void dropEvent ( QDropEvent * event );

Q_SIGNALS:
    void insertTemplate(const QString &);
    void insertNewTemplate(const QString &);
private:
    friend class TemplateListWidgetPrivate;
    TemplateListWidgetPrivate * const d;
    Q_PRIVATE_SLOT( d, void slotAdd() )
    Q_PRIVATE_SLOT( d, void slotRemove() )
    Q_PRIVATE_SLOT( d, void slotModify() )
    Q_PRIVATE_SLOT( d, void slotInsertTemplate() )
    Q_PRIVATE_SLOT( d, void slotContextMenu(const QPoint &pos) )
    Q_PRIVATE_SLOT( d, void slotInsertNewTemplate(const QString&) )
    Q_PRIVATE_SLOT( d, void slotExportTemplates() )
    Q_PRIVATE_SLOT( d, void slotImportTemplates() )
    Q_PRIVATE_SLOT( d, void slotDuplicate() )
    enum TemplateData {
        Text = Qt::UserRole + 1,
        DefaultTemplate = Qt::UserRole + 2
    };
};
}

#endif // TEMPLATELISTWIDGET_H
