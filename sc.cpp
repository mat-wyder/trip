#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

scriptClass::scriptClass() { mkNewStruct(); }
scriptClass::~scriptClass() { destroyStruct(); }

void scriptClass::mkNewStruct() {

  evLev=0;

  noDo=strdup("_noDo_");
  sThen=strdup("_sThen_");
  sWhile=strdup("_sWhile_");
  aVal=strdup("_assign_");
  sRen=strdup("_rename_");
  iPr=strdup("_iPL_");
  dPr=strdup("_dPL_");
  lOR=strdup("_lOR_");
  lAND=strdup("_lAND_");
  lNOT=strdup("_lNOT_");
  lLT=strdup("_lLT_");
  lEQ=strdup("_lEQ_");
  setPri=strdup("_setPri_");
  sChop=strdup("_sChop_");
  sFind=strdup("_sFind_");
  mAdd=strdup("_mAdd_");
  mSub=strdup("_mSub_");
  mMul=strdup("_mMul_");
  mDiv=strdup("_mDiv_");
  mRand=strdup("_mRand_");
  sLen=strdup("_sLen_");
  getPri=strdup("_getPri_");
  getCode=strdup("_getCode_");
  getFile=strdup("_getFile_");
  putFile=strdup("_putFile_");
  cID=strdup("_chID_");

  iEL=strdup("_iEL_");
  dEL=strdup("_dEL_");
  iELwrap=strdup("_iELwrap_");
  dELwrap=strdup("_dELwrap_");

  lScp=strdup("_left_");
  rScp=strdup("_right_");
  lTrue=strdup("_true_");
  lFalse=strdup("_false_");

  long b=1024,s=16; //base and step for priority levels

  rootSymbol=(sStruct*)malloc(sizeof(sStruct));
  rootSymbol->_case=strdup(noDo);
  rootSymbol->_value=strdup("INTERNAL");
  rootSymbol->_priority=b;
  rootSymbol->_next=(sStruct*) NULL;

  setSymbol(sThen, "INTERNAL", b+s*1);
  setSymbol(sWhile, "INTERNAL", b+s*1);

  setSymbol(aVal, "INTERNAL", b+s*2);
  setSymbol(sRen, "INTERNAL", b+s*2);
  setSymbol(iPr, "INTERNAL", b+s*2+2);

  setSymbol(lOR, "INTERNAL", b+s*3);
  setSymbol(lAND, "INTERNAL", b+s*3+2);
  setSymbol(lNOT, "INTERNAL", b+s*3+4);
  setSymbol(lLT, "INTERNAL", b+s*3+6);
  setSymbol(lEQ, "INTERNAL", b+s*3+6);

  setSymbol(setPri, "INTERNAL", b+s*4);

  setSymbol(sChop, "INTERNAL", b+s*5);
  setSymbol(sFind, "INTERNAL", b+s*5);

  setSymbol(mAdd, "INTERNAL", b+s*6);
  setSymbol(mSub, "INTERNAL", -(b+s*6+2));
  setSymbol(mMul, "INTERNAL", b+s*6+4);
  setSymbol(mDiv, "INTERNAL", -(b+s*6+6));

  setSymbol(mRand, "INTERNAL", b+s*7);
  setSymbol(sLen, "INTERNAL", b+s*7);
  setSymbol(getPri, "INTERNAL", b+s*7);
  setSymbol(getCode, "INTERNAL", b+s*7);
  setSymbol(getFile, "INTERNAL", b+s*7);
  setSymbol(putFile, "INTERNAL", b+s*7);

  setSymbol(cID, "INTERNAL", b+s*7+2);

  setSymbol(dELwrap, "INTERNAL",  (b+s*8));
  setSymbol(iELwrap, "INTERNAL", -(b+s*8+2));

// these should NOT be evaluated at all..

  setSymbol(dPr, "INTERNAL", 0x7fffffff);

  setSymbol(iEL, "INTERNAL", 0x7ffffffe);
  setSymbol(dEL, "INTERNAL", 0x7fffffff);

  setSymbol(lScp, "INTERNAL", 0x7fffffff);
  setSymbol(rScp, "INTERNAL", 0x7fffffff);
  setSymbol(lTrue, "INTERNAL", 0x7fffffff);
  setSymbol(lFalse, "INTERNAL", 0x7fffffff);

}

void scriptClass::destroyStruct() {
 sStruct* oS; sStruct* S=rootSymbol; while (S!=NULL) { free(S->_case); free(S->_value); oS=S; S=S->_next; free(oS); }
}





bool fstrcmp(char* s1, char* s2) {
 bool res=true; long i=0; while (s1[i]==s2[i] && s1[i]!='\0' && s2[i]!='\0') i++;
 return (s1[i]=='\0' && s2[i]=='\0');
}















