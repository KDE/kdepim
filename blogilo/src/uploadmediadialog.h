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

#ifndef UPLOADMEDIADIALOG_H
#define UPLOADMEDIADIALOG_H

#include <KDialog>
#include "ui_uploadmediabase.h"
class BilboBlog;
class BilboMedia;

class UploadMediaDialog : public KDialog
{
Q_OBJECT
public:
    UploadMediaDialog( QWidget *parent=0 );
    ~UploadMediaDialog();
    enum UploadType{BlogAPI=0, FTP};
    void init( const BilboBlog* currentBlog );
signals:
//     void error(const QString &msg);
    void sigBusy(bool isBusy);
protected slots:
    bool selectNewFile();
    void currentMediaChanged(QString);
    void slotUploadTypeChanged(int index);
    void slotButtonClicked(int button);
    void slotMediaObjectUploaded(KJob *);
    void slotMediaObjectUploaded(BilboMedia*);
    void slotError( const QString &msg );
private:
    const BilboBlog *mCurrentBlog;
    Ui::UploadMediaBase ui;
};

#endif // UPLOADMEDIADIALOG_H
