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

#include "templatelistwidget.h"
#include "templateeditdialog.h"

#include <KConfigGroup>
#include <KSharedConfig>
#include <KMessageBox>
#include <KLocalizedString>
#include <QMenu>
#include <KFileDialog>
#include <KNS3/DownloadDialog>
#include <KIcon>

#include <QListWidgetItem>
#include <QPointer>
#include <QMimeData>
#include <QDropEvent>

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
        save();
    }

    void createListWidgetItem(const QString &name, const QString &text, bool isDefaultTemplate)
    {
        QListWidgetItem *item = new QListWidgetItem(name, q);
        item->setData(TemplateListWidget::Text, text);
        item->setData(TemplateListWidget::DefaultTemplate, isDefaultTemplate);
        q->setCurrentItem(item);
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

    void slotInsertNewTemplate(const QString &newTemplateScript)
    {
        QString templateName;
        QString templateScript = newTemplateScript;
        if (q->modifyTemplate(templateName, templateScript, false) ) {
            createListWidgetItem(templateName, templateScript, false);
            dirty = true;
        }
    }

    void slotRemove()
    {
        if (KMessageBox::Yes == KMessageBox::questionYesNo(q, i18n("Do you want to delete selected template?"), i18n("Delete template"))) {
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
        if (item) {
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

    void slotDuplicate()
    {
        QListWidgetItem *item = q->currentItem();
        if (item) {
            QStringList name;
            for ( int i = 0; i < q->count(); ++i ) {
                name.append(q->item(i)->text());
            }
            QString templateName = item->text() + QString::fromLatin1(" (%1)");
            QString newName;
            int i = 1;
            do {
                newName = templateName.arg(i);
                i++;
            } while(name.contains(newName));

            const QString templateScript = item->data(TemplateListWidget::Text).toString();
            createListWidgetItem(newName, templateScript, false);
            dirty = true;
        }
    }

    void slotImportTemplates()
    {
        const QString templateFile = KFileDialog::getOpenFileName();
        if (!templateFile.isEmpty()) {
            KConfig conf(templateFile);
            loadTemplates(&conf);
        }
    }

    void slotExportTemplates()
    {
        const QString templateFile = KFileDialog::getSaveFileName();
        if (!templateFile.isEmpty()) {
            KConfig conf(templateFile);
            saveTemplates(&conf);
        }
    }

    void slotContextMenu(const QPoint &pos)
    {
        const QList<QListWidgetItem *> lstSelectedItems = q->selectedItems();
        const bool listSelectedIsEmpty = lstSelectedItems.isEmpty();
        QMenu *menu = new QMenu( q );

        if (!listSelectedIsEmpty) {
            menu->addAction( i18n("Insert template"), q, SLOT(slotInsertTemplate()));
            menu->addSeparator();
        }


        menu->addAction( i18n("Add..."), q, SLOT(slotAdd()));
        if (!listSelectedIsEmpty) {
            const bool defaultTemplate = lstSelectedItems.first()->data(TemplateListWidget::DefaultTemplate).toBool();
            if (lstSelectedItems.count() == 1) {
                menu->addAction( defaultTemplate ? i18n("Show...") : i18n("Modify..."), q, SLOT(slotModify()));
                menu->addAction( i18n("Duplicate"), q, SLOT(slotDuplicate()));
            }
            if (lstSelectedItems.count() == 1 && !defaultTemplate) {
                menu->addSeparator();
                menu->addAction( i18n("Remove"), q, SLOT(slotRemove()));
            }
        }
        menu->addSeparator();
        if (q->count()>0)
            menu->addAction( i18n("Export..."), q, SLOT(slotExportTemplates()));
        menu->addAction( i18n("Import..."), q, SLOT(slotImportTemplates()));

        if (!knewstuffConfigName.isEmpty()) {
            menu->addSeparator();
            QAction *act = menu->addAction( i18n("Download new Templates..."), q, SLOT(slotDownloadTemplates()));
            act->setIcon(KIcon(QLatin1String("get-hot-new-stuff")));
        }

        menu->exec( q->mapToGlobal( pos ) );
        delete menu;
    }

    void load()
    {
        q->clear();
        const QList<PimCommon::defaultTemplate> templatesLst = q->defaultTemplates();
        Q_FOREACH (const PimCommon::defaultTemplate &tmp, templatesLst) {
            createListWidgetItem(tmp.name, tmp.text, true);
        }
        loadTemplates(&(*config));
        dirty = false;
    }

    void loadTemplates(KConfig *configFile)
    {
        KConfigGroup group = configFile->group( "template" );
        if (group.hasKey(QLatin1String("templateCount"))) {
            const int numberTemplate = group.readEntry( "templateCount", 0 );
            for (int i = 0; i < numberTemplate; ++i) {
                KConfigGroup group = configFile->group( QString::fromLatin1( "templateDefine_%1" ).arg ( i ) );
                const QString name = group.readEntry( "Name", QString() );
                const QString text = group.readEntry( "Text", QString() );

                createListWidgetItem(name, text, false);
            }
        }   
    }

    void saveTemplates(KConfig *configFile)
    {
        // clear everything
        foreach ( const QString &group, configFile->groupList() ) {
            configFile->deleteGroup( group );
        }

        int numberOfTemplate = 0;
        for ( int i = 0; i < q->count(); ++i ) {
            QListWidgetItem *templateItem = q->item(i);
            if (templateItem->data(TemplateListWidget::DefaultTemplate).toBool() == false) {
                KConfigGroup group = configFile->group( QString::fromLatin1( "templateDefine_%1" ).arg ( numberOfTemplate ) );
                group.writeEntry( "Name", templateItem->text() );
                group.writeEntry( "Text", templateItem->data(TemplateListWidget::Text) );
                ++numberOfTemplate;
            }
        }
        KConfigGroup group = configFile->group( "template" );
        group.writeEntry( "templateCount", numberOfTemplate );
        configFile->sync();
    }

    void slotDownloadTemplates()
    {
        QPointer<KNS3::DownloadDialog> downloadThemesDialog = new KNS3::DownloadDialog(knewstuffConfigName);
        downloadThemesDialog->exec();
        delete downloadThemesDialog;
    }

    void save()
    {
        if (!dirty)
            return;

        saveTemplates(&(*config));
        dirty = false;
    }
    QString knewstuffConfigName;
    bool dirty;
    KSharedConfig::Ptr config;
    TemplateListWidget *q;
};


TemplateListWidget::TemplateListWidget(const QString &configName, QWidget *parent)
    : QListWidget(parent), d(new TemplateListWidgetPrivate(configName, this))
{
    setContextMenuPolicy( Qt::CustomContextMenu );
    setDragDropMode(QAbstractItemView::DragDrop);

    connect( this, SIGNAL(customContextMenuRequested(QPoint)),
             SLOT(slotContextMenu(QPoint)) );
    connect(this, SIGNAL(doubleClicked(QModelIndex)), SLOT(slotModify()));
    connect(this, SIGNAL(insertNewTemplate(QString)), SLOT(slotInsertNewTemplate(QString)));
}

TemplateListWidget::~TemplateListWidget()
{
    delete d;
}

void TemplateListWidget::loadTemplates()
{
    d->load();
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
    bool result = false;
    if (dlg->exec()) {
        templateName = dlg->templateName();
        templateScript = dlg->script();
        result = true;
    }
    delete dlg;
    return result;
}

bool TemplateListWidget::modifyTemplate(QString &templateName, QString &templateScript, bool defaultTemplate)
{
    QPointer<TemplateEditDialog> dlg = new TemplateEditDialog(this, defaultTemplate);
    dlg->setTemplateName(templateName);
    dlg->setScript(templateScript);
    bool result = false;
    if (dlg->exec()) {
        if (!defaultTemplate) {
            templateName = dlg->templateName();
            templateScript = dlg->script();
        }
        result = true;
    }
    delete dlg;
    return result;
}

void TemplateListWidget::dropEvent( QDropEvent * event )
{
    if ( event->source() == this ) {
        event->ignore();
        return;
    }
     if (event->mimeData()->hasText()) {
        event->setDropAction( Qt::CopyAction );
        Q_EMIT insertNewTemplate(event->mimeData()->text());
        event->accept();
    }
    QListWidget::dropEvent( event );
}

void TemplateListWidget::setKNewStuffConfigFile(const QString &configName)
{
    d->knewstuffConfigName = configName;
}

void TemplateListWidget::addDefaultTemplate(const QString &templateName, const QString &templateScript)
{
    d->createListWidgetItem(templateName, templateScript, true);
}

}
#include "moc_templatelistwidget.cpp"
