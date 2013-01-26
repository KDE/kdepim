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

#include <kdebug.h>
#include <KMessageBox>


BlogSettings::BlogSettings( QWidget *parent )
    : QWidget( parent )
{
    kDebug();
    setupUi( this );

    connect( btnAdd, SIGNAL(clicked()), this, SLOT(addBlog()) );
    connect( btnEdit, SIGNAL(clicked()), this, SLOT(editBlog()) );
    connect( btnRemove, SIGNAL(clicked()), this, SLOT(removeBlog()) );
    connect( blogsTable, SIGNAL(currentItemChanged(QTableWidgetItem*,QTableWidgetItem*)),
             this, SLOT(blogsTablestateChanged()) );

    btnAdd->setIcon( KIcon( "list-add" ) );
    btnEdit->setIcon( KIcon( "edit-rename" ) );
    btnRemove->setIcon( KIcon( "list-remove" ) );
    loadBlogsList();
}

BlogSettings::~BlogSettings()
{
    kDebug();
}

void BlogSettings::addBlog()
{
    kDebug();
    AddEditBlog *addEditBlogWindow = new AddEditBlog( -1, this );
    addEditBlogWindow->setWindowModality( Qt::ApplicationModal );
    addEditBlogWindow->setAttribute( Qt::WA_DeleteOnClose );
    connect( addEditBlogWindow, SIGNAL(sigBlogAdded(BilboBlog)),
             this, SLOT(slotBlogAdded(BilboBlog)) );
    connect( addEditBlogWindow, SIGNAL(sigBlogAdded(BilboBlog)),
             this, SIGNAL(blogAdded(BilboBlog)));
    addEditBlogWindow->show();
}

void BlogSettings::slotBlogAdded( const BilboBlog &blog )
{
    kDebug();
    addBlogToList( blog );
}

void BlogSettings::editBlog()
{
    kDebug();
    if( blogsTable->selectedItems().count() <= 0 )
        return;
    int blog_id = blogsTable->item( blogsTable->currentRow(), 0 )->data( 32 ).toInt();
    AddEditBlog *addEditBlogWindow = new AddEditBlog( blog_id, this );
    addEditBlogWindow->setAttribute( Qt::WA_DeleteOnClose );
    addEditBlogWindow->setWindowModality( Qt::ApplicationModal );
    connect( addEditBlogWindow, SIGNAL(sigBlogEdited(BilboBlog)),
             this, SLOT(slotBlogEdited(BilboBlog)) );
    connect( addEditBlogWindow, SIGNAL(sigBlogEdited(BilboBlog)),
             this, SIGNAL(blogEdited(BilboBlog)));
    addEditBlogWindow->show();
}

void BlogSettings::slotBlogEdited( const BilboBlog &blog )
{
    kDebug();
    blogsTable->item( blogsTable->currentRow(), 0 )->setText( blog.title() );
    blogsTable->item( blogsTable->currentRow(), 1 )->setText( blog.blogUrl() );
}

void BlogSettings::removeBlog()
{
    kDebug();
    if( blogsTable->selectedItems().count() <= 0 )
        return;
    if(KMessageBox::warningYesNo(this, i18n("Are you sure you want to remove the selected blog?")) 
        == KMessageBox::No)
        return;
    int blog_id = blogsTable->item( blogsTable->currentRow(), 0 )->data( 32 ).toInt();
    if ( DBMan::self()->removeBlog( blog_id ) ) {
        blogsTable->removeRow( blogsTable->currentRow() );
        emit blogRemoved( blog_id );
    } else {
        ///cannot remove
        kError()<<"Cannot remove blog with id "<<blog_id;
    }
}

void BlogSettings::loadBlogsList()
{
    kDebug();
    QList<BilboBlog*> list = DBMan::self()->blogList().values();
    int count = list.count();
    for(int i=0; i<count; ++i) {
        addBlogToList( *list[i] );
    }
}

void BlogSettings::blogsTablestateChanged()
{
    if ( blogsTable->currentRow() >= 0 ) {
        btnEdit->setEnabled( true );
        btnRemove->setEnabled( true );
    } else {
        btnEdit->setEnabled( false );
        btnRemove->setEnabled( false );
    }
}

void BlogSettings::addBlogToList( const BilboBlog &blog )
{
    kDebug();
    int newRow = blogsTable->rowCount();
    blogsTable->insertRow( newRow );
    QTableWidgetItem *item1 = new QTableWidgetItem( blog.title() );
    item1->setData( 32, blog.id() );//blog_id
    blogsTable->setItem( newRow, 0, item1 );
    QTableWidgetItem *item2 = new QTableWidgetItem( blog.blogUrl() );
//     item2->setData( 32, ... );
    blogsTable->setItem( newRow, 1, item2 );
}

#include "blogsettings.moc"