char* scriptClass::evaluate(char* expr) { // quick hack, wrapper to manage evLev
  evLev++;
  char* res=evaluate_real(expr);

if (debug) {
  for (int i=0; i<evLev; i++) printf(" ");
  printf("\x1B[32m\"%s\"\033[0m\n",res);
}

  evLev--;
  return res;
}



char* scriptClass::evaluate_real(char* expr) {

  if (expr[0]=='\0') return strdup("");

  sStruct* sS=rootSymbol; sStruct* fS=NULL;

  long lexpr=strlen(expr);
  long minPriority=0x7fffffff;
  long lastMatch=lexpr;
  long actualMatch;
  long pw=0;
//  long level=0, maxLevel=0;

  while (sS!=NULL && (pw==0 || pw==sS->_priority )) {

    long step=1, pri=sS->_priority;
    if (sS->_priority<0) {step=(-1); pri=pri*(-1);}
    long I=step;
    actualMatch = whereIs(expr,sS->_case,I);

    while (actualMatch<lexpr && levelOf(actualMatch,expr,iEL,dEL)!=0)
      actualMatch=whereIs(expr,sS->_case,I+=step);

    if (actualMatch!=lexpr && pri<minPriority && pri!=0) {
      if (pw==0) { pw=sS->_priority; lastMatch=actualMatch; fS=sS; }
      else if (lastMatch==actualMatch && strlen(fS->_case)<strlen(sS->_case)) {
        lastMatch=actualMatch; fS=sS;
      }
    }
    sS=sS->_next;
  }

  if (fS==NULL) return strdup(expr); else {

    long ofs=lastMatch+strlen(fS->_case);

    char* left=(char*)malloc(lastMatch+1);
    memcpy(left,expr,lastMatch); left[lastMatch]='\0';
    char* right=(char*)malloc(strlen(expr)+1-(lastMatch+strlen(fS->_case)));
    memcpy(right,expr+ofs,lexpr-ofs); right[lexpr-ofs]='\0';

if (debug) {
  for (int i=0; i<evLev; i++) printf(" ");
  printf("\x1B[90mevaluating\033[0m \"%s\", \x1B[31m\"%s\"\033[0m, \"%s\"\n",left,fS->_case,right);
}

    char* lEv; char* rEv;

    if (fstrcmp(fS->_case,iEL)) {

      if (lexpr==strlen(iEL)) return strdup(iEL);
      long a=lastMatch, b=a+strlen(iEL), x=1, iELl=strlen(iEL), dELl=strlen(dEL);
      while (x>0 && b+dELl<=lexpr && b+iELl<=lexpr) {
       if (!strncmp(expr+b,dEL,dELl)) {x--; b+=dELl-1;}
       else if (!strncmp(expr+b,iEL,iELl)) {x++; b+=iELl-1;}
       b++;
      };

      long ll=a, pl=b-(a+dELl), rl=lexpr-b; if (x==0) pl-=iELl;
      char* prot=(char*)malloc(pl+1); prot[pl]='\0'; memcpy(prot,expr+a+iELl,pl);
      char* rgexpr;
      if (b!=strlen(expr)) {rgexpr=(char*)malloc(rl+1); rgexpr[rl]='\0'; memcpy(rgexpr,expr+b,rl); } else rgexpr=strdup("");
      lEv=evaluate(left); rEv=evaluate(rgexpr); free(left); free(right); free(rgexpr);

      long lg=strlen(lEv)+strlen(rEv)+pl+1;
      char* tmp=(char*)malloc(lg); tmp[0]='\0'; memset(tmp,'\0',lg);
      strcat(tmp,lEv); strcat(tmp,prot); strcat(tmp,rEv);
      free(lEv); free(prot); free(rEv);
      return tmp;

    } else if (fstrcmp(fS->_case,noDo)) {

      if (left[0]=='\0' && right[0]=='\0') return strdup(noDo);
//      if (right[0]=='\0') warning(1,noDo,NULL);
      lEv=evaluate(left); free(left); free(lEv);
      rEv=evaluate(right); free(right); return rEv;

    } else if (fstrcmp(fS->_case,sThen)) {

      if (left[0]=='\0' & right[0]=='\0') return strdup(sThen);
      if (left[0]=='\0') {warning(0,sThen,NULL); free(left); free(right); return strdup("");}
      if (right[0]=='\0') {warning(1,sThen,NULL); free(left); free(right); return strdup("");}
      lEv = evaluate(left); free(left);
      long x=whereIs(right,sThen,1), y=strlen(right);
      char* _then; char* _else=strdup("");
      if (x!=y) {
       _then=(char*)malloc(x+1); memset(_then,'\0',x+1); memcpy(_then,right,x); long el=y-(x+strlen(sThen));
       free(_else); _else=(char*)malloc(el+1); memset(_else,'\0',el+1); memcpy(_else,right+x+strlen(sThen),el);
       free(right);
      } else _then=right;
      if (fstrcmp(lEv, lTrue)) {free(lEv); rEv=evaluate(_then); free(_then); return(rEv);}
      else {free(lEv); rEv=evaluate(_else); free(_then); free(_else); return(rEv);}

    } else if (fstrcmp(fS->_case,sWhile)) {

      if (left[0]=='\0' && right[0]=='\0') return strdup(sWhile);
      if (left[0]=='\0') {warning(0,sWhile,NULL); free(left); free(right); return rEv;}
      lEv=evaluate(left); rEv=NULL;
      while (true) {
        if (strcmp(lEv,lTrue)) {free(lEv); free(left); return rEv;}
        free(rEv); free(lEv); rEv=evaluate(right); lEv=evaluate(left);
      }
      free(left); free(lEv); return rEv;

    } else if (fstrcmp(fS->_case,aVal)) {
      if (left[0]=='\0' && right[0]=='\0') return strdup(aVal);
      lEv = evaluate(left); rEv = evaluate(right); long tmp=getPrio(lEv);
      if (lEv[0]=='\0') {warning(6,aVal,NULL); free(left); free(right); free(lEv); return rEv;}
      if (tmp==0) tmp=getPrio(rEv); if (tmp==0 || tmp==0x7fffffff) tmp=0x7ffffffe;
      setSymbol(lEv, rEv, tmp); free(left); free(lEv); free(right); return rEv;

    } else if (fstrcmp(fS->_case,sRen)) {

      if (left[0]=='\0' && right[0]=='\0') return strdup(sRen);
      lEv = evaluate(left); rEv = evaluate(right); free(left); free(right);
      if (lEv[0]=='\0') {warning(0,sRen,NULL); free(lEv); free(rEv); return strdup("");}
      if (rEv[0]=='\0') {warning(1,sRen,NULL); free(lEv); free(rEv); return strdup("");}

// printf("** RENAME %s to %s\n",lEv,rEv);

      if(fstrcmp(lEv,noDo)) {free(noDo); noDo=strdup(rEv);} else
      if(fstrcmp(lEv,setPri)) {free(setPri); setPri=strdup(rEv);} else
      if(fstrcmp(lEv,sThen)) {free(sThen); sThen=strdup(rEv);} else
      if(fstrcmp(lEv,sWhile)) {free(sWhile); sWhile=strdup(rEv);} else
      if(fstrcmp(lEv,aVal)) {free(aVal); aVal=strdup(rEv);} else
      if(fstrcmp(lEv,sRen)) {free(sRen); sRen=strdup(rEv);} else
      if(fstrcmp(lEv,setPri)) {free(setPri); setPri=strdup(rEv);} else
      if(fstrcmp(lEv,getPri)) {free(getPri); getPri=strdup(rEv);} else
      if(fstrcmp(lEv,iEL)) {free(iEL); iEL=strdup(rEv);} else
      if(fstrcmp(lEv,dEL)) {free(dEL); dEL=strdup(rEv);} else
      if(fstrcmp(lEv,iELwrap)) {free(iELwrap); iELwrap=strdup(rEv);} else
      if(fstrcmp(lEv,dELwrap)) {free(dELwrap); dELwrap=strdup(rEv);} else
      if(fstrcmp(lEv,iPr)) {free(iPr); iPr=strdup(rEv);} else
      if(fstrcmp(lEv,dPr)) {free(dPr); dPr=strdup(rEv);} else

      if(fstrcmp(lEv,sChop)) {free(sChop); sChop=strdup(rEv);} else
      if(fstrcmp(lEv,sFind)) {free(sFind); sFind=strdup(rEv);} else
      if(fstrcmp(lEv,sLen)) {free(sLen); sLen=strdup(rEv);} else
      if(fstrcmp(lEv,lScp)) {free(lScp); lScp=strdup(rEv);} else
      if(fstrcmp(lEv,rScp)) {free(rScp); rScp=strdup(rEv);} else

      if(fstrcmp(lEv,lTrue)) {free(lTrue); lTrue=strdup(rEv);} else
      if(fstrcmp(lEv,lFalse)) {free(lFalse); lFalse=strdup(rEv);} else
      if(fstrcmp(lEv,lLT)) {free(lLT); lLT=strdup(rEv);} else
      if(fstrcmp(lEv,lEQ)) {free(lEQ); lEQ=strdup(rEv);} else

      if(fstrcmp(lEv,lAND)) {free(lAND); lAND=strdup(rEv);} else
      if(fstrcmp(lEv,lOR)) {free(lOR); lOR=strdup(rEv);} else
      if(fstrcmp(lEv,lNOT)) {free(lNOT); lNOT=strdup(rEv);} else

      if(fstrcmp(lEv,mAdd)) {free(mAdd); mAdd=strdup(rEv);} else
      if(fstrcmp(lEv,mSub)) {free(mSub); mSub=strdup(rEv);} else
      if(fstrcmp(lEv,mMul)) {free(mMul); mMul=strdup(rEv);} else
      if(fstrcmp(lEv,mDiv)) {free(mDiv); mDiv=strdup(rEv);} else
      if(fstrcmp(lEv,mRand)) {free(mRand); mRand=strdup(rEv);} else
      if(fstrcmp(lEv,getCode)) {free(getCode); getCode=strdup(rEv);} else
      if(fstrcmp(lEv,getFile)) {free(getFile); getFile=strdup(rEv);} else
      if(fstrcmp(lEv,putFile)) {free(putFile); putFile=strdup(rEv);} else
      if(fstrcmp(lEv,cID)) {free(cID); cID=strdup(rEv);}

      sStruct* S=rootSymbol;
      while (S!=NULL) { if (fstrcmp(lEv,S->_case)) { free(S->_case); S->_case=rEv; return strdup("");} S=S->_next; }

      warning(5,sRen,NULL);
      return strdup("");

    } else if (fstrcmp(fS->_case,getFile)) {

      lEv=evaluate(left); rEv=evaluate(right); free(left); free(right);
      int f=open(rEv,O_RDONLY); char* b=(char*)malloc(65536); memset(b,'\0',65536); read(f,b,65535); close(f);
      long x=strlen(lEv)+strlen(b)+1;
      char* b2=(char*)malloc(x); b2[x-1]='\0'; memcpy(b2,lEv,strlen(lEv)); memcpy(b2+strlen(lEv),b,strlen(b));
      free(b); free(lEv); free(rEv); return b2;

    } else if (fstrcmp(fS->_case,putFile)) {

      lEv=evaluate(left); rEv=evaluate(right); free(left); free(right);
      int f=open(rEv,O_WRONLY|O_CREAT,00644); write(f,lEv,strlen(lEv)); close(f);
      free(rEv); return lEv;

    } else if (fstrcmp(fS->_case,iPr)) {

      if (left[0]=='\0' && right[0]=='\0') return strdup(iPr);
      long a=whereIs(expr,iPr,1), b=a+strlen(iPr), x=1, lx=strlen(expr), il=strlen(iPr), dl=strlen(dPr);
      while (x>0 && b+dl<=lx && b+il<=lx) {
       if (!strncmp(expr+b,dPr,dl)) {x--; b+=dl-1;}
       else if (!strncmp(expr+b,iPr,il)) {x++; b+=il-1;}
       b++;
      };

      long ll=a, pl=b-(a+strlen(iPr)+strlen(dPr)), rl=lexpr-b;

      char* inside=(char*)malloc(pl+1); inside[pl]='\0'; memcpy(inside,expr+a+strlen(iPr),pl);
      char* rexpr=(char*)malloc(rl+1); rexpr[rl]='\0'; memcpy(rexpr,expr+b,rl);
      char* res=evaluate(inside); free(inside);

      long lg=strlen(left)+strlen(rexpr)+strlen(res)+1;
      char* tmp=(char*)malloc(lg); memset(tmp,'\0',lg);
      strcat(tmp,left); strcat(tmp,res); strcat(tmp,rexpr);
      free(left); free(right); free(res); free(rexpr);
      return evaluate(tmp);

    } else if (fstrcmp(fS->_case,setPri)) {

      if (left[0]=='\0' && right[0]=='\0') return strdup(setPri);
      if (left[0]=='\0') warning(0,setPri,NULL); if (right[0]=='\0') warning(1,setPri,NULL);
      lEv=evaluate(left); rEv=evaluate(right); free(left); free(right);
      long tmp=strtol(rEv,NULL,10); free(rEv); rEv=getSymbol(lEv);
      if (lEv[0]=='\0') {warning(5,setPri,NULL); return rEv;}
      if (tmp==0) {warning(6,setPri,NULL); tmp=0x7fffffff;}
      else setSymbol(lEv,rEv,tmp); free(rEv); return lEv;

    } else if (fstrcmp(fS->_case,lAND)) {

      if (left[0]=='\0' && right[0]=='\0') return strdup(lAND);
      if (left[0]=='\0') warning(0,lAND,NULL); if (right[0]=='\0') warning(1,lAND,NULL);
      lEv = evaluate(left); rEv = evaluate(right);
      bool b=(fstrcmp(lEv,lTrue) && fstrcmp(rEv,lTrue)); free(left); free(right); free(lEv); free(rEv);
      if (b) return strdup(lTrue); else return strdup(lFalse);

    } else if (fstrcmp(fS->_case,lOR)) {

      if (left[0]=='\0' && right[0]=='\0') return strdup(lOR);
      if (left[0]=='\0') warning(0,lOR,NULL); if (right[0]=='\0') warning(1,lOR,NULL);
      lEv = evaluate(left); rEv = evaluate(right);
      bool b=(fstrcmp(lEv,lTrue) || fstrcmp(rEv,lTrue));
      free(left); free(right); free(lEv); free(rEv);
      if (b) return strdup(lTrue); else return strdup(lFalse);

    } else if (fstrcmp(fS->_case,lNOT)) {

      if (left[0]=='\0' && right[0]=='\0') return strdup(lNOT);
      lEv=evaluate(left); rEv=evaluate(right); free(left); free(right);
      char* lnot=lTrue; if (fstrcmp(rEv,lTrue)) lnot=lFalse;
      long l=strlen(lEv)+strlen(lnot)+1;
      char* res=(char*)malloc(l); memset(res,'\0',l);
      strcat(res,lEv); strcat(res,lnot); free(lEv); free(rEv);
      return res;

    } else if (fstrcmp(fS->_case,lLT)) {

      if (left[0]=='\0' && right[0]=='\0') return strdup(lLT);
      if (left[0]=='\0') warning(0,lLT,NULL); if (right[0]=='\0') warning(1,lLT,NULL);
      lEv = evaluate(left); rEv = evaluate(right);
      char* chk; bool b; long x,y;
      x=strtol(lEv,&chk,10);
      if (chk[0]=='\0') y=strtol(rEv,&chk,10);
      if (chk[0]=='\0') b=(x<y); else b=(strcmp(lEv,rEv)<0);
      free(left); free(right); free(lEv); free(rEv);
      if (b) return strdup(lTrue); else return strdup(lFalse);

    } else if (fstrcmp(fS->_case,lEQ)) {

      if (left[0]=='\0' && right[0]=='\0') return strdup(lEQ);
      if (left[0]=='\0') warning(0,lEQ,NULL); if (right[0]=='\0') warning(1,lEQ,NULL);
      lEv = evaluate(left); rEv = evaluate(right);
      bool b=(fstrcmp(lEv,rEv));
      free(left); free(right); free(lEv); free(rEv);
      if (b) return strdup(lTrue); else return strdup(lFalse);

    } else if (fstrcmp(fS->_case,mAdd)) {
      if (left[0]=='\0' && right[0]=='\0') return strdup(mAdd);
      if (left[0]=='\0') warning(0,mAdd,NULL); if (right[0]=='\0') warning(1,mAdd,NULL);
      lEv=evaluate(left); rEv=evaluate(right); free(left); free(right); left=lEv; right=rEv;
      long x=strtol(left,NULL,10); long y=strtol(right,NULL,10);  free(left); free(right);  return n2s(x+y);

    } else if (fstrcmp(fS->_case,mSub)) {
      if (left[0]=='\0' && right[0]=='\0') return strdup(mSub);
/*      if (left[0]=='\0') warning(0,mSub,NULL);*/ if (right[0]=='\0') warning(1,mSub,NULL);
      lEv=evaluate(left); rEv=evaluate(right); free(left); free(right); left=lEv; right=rEv;
      long x=strtol(left,NULL,10);  long y=strtol(right,NULL,10); free(left); free(right);  return n2s(x-y);

    } else if (fstrcmp(fS->_case,mMul)) {
      if (left[0]=='\0' && right[0]=='\0') return strdup(mMul);
      if (left[0]=='\0') warning(0,mMul,NULL); if (right[0]=='\0') warning(1,mMul,NULL);
      lEv=evaluate(left); rEv=evaluate(right); free(left); free(right); left=lEv; right=rEv;
      long x=strtol(left,NULL,10); long y=strtol(right,NULL,10); free(left); free(right);  return n2s(x*y);

    } else if (fstrcmp(fS->_case,mDiv)) {
      if (left[0]=='\0' && right[0]=='\0') return strdup(mDiv);
      if (left[0]=='\0') warning(0,mDiv,NULL); if (right[0]=='\0') warning(1,mDiv,NULL);
      lEv=evaluate(left); rEv=evaluate(right); free(left); free(right); left=lEv; right=rEv;
      long x=strtol(left,NULL,10); long y=strtol(right,NULL,10); free(left); free(right);  return n2s(x/y);

    } else if (fstrcmp(fS->_case,mRand)) {
      if (left[0]!='\0') warning(2,mRand,NULL);
      if (right[0]!='\0') warning(3,mRand,NULL);
      free(left); free(right); return n2s(random());

    } else if (fstrcmp(fS->_case,getPri)) {

      if (left[0]=='\0' && right[0]=='\0') return strdup(getPri);
      lEv=evaluate(left); rEv=evaluate(right); free(left); free(right);
      char* pri=n2s(getPrio(rEv)); long l=strlen(lEv)+strlen(pri)+1;
      char* res=(char*)malloc(l); memset(res,'\0',l);
      strcat(res,lEv); strcat(res,pri); free(lEv); free(rEv); free(pri);
      return res;

    } else if (fstrcmp(fS->_case,sLen)) {
      if (left[0]=='\0' && right[0]=='\0') return strdup(sLen);
      lEv=evaluate(left); rEv=evaluate(right); free(left); free(right);
      char* len=n2s(strlen(rEv)); long l=strlen(lEv)+strlen(len)+1;
      char* res=(char*)malloc(l); memset(res,'\0',l);
      strcat(res,lEv); strcat(res,len); free(lEv); free(rEv); free(len);
      return res;

    } else if (fstrcmp(fS->_case,sChop)) {

      char* res=NULL; char* t;
      long wl,from,l;

      if (left[0]=='\0' && right[0]=='\0') res=strdup(sChop);
      else if (left[0]=='\0') res=strdup("");
      else if (right[0]=='\0') res=evaluate(left);
      else {
        lEv=evaluate(left); wl=whereIs(right,sChop,1);
	if (wl==strlen(right)) {
	  t=evaluate(right); from=strtol(t,NULL,10); free(t);
          if (from>=strlen(lEv)) { warning(4,sChop,NULL);printf("%s,%s\n",left,right); res=strdup("");}
	  else {res=(char*)malloc(strlen(lEv)-from+1); memset(res,'\0',strlen(lEv)-from+1); memcpy(res,lEv+from,strlen(lEv)-from);}
        } else {
          char* frm=(char*)malloc(wl+1); memset(frm,'\0',wl+1); memcpy(frm,right,wl);
          t=evaluate(frm); free(frm); from=strtol(t,NULL,10); free(t);
          if(from>=strlen(lEv)) { warning(4,sChop,NULL); res=strdup("");}
	  else {
            wl+=strlen(sChop); char* len;
            if (wl<strlen(right)) {
              len=(char*)malloc(strlen(right)-wl+1); memset(len,'\0',strlen(right)-wl+1); memcpy(len,right+wl,strlen(right)-wl);
	      t=evaluate(len); free(len); l=strtol(t,NULL,10); free(t);
	      if (l+from>strlen(lEv)) {warning(4,sChop,NULL); l=strlen(lEv)-from;}
            } else {warning(1,sChop,NULL); l=strlen(lEv)-from;}
            res=(char*)malloc(l+1); res[l]='\0'; memcpy(res,lEv+from,l);
	  }
        }
	free(lEv);
      }
      free(left); free(right);
      return res;

    } else if (fstrcmp(fS->_case,sFind)) {

      if (left[0]=='\0' && right[0]=='\0') return strdup(sFind);
      lEv=evaluate(left);
      long wl=whereIs(right,sFind,1); if (wl==strlen(right)) {
        rEv=evaluate(right);
	char* res=n2s(whereIs(lEv,rEv,1));
	free(left); free(right); free(rEv); free(lEv);
	return res;
      }
      char* what=(char*)malloc(wl+1); what[wl]='\0'; memcpy(what,right,wl);
      char* wtmp=evaluate(what); free(what); what=wtmp;
      wl+=strlen(sFind);
      char* n=(char*)malloc(strlen(right)-wl+1); n[strlen(right)-wl]='\0'; strcpy(n,right+wl);
      wtmp=evaluate(n); free(n); n=wtmp;
      long N=strtol(n,NULL,10);
      char* res=n2s(whereIs(lEv,what,N));
      free(left); free(right); free(lEv); free(what); free(n);
      return res;

    } else if (fstrcmp(fS->_case,getCode)) {

      if (left[0]=='\0' && right[0]=='\0') return strdup(getCode);
      lEv=evaluate(left); rEv=evaluate(right); free(left); free(right);
      char* code=getSymbol(rEv); if (code[0]=='\0') {free(code); code=strdup(rEv);}
      long l=strlen(code)+strlen(lEv)+1;
      char* res=(char*)malloc(l); memset(res,'\0',l);
      strcat(res,lEv); strcat(res,code); free(lEv); free(rEv); free(code);
      return res;

    } else if (fstrcmp(fS->_case,cID)) {

      lEv=evaluate(left); rEv=evaluate(right); free(left); free(right);
      long I=strlen(lEv); lEv[I-1]+=strtol(rEv,NULL,10); free(rEv); return lEv;

    } else if (fstrcmp(fS->_case,iELwrap)) {

      lEv=evaluate(left); rEv=evaluate(right); free(left); free(right);
      long size=strlen(lEv)+strlen(iEL)+strlen(rEv);
      char* res=(char*)malloc(size+1); memset(res,'\0',size+1);
      strcpy(res,lEv); strcat(res,iEL); strcat(res,rEv);
      free(lEv); free(rEv); return res;

    } else if (fstrcmp(fS->_case,dELwrap)) {

      lEv=evaluate(left); rEv=evaluate(right); free(left); free(right);
      long size=strlen(lEv)+strlen(dEL)+strlen(rEv);
      char* res=(char*)malloc(size+1); memset(res,'\0',size+1);
      strcpy(res,lEv); strcat(res,dEL); strcat(res,rEv);
      free(lEv); free(rEv); return res;

    } else {

      char* buf=getSymbol(fS->_case); if (fstrcmp(buf,fS->_case)) return buf; char* b2=buf;
      char* ql=(char*)malloc(strlen(left)+strlen(iEL)+strlen(dEL)+1); memset(ql,'\0',strlen(left)+strlen(iEL)+strlen(dEL)+1);
      strcat(ql,iEL); strcat(ql,left); strcat(ql,dEL);
      char* qr=(char*)malloc(strlen(right)+strlen(iEL)+strlen(dEL)+1); memset(qr,'\0',strlen(right)+strlen(iEL)+strlen(dEL)+1);
      strcat(qr,iEL); strcat(qr,right); strcat(qr,dEL);

      bool b, lfound=false, rfound=false;
      long I=0, ln=strlen(buf), ll=strlen(lScp),rl=strlen(rScp);//, lmax=ln-ll,  rmax=ln-rl;
//printf("%i,%i,%i,%i,%i,%i\n",I,ln,ll,lmax,rl,rmax);
//if (fstrcmp(buf+I,lScp)) printf("left found @ %i\n",I);
      while (I<ln) {
//        printf("%i,%i,%i\n",I,lmax,levelOf(I,buf,iEL,dEL));

        if (I<=ln-ll) if (!strncmp(buf+I,lScp,ll)) if (levelOf(I,buf,iEL,dEL)==0) { lfound=true; b2=replaceStr(buf,I,ll,ql); free(buf); buf=b2; ln+=strlen(ql)-ll; I+=strlen(ql)-1; };
        if (I<=ln-rl) if (!strncmp(buf+I,rScp,rl)) if (levelOf(I,buf,iEL,dEL)==0) { rfound=true; b2=replaceStr(buf,I,rl,qr); free(buf); buf=b2; ln+=strlen(qr)-rl; I+=strlen(qr)-1; };
//	printf("%i\n",I);
	I++;
      }
//printf("%s,%s,%s\n",lScp,rScp,buf);
      if (!lfound) { b2=replaceStr(buf,0,0,left); free(buf); buf=b2; }
      if (!rfound) { b2=replaceStr(buf,strlen(buf),0,right); free(buf); buf=b2; }
/*
      where=whereIs(buf,lScp,1); b=false; I=1;
      while (where<strlen(buf)) {
        if (levelOf(where,buf,iEL,dEL)==0) {b=true; b2=replaceStr(buf,where,strlen(lScp),ql);} else {b2=strdup(buf); I++;}
        free(buf); buf=b2;
        where=whereIs(buf,lScp,I);
      }

      where=whereIs(buf,rScp,1); b=false; I=1;
      while (where<strlen(buf)) {
        if (levelOf(where,buf,iEL,dEL)==0) {b=true; b2=replaceStr(buf,where,strlen(rScp),qr);} else {b2=strdup(buf); I++;}
        free(buf); buf=b2;
        where=whereIs(buf,rScp,I);
      }
      if (!b) {
        b2=replaceStr(buf,strlen(buf),0,right);
        free(buf); buf=b2;
      }
*/
      free(ql); free(qr); free(left); free(right);

      return(evaluate(buf));

    }
  }
}


















