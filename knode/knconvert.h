/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2005 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifndef KNCONVERT_H
#define KNCONVERT_H

#include <time.h>

#include <tqdialog.h>

#include <tqglobal.h>
#include <tqvaluelist.h>
#include <kdepimmacros.h>

class TQListBox;
class TQLabel;
class TQWidgetStack;
class TQCheckBox;

class KLineEdit;
class KProcess;


class KDE_EXPORT KNConvert : public TQDialog {

  Q_OBJECT

  public:
    static bool needToConvert(const TQString &oldVersion);

    KNConvert(const TQString &version);
    ~KNConvert();
    bool conversionDone()const { return c_onversionDone; }


  protected:

    //------------ <Converter-classes> ---------------

    //Base class for all converters
    class Converter {

      public:
        Converter(TQStringList *log) { l_og=log; }
        virtual ~Converter() {}
        virtual bool doConvert()=0;

      protected:
        TQStringList *l_og;
    };


    //Converter for version 0.4
    class Converter04 : public Converter {

      public:
        Converter04(TQStringList *log) : Converter(log) {}
        ~Converter04() {}
        bool doConvert();

      protected:
        int convertFolder(TQString srcPrefix, TQString dstPrefix);

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

    TQWidgetStack  *s_tack;
    TQWidget       *w_1,
                  *w_3;
    TQCheckBox     *c_reateBkup;
    TQLabel        *b_ackupPathLabel,
                  *w_2,
                  *r_esultLabel;
    KLineEdit     *b_ackupPath;
    TQPushButton   *b_rowseBtn,
                  *s_tartBtn,
                  *c_ancelBtn;
    TQListBox      *l_ogList;

    TQValueList<Converter*> mConverters;
    TQStringList l_og;
    bool c_onversionDone;
    TQString v_ersion;
    KProcess     *t_ar;

    void convert();

  protected slots:
    void slotStart();
    void slotCreateBkupToggled(bool b);
    void slotBrowse();
    void slotTarExited(KProcess *proc);

};


#endif // KNCONVERT_H
