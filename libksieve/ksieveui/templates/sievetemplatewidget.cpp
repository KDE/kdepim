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

#include <QListWidget>
#include <QHBoxLayout>
#include <QPointer>
#include <QMimeData>
#include <QLabel>
#include <QDebug>

using namespace KSieveUi;

SieveTemplateListWidget::SieveTemplateListWidget(QWidget *parent)
    : QListWidget(parent), mDirty(false)
{
    setContextMenuPolicy( Qt::CustomContextMenu );
    setDragDropMode(QAbstractItemView::DragOnly);

    connect( this, SIGNAL(customContextMenuRequested(QPoint)),
             SLOT(slotContextMenu(QPoint)) );
    connect(this, SIGNAL(doubleClicked(QModelIndex)), SLOT(slotModify()));
    loadTemplates();
}

SieveTemplateListWidget::~SieveTemplateListWidget()
{
    saveTemplates();
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

void SieveTemplateListWidget::slotContextMenu(const QPoint &pos)
{
    QList<QListWidgetItem *> lstSelectedItems = selectedItems();
    KMenu *menu = new KMenu( this );

    menu->addAction( i18n("Insert template"), this, SLOT(slotInsertTemplate()));
    menu->addSeparator();

    menu->addAction( i18n("Add..."), this, SLOT(slotAdd()));
    if (lstSelectedItems.count() == 1) {
        menu->addAction( i18n("Modify..."), this, SLOT(slotModify()));
    }
    if (lstSelectedItems.count() == 1 && !lstSelectedItems.first()->data(SieveTemplateListWidget::DefaultTemplate).toBool()) {
        menu->addAction( i18n("Remove"), this, SLOT(slotRemove()));
    }
    menu->exec( mapToGlobal( pos ) );
    delete menu;
}

void SieveTemplateListWidget::slotInsertTemplate()
{
    QListWidgetItem *item = currentItem();
    if (item) {
        const QString templateScript = item->data(SieveTemplateListWidget::SieveText).toString();
        Q_EMIT insertTemplate(templateScript);
    }
}

void SieveTemplateListWidget::slotRemove()
{
    QList<QListWidgetItem *> lstSelectedItems = selectedItems();
    Q_FOREACH (QListWidgetItem *item, lstSelectedItems) {
        if (item->data(SieveTemplateListWidget::DefaultTemplate).toBool() == false)
            delete item;
    }
    mDirty = true;
}

void SieveTemplateListWidget::slotAdd()
{
    QPointer<SieveTemplateEditDialog> dlg = new SieveTemplateEditDialog(this);
    if (dlg->exec()) {
        const QString templateName = dlg->templateName();
        const QString templateScript = dlg->script();
        createListWidgetItem(templateName, templateScript, false);

        mDirty = true;
    }
    delete dlg;
}

void SieveTemplateListWidget::slotModify()
{
    QListWidgetItem * item = currentItem();
    if(item) {
        QPointer<SieveTemplateEditDialog> dlg = new SieveTemplateEditDialog(this);
        dlg->setTemplateName(item->text());
        dlg->setScript(item->data(SieveTemplateListWidget::SieveText).toString());
        if (dlg->exec()) {
            const QString templateName = dlg->templateName();
            const QString templateScript = dlg->script();
            item->setText(templateName);
            item->setData(SieveTemplateListWidget::SieveText, templateScript);
            mDirty = true;
        }
        delete dlg;
    }
}


void SieveTemplateListWidget::loadTemplates()
{
    clear();
    const QList<KSieveUi::SieveDefaultTemplate::defaultTemplate> templatesLst = KSieveUi::SieveDefaultTemplate::defaultTemplates();
    Q_FOREACH (const KSieveUi::SieveDefaultTemplate::defaultTemplate &tmp, templatesLst) {
        createListWidgetItem(tmp.name, tmp.text, true);
    }
    KSharedConfig::Ptr config = KSharedConfig::openConfig( "sievetemplaterc", KConfig::NoGlobals );
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
    mDirty = false;
}

void SieveTemplateListWidget::createListWidgetItem(const QString &name, const QString &text, bool isDefaultTemplate)
{
    QListWidgetItem *item = new QListWidgetItem(name, this);
    item->setData(SieveTemplateListWidget::SieveText, text);
    item->setData(SieveTemplateListWidget::DefaultTemplate, isDefaultTemplate);
}

void SieveTemplateListWidget::saveTemplates()
{
    if (!mDirty)
        return;

    KSharedConfig::Ptr config = KSharedConfig::openConfig( "sievetemplaterc", KConfig::NoGlobals );
    // clear everything
    foreach ( const QString &group, config->groupList() ) {
        config->deleteGroup( group );
    }

    int numberOfTemplate = 0;
    for ( int i = 0; i < count(); ++i ) {
        if (item(i)->data(SieveTemplateListWidget::DefaultTemplate).toBool() == false) {
            KConfigGroup group = config->group( QString::fromLatin1( "templateDefine_%1" ).arg ( numberOfTemplate ) );
            group.writeEntry( "Name", item(i)->text() );
            group.writeEntry( "Text", item(i)->data(SieveTemplateListWidget::SieveText) );
            numberOfTemplate ++;
        }
    }
    KConfigGroup group = config->group( "template" );
    group.writeEntry( "templateCount", numberOfTemplate );
    config->sync();
    mDirty = false;
}


SieveTemplateWidget::SieveTemplateWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *lay = new QVBoxLayout;
    QLabel *lab = new QLabel(i18n("Sieve Template:"));
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

#include "sievetemplatewidget.moc"