// string, parsing, etc. functions

long scriptClass::whereIs(char* str, char* search, long n) {
  long step, iResBuf, sIndexStr, eIndexStr, sIndexSearch, eIndexSearch, iS, i, m;
  if (search==NULL || str==NULL || n=='\0' || search[0]=='\0' || str[0]=='\0' || strlen(search)>strlen(str)) return strlen(str);
  if (n > 0) { step = 1; sIndexStr = 0; eIndexStr = strlen(str);
    sIndexSearch = 0; eIndexSearch = strlen(search)-1; iResBuf = (-strlen(search))+1;
  } else { step = -1; sIndexStr = strlen(str)-1; eIndexStr = -1;
    sIndexSearch = strlen(search)-1; eIndexSearch = 0; iResBuf = 0; }
  iS = sIndexSearch; i=sIndexStr; m=0;
  while (i!=eIndexStr) {
    if (str[i]==search[iS]) {
      m++; if (m==strlen(search)) { if (n==step) return iResBuf+i; else n-=step; } i+=step; iS+=step;
    } else { i-=(step*(m-1)); iS=sIndexSearch; m=0; }
  }
  return strlen(str);
}

char* strdup(char* s) { char* tmp=(char*)malloc(strlen(s)+1); memcpy(tmp,s,strlen(s)); tmp[strlen(s)]='\0'; return tmp; }

