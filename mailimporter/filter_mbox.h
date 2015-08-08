/***************************************************************************
                          filter_mbox.h  -  mbox mail import
                             -------------------
    begin                : Sat Apr 5 2003
    copyright            : (C) 2003 by Laurence Anderson
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

#ifndef MAILIMPORTER_FILTER_MBOX_HXX
#define MAILIMPORTER_FILTER_MBOX_HXX

#include "filters.h"

/**
 * imports mbox archives messages into KMail
 * @author Laurence Anderson
 */
namespace MailImporter
{
class MAILIMPORTER_EXPORT FilterMBox : public Filter
{
public:
    FilterMBox();
    ~FilterMBox();

    void importMails(const QStringList &filenames);
    void import() Q_DECL_OVERRIDE;
};
}

#endif
