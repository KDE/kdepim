/*
  Copyright (c) 2013, 2014 Montel Laurent <montel.org>

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

#ifndef ADBLOCKBLOCKABLEITEMSWIDGET_H
#define ADBLOCKBLOCKABLEITEMSWIDGET_H

#include <QWidget>
#include "messageviewer_export.h"

class QTreeWidget;
class QWebFrame;
namespace MessageViewer {
class MESSAGEVIEWER_EXPORT AdBlockBlockableItemsWidget : public QWidget
{
    Q_OBJECT
public:
    enum TypeElement {
        None = 0,
        Image,
        Script,
        StyleSheet,
        Font,
        Frame,
        XmlRequest,
        Object,
        Media,
        Popup,

        MaxTypeElement
    };
    explicit AdBlockBlockableItemsWidget(QWidget *parent=0);
    ~AdBlockBlockableItemsWidget();

    static QString elementTypeToI18n(AdBlockBlockableItemsWidget::TypeElement type);
    static QString elementType(AdBlockBlockableItemsWidget::TypeElement type);

    void setWebFrame(QWebFrame *frame);
    void saveFilters();

private Q_SLOTS:
    void slotCopyItem();
    void slotBlockItem();
    void slotOpenItem();
    void slotCopyFilterItem();
    void customContextMenuRequested(const QPoint &);
    void slotRemoveFilter();

private:
    enum BlockType {
        FilterValue = 0,
        Url,
        Type
    };


    enum TypeItem {
        Element = Qt::UserRole + 1
    };

    void searchBlockableElement(QWebFrame *frame);
    void adaptSrc(QString &src,const QString &hostName);

private:
    QTreeWidget *mListItems;
};
}

#endif // ADBLOCKBLOCKABLEITEMSWIDGET_H
