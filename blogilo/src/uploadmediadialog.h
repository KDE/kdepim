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
    explicit UploadMediaDialog(QWidget *parent = 0);
    ~UploadMediaDialog();
    enum UploadType {BlogAPI = 0, FTP};

    void init(const BilboBlog *currentBlog);

Q_SIGNALS:
    void sigBusy(bool isBusy);

private Q_SLOTS:
    bool selectNewFile();
    void currentMediaChanged(const QString &);
    void slotUploadTypeChanged(int index);
    void slotMediaObjectUploaded(KJob *);
    void slotMediaObjectUploaded(BilboMedia *);
    void slotError(const QString &msg);
    void slotButtonClicked(int button);
private:
    const BilboBlog *mCurrentBlog;
    Ui::UploadMediaBase ui;
};

#endif // UPLOADMEDIADIALOG_H
