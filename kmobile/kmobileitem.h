/*
 * Copyright (C) 2003 Helge Deller <deller@kde.org>
 */

#ifndef _KMOBILEITEM_H_
#define _KMOBILEITEM_H_

#include <tqiconview.h>
#include <tqpixmap.h>
#include <ktrader.h>
#include "kmobiledevice.h"

class KMobileItem : public TQObject, public TQIconViewItem 
{
    Q_OBJECT
    friend class KMobileView;
public:
    KMobileItem(TQIconView *parent, KConfig *config, KService::Ptr service);
    KMobileItem(TQIconView *parent, KConfig *config, int reload_index);
    virtual ~KMobileItem();

    void configSave() const;
    bool configLoad(int index);

    TQString config_SectionName( int idx = -1 ) const;
    TQPixmap getIcon() const;

    static KTrader::OfferList getMobileDevicesList();

protected:
    TQString getKonquMimeType() const;
    void writeKonquMimeFile() const;


    KService::Ptr getServicePtr() const;
    bool driverAvailable();
    KMobileDevice *m_dev;

signals:

private slots:

private:
    KConfig *config;

    TQString m_deviceConfigFile;
    TQString m_deviceDesktopFile;

    TQString m_iconName;
};

#endif // _KMOBILEITEM_H_
