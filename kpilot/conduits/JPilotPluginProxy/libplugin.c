/* libplugin.c
 *
 * Copyright (C) 1999 by Judd Montgomery
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <sys/types.h>
#include <netinet/in.h>


#include <glib.h>

#include "libplugin.h"

void jp_init()
{
   jp_logf(0, "jp_init()\n");
}

const char *jp_strstr(const char *haystack, const char *needle, int case_sense)
{
   char *needle2;
   char *haystack2;
   register char *Ps2;
   register const char *Ps1;
   char *r;

   if (case_sense) {
      return strstr(haystack, needle);
   } else {
      if (!haystack) {
	 return NULL;
      }
      if (!needle) {
	 return haystack;
      }
      needle2 = malloc(strlen(needle)+2);
      haystack2 = malloc(strlen(haystack)+2);

      Ps1 = needle;
      Ps2 = needle2;
      while (Ps1[0]) {
	 Ps2[0] = tolower(Ps1[0]);
	 Ps1++;
	 Ps2++;
      }
      Ps2[0]='\0';

      Ps1 = haystack;
      Ps2 = haystack2;
      while (Ps1[0]) {
	 Ps2[0] = tolower(Ps1[0]);
	 Ps1++;
	 Ps2++;
      }
      Ps2[0]='\0';

      r = strstr(haystack2, needle2);
      if (r) {
	 r = (char *)((r-haystack2)+haystack);
      }
      free(needle2);
      free(haystack2);
      return r;
   }
}

static int pack_header(PC3RecordHeader *header, char *packed_header)
{
   unsigned char *p;
   unsigned long l;
   unsigned long len;

   l=0;
   p=packed_header;
   /*
    * Header structure:
    * unsigned long header_len;
    * unsigned long header_version;
    * unsigned long rec_len;
    * unsigned long unique_id;
    * unsigned long rt;
    * unsigned char attrib;
    */
   len = sizeof(unsigned long) +
     sizeof(unsigned long) +
     sizeof(unsigned long) +
     sizeof(unsigned long) +
     sizeof(unsigned long) +
     sizeof(unsigned char);

   header->header_version = 2;

   header->header_len = len;

   l=htonl(header->header_len);
   memcpy(p, &l, sizeof(l));
   p+=sizeof(l);

   l=htonl(header->header_version);
   memcpy(p, &l, sizeof(l));
   p+=sizeof(l);

   l=htonl(header->rec_len);
   memcpy(p, &l, sizeof(l));
   p+=sizeof(l);

   l=htonl(header->unique_id);
   memcpy(p, &l, sizeof(l));
   p+=sizeof(l);

   l=htonl(header->rt);
   memcpy(p, &l, sizeof(l));
   p+=sizeof(l);

   memcpy(p, &header->attrib, sizeof(unsigned char));
   p+=sizeof(unsigned char);

   return len;
}

static int unpack_header(PC3RecordHeader *header, unsigned char *packed_header)
{
   unsigned char *p;
   unsigned long l;

   /*
    * Header structure:
    * unsigned long header_len;
    * unsigned long header_version;
    * unsigned long rec_len;
    * unsigned long unique_id;
    * unsigned long rt;
    * unsigned char attrib;
    */
   p = packed_header;

   memcpy(&l, p, sizeof(l));
   header->header_len=ntohl(l);
   p+=sizeof(l);

   memcpy(&l, p, sizeof(l));
   header->header_version=ntohl(l);
   p+=sizeof(l);

   if (header->header_version > 2) {
      jp_logf(LOG_WARN, "Unknown PC header version = %d\n", header->header_version);
   }

   memcpy(&l, p, sizeof(l));
   header->rec_len=ntohl(l);
   p+=sizeof(l);

   memcpy(&l, p, sizeof(l));
   header->unique_id=ntohl(l);
   p+=sizeof(l);

   memcpy(&l, p, sizeof(l));
   header->rt=ntohl(l);
   p+=sizeof(l);

   memcpy(&(header->attrib), p, sizeof(unsigned char));
   p+=sizeof(unsigned char);

   return 0;
}

