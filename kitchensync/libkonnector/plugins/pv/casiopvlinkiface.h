/***************************************************************************
                        casiopvlinkiface.h  -  description
                             -------------------
    begin                : Sun Oct 06 2002
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

#ifndef CASIOPVLINKIFACE_H
#define CASIOPVLINKIFACE_H

#include <dcopobject.h>

/** This class implements the public DCOP interface of the PV Konnector.
  * The DCOP calls are used to get signals from the PV Library (e.g. errors,
  * all data fetched, ...).
  * @author Maurus Erni
  */

class CasioPVLinkIface : virtual public DCOPObject
{
  K_DCOP
  k_dcop:

    /**
       * Belongs to the DCOP interface of the PV Plugin.
       * This method is called when all changes are fetched from the PV.
       * @param stream The changes from PV to be synchronized.
       */
    virtual void getChangesDone(const QByteArray& stream) = 0;

    /**
       * Belongs to the DCOP interface of the PV Plugin.
       * This method is called when all data is fetched from the PV.
       * @param stream The data from PV to be synchronized.
       */
    virtual void getAllEntriesDone(const QByteArray& stream) = 0;

    /**
       * Belongs to the DCOP interface of the PV Plugin.
       * This method is called when all changes were written to the PV
       * after synchronization.
       * @param ok Writing successful (yes / no)
       */
    virtual void setChangesDone(const bool ok) = 0;

    /**
       * Belongs to the DCOP interface of the PV Plugin.
       * This method is called when all data was written to the PV
       * after synchronization.
       * @param ok Writing successful (yes / no)
       */
    virtual void setAllEntriesDone(const bool ok) = 0;

    /**
       * Belongs to the DCOP interface of the PV Plugin.
       * This method is called when an error occurred in the PV Library
       * @param msg The error message to be displayed
       * @param errorcode The error number of the exception
       */
    virtual void errorPV(const QString& msg, const unsigned int errorcode) = 0;
};

#endif