char* scriptClass::replaceStr(char* str,long from,long length,char* replace) {
  long sl=strlen(str); if (from+length>sl) return strdup(str); long nl=strlen(str)-length+strlen(replace)+1;
  char* buffer=(char*) malloc(nl); memset(buffer,'\0',nl);
  memcpy(buffer, str, from); strcat(buffer, replace); strcat(buffer, str + from + length); return buffer;
}

long scriptClass::levelOf(long pos, char* str, char* lD, char* rD) {
  if (pos==whereIs(str,lD,1)) return 0; long sl=strlen(str); long i,j,P;
  i=1; P=whereIs(str,lD,i); while (P+strlen(lD)-1<pos && P<sl) P=whereIs(str,lD,++i);
  j=1; P=whereIs(str,rD,j); while (P+strlen(rD)-1<pos && P<sl) P=whereIs(str,rD,++j);
  return i-j;
}

//using namespace std;

char* scriptClass::n2s(long n) {
  char* res=(char*)malloc(12); memset(res,'\0',12); bool neg=false; if (n<0) {neg=true; n=n*(-1);}
  ldiv_t dt=ldiv(n,10); long i=10;
  while (dt.quot>0 && i>0) { res[i--]=dt.rem+48; n=dt.quot; dt=ldiv(n,10); }
  res[i]=dt.rem+48; if (neg) {i--; res[i]='-';};
  char* r2=(char*)malloc(12-i); memcpy(r2,res+i,12-i); free(res); return r2;
}

