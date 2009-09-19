/***************************************************************************
 *   This file is part of the Bilbo Blogger.                               *
 *   Copyright (C) 2008-2009 Mehrdad Momeny <mehrdad.momeny@gmail.com>     *
 *   Copyright (C) 2008-2009 Golnaz Nilieh <g382nilieh@gmail.com>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

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
    void sltEditProperties();
    void sltSetProperties();
    void sltCopyUrl();
    void sltRemoveMedia();

private:
    KAction *actEdit;
    KAction *actCopyUrl;
    KAction *actRemove;

    Ui::EditImageBase ui;
};

#endif
