/***************************************************************************
                          kncleanup.h  -  description
                             -------------------
    
    copyright            : (C) 2000 by Christian Thurner
    email                : cthurner@freepage.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#ifndef KNCLEANUP_H
#define KNCLEANUP_H

#include <qsemimodal.h>
#include <qlabel.h>
#include <qprogressbar.h>

class KNArticleCollection;
class KNGroup;
class KNFolder;

namespace KNConfig {
class Cleanup;
};


class KNCleanUp {

  public:
    KNCleanUp(KNConfig::Cleanup *cfg);
    ~KNCleanUp();

    void appendCollection(KNArticleCollection *c)   { c_olList.append(c); }
    void start();
    void reset();

    void expireGroup(KNGroup *g, bool showResult=false);
    void compactFolder(KNFolder *f);

  protected:

    class ProgressDialog : public QSemiModal  {

      public:
        ProgressDialog(int steps);
        ~ProgressDialog();

        void showMessage(const QString &s)  { m_sg->setText(s); }
        void doProgress();

      protected:
        QLabel *m_sg;
        QProgressBar *p_bar;

        int s_teps, p_rogress;

    };

    ProgressDialog *d_lg;
    QList<KNArticleCollection> c_olList;
    KNConfig::Cleanup *c_onfig;

};




#endif