char* scriptClass::pad(long n, long l) {
 char* ns=n2s(n); if (strlen(ns)>l) l=strlen(ns);
 char* buf=(char*)malloc(l+1); memset(buf,'\0',l+1); long i=l-strlen(ns); while(i>0) buf[--i]='0';
 memcpy(buf+l-strlen(ns),ns,strlen(ns)); free(ns); return buf;
}

void scriptClass::warning(long code,char* symbol,char* desc) {
  if (noWarnings) return;
  char* buf=pad(code,3);
  if (desc==NULL || desc[0]=='\0') {
    printf("#%s :\t \"%s\" :\t ",buf,symbol);
    switch(code) {
      case 0: printf("left side missing.\n"); break;
      case 1: printf("right side missing.\n"); break;
      case 2: printf("left side dropped by symbol.\n"); break;
      case 3: printf("right side dropped by symbol.\n"); break;
      case 4: printf("value out of range, or not specified.\n"); break;
      case 5: printf("referenced symbol does not exist.\n"); break;
      case 6: printf("invalid setting.\n"); break;
    }
  } else { printf("#%s :\t \"%s\" :\t %s\n",buf,symbol,desc); }
  free(buf);
}







































// symbol related stuff

long sc_abs(long n) {if (n<0) return -n; else return n;}