/* FIXME: Add jp_ and document. */
int read_header(FILE *pc_in, PC3RecordHeader *header)
{
   unsigned long l, len;
   unsigned char packed_header[256];
   int num;

   num = fread(&l, sizeof(l), 1, pc_in);
   if (feof(pc_in)) {
      return JPILOT_EOF;
   }
   if (num!=1) {
      return num;
   }
   memcpy(packed_header, &l, sizeof(l));
   len=ntohl(l);
   if (len > 255) {
      jp_logf(LOG_WARN, "read_header() error\n");
      return -1;
   }
   num = fread(packed_header+sizeof(l), len-sizeof(l), 1, pc_in);
   if (feof(pc_in)) {
      return JPILOT_EOF;
   }
   if (num!=1) {
      return num;
   }
   unpack_header(header, packed_header);
#ifdef DEBUG
   printf("header_len    =%ld\n", header->header_len);
   printf("header_version=%ld\n", header->header_version);
   printf("rec_len       =%ld\n", header->rec_len);
   printf("unique_id     =%ld\n", header->unique_id);
   printf("rt            =%ld\n", header->rt);
   printf("attrib        =%d\n", header->attrib);
#endif
   return 1;
}

/* Add jp_ and document */
int write_header(FILE *pc_out, PC3RecordHeader *header)
{
   unsigned long len;
   unsigned char packed_header[256];

   len = pack_header(header, packed_header);
   if (len>0) {
      fwrite(packed_header, len, 1, pc_out);
   }

   return len;
}

/*
 * file must not be open elsewhere when this is called
 * the first line in file is 0
 */
int jp_install_remove_line(int deleted_line)
{
   FILE *in;
   FILE *out;
   char line[1002];
   char *Pc;
   int r, line_count;

   in = jp_open_home_file("jpilot_to_install", "r");
   if (!in) {
      jp_logf(LOG_DEBUG, "failed opening install_file\n");
      return -1;
   }

   out = jp_open_home_file("jpilot_to_install.tmp", "w");
   if (!out) {
      fclose(in);
      jp_logf(LOG_DEBUG, "failed opening install_file.tmp\n");
      return -1;
   }

   for (line_count=0; (!feof(in)); line_count++) {
      line[0]='\0';
      Pc = fgets(line, 1000, in);
      if (!Pc) {
	 break;
      }
      if (line_count == deleted_line) {
	 continue;
      }
      r = fprintf(out, "%s", line);
      if (r==EOF) {
	 break;
      }
   }
   fclose(in);
   fclose(out);

   rename_file("jpilot_to_install.tmp", "jpilot_to_install");

   return 0;
}

int jp_install_append_line(char *line)
{
   FILE *out;
   int r;

   out = jp_open_home_file("jpilot_to_install", "a");
   if (!out) {
      return -1;
   }

   r = fprintf(out, "%s\n", line);
   if (r==EOF) {
      fclose(out);
      return -1;
   }
   fclose(out);

   return 0;
}

/*returns 1 if found */
/*        0 if eof */
static int find_next_offset(mem_rec_header *mem_rh, long fpos,
		     unsigned int *next_offset,
		     unsigned char *attrib, unsigned int *unique_id)
{
   mem_rec_header *temp_mem_rh;
   unsigned char found = 0;
   unsigned long found_at;

   found_at=0xFFFFFF;
   for (temp_mem_rh=mem_rh; temp_mem_rh; temp_mem_rh = temp_mem_rh->next) {
      if ((temp_mem_rh->offset > fpos) && (temp_mem_rh->offset < found_at)) {
	 found_at = temp_mem_rh->offset;
	 /* *attrib = temp_mem_rh->attrib; */
	 /* *unique_id = temp_mem_rh->unique_id; */
      }
      if ((temp_mem_rh->offset == fpos)) {
	 found = 1;
	 *attrib = temp_mem_rh->attrib;
	 *unique_id = temp_mem_rh->unique_id;
      }
   }
   *next_offset = found_at;
   return found;
}

static void free_mem_rec_header(mem_rec_header **mem_rh)
{
  mem_rec_header *h, *next_h;

   for (h=*mem_rh; h; h=next_h) {
      next_h=h->next;
      free(h);
   }
   *mem_rh=NULL;
}

/* int jp_free_DB_records(GList **records) */
/* void free_buf_rec_list(GList **br_list) */
int jp_free_DB_records(GList **br_list)
{
   GList *temp_list, *first;
   buf_rec *br;

   /* Go to first entry in the list */
   first=NULL;
   for (temp_list = *br_list; temp_list; temp_list = temp_list->prev) {
      first = temp_list;
   }
   for (temp_list = first; temp_list; temp_list = temp_list->next) {
      if (temp_list->data) {
	 br=temp_list->data;
	 if (br->buf) {
	    free(br->buf);
	    temp_list->data=NULL;
	 }
	 free(br);
      }
   }
   g_list_free(*br_list);
   *br_list=NULL;

   return 0;
}

