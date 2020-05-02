#include "sc.h"
#include "sc.cpp"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


typedef struct hist {
 char* _buf;
 struct hist* _next;
} history;

history H;
char* expr;

int main(int argc, char** argv) {

  char** libpath; int f,i=1; scriptClass sc; sc.noWarnings=false; sc.debug=false; char* result;

  char* buf=(char*) malloc(65536); memset(buf,'\0',65536);
  char* b2=(char*) malloc(1024*1024); memset(b2,'\0',1024*1024);

  bool noCon=false,silent=false;

  while (i<argc) {
   if (argv[i][0]=='-') {
    if (sc.whereIs(argv[i],"n",1)<strlen(argv[i])) noCon=true;
    if (sc.whereIs(argv[i],"s",1)<strlen(argv[i])) {silent=true; sc.noWarnings=true;}
    if (sc.whereIs(argv[i],"w",1)<strlen(argv[i])) sc.noWarnings=true;
    if (sc.whereIs(argv[i],"d",1)<strlen(argv[i])) sc.debug=true;
   }
   i++;
  }

 if (!noCon) {
   printf("TRIP v0.5 by MW\n");
 }

 i=1; while (i<argc) {
  if (argv[i][0]!='-') {
   f=open(argv[i],O_RDONLY);
   if (f!=-1) {
//    if (!silent) printf("found \"%s\".\nloading..\n",argv[i]);
    memset(b2,'\0',1024*1024);

    read(f,b2,1024*1024-1);






//    if (!silent) printf("parsing..\n");
    if (!noCon) {
      printf("%s\n",sc.evaluate(b2));
    } else {
      printf("%s",sc.evaluate(b2));
    }
   }// else if(!silent) printf("specified file not found (%s)\n",argv[i]);
   close(f);
  }
  i++;
 }
// if(!silent && !noCon) printf("\nconsole commands:\n\n_write_<filename>: writes console buffer to file (no space before filename!)\n_dump_ lists symbols\nto exit, press ctrl+c..\n\n",argv[0]);
 memset(b2,'\0',1024*1024);


 if (!noCon) {
  while (memcmp(buf,"_exit_",6)) {
   memset(buf,'\0',65536);
   if (!silent) printf(">");
/*    long n=0;
    unsigned char ch=' ';
    printf(">");
    while (ch!='\n') {
      ch=getchar();
      buf[n++]=ch;
      printf("%i\n",ch);
    }*/

   fgets(buf,65535,stdin);

   int i=65535; while (buf[i]!='\n') i--; buf[i]='\0';

   if (!memcmp(buf,"_write_",7)) {

    int f2=open(buf+7,O_WRONLY | O_CREAT);
    if (f2==-1 && !silent) printf("error opening file (%s), operation aborted..\n",buf+7);
    else {write(f2,b2,strlen(b2)); memset(b2,'\0',1024*1024); }
    close(f2);

   } else
   if (!strcmp(buf,"_dump_")) {

    symbolStruct* S=sc.rootSymbol;

    char* L=(char*)malloc(100);

    while (S!=NULL) {
     memset(L,' ',99); L[99]='\0';
     char* P=sc.n2s(S->_priority);
     memcpy(L+12-strlen(P),P,strlen(P));
     long m=strlen(S->_case); if (m>15) m=15;
     memcpy(L+13,S->_case,m);
     m=strlen(S->_value); if (m>69) m=69;
     memcpy(L+30,S->_value,m);
     for (int i=0; i<100; i++) if (L[i]=='\n') L[i]=' ';
     printf("%s\n",L);
     S=S->_next;
    }

//    free(L);
    result=NULL;

   } else {
    strcat(b2,buf); strcat(b2,sc.noDo); strcat(b2,"\n");
    result=sc.evaluate(buf);
    printf("%s\n",result);
   }
  }
 }
 return 0;
}
