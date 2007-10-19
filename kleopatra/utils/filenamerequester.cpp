/* -*- mode: c++; c-basic-offset:4 -*-
    utils/filenamerequester.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2007 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include "filenamerequester.h"

#include <KIcon>
#include <KLocale>

#include <QLayout>
#include <QToolButton>
#include <QLineEdit>
#include <QCompleter>
#include <QDirModel>
#include <QVariant>
#include <QString>
#include <QFileDialog>

using namespace Kleo;

class FileNameRequester::Private {
    friend class ::Kleo::FileNameRequester;
    FileNameRequester * const q;
public:
    explicit Private( FileNameRequester * qq );
    ~Private();

private:
    void slotButtonClicked();

private:
    QDirModel  dirmodel;
    QCompleter completer;

    QHBoxLayout hlay;
    QLineEdit    lineedit;
    QToolButton  button;

    bool onlyExistingFiles;
};

FileNameRequester::Private::Private( FileNameRequester * qq )
    : q( qq ),
      dirmodel(),
      completer( &dirmodel ),
      hlay( q ),
      lineedit( q ),
      button( q ),
      onlyExistingFiles( true )
{
    KDAB_SET_OBJECT_NAME( dirmodel );
    KDAB_SET_OBJECT_NAME( completer );
    KDAB_SET_OBJECT_NAME( hlay );
    KDAB_SET_OBJECT_NAME( lineedit );
    KDAB_SET_OBJECT_NAME( button );

    //completer.setCompletionMode( QCompleter::PopupCompletion );

    button.setIcon( KIcon("document-open") );
    lineedit.setCompleter( &completer );

    hlay.addWidget( &lineedit );
    hlay.addWidget( &button );

    connect( &button, SIGNAL(clicked()),
             q, SLOT(slotButtonClicked()) );
}

FileNameRequester::Private::~Private() {}

FileNameRequester::FileNameRequester( QWidget * p )
    : QWidget( p ), d( new Private( this ) )
{

}

FileNameRequester::FileNameRequester( const QString & file, QWidget * p )
    : QWidget( p ), d( new Private( this ) )
{
    setFileName( file );
}

FileNameRequester::~FileNameRequester() {}

void FileNameRequester::setFileName( const QString & file ) {
    d->lineedit.setText( file );
}

QString FileNameRequester::fileName() const {
    return d->lineedit.text();
}

void FileNameRequester::setOnlyExistingFiles( bool on ) {
    d->onlyExistingFiles = on;
}

bool FileNameRequester::onlyExistingFiles() const {
    return d->onlyExistingFiles;
}

void FileNameRequester::Private::slotButtonClicked() {
    const QString fileName = q->requestFileName();
    if ( !fileName.isEmpty() )
        q->setFileName( fileName );
}

QString FileNameRequester::requestFileName() {
    if ( d->onlyExistingFiles )
        return QFileDialog::getOpenFileName( this );
    else
        return QFileDialog::getSaveFileName( this );
}
      
#include "moc_filenamerequester.cpp"