/*These next 2 functions were copied from pi-file.c in the pilot-link app */
/* Exact value of "Jan 1, 1970 0:00:00 GMT" - "Jan 1, 1904 0:00:00 GMT" */
#define PILOT_TIME_DELTA (unsigned)(2082844800)

static time_t
pilot_time_to_unix_time (unsigned long raw_time)
{
   return (time_t)(raw_time - PILOT_TIME_DELTA);
}

/*
static unsigned long
unix_time_to_pilot_time (time_t t)
{
   return (unsigned long)((unsigned long)t + PILOT_TIME_DELTA);
}
*/

static unsigned int bytes_to_bin(unsigned char *bytes, unsigned int num_bytes)
{
   unsigned int i, n;
   n=0;
   for (i=0;i<num_bytes;i++) {
      n = n*256+bytes[i];
   }
   return n;
}

static int raw_header_to_header(RawDBHeader *rdbh, DBHeader *dbh)
{
   unsigned long temp;

   strncpy(dbh->db_name, rdbh->db_name, 31);
   dbh->db_name[31] = '\0';
   dbh->flags = bytes_to_bin(rdbh->flags, 2);
   dbh->version = bytes_to_bin(rdbh->version, 2);
   temp = bytes_to_bin(rdbh->creation_time, 4);
   dbh->creation_time = pilot_time_to_unix_time(temp);
   temp = bytes_to_bin(rdbh->modification_time, 4);
   dbh->modification_time = pilot_time_to_unix_time(temp);
   temp = bytes_to_bin(rdbh->backup_time, 4);
   dbh->backup_time = pilot_time_to_unix_time(temp);
   dbh->modification_number = bytes_to_bin(rdbh->modification_number, 4);
   dbh->app_info_offset = bytes_to_bin(rdbh->app_info_offset, 4);
   dbh->sort_info_offset = bytes_to_bin(rdbh->sort_info_offset, 4);
   strncpy(dbh->type, rdbh->type, 4);
   dbh->type[4] = '\0';
   strncpy(dbh->creator_id, rdbh->creator_id, 4);
   dbh->creator_id[4] = '\0';
   strncpy(dbh->unique_id_seed, rdbh->unique_id_seed, 4);
   dbh->unique_id_seed[4] = '\0';
   dbh->next_record_list_id = bytes_to_bin(rdbh->next_record_list_id, 4);
   dbh->number_of_records = bytes_to_bin(rdbh->number_of_records, 2);

   return 0;
}

int jp_get_app_info(char *DB_name, unsigned char **buf, int *buf_size)
{
   FILE *in;
   int num;
   unsigned int rec_size;
   RawDBHeader rdbh;
   DBHeader dbh;
   char PDB_name[256];

   if ((!buf_size) || (!buf)) {
      return -1;
   }
   *buf = NULL;
   *buf_size=0;

   g_snprintf(PDB_name, 255, "%s.pdb", DB_name);
   in = jp_open_home_file(PDB_name, "r");
   if (!in) {
      jp_logf(LOG_WARN, "Error opening %s\n", PDB_name);
      return -1;
   }
   num = fread(&rdbh, sizeof(RawDBHeader), 1, in);
   if (num != 1) {
      if (ferror(in)) {
	 jp_logf(LOG_WARN, "Error reading %s\n", PDB_name);
	 fclose(in);
	 return -1;
      }
      if (feof(in)) {
	 fclose(in);
	 return JPILOT_EOF;
      }
   }
   raw_header_to_header(&rdbh, &dbh);

   num = get_app_info_size(in, &rec_size);
   if (num) {
      fclose(in);
      return -1;
   }

   fseek(in, dbh.app_info_offset, SEEK_SET);
   *buf=malloc(rec_size);
   if (!(*buf)) {
      jp_logf(LOG_WARN, "jp_get_app_info(): Out of memory\n");
      fclose(in);
      return -1;
   }
   num = fread(*buf, rec_size, 1, in);
   if (num != 1) {
      if (ferror(in)) {
	 fclose(in);
	 free(*buf);
	 jp_logf(LOG_WARN, "Error reading %s\n", PDB_name);
	 return -1;
      }
   }
   fclose(in);

   *buf_size=rec_size;

   return 0;
}

