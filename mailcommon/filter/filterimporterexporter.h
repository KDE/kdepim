/*
  Copyright (c) 2007 Till Adam <adam@kde.org>
  Copyright (c) 2012 Laurent Montel <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
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

#ifndef MAILCOMMON_FILTERIMPORTEREXPORTER_H
#define MAILCOMMON_FILTERIMPORTEREXPORTER_H

#include "../mailcommon_export.h"

#include <KSharedConfig>
#include <KUrl>

#include <QList>
#include <QStringList>

class QWidget;

namespace MailCommon
{

class MailFilter;

/**
 * @short Utility class that provides persisting of filters to/from KConfig.
 *
 * @author Till Adam <till@kdab.net>
 */
class MAILCOMMON_EXPORT FilterImporterExporter
{
  public:
    enum FilterType {
        KMailFilter = 0,
        ThunderBirdFilter = 1,
        EvolutionFilter = 2,
        SylpheedFilter = 3, 
        ProcmailFilter = 4,
        BalsaFilter = 5,
        ClawsMailFilter = 6
    };

    /**
     * Creates a new filter importer/exporter.
     *
     * @param parent The parent widget.
     */
    explicit FilterImporterExporter( QWidget *parent = 0 );

    /**
     * Destroys the filter importer/exporter.
     */
    virtual ~FilterImporterExporter();

    /**
     * Exports the given @p filters to a file which
     * is asked from the user. The list to export is also
     * presented for confirmation/selection.
     */
    void exportFilters(const QList<MailFilter*> &filters , const KUrl &fileName = KUrl(), bool saveAll = false);

    /**
     * Imports filters. Ask the user where to import them from
     * and which filters to import.
     */
    QList<MailFilter*> importFilters(
      bool &canceled,
      FilterImporterExporter::FilterType type =  FilterImporterExporter::KMailFilter, const QString& filename = QString() );

    /**
     * Writes the given list of @p filters to the given @p config file.
     */
    static void writeFiltersToConfig(
      const QList<MailFilter*> &filters,
      KSharedConfig::Ptr config,
      bool exportFilter = false );

    /**
     * Reads a list of filters from the given @p config file.
     * Return list of empty filter
     */
    static QList<MailFilter*> readFiltersFromConfig(
      const KSharedConfig::Ptr config,
      QStringList &emptyFilter );

  private:
    //@cond PRIVATE
    Q_DISABLE_COPY( FilterImporterExporter )

    class Private;
    Private *const d;
    //@endcond
};

}

#endif
