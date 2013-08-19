/***************************************************************************
                          kmailcvt.h  -  description
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KMAILCVT_H
#define KMAILCVT_H

#include <KAssistantDialog>
#include <Akonadi/Collection>

class KPageWidgetItem;
class KSelFilterPage;
class KImportPage;


/** KMailCVT is the base class of the project */
class KMailCVT : public KAssistantDialog
{
    Q_OBJECT
public:
    explicit KMailCVT(QWidget* parent=0);
    ~KMailCVT();

    void next();
    void reject();

private slots:
    void collectionChanged( const Akonadi::Collection &selectedCollection );

private:
    void writeConfig();
    void readConfig();
    KPageWidgetItem* page1;
    KPageWidgetItem* page2;
    KSelFilterPage *selfilterpage;
    KImportPage *importpage;

};

#endif