/*
 * This deletes a record from the appropriate Datafile
 */
int jp_delete_record(char *DB_name, buf_rec *br, int flag)
{
   FILE *pc_in;
   PC3RecordHeader header;
   char PC_name[256];

   if (br==NULL) {
      return -1;
   }

   g_snprintf(PC_name, 255, "%s.pc3", DB_name);

   if ((br->rt==DELETED_PALM_REC) || (br->rt==MODIFIED_PALM_REC)) {
      jp_logf(LOG_INFO, "This record is already deleted.\n"
		  "It is scheduled to be deleted from the Palm on the next sync.\n");
      return 0;
   }
   switch (br->rt) {
    case NEW_PC_REC:
      pc_in=jp_open_home_file(PC_name, "r+");
      if (pc_in==NULL) {
	 jp_logf(LOG_WARN, "Couldn't open PC records file\n");
	 return -1;
      }
      while(!feof(pc_in)) {
	 read_header(pc_in, &header);
	 if (feof(pc_in)) {
	    jp_logf(LOG_WARN, "couldn't find record to delete\n");
	    fclose(pc_in);
	    return -1;
	 }
	 if (header.header_version==2) {
	    if (header.unique_id==br->unique_id) {
	       if (fseek(pc_in, -header.header_len, SEEK_CUR)) {
		  jp_logf(LOG_WARN, "fseek failed\n");
	       }
	       header.rt=DELETED_PC_REC;
	       write_header(pc_in, &header);
	       jp_logf(LOG_DEBUG, "record deleted\n");
	       fclose(pc_in);
	       return 0;
	    }
	 } else {
	    jp_logf(LOG_WARN, "unknown header version %d\n", header.header_version);
	 }
	 if (fseek(pc_in, header.rec_len, SEEK_CUR)) {
	    jp_logf(LOG_WARN, "fseek failed\n");
	 }
      }
      fclose(pc_in);
      return -1;
	
    case PALM_REC:
      jp_logf(LOG_DEBUG, "Deleteing Palm ID %d\n", br->unique_id);
      pc_in=jp_open_home_file(PC_name, "a");
      if (pc_in==NULL) {
	 jp_logf(LOG_WARN, "Couldn't open PC records file\n");
	 return -1;
      }
      header.unique_id=br->unique_id;
      if (flag==MODIFY_FLAG) {
	 header.rt=MODIFIED_PALM_REC;
      } else {
	 header.rt=DELETED_PALM_REC;
      }

      header.rec_len = br->size;

      jp_logf(LOG_DEBUG, "writing header to pc file\n");
      write_header(pc_in, &header);
      /*todo write the real appointment from palm db */
      /*Right now I am just writing an empty record */
      /*This will be used for making sure that the palm record hasn't changed */
      /*before we delete it */
      jp_logf(LOG_DEBUG, "writing record to pc file, %d bytes\n", header.rec_len);
      fwrite(br->buf, header.rec_len, 1, pc_in);
      jp_logf(LOG_DEBUG, "record deleted\n");
      fclose(pc_in);
      break;
    default:
      break;
   }

   return 0;
}

int jp_pc_write(char *DB_name, buf_rec *br)
{
   PC3RecordHeader header;
   FILE *out;
   unsigned int next_unique_id;
   char packed_header[256];
   int len;
   char PC_name[256];

   g_snprintf(PC_name, 255, "%s.pc3", DB_name);
   PC_name[255]='\0';

   get_next_unique_pc_id(&next_unique_id);
#ifdef JPILOT_DEBUG
   jp_logf(LOG_DEBUG, "next unique id = %d\n",next_unique_id);
#endif

   out = jp_open_home_file(PC_name, "a");
   if (!out) {
      jp_logf(LOG_WARN, "Error opening %s\n", PC_name);
      return -1;
   }

   header.rec_len=br->size;
   header.rt=br->rt;
   header.attrib=br->attrib;
   header.unique_id=next_unique_id;
   br->unique_id=next_unique_id;

   len = pack_header(&header, packed_header);
   write_header(out, &header);
   fwrite(br->buf, header.rec_len, 1, out);

   fclose(out);

   return 0;
}

