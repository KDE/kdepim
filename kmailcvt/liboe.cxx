/***************************************************************************
                          liboe.cxx  -  description
                             -------------------
    begin                : Thu Aug 24 2000
    copyright            : (C) 2000 by Stephan B. Nedregård
    email                : stephan@micropop.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* LIBOE 0.92 - STABLE
   Copyright (C) 2000 Stephan B. Nedregård (stephan@micropop.com) */

/*  This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef WINDOWS
#include <unistd.h>
#endif
#include <sys/stat.h>

typedef int FPOS_TYPE;

#define OE_CANNOTREAD 1
#define OE_NOTOEBOX   2
#define OE_POSITION   3
#define OE_NOBODY     4
#define OE_PANIC      5

/* #define DEBUG -- uncomment to get some DEBUG output to stdout */


/* TABLE STRUCTURES 
   -- tables store pointers to message headers and other tables also
   containing pointers to message headers and other tables -- */

struct oe_table_header { /* At the beginning of each table */
  int self,   /* Pointer to self (filepos) */
    unknown1, /* Unknown */
    list,     /* Pointer to list */
    next,     /* Pointer to next */
    unknown3, /* Unknown */
    unknown4; /* Unknown */
};
typedef struct oe_table_header oe_table_header;

struct oe_table_node { /* Actual table entries */
  int message, /* Pointer to message | 0 */
    list,   /* Pointer to another table | 0 */
    unknown; /* Unknown */
};
typedef struct oe_table_node oe_table_node;

struct oe_list { /* Internal use only */
  FPOS_TYPE pos;
  struct oe_list *next;
};
typedef struct oe_list oe_list;



/* MESSAGE STRUCTURES
   -- OE uses 16-byte segment headers inside the actual messages. These
   are meaningful, as described below, but were not very easy to hack
   correctly -- note that a message may be composed of segments located
   anywhere in the mailbox file, some times far from each other. */

struct oe_msg_segmentheader {
  int self,  /* Pointer to self (filepos) */
    increase, /* Increase to next segment header (not in msg, in file!) */
    include, /* Number of bytes to include from this segment */
    next,  /* Pointer to next message segment (in msg) (filepos) */
    usenet; /* Only used with usenet posts */
};
typedef struct oe_msg_segmentheader oe_msg_segmentheader;




/* INTERAL STRUCTURES */
struct oe_internaldata{
  void (*oput)(const char*,int);
  FILE *oe;
  oe_list *used;
  int success, justheaders, failure;
  int errcode;
  struct stat *stat;
};
typedef struct oe_internaldata oe_data;



/* LIST OF USED TABLES */

void oe_posused(oe_data *data, FPOS_TYPE pos) {
  oe_list *n = (oe_list *) malloc(sizeof(oe_list));
  n->pos = pos;
  n->next = data->used;
  data->used = n;
}

int oe_isposused(oe_data *data, FPOS_TYPE pos) {
  oe_list *n = data->used;
  while (n!=NULL) {
    if (pos==n->pos) return 1;
    n = n->next;
  }
  return 0;
}

void oe_freeposused(oe_data *data) {
  oe_list *n;
  while (data->used!=NULL) {n=data->used->next; free(data->used); data->used=n;}
}


/* ACTUAL MESSAGE PARSER */

int oe_readmessage(oe_data *data,
		   FPOS_TYPE pos,
		   int /*newsarticle*/) {
  int segheadsize = sizeof(oe_msg_segmentheader)-4; /*+(newsarticle<<2);*/
  oe_msg_segmentheader *sgm = (oe_msg_segmentheader *) malloc(sizeof(oe_msg_segmentheader));
  char buff[16], *ss = (char *) malloc(2048), *s = ss;
  int nextsegment, endofsegment, i, headerwritten = 0;
  fseek(data->oe,pos,SEEK_SET);
  //fsetpos(data->oe,&pos);
  while (1) {
    fread(sgm,segheadsize,1,data->oe);
    if (pos!=sgm->self) { /* No body found*/
#ifdef DEBUG
      printf("- Fail reported at %.8x (%.8x)\n",pos,sgm->self);
#endif
      free(sgm);
      free(ss);
      data->failure++;
      return OE_NOBODY;
    }
    pos+=segheadsize;
    nextsegment = pos+sgm->increase;
    endofsegment = pos+sgm->include;
    if (!headerwritten) {
#ifdef DEBUG
      printf("%.8x : \n",pos-segheadsize);
#endif
      data->oput("From liboe@linux  Sun Jun 11 19:48:24 2000\n",1);
      headerwritten = 1;
    }
    while (pos<endofsegment) {
      fread(&buff,1,16,data->oe);
      for (i=0;i<16;i++,pos++)
	if ((pos<endofsegment) && (buff[i]!=0x0d)) { /* rm extra DOS newline */
	  *(s++)=buff[i];
	  if (buff[i]==0x0a) { *s='\0'; data->oput(ss,2); s=ss; }
	}
    }
    fseek(data->oe,sgm->next,SEEK_SET);
    //fsetpos(data->oe,(FPOS_TYPE *) &sgm->next);
    pos = sgm->next;
    if (pos==0) break;
  }
  if (s!=ss) {
    strcpy(s,"\n");
    data->oput(s,2);
  }
  data->oput("\n",0);

  data->success++;
  free(sgm);
  free(ss);
  return 0;
}


