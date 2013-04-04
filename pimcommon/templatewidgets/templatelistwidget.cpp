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

#include "templatelistwidget.h"
#include "templateeditdialog.h"

#include <KConfigGroup>
#include <KMessageBox>
#include <KLocale>
#include <KMenu>


#include <QListWidgetItem>
#include <QPointer>
#include <QMimeData>
#include <QDebug>

namespace PimCommon {

class TemplateListWidgetPrivate
{
public:
    TemplateListWidgetPrivate(const QString &configName, TemplateListWidget *qq)
        : dirty(false), config(KSharedConfig::openConfig(configName, KConfig::NoGlobals)), q(qq)
    {
    }
    ~TemplateListWidgetPrivate()
    {
        saveTemplates();
    }
    void createListWidgetItem(const QString &name, const QString &text, bool isDefaultTemplate)
    {
        QListWidgetItem *item = new QListWidgetItem(name, q);
        item->setData(TemplateListWidget::Text, text);
        item->setData(TemplateListWidget::DefaultTemplate, isDefaultTemplate);
    }

    void slotAdd()
    {
        QString templateName;
        QString templateScript;
        if (q->addNewTemplate(templateName, templateScript) ) {
            createListWidgetItem(templateName, templateScript, false);
            dirty = true;
        }
    }

    void slotRemove()
    {
        if(KMessageBox::Yes == KMessageBox::questionYesNo(q, i18n("Do you want to delete selected template?"), i18n("Delete template"))) {
            const QList<QListWidgetItem *> lstSelectedItems = q->selectedItems();
            Q_FOREACH (QListWidgetItem *item, lstSelectedItems) {
                if (item->data(TemplateListWidget::DefaultTemplate).toBool() == false)
                    delete item;
            }
            dirty = true;
        }
    }

    void slotModify()
    {
        QListWidgetItem * item = q->currentItem();
        if(item) {
            const bool defaultTemplate = item->data(TemplateListWidget::DefaultTemplate).toBool();

            QString templateName = item->text();
            QString templateScript = item->data(TemplateListWidget::Text).toString();
            if (q->modifyTemplate(templateName, templateScript, defaultTemplate) ) {
                if (!defaultTemplate) {
                    item->setText(templateName);
                    item->setData(TemplateListWidget::Text, templateScript);
                    dirty = true;
                }
            }
        }
    }

    void slotInsertTemplate()
    {
        QListWidgetItem *item = q->currentItem();
        if (item) {
            const QString templateScript = item->data(TemplateListWidget::Text).toString();
            Q_EMIT q->insertTemplate(templateScript);
        }
    }

    void slotContextMenu(const QPoint &pos)
    {
        const QList<QListWidgetItem *> lstSelectedItems = q->selectedItems();
        const bool listSelectedIsEmpty = lstSelectedItems.isEmpty();
        KMenu *menu = new KMenu( q );

        if (!listSelectedIsEmpty) {
            menu->addAction( i18n("Insert template"), q, SLOT(slotInsertTemplate()));
            menu->addSeparator();
        }


        menu->addAction( i18n("Add..."), q, SLOT(slotAdd()));
        if (!listSelectedIsEmpty) {
            const bool defaultTemplate = lstSelectedItems.first()->data(TemplateListWidget::DefaultTemplate).toBool();
            if (lstSelectedItems.count() == 1) {
                menu->addAction( defaultTemplate ? i18n("Show...") : i18n("Modify..."), q, SLOT(slotModify()));
            }
            if (lstSelectedItems.count() == 1 && !defaultTemplate) {
                menu->addAction( i18n("Remove"), q, SLOT(slotRemove()));
            }
        }
        menu->exec( q->mapToGlobal( pos ) );
        delete menu;
    }

