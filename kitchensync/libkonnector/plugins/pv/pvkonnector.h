/***************************************************************************
                          pvkonnector.h  -  description
                             -------------------
    begin                : Wed Sep 18 2002
    copyright            : (C) 2002 by Maurus Erni
    email                : erni@pocketviewer.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef pvplugin_h
#define pvplugin_h

#include <qiconset.h>
#include <qptrlist.h>
#include <konnectorplugin.h>

namespace KSync {

    class PVPlugin : public KonnectorPlugin
    {
        Q_OBJECT
    public:
        PVPlugin(QObject *obj, const char *name, const QStringList );
        ~PVPlugin();

        virtual void setUDI(const QString & );
        virtual QString udi()const;
        virtual Kapabilities capabilities( );
        virtual void setCapabilities( const KSync::Kapabilities &kaps );
        virtual bool startSync();
        virtual bool startBackup(const QString& path);
        virtual bool startRestore(const QString& path);
        virtual bool connectDevice() { return true; }
        virtual void disconnectDevice() { }
        virtual bool isConnected();
        virtual bool insertFile(const QString &fileName );
        virtual QByteArray retrFile(const QString &path );
        virtual Syncee* retrEntry(const QString &path);
        virtual QString metaId()const;
        virtual QIconSet iconSet() const;
        virtual QString iconName()const;
        /* FIXME get rid of inline without inline */
        virtual QString id()const { return QString::fromLatin1("Pocket Viewer"); };
        virtual ConfigWidget* configWidget( const Kapabilities&, QWidget*,  const char* ) { return 0l; }
        virtual ConfigWidget* configWidget( QWidget*, const char* ) { return 0l; }

    private:
        QString m_udi;
        CasioPVLink* casioPVLink;

    signals:
        void sync(const QString&, Syncee::PtrList );
        void errorKonnector(const QString&, int, const QString& );
        void stateChanged( const QString&,  bool );

    public slots:
        virtual void slotWrite(const QString &, const QByteArray & ) ;
        virtual void slotWrite(Syncee::PtrList ) ;
        virtual void slotWrite(KOperations::ValueList ) ;

    private slots:
        void slotSync(Syncee::PtrList);
        void slotErrorKonnector(int , QString);
        void slotChanged(bool);
    };
};

#endif