void scriptClass::setSymbol (char* _case, char* _value, long _priority) {

  if (_case==NULL || _case[0]=='\0') return;

  bool rem=(fstrcmp(_case,_value));
//  printf("setS: \"%s\" to \"%s\"\n",_case,_value);
  sStruct* S=rootSymbol;
  sStruct* S2=S;
  sStruct* oS=S;
  while (S!=NULL && !fstrcmp(_case,S->_case)) {oS=S; S=S->_next;}

  if (S==NULL) {// not existing
//printf("not exists..\n");
    if (!rem) {
      S=rootSymbol;
      while (S!=NULL && sc_abs(S->_priority)<sc_abs(_priority)) {oS=S; S=S->_next;}
      S2=(sStruct*)malloc(sizeof(sStruct));
      S2->_case=strdup(_case);
      S2->_value=strdup(_value);
      S2->_priority=_priority;

      oS->_next=S2; S2->_next=S;
    }
  } else { /* existing symbol, there's a bit more work as
              we have to keep the structure ordered by priority,
              this gives the huge boost in performance compared to
	      the previous releases */
    if (!rem && sc_abs(_priority)==sc_abs(S->_priority)) { // only value changes
//printf("exists, value changed..\n");

      free(S->_value); S->_value=strdup(_value);
    } else if (rem) { // remove a symbol by assigning it to itself
//printf("exists, removing..\n");

      free(S->_value); free(S->_case); oS->_next=S->_next; free(S);
    } else { // priority changed, we'll have to correct the order
//printf("exists, priority changed..\n");

      oS->_next=S->_next;
      oS=rootSymbol; while (oS!=NULL && sc_abs(oS->_priority)<sc_abs(_priority)) { S2=oS; oS=oS->_next;}
      S2->_next=S; S->_next=oS;
      free(S->_value); S->_value=strdup(_value); S->_priority=_priority;
    }
  }
}

char* scriptClass::getSymbol (char* _case) {
  sStruct* oS = rootSymbol;
  while (oS != NULL) {
    if (fstrcmp(oS->_case, _case)) if (oS->_value!=NULL) return strdup(oS->_value); else return strdup(oS->_case);
    oS=oS->_next;
  }
  return strdup("");
}

long scriptClass::getPrio(char* sym) {
  sStruct* S=rootSymbol;
  while (S!=NULL) {
   if (fstrcmp(S->_case,sym)) return S->_priority;
   S=S->_next;
  }
  return 0;
}
