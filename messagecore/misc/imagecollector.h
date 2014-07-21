/*  -*- c++ -*-
 *    imagecollector.h
 *
 *    This file is part of KMail, the KDE mail client.
 *    Copyright (c) 2004 Marc Mutz <mutz@kde.org>
 *    Copyright (c) 2011 Torgny Nyblom <nyblom@kde.org>
 *
 *    KMail is free software; you can redistribute it and/or modify it
 *    under the terms of the GNU General Public License, version 2, as
 *    published by the Free Software Foundation.
 *
 *    KMail is distributed in the hope that it will be useful, but
 *    WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *    In addition, as a special exception, the copyright holders give
 *    permission to link the code of this program with any edition of
 *    the Qt library by Trolltech AS, Norway (or with modified versions
 *    of Qt that use the same license as Qt), and distribute linked
 *    combinations including the two.  You must obey the GNU General
 *    Public License in all respects for all of the code used other than
 *    Qt.  If you modify this file, you may extend this exception to
 *    your version of the file, but you are not obligated to do so.  If
 *    you do not wish to do so, delete this exception statement from
 *    your version.
 */

#ifndef MESSAGECORE_IMAGECOLLECTOR_H
#define MESSAGECORE_IMAGECOLLECTOR_H

#include "messagecore_export.h"

#include <vector>
#include <QObject>

namespace KMime {
class Content;
}

namespace MessageCore {

/**
   * @short A helper class to collect the embedded images of a email.
   *
   * @author Marc Mutz <mutz@kde.org>
   * @author Torgny Nyblom <nyblom@kde.org>
   * @todo: Make it a simple static method?!?
   */
class MESSAGECORE_EXPORT ImageCollector
{
public:
    /**
     * Creates a new image collector.
     */
    ImageCollector();
    
    /**
     * Destroys the image collector.
     */
    ~ImageCollector();
    
    /**
     * Starts collecting the images.
     *
     * @param content The email content that contains the images.
     */
    void collectImagesFrom( KMime::Content *content );
    
    /**
     * Returns the collected images.
     */
    const std::vector<KMime::Content*>& images() const;
    
private:
    //@cond PRIVATE
    class Private;
    Private* const d;
    
    Q_DISABLE_COPY( ImageCollector )
    //@endcond
};

}

#endif
