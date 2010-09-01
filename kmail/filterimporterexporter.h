/*
    This file is part of KMail.
    Copyright (c) 2007 Till Adam <adam@kde.org>

    KMail is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License, version 2, as
    published by the Free Software Foundation.

    KMail is distributed in the hope that it will be useful, but
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

#ifndef __FILTERIMPORTEREXPORTER_H__
#define __FILTERIMPORTEREXPORTER_H__

#include <tqvaluelist.h>
#include <kdialogbase.h>
class KMFilter;
class KConfig;
class TQWidget;
class KPushButton;

namespace KMail
{

/**
    @short Utility class that provides persisting of filters to/from KConfig.
    @author Till Adam <till@kdab.net>
 */
class FilterImporterExporter
{
public:
      FilterImporterExporter( TQWidget* parent, bool popFilter = false );
      virtual ~FilterImporterExporter();

      /** Export the given filter rules to a file which
       * is asked from the user. The list to export is also
       * presented for confirmation/selection. */
      void exportFilters( const TQValueList<KMFilter*> & );

      /** Import filters. Ask the user where to import them from
       * and which filters to import. */
      TQValueList<KMFilter*> importFilters();

      static void writeFiltersToConfig( const TQValueList<KMFilter*>& filters, KConfig* config, bool bPopFilter );
      static TQValueList<KMFilter*> readFiltersFromConfig( KConfig* config, bool bPopFilter );
private:
      TQWidget* mParent;
      bool mPopFilter;
};
class FilterSelectionDialog : public KDialogBase
{
  Q_OBJECT
public:
  FilterSelectionDialog( TQWidget * parent = 0 );

  virtual ~FilterSelectionDialog();
  virtual void slotCancel();
  bool cancelled();
  void setFilters( const TQValueList<KMFilter*>& filters );

  TQValueList<KMFilter*> selectedFilters() const;
public slots:
  void slotUnselectAllButton();
  void slotSelectAllButton();
private:
  KListView *filtersListView;
  TQValueList<KMFilter*> originalFilters;
  bool wasCancelled;
  KPushButton *selectAllButton;
  KPushButton *unselectAllButton;
};

}

#endif /* __FILTERIMPORTEREXPORTER_H__ */
