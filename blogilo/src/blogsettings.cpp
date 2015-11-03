/*
    This file is part of Blogilo, A KDE Blogging Client

    Copyright (C) 2008-2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>
    Copyright (C) 2008-2010 Golnaz Nilieh <g382nilieh@gmail.com>

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

#include "blogsettings.h"
#include "addeditblog.h"
#include "dbman.h"

#include "blogilo_debug.h"
#include <KMessageBox>
#include <QIcon>
#include <KLocalizedString>

BlogSettings::BlogSettings(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

    connect(btnAdd, &QPushButton::clicked, this, &BlogSettings::addBlog);
    connect(btnEdit, &QPushButton::clicked, this, &BlogSettings::editBlog);
    connect(btnRemove, &QPushButton::clicked, this, &BlogSettings::removeBlog);
    connect(blogsTable, &QTreeWidget::currentItemChanged, this, &BlogSettings::blogsTablestateChanged);
    connect(blogsTable, &QTreeWidget::doubleClicked, this, &BlogSettings::editBlog);

    blogsTable->setHeaderLabels(QStringList() << i18n("Title") << i18n("URL"));
    btnAdd->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
    btnEdit->setIcon(QIcon::fromTheme(QStringLiteral("edit-rename")));
    btnRemove->setIcon(QIcon::fromTheme(QStringLiteral("list-remove")));
    loadBlogsList();
}

BlogSettings::~BlogSettings()
{
}

void BlogSettings::addBlog()
{
    AddEditBlog *addEditBlogWindow = new AddEditBlog(-1, this);
    addEditBlogWindow->setWindowModality(Qt::ApplicationModal);
    addEditBlogWindow->setAttribute(Qt::WA_DeleteOnClose);
    connect(addEditBlogWindow, &AddEditBlog::sigBlogAdded, this, &BlogSettings::slotBlogAdded);
    connect(addEditBlogWindow, &AddEditBlog::sigBlogAdded, this, &BlogSettings::blogAdded);
    addEditBlogWindow->show();
}

void BlogSettings::slotBlogAdded(const BilboBlog &blog)
{
    addBlogToList(blog);
}

void BlogSettings::editBlog()
{
    QTreeWidgetItem *item = blogsTable->currentItem();
    if (item) {
        const int blog_id = item->data(0, BlogId).toInt();
        AddEditBlog *addEditBlogWindow = new AddEditBlog(blog_id, this);
        addEditBlogWindow->setAttribute(Qt::WA_DeleteOnClose);
        addEditBlogWindow->setWindowModality(Qt::ApplicationModal);
        connect(addEditBlogWindow, &AddEditBlog::sigBlogEdited, this, &BlogSettings::slotBlogEdited);
        connect(addEditBlogWindow, &AddEditBlog::sigBlogEdited, this, &BlogSettings::blogEdited);
        addEditBlogWindow->show();
    }
}

void BlogSettings::slotBlogEdited(const BilboBlog &blog)
{
    QTreeWidgetItem *item = blogsTable->currentItem();
    if (item) {
        item->setText(0, blog.title());
        item->setText(1, blog.blogUrl());
    }
}

void BlogSettings::removeBlog()
{
    QTreeWidgetItem *item = blogsTable->currentItem();
    if (item) {
        if (KMessageBox::warningYesNo(this, i18n("Are you sure you want to remove the selected blog?"))
                == KMessageBox::No) {
            return;
        }
        const int blog_id = item->data(0, BlogId).toInt();
        if (DBMan::self()->removeBlog(blog_id)) {
            delete blogsTable->currentItem();
            Q_EMIT blogRemoved(blog_id);
        } else {
            ///cannot remove
            qCCritical(BLOGILO_LOG) << "Cannot remove blog with id " << blog_id;
        }
    }
}

void BlogSettings::loadBlogsList()
{
    foreach (BilboBlog *blog, DBMan::self()->blogList()) {
        addBlogToList(*blog);
    }
}

void BlogSettings::blogsTablestateChanged()
{
    if (blogsTable->currentItem()) {
        btnEdit->setEnabled(true);
        btnRemove->setEnabled(true);
    } else {
        btnEdit->setEnabled(false);
        btnRemove->setEnabled(false);
    }
}

void BlogSettings::addBlogToList(const BilboBlog &blog)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(blogsTable, QStringList() << blog.title() << blog.blogUrl());
    item->setData(0, BlogId, blog.id());
}

