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

#include "sievetemplatewidget.h"
#include "sievetemplateeditdialog.h"
#include "sievedefaulttemplate.h"

#include <KMenu>
#include <KLocale>
#include <KMessageBox>

#include <QListWidget>
#include <QHBoxLayout>
#include <QPointer>
#include <QMimeData>
#include <QLabel>
#include <QDebug>

namespace KSieveUi {

class SieveTemplateListWidgetPrivate
{
public:
    SieveTemplateListWidgetPrivate(const QString &configName, SieveTemplateListWidget *qq)
        : dirty(false), config(KSharedConfig::openConfig(configName, KConfig::NoGlobals)), q(qq)
    {

    }
    ~SieveTemplateListWidgetPrivate()
    {
        saveTemplates();
    }
    void createListWidgetItem(const QString &name, const QString &text, bool isDefaultTemplate)
    {
        QListWidgetItem *item = new QListWidgetItem(name, q);
        item->setData(SieveTemplateListWidget::SieveText, text);
        item->setData(SieveTemplateListWidget::DefaultTemplate, isDefaultTemplate);
    }

    void slotAdd()
    {
        QPointer<SieveTemplateEditDialog> dlg = new SieveTemplateEditDialog(q);
        if (dlg->exec()) {
            const QString templateName = dlg->templateName();
            const QString templateScript = dlg->script();
            createListWidgetItem(templateName, templateScript, false);
            dirty = true;
        }
        delete dlg;
    }

    void slotRemove()
    {
        if(KMessageBox::Yes == KMessageBox::questionYesNo(q, i18n("Do you want to delete selected template?"), i18n("Delete template"))) {
            const QList<QListWidgetItem *> lstSelectedItems = q->selectedItems();
            Q_FOREACH (QListWidgetItem *item, lstSelectedItems) {
                if (item->data(SieveTemplateListWidget::DefaultTemplate).toBool() == false)
                    delete item;
            }
            dirty = true;
        }
    }

    void slotModify()
    {
        QListWidgetItem * item = q->currentItem();
        if(item) {
            const bool defaultTemplate = item->data(SieveTemplateListWidget::DefaultTemplate).toBool();
            QPointer<SieveTemplateEditDialog> dlg = new SieveTemplateEditDialog(q, defaultTemplate);
            dlg->setTemplateName(item->text());
            dlg->setScript(item->data(SieveTemplateListWidget::SieveText).toString());
            if (dlg->exec()) {
                if (!defaultTemplate) {
                    const QString templateName = dlg->templateName();
                    const QString templateScript = dlg->script();
                    item->setText(templateName);
                    item->setData(SieveTemplateListWidget::SieveText, templateScript);
                    dirty = true;
                }
            }
            delete dlg;
        }
    }

    void slotInsertTemplate()
    {
        QListWidgetItem *item = q->currentItem();
        if (item) {
            const QString templateScript = item->data(SieveTemplateListWidget::SieveText).toString();
            Q_EMIT q->insertTemplate(templateScript);
        }
    }

    void slotContextMenu(const QPoint &pos)
    {
        QList<QListWidgetItem *> lstSelectedItems = q->selectedItems();
        if (lstSelectedItems.isEmpty())
            return;
        KMenu *menu = new KMenu( q );

        menu->addAction( i18n("Insert template"), q, SLOT(slotInsertTemplate()));
        menu->addSeparator();

        const bool defaultTemplate = lstSelectedItems.first()->data(SieveTemplateListWidget::DefaultTemplate).toBool();

        menu->addAction( i18n("Add..."), q, SLOT(slotAdd()));
        if (lstSelectedItems.count() == 1) {
            menu->addAction( defaultTemplate ? i18n("Show...") : i18n("Modify..."), q, SLOT(slotModify()));
        }
        if (lstSelectedItems.count() == 1 && !defaultTemplate) {
            menu->addAction( i18n("Remove"), q, SLOT(slotRemove()));
        }
        menu->exec( q->mapToGlobal( pos ) );
        delete menu;
    }

    void loadTemplates()
    {
        q->clear();
        const QList<KSieveUi::defaultTemplate> templatesLst = q->defaultTemplates();
        Q_FOREACH (const KSieveUi::defaultTemplate &tmp, templatesLst) {
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
            if (templateItem->data(SieveTemplateListWidget::DefaultTemplate).toBool() == false) {
                KConfigGroup group = config->group( QString::fromLatin1( "templateDefine_%1" ).arg ( numberOfTemplate ) );
                group.writeEntry( "Name", templateItem->text() );
                group.writeEntry( "Text", templateItem->data(SieveTemplateListWidget::SieveText) );
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
    SieveTemplateListWidget *q;
};


SieveTemplateListWidget::SieveTemplateListWidget(QWidget *parent)
    : QListWidget(parent), d(new SieveTemplateListWidgetPrivate(QLatin1String("sievetemplaterc"), this))
{
    setContextMenuPolicy( Qt::CustomContextMenu );
    setDragDropMode(QAbstractItemView::DragOnly);

    connect( this, SIGNAL(customContextMenuRequested(QPoint)),
             SLOT(slotContextMenu(QPoint)) );
    connect(this, SIGNAL(doubleClicked(QModelIndex)), SLOT(slotModify()));
    d->loadTemplates();
}

SieveTemplateListWidget::~SieveTemplateListWidget()
{
    delete d;
}

QList<KSieveUi::defaultTemplate> SieveTemplateListWidget::defaultTemplates()
{
    return KSieveUi::SieveDefaultTemplate::defaultTemplates();
}

QStringList SieveTemplateListWidget::mimeTypes() const
{
    QStringList lst;
    lst << QLatin1String( "text/plain" );
    return lst;
}

QMimeData *SieveTemplateListWidget::mimeData ( const QList<QListWidgetItem *> items ) const
{
    if ( items.isEmpty() ) {
        return 0;
    }
    QMimeData *mimeData = new QMimeData();
    QListWidgetItem *item = items.first();
    mimeData->setText( item->data(SieveTemplateListWidget::SieveText).toString() );
    return mimeData;
}



SieveTemplateWidget::SieveTemplateWidget(const QString &title, QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *lay = new QVBoxLayout;
    QLabel *lab = new QLabel(title);
    lay->addWidget(lab);
    mListTemplate = new SieveTemplateListWidget;
    mListTemplate->setWhatsThis(i18n("You can drag and drop element on editor to import template"));
    connect(mListTemplate, SIGNAL(insertTemplate(QString)), SIGNAL(insertTemplate(QString)));
    lay->addWidget(mListTemplate);
    setLayout(lay);
}


SieveTemplateWidget::~SieveTemplateWidget()
{
}

}
#include "sievetemplatewidget.moc"
