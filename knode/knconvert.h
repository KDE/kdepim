/*
    knconvert.h

    KNode, the KDE newsreader
    Copyright (c) 1999-2001 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#ifndef KNCONVERT_H
#define KNCONVERT_H

#include <time.h>

#include <qdialog.h>

#include <qglobal.h>
#include <qptrlist.h>
#include <kdepimmacros.h>

class QListBox;
class QLabel;
class QWidgetStack;
class QCheckBox;

class KLineEdit;
class KProcess;


class KDE_EXPORT KNConvert : public QDialog {

  Q_OBJECT

  public:
    static bool needToConvert(const QString &oldVersion);

    KNConvert(const QString &version);
    ~KNConvert();
    bool conversionDone()const { return c_onversionDone; }


  protected:

    //------------ <Converter-classes> ---------------

    //Base class for all converters
    class Converter {

      public:
        Converter(QStringList *log) { l_og=log; }
        virtual ~Converter() {}
        virtual bool doConvert()=0;

      protected:
        QStringList *l_og;
    };


    //Converter for version 0.4
    class Converter04 : public Converter {

      public:
        Converter04(QStringList *log) : Converter(log) {}
        ~Converter04() {}
        bool doConvert();

      protected:
        int convertFolder(QString srcPrefix, QString dstPrefix);

        struct OldFolderIndex {
          int id,
              status,
              so,
              eo,
              sId;
          time_t ti;
        };

        struct NewFolderIndex {
          int id,
              so,
              eo,
              sId;
          time_t ti;
          bool flags[6];
        };
    };

    //------------ </Converter-classes> --------------

    QWidgetStack  *s_tack;
    QWidget       *w_1,
                  *w_3;
    QCheckBox     *c_reateBkup;
    QLabel        *b_ackupPathLabel,
                  *w_2,
                  *r_esultLabel;
    KLineEdit     *b_ackupPath;
    QPushButton   *b_rowseBtn,
                  *s_tartBtn,
                  *c_ancelBtn;
    QListBox      *l_ogList;

    QPtrList<Converter> c_onverters;
    QStringList l_og;
    bool c_onversionDone;
    QString v_ersion;
    KProcess     *t_ar;

    void convert();

  protected slots:
    void slotStart();
    void slotCreateBkupToggled(bool b);
    void slotBrowse();
    void slotTarExited(KProcess *proc);

};


#endif // KNCONVERT_H
