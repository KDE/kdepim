/*
    This file is part of libkolabformat - the library implementing the
    Kolab storage format. See www.kolab.org for documentation on this.

    Copyright (c) 2004  Bo Thorsen <bo@sonofthor.dk>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef KOLAB_JOURNAL_H
#define KOLAB_JOURNAL_H

#include "base.h"

namespace KCal {
  class Journal;
}

namespace KolabFormat {

class Journal : public Base {
public:
  Journal();
  virtual ~Journal();

  virtual QString type() const { return "Journal"; }

  /// Set the fields to hold this journals entries
  void setFields( KCal::Journal* journal );

  virtual void setSummary( const QString& summary );
  virtual QString summary() const;

  virtual void setStartDate( const QDateTime& startDate );
  virtual QDateTime startDate() const;

  virtual void setEndDate( const QDateTime& endDate );
  virtual QDateTime endDate() const;

  // Load the attributes of this class
  virtual bool loadAttribute( QDomElement& );

  // Save the attributes of this class
  virtual bool saveAttributes( QDomElement& ) const;

  // Load this journal by reading the XML file
  virtual bool load( const QDomDocument& xml );

  // Serialize this journal to an XML string
  virtual QString save() const;

protected:
  QString mSummary;
  QDateTime mStartDate;
  QDateTime mEndDate;
};

}

#endif // KOLAB_JOURNAL_H
