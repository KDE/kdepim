/**************************************************************************
*   Copyright (C) 2004 by Reinhold Kainhofer <reinhold@kainhofer.com>     *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
***************************************************************************/
#ifndef BLOGGERWRAPPER_H
#define BLOGGERWRAPPER_H

#include <bloginterface.h>
#include <qobject.h>

namespace KBlog {

class BloggerPostingWrapper : public QObject
{
  Q_OBJECT
public:
  BloggerPostingWrapper( BlogPosting *post, QObject *parent = 0 ) : 
         QObject( parent ), mBlog( post ) {}
  virtual ~BloggerPostingWrapper() {}
  
  BlogPosting *mBlog;

signals:
  void errorSignal( const QString & );
  void deleteFinishedSignal( bool );
  void postFinishedSignal( bool );
  void editFinishedSignal( bool );
  
public slots:
  void deleteFinished( const QValueList<QVariant> & );
  void postFinished( const QValueList<QVariant> & );
  void editFinished( const QValueList<QVariant> & );
  void deleteFault( int, const QString& );
  void postFault( int, const QString& );
};

};
#endif
