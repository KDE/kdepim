/***************************************************************************
 *   Copyright (C) 2004 by Teemu Rytilahti                                 *
 *   tpr@d5k.net                                                           *
 *                                                                         *
 *   Licensed under GPL.                                                   *
 ***************************************************************************/

#ifndef ABOUTDATA_H
#define ABOUTDATA_H

#include <kaboutdata.h>
#include <kdepimmacros.h>

namespace Akregator {
/**
@author Teemu Rytilahti
*/
class KDE_EXPORT AboutData : public KAboutData
{
public:
    AboutData();
    ~AboutData();
};

}

#endif
