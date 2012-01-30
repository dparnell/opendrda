/* 
 * Copyright (C) 2001 Brian Bruns (camber@ais.org)
 */
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <time.h>

#define DRDALOG 0

void drda_dump_buf(
   FILE 	 *dumpfile, 
   const void    *buf,     
   int            length)  
{
   int                   i;
   int                   j;
   const int             bytesPerLine = 16;
   const unsigned char  *data         = buf;

#if DRDALOG
   for(i=0; i<length; i+=bytesPerLine) {

         fprintf(dumpfile, "%04x  ", i);

         for(j=i; j<length && (j-i)<bytesPerLine; j++)
         {
            fprintf(dumpfile, "%02x ", data[j]);
	 }

         for(; 0!=(j % bytesPerLine); j++)
         {
            fprintf(dumpfile, "   ");
         }
         fprintf(dumpfile, "  |");

         for(j=i; j<length && (j-i)<bytesPerLine; j++)
         {
            fprintf(dumpfile, "%c", (isprint(data[j])) ? data[j] : '.');
         }
         fprintf(dumpfile, "|\n");
   }
   fprintf(dumpfile, "\n");
#endif
} 
  
void drda_log(int debug_lvl, FILE *dumpfile, const char *fmt, ...)
{
int j = 0;
va_list   ap;
char *ptr;
char mask[255];

#if DRDALOG
	va_start(ap, fmt);
      
	for(ptr = (char *) fmt; *ptr != '\0'; ptr++) {
		if (*ptr == '%') {
			ptr++;
			mask[0]='%';
			j = 1;
			while (isdigit(*ptr)) {
				mask[j++]=*ptr++;
			}
			mask[j++]=*ptr;
			mask[j]='\0';
			switch(*ptr) {
				case 's':
				{
					char   *s = va_arg(ap, char *);
					fputs(s, dumpfile);
					break;
				}
				case 'd':
            		{
					int i = va_arg(ap, int);
					fprintf(dumpfile, "%d", i);
					break;
				}
				case 'x':
				{
					int i = va_arg(ap, int);
					fprintf(dumpfile, mask, i);
					break;
				}
				case 'D':
				{
					char  *buf = va_arg(ap, char *);
					int    len = va_arg(ap, int);
					drda_dump_buf(dumpfile, buf, len);
					break;
				}
				case 'L': /* current local time */
				{
					char        buf[1024];
					struct tm  *tm;
					time_t      t;

					time(&t);
					tm = localtime(&t);
					strftime(buf, sizeof(buf)-1, "%Y-%m-%d %H:%M:%S", tm);
					fputs(buf, dumpfile);
				}
				default:
				{
					break;
				}
			}
		} else {
			fputc(*ptr, dumpfile);
		}
	}
#endif
}

