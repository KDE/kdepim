/*
    This file is part of Blogilo, A KDE Blogging Client

    Copyright (C) 2008-2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>
    Copyright (C) 2008-2010 Golnaz Nilieh <g382nilieh@gmail.com>
    Copyright (C) 2013 Laurent Montel <montel@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.


    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, see http://www.gnu.org/licenses/
*/

#ifndef TOOLBOX_H
#define TOOLBOX_H

#include "category.h"
#include "ui_toolboxbase.h"

class BilboPost;
/**
 @author Mehrdad Momeny <mehrdad.momeny@gmail.com>
 @author Golnaz Nilieh <g382nilieh@gmail.com>
 */
class Toolbox: public QWidget, public Ui::ToolboxBase
{
    Q_OBJECT
public:
    explicit Toolbox( QWidget *parent );
    ~Toolbox();

    /**
     *    Will set current state of toolbox (Current post) properties on input pointer!
     * @param currentPost input and output of this Function.
     */
    void getFieldsValue( BilboPost* currentPost );
    void setFieldsValue( BilboPost* post = 0 );
    void setCurrentBlogId( int blog_id );
    void setCurrentPage( int index );
    void clearFields();

public slots:
    void slotReloadCategoryList();
    void slotLoadCategoryListFromDB( int blog_id );
    void slotUpdateEntries(int count = 0);
    void slotLoadEntriesFromDB( int blog_id );
    void slotRemoveSelectedEntryFromServer();
    void resetFields();
    void slotEntrySelected( QListWidgetItem *item );
    void slotEntriesCopyUrl();
    void slotLocalEntrySelected(QTreeWidgetItem *, int column );
    void reloadLocalPosts();
    void slotRemoveLocalEntry();
    void clearEntries();
    void setDateTimeNow();

signals:
    void sigEntrySelected( BilboPost &post, int blog_id );
    void sigError( const QString& );
    void sigBusy( bool isBusy );

protected slots:
    void slotPostRemoved( int blog_id, const BilboPost &post );
    void openPostInBrowser();
    void copyPostTitle();
    void requestEntriesListContextMenu( const QPoint & pos );
    void slotError(const QString& errorMessage);

private:
    enum LocalEntryType {
        LocalEntryID = QTreeWidgetItem::UserType +1
    };

    enum BlogEntryType {
        BlogEntryID = QListWidgetItem::UserType +1
    };


    QStringList selectedCategoriesTitle() const;
    QList<Category> selectedCategories() const;
    void setSelectedCategories( const QStringList& );
    QStringList currentTags();
    void clearCatList();
    void unCheckCatList();
    void setButtonsIcon();

    class Private;
    Private * const d;
};

#endif