    void loadTemplates()
    {
        q->clear();
        const QList<PimCommon::defaultTemplate> templatesLst = q->defaultTemplates();
        Q_FOREACH (const PimCommon::defaultTemplate &tmp, templatesLst) {
            createListWidgetItem(tmp.name, tmp.text, true);
        }
        KConfigGroup group = config->group( "template" );
        if (group.hasKey(QLatin1String("templateCount"))) {
            const int numberTemplate = group.readEntry( "templateCount", 0 );
            for (int i = 0; i < numberTemplate; ++i) {
                KConfigGroup group = config->group( QString::fromLatin1( "templateDefine_%1" ).arg ( i ) );
                const QString name = group.readEntry( "Name", QString() );
                const QString text = group.readEntry( "Text", QString() );

                createListWidgetItem(name, text, false);
            }
        }
        dirty = false;
    }

    void saveTemplates()
    {
        if (!dirty)
            return;

        // clear everything
        foreach ( const QString &group, config->groupList() ) {
            config->deleteGroup( group );
        }

        int numberOfTemplate = 0;
        for ( int i = 0; i < q->count(); ++i ) {
            QListWidgetItem *templateItem = q->item(i);
            if (templateItem->data(TemplateListWidget::DefaultTemplate).toBool() == false) {
                KConfigGroup group = config->group( QString::fromLatin1( "templateDefine_%1" ).arg ( numberOfTemplate ) );
                group.writeEntry( "Name", templateItem->text() );
                group.writeEntry( "Text", templateItem->data(TemplateListWidget::Text) );
                ++numberOfTemplate;
            }
        }
        KConfigGroup group = config->group( "template" );
        group.writeEntry( "templateCount", numberOfTemplate );
        config->sync();
        dirty = false;
    }
    bool dirty;
    KSharedConfig::Ptr config;
    TemplateListWidget *q;
};


TemplateListWidget::TemplateListWidget(const QString &configName, QWidget *parent)
    : QListWidget(parent), d(new TemplateListWidgetPrivate(configName, this))
{
    setContextMenuPolicy( Qt::CustomContextMenu );
    setDragDropMode(QAbstractItemView::DragOnly);

    connect( this, SIGNAL(customContextMenuRequested(QPoint)),
             SLOT(slotContextMenu(QPoint)) );
    connect(this, SIGNAL(doubleClicked(QModelIndex)), SLOT(slotModify()));
}

TemplateListWidget::~TemplateListWidget()
{
    delete d;
}

void TemplateListWidget::loadTemplates()
{
    d->loadTemplates();
}

QList<PimCommon::defaultTemplate> TemplateListWidget::defaultTemplates()
{
    return QList<PimCommon::defaultTemplate>();
}

QStringList TemplateListWidget::mimeTypes() const
{
    QStringList lst;
    lst << QLatin1String( "text/plain" );
    return lst;
}

QMimeData *TemplateListWidget::mimeData ( const QList<QListWidgetItem *> items ) const
{
    if ( items.isEmpty() ) {
        return 0;
    }
    QMimeData *mimeData = new QMimeData();
    QListWidgetItem *item = items.first();
    mimeData->setText( item->data(TemplateListWidget::Text).toString() );
    return mimeData;
}

bool TemplateListWidget::addNewTemplate(QString &templateName, QString &templateScript)
{
    QPointer<TemplateEditDialog> dlg = new TemplateEditDialog(this);
    if (dlg->exec()) {
        templateName = dlg->templateName();
        templateScript = dlg->script();
        delete dlg;
        return true;
    }
    delete dlg;
    return false;
}

bool TemplateListWidget::modifyTemplate(QString &templateName, QString &templateScript, bool defaultTemplate)
{
    QPointer<TemplateEditDialog> dlg = new TemplateEditDialog(this, defaultTemplate);
    dlg->setTemplateName(templateName);
    dlg->setScript(templateScript);
    if (dlg->exec()) {
        if (!defaultTemplate) {
            templateName = dlg->templateName();
            templateScript = dlg->script();
        }
        delete dlg;
        return true;
    }
    delete dlg;
    return false;
}

}
#include "templatelistwidget.moc"
