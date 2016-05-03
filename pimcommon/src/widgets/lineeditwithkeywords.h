/*
  Copyright (c) 2016 Rebois Guillaume <guillaume.rebois@orange.fr>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef LINEEDITWITHKEYWORDS_H
#define LINEEDITWITHKEYWORDS_H

#include "lineeditwithcompleter.h"

namespace PimCommon
{
class PIMCOMMON_EXPORT LineEditWithKeywords : public LineEditWithCompleter
{
    Q_OBJECT
public:
    explicit LineEditWithKeywords(QWidget *parent = Q_NULLPTR);
    ~LineEditWithKeywords();
private:
    void slotClearHistory() Q_DECL_OVERRIDE;
    class Private;
    Private *const d;
};
}
#endif // KEYWORDS_H
