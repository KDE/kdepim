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

/**
  * PVPlugin implements the KitchenSync API. This class is used to handle the
  * communication between the Plugin and KitchenSync.
  * @author Maurus Erni
  */
 
namespace KSync {

  class PVPlugin : public KonnectorPlugin
  {
    Q_OBJECT
      public:
    
        /** Constructor
          *
          * @param obj The Parent QObject
          * @param name The name of this instance
          */
        PVPlugin(QObject *obj, const char *name, const QStringList );
      
        /** Destructor
          *
          */
        ~PVPlugin();

        /**
          * Sets the uid of the konnector. This uid is given at loading of
          * the plugin.
          * @param uid The uid of the konnector
          */        
        virtual void setUDI(const QString & uid);
        
        /**
          * Returns the uid of the konnector.
          * @return QString The uid of the konnector
          */        
        virtual QString udi()const;
        
        /**
          * Sets the kapabilities of the konnector depending on the
          * configuration.
          * @param uid The uid of the konnector
          */        
        virtual void setCapabilities( const KSync::Kapabilities &kaps );
        
        /**
          * Returns the kapabilities of the konnector.
          * @return Kapabilities The kapabilities of the konnector
          */                
        virtual Kapabilities capabilities( );
        
        /**
          * Starts the synchronization procedure of the Plugin
          * @return bool Starting of synchronization successful (yes / no)
          */
        virtual bool startSync();
        
        /**
          * Starts the backup procedure of the Plugin.
          * @param path The path of the backup file
          * @return bool Starting of backup successful (yes / no)
          */
        virtual bool startBackup(const QString& path);

        /**
          * Starts the restore procedure of the Plugin.
          * @param path The path of the restore file
          * @return bool Starting of restore successful (yes / no)
          */
        virtual bool startRestore(const QString& path);

        /**
          * Returns whether the PV is connected.
          * @return bool PV connected (yes / no)
          */        
        virtual bool isConnected();
        
        /**
          * Filedownload to the PV. Not used yet!
          * @param filename The path of the file to be donwloaded
          * @return bool File download successful (yes / no)
          */        
        virtual bool insertFile(const QString &fileName);

        /**
          * Fileupload from the PV. Not used yet!
          * @param filename The path of the file to be donwloaded
          * @return QByteArray The requested file as a QByteArray
          */        
        virtual QByteArray retrFile(const QString &path);

        /**
          * Getting of a file from the PV returned as Syncee*.
          * @param path The path of the file to be donwloaded
          * @return Syncee* The requested file as a Syncee*
          */                
        virtual Syncee* retrEntry(const QString &path);

        /**
          * Returns the Id of the connected PV.
          * @return QString The Id of the connected PV
          */                        
        virtual QString metaId()const;

        /**
          * Returns the icon of the Konnector. Not used yet!
          * @return QIconSet The icon of the Konnector
          */                                
        virtual QIconSet iconSet() const;
        
        /**
          * Returns the icon name of the Konnector. Not used yet!
          * @return QString The icon name of the Konnector
          */                                
        virtual QString iconName()const;

        /**
          * Returns the icon name of the Konnector. Not used yet!
          * @return QString The icon name of the Konnector
          */                                        
        virtual QString id()const { return QString::fromLatin1("Pocket Viewer"); };

        /**
          * Returns a ConfigureWidget which can be used to configure the Konnector.
          * Not used yet!
          * Note this only works if a QApplication with type Gui* was created
          * @param kapa The kapabilities of the plugin
          * @param parent The QWidget parent
          * @param name The name of the parent widget
          * @return ConfigWidget* 0 if no QWidget could be created
          */        
        virtual ConfigWidget* configWidget(const Kapabilities& kapa,
                                            QWidget* parent, const char* name)
          { return 0l; }

        /**
          * Returns a ConfigureWidget which can be used to configure the Konnector.
          * Not used yet!
          * Note this only works if a QApplication with type Gui* was created
          * @param parent The QWidget parent
          * @param name The name of the parent widget
          * @return ConfigWidget* 0 if no QWidget could be created
          */                
        virtual ConfigWidget* configWidget(QWidget* parent, const char* name)
          { return 0l; }

        /**
          * Connects the PV. Not used yet!
          * @return bool Could PV be connected (yes / no)
          */                    
        virtual bool connectDevice() { return true; }

        /**
          * Disconnects the PV. Not used yet!
          */                            
        virtual void disconnectDevice() { }          

      // ------------------------ Private methods --------------------------- //
      
    private:
      /**
         * Holds the uid of the connected PV
         */    
        QString m_udi;
        
      /**
         * CasioPVLink* The CasioPVLink object
         */
        CasioPVLink* casioPVLink;


      // --------------------- Signals to be emitted ----------------------- //

    signals:
    
      /**
        * When the Konnector fetched all data, sync is emitted
        * @param udi The uid of the Konnector
        * @param lis The data of the PV as a Syncee::PtrList
        */      
      void sync(const QString& udi, Syncee::PtrList lis);

      /**
        * When the connected PV reports an error, errorKonnector is emitted
        * @param udi The konnector id
        * @param number The error number
        * @param msg The error message
        */
      void errorKonnector(const QString& udi, int number, const QString& msg);
        
      /**
        * When the connection state of the PV changed (connected /
        * disconnected), stateChanged is emitted.
        * @param udi The konnector id
        * @param state The state of the PV (connected / disconnected)
        */        
      void stateChanged( const QString& udi, bool state);

      
      // ---------------------- Slots to be received ------------------------ //
      
    public slots:
        
        /**
          * This will write a List of Syncee to the PV. Is called from
          * KitchenSync after synchronization.
          * @param lis The list of Syncee
          */
        virtual void slotWrite(Syncee::PtrList lis);
        
        /**
          * This will do the KOperations. Not used yet!
          * @param ops Operations like delete
          */
        virtual void slotWrite(KOperations::ValueList ops);

        /**
          * This will write the QByteArray to the PV. Not used yet!
          * @param dest The destination of the array
          * @param array The array
          */
        virtual void slotWrite(const QString & dest, const QByteArray & array);

        
      // ------------------ Private slots to be received -------------------- //
      
    private slots:
    
      /**
        * Will be called from the PV Library if all data was fetched from the PV
        * @param lis The data of the PV as a Syncee::PtrList
        */      
        void slotSync(Syncee::PtrList);
        
      /**
        * Will be called from the PV Library when the connected PV reported an
        * error
        * @param number The error number
        * @param msg The error message
        */
         void slotErrorKonnector(int number, QString msg);
        
      /**
        * Will be called from the PV Library when the state of the connected PV
        * has changed.
        * @param state The state of the PV (connected / disconnected)
        */          
        void slotChanged(bool state);
    };
};

#endif
