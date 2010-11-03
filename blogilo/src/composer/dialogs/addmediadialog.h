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

#ifndef ADDMEDIADIALOG_H
#define ADDMEDIADIALOG_H

#include <KDialog>
#include <kio/jobclasses.h>

#include "ui_addmediadialogbase.h"

class BilboMedia;
/**
Implements a dialog to enter address of a local or remote media file.

 @author Mehrdad Momeny <mehrdad.momeny@gmail.com>
 @author Golnaz Nilieh <g382nilieh@gmail.com>
 */
// TODO change the class to support more than one type of media.

class AddMediaDialog: public KDialog
{
    Q_OBJECT
public:
    /// AddImageDialog constructor.
    /**
     * Creates a new AddMediaDialog instance, and opens it.
     * @param parent is needed for QDialog constructor, which is the parent class of
     * AddMediaDialog.
     */
    AddMediaDialog( QWidget *parent = 0 );

    /// AddMediaDialog destructor.
    ~AddMediaDialog();

    BilboMedia *selectedMedia() const;
    QVariantMap selectedMediaProperties() const;

Q_SIGNALS:
    /**
     * when the dialog is accepted, sends a BilboMedia object to the caller function.this object
     * contains media properties, like path and mimetype.
     * @param media is the created BilboMedia object.
     */
    void sigAddMedia( BilboMedia *media );
    
    /**
     * When the mimetype of a media files is determined, and set in the related
     * BilboMedia object, this signal will be emitted.
     * @param media is the BilboMedia object which its mimetype is set.
     */
    void sigMediaTypeFound( BilboMedia *media );

protected:
//     virtual void addOtherMediaAttributes();

    QVariantMap _selectedMedia;
    Ui::AddMediaDialogBase ui;
    BilboMedia *media;

protected Q_SLOTS:
    virtual void slotButtonClicked(int button);
    virtual void slotSelectLocalFile();
//     virtual void sltOkClicked();
    virtual void slotRemoteFileTypeFound( KIO::Job *job, const QString &type );
//     void sltMediaSourceChanged();
};

#endif
