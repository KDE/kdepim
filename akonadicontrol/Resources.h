/*******************************************************************************
**
** Filename   : Resources.h
** Created on : 27 May, 2006
** Copyright  : (c) 2006 Till Adam
** Email      : adam@kde.org
**
*******************************************************************************/

/*******************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
*******************************************************************************/

#ifndef RESOURCES_H
#define RESOURCES_H

#include <qobject.h> // include for the base class

namespace Akonadi
{

class Resources : public QObject
{
Q_OBJECT
public:
    Resources();
    ~Resources();

public slots:
    QStringList listAvailableResources();
                
}; // End of class Resources

} // End of namespace Akonadi


#endif // RESOURCES_H
