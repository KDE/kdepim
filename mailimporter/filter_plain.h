/***************************************************************************
                          FilterPlain.h  -  Plain mail import
                             -------------------
    begin                : Fri Jun 24 2002
    copyright            : (C) 2002 by Laurence Anderson
    email                : l.d.anderson@warwick.ac.uk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef FILTER_PLAIN_HXX
#define FILTER_PLAIN_HXX

#include "filters.h"
#include "filters.h"
#include "mailimporter_export.h"
/**
 *imports Plain text messages into KMail
 *@author laurence
 */
namespace MailImporter {
class MAILIMPORTER_EXPORT FilterPlain : public Filter
{
public:
  explicit FilterPlain();
  ~FilterPlain();

  void import();
};
}

#endif
