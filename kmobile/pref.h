/*
 * Copyright (C) 2003 Helge Deller <deller@kde.org>
 */

#ifndef _KMOBILEPREF_H_
#define _KMOBILEPREF_H_

#include <kdialogbase.h>
#include <qframe.h>

class KMobilePrefPageOne;
class KMobilePrefPageTwo;

class KMobilePreferences : public KDialogBase
{
    Q_OBJECT
public:
    KMobilePreferences();

private:
    KMobilePrefPageOne *m_pageOne;
    KMobilePrefPageTwo *m_pageTwo;
};

class KMobilePrefPageOne : public QFrame
{
    Q_OBJECT
public:
    KMobilePrefPageOne(QWidget *parent = 0);
};

class KMobilePrefPageTwo : public QFrame
{
    Q_OBJECT
public:
    KMobilePrefPageTwo(QWidget *parent = 0);
};

#endif // _KMOBILEPREF_H_
