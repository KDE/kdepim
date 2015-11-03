/*
 *  filter_evolution.h
 *  Author : Simon MARTIN <simartin@users.sourceforge.net>
 *  Copyright (c) 2004 Simon MARTIN <simartin@users.sourceforge.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef MAILIMPORTER_FILTER_EVOLUTION_HXX
#define MAILIMPORTER_FILTER_EVOLUTION_HXX

#include "filters.h"
/**
 * Imports Evolution mboxes recursively, keeping the folder structure.
 * @author Simon MARTIN
 */
namespace MailImporter
{
class MAILIMPORTER_EXPORT FilterEvolution : public Filter
{
public:
    explicit FilterEvolution();
    ~FilterEvolution();

    void import() Q_DECL_OVERRIDE;
    void importMails(const QString &maildir);
    static QString defaultSettingsPath();
private:
    void importDirContents(const QString &, const QString &, const QString &);
    void importMBox(const QString &, const QString &, const QString &);
};
}

#endif
