/*
    This file is part of KitchenSync.

    Copyright (c) 2004 Holger Hans Peter Freyther <freyther@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/


#ifndef KS_PLUCKER_PART_H
#define KS_PLUCKER_PART_H

#include "pluckerinterface.h"
#include <actionpart.h>
#include <profile.h>

#include <synceelist.h>

#include <kaboutdata.h>

#include <qpixmap.h>


class KSPluckerConfigWidget;
class QTextEdit;
class KTempDir;

using KSync::Profile; // ugly ugly #FIXME

namespace KSync {
class KonnectorView;
}

namespace KSPlucker {
class PluckerProcessHandler;
class PluckerPart : public KSync::ActionPart, virtual public PluckerInterface 
{
    Q_OBJECT
public:
    PluckerPart( QWidget* parent, const char* name, QObject* obj,
                 const char* name2, const QStringList& = QStringList() );
    virtual ~PluckerPart();

    static KAboutData *createAboutData();

    QString type()const;
    QString title()const;
    QString description()const;
    bool hasGui()const;
    QPixmap *pixmap();
    QString iconName()const;
    QWidget *widget();

    bool needsKonnectorWrite()const;

    void executeAction();

    bool configIsVisible()const;
    QWidget* configWidget();

//////
/// Plucker Interface
    ASYNC addPluckerUrl( KURL );
    ASYNC addPluckerFeed( KURL );

public slots:
    void slotConfigOk();

private slots:
    void slotCleanUp();
    void slotFinished(PluckerProcessHandler*);
    void slotProfileChanged(const Profile&);

private:
    QPixmap m_pixmap;
    QWidget *m_widget;
    KSPluckerConfigWidget *m_config;
    QTextEdit *m_edit;
    KSync::KonnectorView *m_view;
    KTempDir *m_temp;
    bool m_done : 1;
};
}

#endif