/* PARSES MESSAGE HEADERS */

int oe_readmessageheader(oe_data *data, FPOS_TYPE pos) {
  int segheadsize = sizeof(oe_msg_segmentheader)-4;
  oe_msg_segmentheader *sgm;
  int self=1, msgpos = 0, newsarticle = 0;

  if (oe_isposused(data,pos)) return 0; else oe_posused(data,pos);
  fseek(data->oe,pos,SEEK_SET);
  //fsetpos(data->oe,&pos);
  sgm = (oe_msg_segmentheader *) malloc(sizeof(oe_msg_segmentheader));
  fread(sgm,segheadsize,1,data->oe);
  if (pos!=sgm->self) { free(sgm); return OE_POSITION; /* ERROR */ }
  free(sgm);

  fread(&self,4,1,data->oe); self=1;
  while ((self & 0x7F)>0) {
    fread(&self,4,1,data->oe);
    if ((self & 0xFF) == 0x84) /* 0x80 = Set, 04 = Index */
      if (msgpos==0)
      msgpos = self >> 8; 
    if ((self & 0xFF) == 0x83) /* 0x80 = Set, 03 = News  */
      newsarticle = 1;
  }
  if (msgpos) oe_readmessage(data,msgpos,newsarticle); else  { 
    fread(&self,4,1,data->oe);
    fread(&msgpos,4,1,data->oe);
    if (oe_readmessage(data,msgpos,newsarticle)) { 
      if (newsarticle) {
	data->justheaders++; 
	data->failure--; 
      }
    }
  }
  return 0;
}


/* PARSES MAILBOX TABLES */

int oe_readtable(oe_data *data, FPOS_TYPE pos) {
  oe_table_header thead;
  oe_table_node tnode;
  int quit = 0;

  if (oe_isposused(data,pos)) return 0;

  fseek(data->oe,pos,SEEK_SET);
  //fsetpos(data->oe,&pos);

  fread(&thead,sizeof(oe_table_header),1,data->oe);
  if (thead.self != pos) return OE_POSITION;
  oe_posused(data,pos);
  pos+=sizeof(oe_table_header);

  oe_readtable(data,thead.next);
  oe_readtable(data,thead.list);
  fseek(data->oe,pos,SEEK_SET);  
  //fsetpos(data->oe,&pos); 

  while (!quit) {
    fread(&tnode,sizeof(oe_table_node),1,data->oe);
    pos+=sizeof(oe_table_node);
    if ( (tnode.message > data->stat->st_size) && 
	 (tnode.list > data->stat->st_size) ) 
      return 0xF0; /* PANIC */
    if ( (tnode.message == tnode.list) && /* Neither message nor list==quit */
	 (tnode.message == 0) ) quit = 1; else {
	   oe_readmessageheader(data,tnode.message);
	   oe_readtable(data,tnode.list);
	 }
    fseek(data->oe,pos,SEEK_SET);
    //fsetpos(data->oe,&pos);
  }

  return 0;
}

void oe_readdamaged(oe_data *data) { 
  /* If nothing else works (needed this to get some mailboxes 
     that even OE couldn't read to work. Should generally not 
     be needed, but is nice to have in here */
  FPOS_TYPE pos = 0x7C;
  int i,check, lastID;
#ifdef DEBUG
  printf("  Trying to construct internal mailbox structure\n");
#endif
  fseek(data->oe,pos,SEEK_SET);
  //fsetpos(data->oe,&pos);
  fread(&pos,sizeof(int),1,data->oe); 
  if (pos==0) return; /* No, sorry, didn't work */
  fseek(data->oe,pos,SEEK_SET);
  //fsetpos(data->oe,&pos);
  fread(&i,sizeof(int),1,data->oe);
  if (i!=pos) return; /* Sorry */
  fread(&pos,sizeof(int),1,data->oe);
  i+=pos+8;
  pos = i+4;
  fseek(data->oe,pos,SEEK_SET);
  //fsetpos(data->oe,&pos);
#ifdef DEBUG
  printf("  Searching for %.8x\n",i);
#endif
  lastID=0;
  while (pos<data->stat->st_size) {
    /* Read through file, notice markers, look for message (gen. 2BD4)*/
    fread(&check,sizeof(int),1,data->oe); 
    if (check==pos) lastID=pos;
    pos+=4;
    if ((check==i) && (lastID)) {
#ifdef DEBUG
      printf("Trying possible table at %.8x\n",lastID);
#endif
      oe_readtable(data,lastID);
      fseek(data->oe,pos,SEEK_SET);
      //fsetpos(data->oe,&pos);
    }
  }
}

