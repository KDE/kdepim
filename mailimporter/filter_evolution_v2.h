/***************************************************************************
            filter_evolution_v2.h  -  Evolution 2.0.x mail import
                             -------------------
    begin                : Januar 26 2005
    copyright            : (C) 2005 by Danny Kukawka <danny.kukawka@web.de>
                           (inspired and partly copied from filter_evolution)
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MAILIMPORTER_FILTER_EVOLUTION_V2_HXX
#define MAILIMPORTER_FILTER_EVOLUTION_V2_HXX

#include "filters.h"
/**
 * Imports Evolution v2.x mail folder recursively, keeping the folder structure.
 * @author Danny Kukawka
 */

namespace MailImporter {
class MAILIMPORTER_EXPORT FilterEvolution_v2 : public Filter
{
public:
    explicit FilterEvolution_v2();
    ~FilterEvolution_v2();

    void import();
    void importMails( const QString& maildir );
    static QString defaultSettingsPath();

private:
    void importDirContents(const QString&, const QString&, const QString&);
    void importMBox(const QString&, const QString&, const QString&);
    bool excludeFiles( const QString & file );

};
}

#endif
