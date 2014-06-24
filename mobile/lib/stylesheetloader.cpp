/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "stylesheetloader.h"

#include <QDebug>
#include <KStandardDirs>
#include <QFile>
#include <QWidget>
#include <QApplication>

class StyleSheetLoaderPrivate
{
  public:
    StyleSheetLoaderPrivate() : appliedGlobally(false)
    {
      QFile f( KStandardDirs::locate( "data", QLatin1String("mobileui/stylesheet.css") ) );
      if ( f.open( QFile::ReadOnly ) ) {
        styleSheet = QString::fromUtf8( f.readAll() );
        qDebug() << "loaded stylesheet" << f.fileName();
      } else {
        qCritical() << "failed to read stylesheet: " << f.fileName();
      }
    }

    QString styleSheet;
    bool appliedGlobally;
};

Q_GLOBAL_STATIC( StyleSheetLoaderPrivate, s_styleSheetLoader )

void StyleSheetLoader::applyStyle(QWidget* widget)
{
#ifndef QT_NO_STYLE_STYLESHEET
  if ( widget && !s_styleSheetLoader->appliedGlobally && !s_styleSheetLoader->styleSheet.isEmpty() )
    widget->setStyleSheet( s_styleSheetLoader->styleSheet );
#endif
}

void StyleSheetLoader::applyStyle(QApplication* app)
{
#ifndef QT_NO_STYLE_STYLESHEET
  if ( app && !s_styleSheetLoader->styleSheet.isEmpty() )
    app->setStyleSheet( s_styleSheetLoader->styleSheet );
#endif
}

QString StyleSheetLoader::styleSheet()
{
  return s_styleSheetLoader->styleSheet;
}

