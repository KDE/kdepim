/*
    job.h

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    Libkleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

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

#ifndef __KLEO_JOB_H__
#define __KLEO_JOB_H__

#include <qobject.h>
#include <qstring.h>

class QWidget;

namespace Kleo {

  /**
     @short An abstract base class for asynchronous crypto operations

     During the operation, you might receive progress updates through
     the progress() signal as they arrive, but an implementation is
     free to not send progress information. You should show a busy
     progressbar until the first progress() signal is received.

     The done() signal is emitted _before_ the result() signals of
     subclasses and should be used to hide and/or reset progress bars,
     not to learn of the end of the operation. Use the result()
     signals for that.

     To cancel the operation, simply call slotCancel(). The result()
     signal of subclasses will still be emitted, though, and will
     carry the information that the operation was canceled.
  */
  class Job : public QObject {
    Q_OBJECT
  protected:
    Job( QObject * parent, const char * name );
  public:
    ~Job();

    virtual void showErrorDialog( QWidget * parent=0, const QString & caption=QString::null ) const;

  public slots:
    virtual void slotCancel() = 0;

  signals:
    void progress( const QString & what, int current, int total );
    void done();
  };

}

#endif // __KLEO_JOB_H__
