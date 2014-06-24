/* -*- mode: c++; c-basic-offset:4 -*-
    ui/filenamerequester.cpp

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

#include <KLineEdit>

#include <QHBoxLayout>
#include <QToolButton>
#include <QCompleter>
#include <QDirModel>
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
#ifndef QT_NO_DIRMODEL
    QDirModel  dirmodel;
    QCompleter completer;
#else
    QDir::Filters filter;
#endif

    KLineEdit    lineedit;
    QToolButton  button;
    QHBoxLayout hlay;

    QString nameFilter;
    bool existingOnly;
};

FileNameRequester::Private::Private( FileNameRequester * qq )
    : q( qq ),
#ifndef QT_NO_DIRMODEL
      dirmodel(),
      completer( &dirmodel ),
#else
      filter(),
#endif
      lineedit( q ),
      button( q ),
      hlay( q ),
      nameFilter(),
      existingOnly( true )
{
#ifndef QT_NO_DIRMODEL
    dirmodel.setObjectName( QLatin1String("dirmodel") );
    completer.setObjectName( QLatin1String("completer") );
#endif
    lineedit.setObjectName( QLatin1String("lineedit") );
    button.setObjectName( QLatin1String("button") );
    hlay.setObjectName( QLatin1String("hlay") );

    button.setIcon( QIcon::fromTheme(QLatin1String("document-open")) );
#ifndef QT_NO_DIRMODEL
    lineedit.setCompleter( &completer );
#endif
    lineedit.setClearButtonShown(true);
    hlay.setMargin( 0 );
    hlay.addWidget( &lineedit );
    hlay.addWidget( &button );

    connect( &button, SIGNAL(clicked()),
             q, SLOT(slotButtonClicked()) );
    connect( &lineedit, SIGNAL(textChanged(QString)),
             q, SIGNAL(fileNameChanged(QString)) );
}

FileNameRequester::Private::~Private() {}

FileNameRequester::FileNameRequester( QWidget * p )
    : QWidget( p ), d( new Private( this ) )
{

}

FileNameRequester::FileNameRequester( QDir::Filters f, QWidget * p )
    : QWidget( p ), d( new Private( this ) )
{
#ifndef QT_NO_DIRMODEL
    d->dirmodel.setFilter( f );
#else
    d->filter = f;
#endif
}

FileNameRequester::~FileNameRequester() {
    delete d;
}

void FileNameRequester::setFileName( const QString & file ) {
    d->lineedit.setText( file );
}

QString FileNameRequester::fileName() const {
    return d->lineedit.text();
}

void FileNameRequester::setExistingOnly( bool on ) {
    d->existingOnly = on;
}

bool FileNameRequester::existingOnly() const {
    return d->existingOnly;
}

void FileNameRequester::setFilter( QDir::Filters f ) {
#ifndef QT_NO_DIRMODEL
    d->dirmodel.setFilter( f );
#else
    d->filter = f;
#endif
}

QDir::Filters FileNameRequester::filter() const {
#ifndef QT_NO_DIRMODEL
    return d->dirmodel.filter();
#else
    return d->filter;
#endif
}

void FileNameRequester::setNameFilter( const QString & nameFilter ) {
    d->nameFilter = nameFilter;
}

QString FileNameRequester::nameFilter() const {
    return d->nameFilter;
}

void FileNameRequester::Private::slotButtonClicked() {
    const QString fileName = q->requestFileName();
    if ( !fileName.isEmpty() )
        q->setFileName( fileName );
}

QString FileNameRequester::requestFileName() {
#ifndef QT_NO_FILEDIALOG
    const QDir::Filters filters = filter();
    if ( (filters & QDir::Dirs) && !(filters & QDir::Files) )
        return QFileDialog::getExistingDirectory( this );
    else if ( d->existingOnly )
        return QFileDialog::getOpenFileName( this, QString(), QString(), d->nameFilter );
    else
        return QFileDialog::getSaveFileName( this, QString(), QString(), d->nameFilter );
#else
    return QString();
#endif
}
      
#include "moc_filenamerequester.cpp"
