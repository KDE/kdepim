/*
    This file is part of Blogilo, A KDE Blogging Client

    Copyright (C) 2008-2009 Mehrdad Momeny <mehrdad.momeny@gmail.com>
    Copyright (C) 2008-2009 Golnaz Nilieh <g382nilieh@gmail.com>

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

#ifndef MULTILINETEXTEDIT_H
#define MULTILINETEXTEDIT_H

#include "krichtextedit.h"
#include <KUrl>

class KJob;
class BilboMedia;

/**
* Class MultiLineTextEdit Implements a TextEdit widget that supports new line 
* characters, and loading images from the web.
* @author Mehrdad Momeny <mehrdad.momeny@gmail.com>
* @author Golnaz Nilieh <g382nilieh@gmail.com>
*/

class MultiLineTextEdit : public KRichTextEdit
{
    Q_OBJECT
public:

    /**
     * @brief MultiLineTextEdit constructor.
     * @param parent is needed for KRichTextEdit constructor, which 
     * MultiLineTextEdit inherits from.
     */
    MultiLineTextEdit( QWidget *parent = 0 );

    /**
     * @brief MultiLineTextEdit destructor.
     */
    ~MultiLineTextEdit();
    
    /**
     * Clears the internal list of downloaded media, so that function 
     * loadResource() gets them from the web at the next call.
     */
    static void clearCache();
    
    /**
     * Makes MultiLineTextEdit to use @param list as its list of media files.
     * In the loadResource() function, it looks for each resource in list, to
     * see if a media object for that resource is created.
     * @sa loadResource().
     */
    void setMediaList( QMap <QString, BilboMedia*> * list );
    
Q_SIGNALS:
    /**
     * when a remote image is downloaded from the web successfully, this signal is emmited.
     * @param imagePath is the image url on the web.
     */
    void sigRemoteImageArrived( const KUrl imagePath );
    
    /**
     * When loadResource() function is called for a media file, which is not
     * inserted in the editor media-list before, it tries to determine the media
     * mimetype. if successful, it creates a BilboMedia object for that media,
     * then emits this signal.
     * @param media is the created BilboMedia object.
     * @sa loadResource().
     */
    void sigMediaTypeFound( BilboMedia *media );

    /**
     * Emitted when current Text Block format changed!
     */
    void currentBlockFormatChanged(const QTextBlockFormat &);

protected:

    /*!
     * When MultiLineTextEdit is focused on, checks each presssed key; then replaces
     * "Return" characters with QChar::LineSeparator special character.
     * this function is defined virtual in parent class: KRichTextEdit.
     */
    void keyPressEvent( QKeyEvent *event );

    /**
     * If the name is the url of a remote image, looks for it in cache, and if it doesn't exist, starts downloading of it and returns an empty QVariant object.
     * else if the image is local or has been downloaded before, returns image data as a
     * QVariant object.
     * 
     * @param type is type of the resource. @see QTextDocument::ResourceType.
     * @param name is the resource url.
     * @return a QVariant object which contains resource data, if it could be found by 
     * the function, else, it's an empty QVariant object.
     */
    virtual QVariant loadResource( int type, const QUrl & name );

private Q_SLOTS:
    void sltRemoteFileCopied(KJob * job);
    void slotCursorPositionChanged();

private:
    static QMap <QString, bool> downloadFinished;
    QMap <QString, BilboMedia*> *mMediaList;
    QTextBlockFormat lastBlockFormat;
};

#endif
