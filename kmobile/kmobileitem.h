/*
 * Copyright (C) 2003 Helge Deller <deller@kde.org>
 */

#ifndef _KMOBILEITEM_H_
#define _KMOBILEITEM_H_

#include <qiconview.h>
#include <qpixmap.h>
#include <ktrader.h>
#include "kmobiledevice.h"

class KMobileItem : public QObject, public QIconViewItem 
{
    Q_OBJECT
    friend class KMobileView;
public:
    KMobileItem(QIconView *parent, KConfig *config, KService::Ptr service);
    KMobileItem(QIconView *parent, KConfig *config, int reload_index);
    virtual ~KMobileItem();

    void configSave() const;
    bool configLoad(int index);

    QString config_SectionName( int idx = -1 ) const;
    QPixmap getIcon() const;

    static KTrader::OfferList getMobileDevicesList();

protected:
    KService::Ptr getServicePtr() const;
    bool driverAvailable();
    KMobileDevice *m_dev;

signals:

private slots:

private:
    KConfig *config;

    QString m_deviceConfigFile;
    QString m_deviceDesktopFile;

    QString m_iconName;
};

#endif // _KMOBILEITEM_H_
