/***************************************************************************
                          oe5_2mbox.cpp  -  description
                             -------------------
    begin                : Thu Aug 24 2000
    copyright            : (C) 2000 by Hans Dijkema
    email                : kmailcvt@hum.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   is under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <klocale.h>
//#define i18n(a) a

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "filters.hxx"
#include "oe5_2mbox.h"
#include "liboe.h"

#define CAP	i18n("Import Outlook Express 5")

filter *      oe5_2mbox::F = 0;
const char *  oe5_2mbox::FOLDER = 0;
filterInfo *  oe5_2mbox::INFO = 0;
int           oe5_2mbox::numOfMessages = 0;
unsigned long oe5_2mbox::added = 0;
unsigned long oe5_2mbox::mails = 0;

oe5_2mbox::oe5_2mbox(const char *in,const char *out,filter *F,filterInfo *I)
{
  info=I;
  f=F;
  folderIn=in;
  folderTo=out;
}

oe5_2mbox::~oe5_2mbox()
{}

int oe5_2mbox::convert(void)
{
oe_data *result;
char s[1024];
  F=f;

  sprintf(s,"OE5-%s",folderTo);
  FOLDER=s;
  INFO=info;

  added=mails=0;

  numOfMessages=0;
  result=oe_readbox((char *) folderIn,addMessage);

  if (!result->success) {
    switch (result->errcode) {
      case OE_CANNOTREAD :
        {QString msg;
           msg.sprintf(" '%s'",folderIn);
           msg=i18n("Cannot read mailbox")+msg;
           info->alert(CAP,msg);
        }
      break;
      case OE_NOTOEBOX :
        {QString msg;
           msg=i18n("%1 is not an OE5 mailbox").arg(folderIn);
           info->alert(CAP,msg);
        }
      break;
      default:
        {QString msg;
           msg.sprintf(" '%s'",folderIn);
           msg=i18n("Unrecoverable error while reading")+msg;
           info->alert(CAP,msg);
        }
      break;
    }
  }
  else {QString msg,m,a;
    m.sprintf("%lu ",mails);
    a.sprintf("%lu ",added);
    msg=m+i18n("mails read, ")+a+i18n("were new to kmail folder");
    info->log(msg);
  }

return result->success;
}

void oe5_2mbox::addMessage(const char *string,int code)
{
static FILE *f=NULL;
static int status=-1;
static float perc=0.0;

  switch(code) {
    case 1: // begin new message
    {
      if (status!=0 && status!=-1) { addMessage("",0); }
      status=1;
      {char s[1024];
        sprintf(s,"/tmp/oe5_2mbox.%d",getpid());
        f=fopen(s,"wb");
        if (f==NULL) {QString msg;
          msg=i18n("FATAL: Cannot open TEMP file '%1'").arg(s);
          INFO->alert(CAP,msg);
          status=-1;
        }
      }
      if (status!=-1) {
        fprintf(f,"%s",string);
      }
    }
    break;
    case 2: // continue message
    {
      if (status!=1 && status!=2 && status!=-1) {QString msg;
        msg=i18n("FATAL: Cannot add to ended message,\n"
                 "skipping until message start");
        INFO->alert(CAP,msg);
        status=-1;
      }
      else if (status!=-1) {
        status=2;
        fprintf(f,"%s",string);
      }
    }
    break;
    case 0: //end of message
    {
      if (status!=1 && status!=2 && status!=-1) {QString msg;
        msg=i18n("FATAL: Cannot end an ended message,\n"
                 "skipping until message start");
        INFO->alert(CAP,msg);
        status=-1;
      }
      else if (status!=-1) {
        status=0;

        perc+=2.0;
        if (perc>100.0) {
          perc=0.0;
          INFO->current();
        }
        INFO->current(perc);

        fprintf(f,"%s",string);
        fclose(f);
        {char s[1024];
          sprintf(s,"/tmp/oe5_2mbox.%d",getpid());
          f=fopen(s,"rb");
          if (f==NULL) {QString msg;
            msg.sprintf(" ('%s')",s);
            msg=i18n("FATAL: Cannot open just made TEMP file for reading")+msg;
            INFO->alert(CAP,msg);
            status=-1;
          }
          else {
            fclose(f);
            F->kmailMessage(INFO, (char *) FOLDER, s, added);
            mails+=1;
            unlink(s);
            status=-1; // skip to next message.
          }
        }
      }
    }
    break;
  }
}
#undef CAP
