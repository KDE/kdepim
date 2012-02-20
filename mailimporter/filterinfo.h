#ifndef FILTERINFO_H
#define FILTERINFO_H


/***************************************************************************
                          filters.h  -  description
                             -------------------
    begin                : Fri Jun 30 2000
    copyright            : (C) 2000 by Hans Dijkema
    email                : kmailcvt@hum.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* Copyright (c) 2012 Montel Laurent <montel@kde.org>                      */

#include <Akonadi/Collection>

#include "mailimporter_export.h"
namespace MailImporter {

class FilterInfoGui;

class MAILIMPORTER_EXPORT FilterInfo
{
public:
  explicit FilterInfo();
  ~FilterInfo();

  void setFilterInfoGui( FilterInfoGui *filterinfogui );
  
  void setStatusMsg( const QString& status );
  void setFrom( const QString& from );
  void setTo( const QString& to );
  void setCurrent( const QString& current );
  void setCurrent( int percent = 0 );
  void setOverall( int percent = 0 );
  void addLog( const QString& log );
  void clear();
  void alert( const QString& message );

  static void terminateASAP();
  bool shouldTerminate() const;
  Akonadi::Collection rootCollection() const;
  void setRootCollection( const Akonadi::Collection &collection );

  QWidget *parent();
  void setRemoveDupMessage( bool removeDupMessage );
  bool removeDupMessage() const;

private:
  Akonadi::Collection m_rootCollection;
  bool m_removeDupMsg;
  static bool s_terminateASAP;
  FilterInfoGui *m_filterInfoGui;
};
}

#endif /* FILTERINFO_H */

