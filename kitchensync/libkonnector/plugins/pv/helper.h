/***************************************************************************
                         helper.h  -  description
                             -------------------
    begin                : Wed Oct 09 2002
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

#ifndef helper_h
#define helper_h

#include <syncer.h>

namespace PVHelper
{
  class Helper
  {
    public:
      static KSync::Syncee::PtrList XML2Syncee(const QByteArray& array);

      static QByteArray Syncee2XML(KSync::Syncee::PtrList* syncee);

      static KSync::Syncee::PtrList doMeta(KSync::Syncee::PtrList* synceePtrListOld,
                                 KSync::Syncee::PtrList* synceePtrListNew);
  };
};

#endif
