//
//  kjots
//
//  Copyright (C) 1997 Christoph Neerfeld <Christoph.Neerfeld@home.ivm.de>
//  Copyright (C) 2002, 2003 Aaron J. Seigo <aseigo@kde.org>
//  Copyright (C) 2003 Stanislav Kljuhhin <crz@hot.ee>
//  Copyright (C) 2005-2006 Jaison Lee <lee.jaison@gmail.com>
//  Copyright (C) 2007-2008 Stephen Kelly <steveire@gmail.com>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#ifndef __KJOTSEDIT_H
#define __KJOTSEDIT_H

#include <KRichTextWidget>
#include <QPointer>


class KActionCollection;
class Bookshelf;
class KJotsPage;


class KJotsEdit : public KRichTextWidget
{
Q_OBJECT
public:
    explicit KJotsEdit ( QWidget* );
    virtual ~KJotsEdit ();

    void DelayedInitialization ( KActionCollection*, Bookshelf* );
    virtual bool canInsertFromMimeData ( const QMimeData *) const;
    virtual void insertFromMimeData ( const QMimeData *);

protected:
    virtual void contextMenuEvent( QContextMenuEvent* );

    /** Override to make ctrl+click follow links */
    virtual void mouseReleaseEvent(QMouseEvent *);

protected slots:
    void onBookshelfSelection ( void );
    void onTextChanged ( void );
    void onAutoBullet ( void );
    void onLinkify ( void );
    void addCheckmark( void );
    void onAutoDecimal( void );
    void DecimalList( void );
private:
    void disableEditing ( void );
    void createAutoDecimalList();
    QPointer<Bookshelf> bookshelf;
    QPointer<KJotsPage> currentPage;
    KActionCollection *actionCollection;
    bool allowAutoDecimal;

};

#endif // __KJOTSEDIT_H
/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 expandtab: */
/* kate: tab-indents off; replace-tabs on; tab-width 4; remove-trailing-space on; encoding utf-8;*/
