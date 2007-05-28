/***************************************************************************
   Copyright (C) 2007
   by Marco Gulino <marco@kmobiletools.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/
#ifndef SMSPART_H
#define SMSPART_H

#include <khtml_part.h>

/**
@author Marco Gulino
*/
class SMS;
class smsPart : public KHTMLPart
{
Q_OBJECT
public:
    explicit smsPart(QWidget *parentWidget=0, const char *widgetname=0, QObject *parent=0, const QString& name=QString(), GUIProfile prof=DefaultGUI);

    ~smsPart();
    SMS *sms() { return p_sms; }
    void setSMS(SMS *sms) { p_sms=sms; }
    static const QString getTemplate();
    private:
        SMS *p_sms;

public slots:
    void openUrlRequest(const KUrl &url);
    void writeHome();
    void show(SMS *sms);
    void slotRemove();
    void slotReply();
    void slotPopupMenu(const QString &url, const QPoint &point);
    signals:
    void getSMSList();
    void writeNew();
    void importList();
    void exportList();
    void exportListToCSV();
    void remove(SMS*);
    void send(SMS*);
    void reply(const QString &);
};

#endif
