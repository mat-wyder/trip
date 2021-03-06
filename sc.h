
typedef struct symbolStruct {
    char* _case;
    char* _value;
    long _priority;
    symbolStruct* _next;
} sStruct;

class scriptClass {

public:

// constructor, destructor & initalization workaround
    scriptClass();
    ~scriptClass();
    void mkNewStruct();
    void destroyStruct();
// the main recursive function (wrapper)
    char* evaluate (char*);
// string related code
    long whereIs(char*, char*, long);
    char* replaceStr (char*, long, long, char*);
    char* n2s (long);
    char* pad (long, long);
// symbol related code
    long levelOf (long, char*, char*, char*);
    long getPrio (char*);
    void setSymbol (char*, char*, long);
    char* getSymbol (char*);
    void warning (long, char*, char*);

    sStruct* rootSymbol;
    bool noWarnings;
    bool debug;

//private:

    char* noDo; char* setPri; char* getPri; char* getCode; char* getFile; char* putFile; char* sThen; char* sWhile; char* aVal; char* sRen;
    char* iEL; char* dEL; char* iELwrap; char* dELwrap; char* iPr; char* dPr; char* cID;
    char* sChop; char* sFind; char* sLen; char* lScp; char* rScp; char* lTrue; char* lFalse;
    char* lLT; char* lEQ; char* lAND; char* lOR; char* lNOT; char* mAdd; char* mSub; char* mMul; char* mDiv; char* mRand;

private:

    char* evaluate_real (char*);
    long evLev;

};
 
