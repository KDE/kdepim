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

#ifndef MEDIALISTWIDGET_H
#define MEDIALISTWIDGET_H

#include <klistwidget.h>
#include "ui_editimagebase.h"

/**
    Implements a ListWidget that displays media files added to a post.
    @author Mehrdad Momeny <mehrdad.momeny@gmail.com>
    @author Golnaz Nilieh <g382nilieh@gmail.com>
*/
class KAction;
class MediaListWidget : public KListWidget
{
    Q_OBJECT
public:
    /**
     * @brief MediaListWidget constructor.
     * @param parent is the parent widget.
     */
    MediaListWidget( QWidget *parent = 0 );

    /**
     * @brief MediaListWidget destructor.
     */
    ~MediaListWidget();

    /**
     * If user requests context menu for a list item, generates one and shows it.
     * Menu items depend to media type. if media is an image, action 
     * "edit properties" will be added to the menu.
     * @param event 
     */
    void contextMenuEvent( QContextMenuEvent *event );

    enum ItemType {
        ImageType = 1001,
        OtherType = 1002
    };

Q_SIGNALS:
    /**
     * This signal is emmited when user selects "Remove media" action from the
     * context menu.
     * @param index is the index of the media item in the list, that user wants
     * to remove.
     */
    void sigRemoveMedia( const int index );

    /**
     * After user sets new properties for an image file, this signal is emmited
     * with new values.
     * @param index is the index of media item in the list, which its properties
     * are set.
     * @param width is the image width in pixels.
     * @param height is the image height in pixels.
     * @param title is the image title, that is shown when cursor hovers on it.
     * @param link is a Url that the images is linked to.
     * @param Alt_text is a text that is shown if the image can't be loaded in a 
     * web browser.
     */
    void sigSetProperties( const int index, const int width, const int height,
                           const QString title, const QString link, const QString Alt_text );

protected Q_SLOTS:
    void slotEditProperties();
    void slotSetProperties();
    void slotCopyUrl();
    void slotRemoveMedia();

private:
    KAction *actEdit;
    KAction *actCopyUrl;
    KAction *actRemove;

    Ui::EditImageBase ui;
};

#endif
