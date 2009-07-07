/****************************************************************************
** Copyright (C) 2001-2009 Klaralvdalens Datakonsult AB.  All rights reserved.
**
** This file is part of the KD Tools library.
**
** Licensees holding valid commercial KD Tools licenses may use this file in
** accordance with the KD Tools Commercial License Agreement provided with
** the Software.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 and version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE.GPL included.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@kdab.net if any conditions of this licensing are not
** clear to you.
**
**********************************************************************/

#ifndef __KDTOOLS__CORE__KDSIGNALBLOCKER_H__
#define __KDTOOLS__CORE__KDSIGNALBLOCKER_H__

#include <utils/kdtoolsglobal.h>

class QObject;

class KDSignalBlocker {
    Q_DISABLE_COPY( KDSignalBlocker )
public:
    explicit KDSignalBlocker( QObject * o );
    explicit KDSignalBlocker( QObject & o );
    ~KDSignalBlocker();
private:
    const bool blocked;
    QObject * const object;
};

#endif /* __KDTOOLS__CORE__KDSIGNALBLOCKER_H__ */

