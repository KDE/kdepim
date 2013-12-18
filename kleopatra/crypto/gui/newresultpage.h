/* -*- mode: c++; c-basic-offset:4 -*-
    crypto/gui/newresultpage.h

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2008, 2009 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
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

#ifndef __KLEOPATRA_CRYPTO_GUI_NEWRESULTPAGE_H__
#define __KLEOPATRA_CRYPTO_GUI_NEWRESULTPAGE_H__


#include <QWizardPage>

#include <utils/pimpl_ptr.h>

namespace boost {
    template <typename T> class shared_ptr;
}

namespace Kleo {
namespace Crypto {
    class TaskCollection;
    class Task;
}
}

namespace Kleo {
namespace Crypto {
namespace Gui {

class NewResultPage : public QWizardPage {
    Q_OBJECT
public:
    explicit NewResultPage( QWidget * parent=0 );
    ~NewResultPage();

    void setTaskCollection( const boost::shared_ptr<TaskCollection> & coll );
    void addTaskCollection( const boost::shared_ptr<TaskCollection> & coll );

    /* reimp */ bool isComplete() const;

    bool keepOpenWhenDone() const;
    void setKeepOpenWhenDone( bool keep );

    void setKeepOpenWhenDoneShown( bool on );

Q_SIGNALS:
    void linkActivated( const QString & link );

private:
    class Private;
    kdtools::pimpl_ptr<Private> d;
    Q_PRIVATE_SLOT( d, void progress( QString, int, int ) )
    Q_PRIVATE_SLOT( d, void result( boost::shared_ptr<const Kleo::Crypto::Task::Result> ) )
    Q_PRIVATE_SLOT( d, void started( boost::shared_ptr<Kleo::Crypto::Task> ) )
    Q_PRIVATE_SLOT( d, void keepOpenWhenDone( bool ) )
    Q_PRIVATE_SLOT( d, void allDone() )
};

}
}
}

#endif // __KLEOPATRA_CRYPTO_GUI_NEWRESULTPAGE_H__
