#ifndef FORMAT_HANDLER_H
#define FORMAT_HANDLER_H

#include <qxml.h>

#include "FormatSpec.h"

class FormatHandler : public QXmlDefaultHandler
{
  public:

    FormatHandler();
    virtual ~FormatHandler();

    FormatSpec spec();

    bool startDocument();

    bool startElement
      (
       const QString & namespaceURI,
       const QString & localName,
       const QString & qName,
       const QXmlAttributes & attributes
      );

    bool endElement
      (
       const QString & namespaceURI,
       const QString & localName,
       const QString & qName
      );

  private:

    QString name_;
    FormatSpec spec_;
};

#endif
