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

#include <qdebug.h>
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
    connect(blogsTable, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
            this, SLOT(blogsTablestateChanged()));
    connect(blogsTable, &QTreeWidget::doubleClicked, this, &BlogSettings::editBlog);

    blogsTable->setHeaderLabels(QStringList() << i18n("Title") << i18n("URL"));
    btnAdd->setIcon(QIcon::fromTheme(QLatin1String("list-add")));
    btnEdit->setIcon(QIcon::fromTheme(QLatin1String("edit-rename")));
    btnRemove->setIcon(QIcon::fromTheme(QLatin1String("list-remove")));
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
    connect(addEditBlogWindow, SIGNAL(sigBlogAdded(BilboBlog)),
            this, SLOT(slotBlogAdded(BilboBlog)));
    connect(addEditBlogWindow, SIGNAL(sigBlogAdded(BilboBlog)),
            this, SIGNAL(blogAdded(BilboBlog)));
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
        connect(addEditBlogWindow, SIGNAL(sigBlogEdited(BilboBlog)),
                this, SLOT(slotBlogEdited(BilboBlog)));
        connect(addEditBlogWindow, SIGNAL(sigBlogEdited(BilboBlog)),
                this, SIGNAL(blogEdited(BilboBlog)));
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
            emit blogRemoved(blog_id);
        } else {
            ///cannot remove
            qCritical() << "Cannot remove blog with id " << blog_id;
        }
    }
}

void BlogSettings::loadBlogsList()
{
    QList<BilboBlog *> list = DBMan::self()->blogList().values();
    const int count = list.count();
    for (int i = 0; i < count; ++i) {
        addBlogToList(*list[i]);
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

