/***************************************************************************
                  filter_opera.h  -  Outlook mail import
                             -------------------
    begin                : Januar 26 2005
    copyright            : (C) 2005 by Danny Kukawka
    email                : danny.kukawka@web.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MAILIMPORTER_FILTER_OUTLOOK_HXX
#define MAILIMPORTER_FILTER_OUTLOOK_HXX

#include "filters.h"
/**
 * imports mails from Outlook pst-files into KMail
 * NOTE: This is a dummy at the moment
 * @author Danny Kukawka
 */
namespace MailImporter {
class MAILIMPORTER_EXPORT FilterOutlook : public Filter
{
public:
  explicit FilterOutlook();
  ~FilterOutlook();

  void import();
};
}

#endif