void oe_readbox_oe4(oe_data *data) {
  FPOS_TYPE pos = 0x54, endpos=0, i;
  oe_msg_segmentheader *header=(oe_msg_segmentheader *) malloc(sizeof(oe_msg_segmentheader));
  char *cb = (char *) malloc(4), *sfull = (char *) malloc(65536), *s = sfull;
  fseek(data->oe,pos,SEEK_SET);
  //fsetpos(data->oe,&pos); 
  while (pos<data->stat->st_size) {
    fseek(data->oe,pos,SEEK_SET);
    //fsetpos(data->oe,&pos);
    fread(header,16,1,data->oe);
    data->oput("From liboe@linux  Sat Jun 17 01:08:25 2000\n",1);
    endpos = pos + header->include;
    if (endpos>data->stat->st_size) endpos=data->stat->st_size;
    pos+=4;
    while (pos<endpos) {
      fread(cb,1,4,data->oe);
      for (i=0;i<4;i++,pos++) 
	if (*(cb+i)!=0x0d) {
	  *s++ = *(cb+i);
	  if (*(cb+i) == 0x0a) {
	    *s = '\0';
	    data->oput(sfull,2);
	    s = sfull;
	  }
	}
    }
    data->success++;
    if (s!=sfull) { *s='\0'; data->oput(sfull,2); s=sfull; }
    data->oput("\n",0);
    pos=endpos;
  }
  free(header);
  free(sfull);
  free(cb);
}

/* CALL THIS ONE */

oe_data* oe_readbox(char*  filename,void (*oput)(const char*,int)) {
  unsigned int signature[4], i;
  oe_data *data = (oe_data *) malloc(sizeof(oe_data));
  data->success=data->failure=data->justheaders=data->errcode=0;
  data->used = NULL;
  data->oput = oput;
  data->oe = fopen(filename,"rb");
  if (data->oe==NULL) {
//    fclose(data->oe);
    data->errcode = OE_CANNOTREAD;
    return data;
  }

  /* SECURITY (Yes, we need this, just in case) */
  data->stat = (struct stat *) malloc(sizeof(struct stat));
  stat(filename,data->stat); 
  
  /* SIGNATURE */
  fread(&signature,16,1,data->oe); 
  if ((signature[0]!=0xFE12ADCF) || /* OE 5 & OE 5 BETA SIGNATURE */
      (signature[1]!=0x6F74FDC5) ||
      (signature[2]!=0x11D1E366) ||
      (signature[3]!=0xC0004E9A)) {
    if ((signature[0]==0x36464D4A) &&
	(signature[1]==0x00010003)) /* OE4 SIGNATURE */ {
      oe_readbox_oe4(data);
      fclose(data->oe);
      free(data->stat);
      return data;
    }
    fclose(data->oe);
    free(data->stat);
    data->errcode = OE_NOTOEBOX;
    return data;
  }

  /* ACTUAL WORK */
  i = 0x30;
  fseek(data->oe,i,SEEK_SET);
  //fsetpos(data->oe,(FPOS_TYPE *) &i);
  fread(&i,4,1,data->oe);
  if (!i) i=0x1e254;
  i = oe_readtable(data,i); /* Reads the box */
  if (i & 0xF0) {
    oe_readdamaged(data);
    data->errcode=OE_PANIC;
  }
  oe_freeposused(data);

  /* CLOSE DOWN */
  fclose(data->oe);
  free(data->stat);
  return data;
}

#define buffsize 65536

/* Just a function to provide the same kind of stream 
   for ordinary mailboxes. */
oe_data* oe_readmbox(char* filename,void (*oput)(const char*)) {
  oe_data *data = (oe_data *) malloc(sizeof(oe_data));
  char *s = (char *) malloc(buffsize);
  data->success=data->failure=data->justheaders=0;
  data->used=NULL;
  data->oe=fopen(filename,"rb");
  for (;;) {
    s=fgets(s,buffsize,data->oe);
    if (s==NULL) break; else oput(s);
  }
  fclose(data->oe);
  return data;
}
