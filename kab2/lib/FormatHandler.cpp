#include "FormatHandler.h"

FormatHandler::FormatHandler()
  : QXmlDefaultHandler()
{
  // Empty.
}

FormatHandler::~FormatHandler()
{
  // Empty.
}


  FormatSpec
FormatHandler::spec()
{
  return spec_;
}


  bool
FormatHandler::startDocument()
{
  return true; // Nothing to do yet.
}


  bool
FormatHandler::startElement
(
 const QString &,
 const QString &,
 const QString & qName,
 const QXmlAttributes & attributes
)
{
  if ("kab-definition" == qName)
  {
    name_ = attributes.value("name");

    return true;
  }
  else if ("field" == qName)
  {
    FieldFormat ff;

    ff.setName(attributes.value("name"));
    ff.setUnique("true" == attributes.value("unique"));

    QString mimeType = attributes.value("mimetype");

    int sep = mimeType.find('/');

    if (-1 == sep)
      ff.setType(mimeType);

    else
    {
      ff.setType(mimeType.left(sep));
      ff.setSubType(mimeType.mid(sep + 1));
    }

    spec_ << ff;

    return true;
  }
  else
  {
    return false;
  }
}


  bool
FormatHandler::endElement
(
 const QString &,
 const QString &,
 const QString &
)
{
  // What to do here ?
  return true;
}