static int pc_read_next_rec(FILE *in, buf_rec *br)
{
   PC3RecordHeader header;
   int rec_len, num;
   char *record;

   if (feof(in)) {
      return JPILOT_EOF;
   }
   num = read_header(in, &header);
   if (num < 1) {
      if (ferror(in)) {
	 jp_logf(LOG_WARN, "Error reading pc file 1\n");
	 return JPILOT_EOF;
      }
      if (feof(in)) {
	 return JPILOT_EOF;
      }
   }
   rec_len = header.rec_len;
   record = malloc(rec_len);
   if (!record) {
      jp_logf(LOG_WARN, "pc_read_next_rec(): Out of memory\n");
      return JPILOT_EOF;
   }
   num = fread(record, rec_len, 1, in);
   if (num != 1) {
      if (ferror(in)) {
	 jp_logf(LOG_WARN, "Error reading pc file 2\n");
	 free(record);
	 return JPILOT_EOF;
      }
   }
   br->rt = header.rt;
   br->unique_id = header.unique_id;
   br->attrib = header.attrib;
   br->buf = record;
   br->size = rec_len;

   return 0;
}

int jp_read_DB_files(char *DB_name, GList **records)
{
   FILE *in;
   FILE *pc_in;
   char *buf;
   GList *temp_list;
   int num_records, recs_returned, i, num, r;
   unsigned int offset, prev_offset, next_offset, rec_size;
   int out_of_order;
   long fpos;  /*file position indicator */
   unsigned char attrib;
   unsigned int unique_id;
   mem_rec_header *mem_rh, *temp_mem_rh, *last_mem_rh;
   record_header rh;
   RawDBHeader rdbh;
   DBHeader dbh;
   buf_rec *temp_br;
   char PDB_name[256];
   char PC_name[256];

   mem_rh = last_mem_rh = NULL;
   *records = NULL;
   recs_returned = 0;

   g_snprintf(PDB_name, 255, "%s.pdb", DB_name);
   PDB_name[255]='\0';
   g_snprintf(PC_name, 255, "%s.pc3", DB_name);
   PC_name[255]='\0';
   in = jp_open_home_file(PDB_name, "r");
   if (!in) {
      jp_logf(LOG_WARN, "Error opening %s\n", PDB_name);
      return -1;
   }
   /*Read the database header */
   num = fread(&rdbh, sizeof(RawDBHeader), 1, in);
   if (num != 1) {
      if (ferror(in)) {
	 jp_logf(LOG_WARN, "Error reading %s\n", PDB_name);
	 fclose(in);
	 return -1;
      }
      if (feof(in)) {
	 return JPILOT_EOF;
      }      
   }
   raw_header_to_header(&rdbh, &dbh);
#ifdef JPILOT_DEBUG
   jp_logf(LOG_DEBUG, "db_name = %s\n", dbh.db_name);
   jp_logf(LOG_DEBUG, "num records = %d\n", dbh.number_of_records);
   jp_logf(LOG_DEBUG, "app info offset = %d\n", dbh.app_info_offset);
#endif

   /*Read each record entry header */
   num_records = dbh.number_of_records;
   out_of_order = 0;
   prev_offset = 0;
   
   for (i=1; i<num_records+1; i++) {
      num = fread(&rh, sizeof(record_header), 1, in);
      if (num != 1) {
	 if (ferror(in)) {
	    jp_logf(LOG_WARN, "Error reading %s\n", PDB_name);
	    break;
	 }
	 if (feof(in)) {
	    return JPILOT_EOF;
	 }      
      }

      offset = ((rh.Offset[0]*256+rh.Offset[1])*256+rh.Offset[2])*256+rh.Offset[3];

      if (offset < prev_offset) {
	 out_of_order = 1;
      }
      prev_offset = offset;

#ifdef JPILOT_DEBUG
      jp_logf(LOG_DEBUG, "record header %u offset = %u\n",i, offset);
      jp_logf(LOG_DEBUG, "       attrib 0x%x\n",rh.attrib);
      jp_logf(LOG_DEBUG, "    unique_ID %d %d %d = ",rh.unique_ID[0],rh.unique_ID[1],rh.unique_ID[2]);
      jp_logf(LOG_DEBUG, "%d\n",(rh.unique_ID[0]*256+rh.unique_ID[1])*256+rh.unique_ID[2]);
#endif
      temp_mem_rh = (mem_rec_header *)malloc(sizeof(mem_rec_header));
      if (!temp_mem_rh) {
	 jp_logf(LOG_WARN, "jp_read_DB_files(): Out of memory 1\n");
	 break;
      }
      temp_mem_rh->next = NULL;
      temp_mem_rh->rec_num = i;
      temp_mem_rh->offset = offset;
      temp_mem_rh->attrib = rh.attrib;
      temp_mem_rh->unique_id = (rh.unique_ID[0]*256+rh.unique_ID[1])*256+rh.unique_ID[2];
      if (mem_rh == NULL) {
	 mem_rh = temp_mem_rh;
	 last_mem_rh = temp_mem_rh;
      } else {
	 last_mem_rh->next = temp_mem_rh;
	 last_mem_rh = temp_mem_rh;
      }
   }

   temp_mem_rh = mem_rh;

   if (num_records) {
      if (out_of_order) {
	 find_next_offset(mem_rh, 0, &next_offset, &attrib, &unique_id);
      } else {
	 if (mem_rh) {
	    next_offset = mem_rh->offset;
	    attrib = mem_rh->attrib;
	    unique_id = mem_rh->unique_id;
	 }
      }
      fseek(in, next_offset, SEEK_SET);
      while(!feof(in)) {
	 fpos = ftell(in);
	 if (out_of_order) {
	    find_next_offset(mem_rh, fpos, &next_offset, &attrib, &unique_id);
	 } else {
	    next_offset = 0xFFFFFF;
	    if (temp_mem_rh) {
	       attrib = temp_mem_rh->attrib;
	       unique_id = temp_mem_rh->unique_id;
	       if (temp_mem_rh->next) {
		  temp_mem_rh = temp_mem_rh->next;
		  next_offset = temp_mem_rh->offset;
	       }
	    }
	 }
	 rec_size = next_offset - fpos;
#ifdef JPILOT_DEBUG
	 jp_logf(LOG_DEBUG, "rec_size = %u\n",rec_size);
	 jp_logf(LOG_DEBUG, "fpos,next_offset = %u %u\n",fpos,next_offset);
	 jp_logf(LOG_DEBUG, "----------\n");
#endif
	 buf = malloc(rec_size);
	 if (!buf) break;
	 num = fread(buf, rec_size, 1, in);
	 if ((num != 1)) {
	    if (ferror(in)) {
	       jp_logf(LOG_WARN, "Error reading %s 5\n", PDB_name);
	       free(buf);
	       break;
	    }
	 }

	 temp_br = malloc(sizeof(buf_rec));
	 if (!temp_br) {
	    jp_logf(LOG_WARN, "jp_read_DB_files(): Out of memory 2\n");
	    break;
	 }
	 temp_br->rt = PALM_REC;
	 temp_br->unique_id = unique_id;
	 temp_br->attrib = attrib;
	 temp_br->buf = buf;
	 temp_br->size = rec_size;

	 *records = g_list_append(*records, temp_br);
	 
	 recs_returned++;
      }
   }
   fclose(in);
   free_mem_rec_header(&mem_rh);
   /* */
   /* Get the appointments out of the PC database */
   /* */
   pc_in = jp_open_home_file(PC_name, "r");
   if (pc_in==NULL) {
      jp_logf(LOG_DEBUG, "jp_open_home_file failed\n");
      return 0;
   }

   while(!feof(pc_in)) {
      temp_br = malloc(sizeof(buf_rec));
      if (!temp_br) {
	 jp_logf(LOG_WARN, "jp_read_DB_files(): Out of memory 3\n");
	 break;
      }
      r = pc_read_next_rec(pc_in, temp_br);
      if ((r==JPILOT_EOF) || (r<0)) {
	 free(temp_br);
	 break;
      }
      if ((temp_br->rt!=DELETED_PC_REC)
	  &&(temp_br->rt!=DELETED_PALM_REC)
	  &&(temp_br->rt!=MODIFIED_PALM_REC)
	  &&(temp_br->rt!=DELETED_DELETED_PALM_REC)) {

	 *records = g_list_append(*records, temp_br);
	 
	 recs_returned++;
      }
      if ((temp_br->rt==DELETED_PALM_REC) || (temp_br->rt==MODIFIED_PALM_REC)) {
	 temp_list=*records;
	 if (*records) {
	    while(temp_list->next) {
	       temp_list=temp_list->next;
	    }
	 }
	 for (; temp_list; temp_list=temp_list->prev) {
	    if (((buf_rec *)temp_list->data)->unique_id == temp_br->unique_id) {
	       ((buf_rec *)temp_list->data)->rt = temp_br->rt;
	    }
	 }
      }
   }
   fclose(pc_in);

   jp_logf(LOG_DEBUG, "Leaving get_recs\n");

   return recs_returned;
}
