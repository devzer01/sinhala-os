#ifndef NBC_H
#define NBC_H

// C Preprocessor stuff
// this will change a #define 'int' into a #define "str"
//#define STR_HELPER(x) #x
//#define STR(x) STR_HELPER(x)

#define TRUE   1
#define FALSE  0

// size of memory operands
typedef   signed  char      bit8s;
typedef unsigned  char      bit8u;
typedef   signed short      bit16s;
typedef unsigned short      bit16u;
typedef   signed  long      bit32s;
typedef unsigned  long      bit32u;
typedef   signed  long long bit64s;
typedef unsigned  long long bit64u;

// pack all structures on the byte boundary
#pragma pack(push, 1)

#define VERSION_STR "0.20.27"
#define VERSION_INT "0.2027" // <--- change to match current ver num ---|
char verbstr[] = "\nNBC   version    00.20.27"    // <------------------|
                 "\n      build        002027"    // <------------------|
                 "\n      date    19 Aug 2016\n"; //                    V
char strtstr[] = "\nNBC     The NewBasic C Compiler     Version  00.20.27"
                 "\nForever Young Software(r)       (C)opyright 1984-2016\n";
char helpstr[] = "\nNBC Help screen"
                 "\n Usage:"
                 "\n  NBC  source[.c] [target.asm] [-parameter list]"
                 "\n    items in brackets are optional"
                 "\n     (*parameters are case sensitive*)"
                 "\n"
                 "\n   see documentation (TODO: list parameters here)";

////////////////////////////////////////////////////////////////////////////////
// all public macros
#define MAX_STRING_LEN       4096
#define MAX_CHAR_QUEUE_LEN    256
#define MAX_IDENT_LEN         127

#define MAX_FILE_NAME_LEN      95
#define MAX_INCLUDES           32

#define PREP_STACK_SIZE         8
#define MAX_SEARCH_PATH       256

#define PREP_STACK_ELIF        -1   // ELIF used, don't execute after #else (must remain a value less than 0)
#define PREP_STACK_FALSE        0   // ELIF not used, execute after #else (must remain a value of 0)
#define PREP_STACK_TRUE         1   // IF/IFDEF used, keep executing (must remain a value of 1)

/* +-~* /% &|^! << >> && || < <= > >= == !=  () *[] ++ -- = += -= ~= *= /= %= &= |= ^= <<= >>= {} ,;: -> ... */

#define tokEof        0
#define tokNumInt     1
#define tokNumUint    2
#define tokLitStr     3
#define tokLitWStr    4

#define tokLShift     5
#define tokRShift     6
#define tokLogAnd     7
#define tokLogOr      8
#define tokEQ         9
#define tokNEQ        10
#define tokLEQ        11
#define tokGEQ        12
#define tokInc        13
#define tokDec        14
#define tokArrow      15
#define tokEllipsis   16

#define tokIdent      17
#define tokVoid       18
#define tokChar       19
#define tokWChar      20
  #define WIDTH_OF_WIDECHAR  2  // can be 1, 2, or 4
#define tokInt        21
#define tokReturn     22
#define tokGoto       23
#define tokIf         24
#define tokElse       25
#define tokWhile      26
#define tokCont       27
#define tokBreak      28
#define tokSizeof     29

#define tokAssignMul  'A'
#define tokAssignDiv  'B'
#define tokAssignMod  'C'
#define tokAssignAdd  'D'
#define tokAssignSub  'E'
#define tokAssignLSh  'F'
#define tokAssignRSh  'G'
#define tokAssignAnd  'H'
#define tokAssignXor  'I'
#define tokAssignOr   'J'

#define tokFloat      'a'
#define tokDouble     'b'
#define tokLong       'c'
#define tokShort      'd'
#define tokUnsigned   'e'
#define tokSigned     'f'
#define tokConst      'g'
#define tokVolatile   'h'
#define tokRestrict   'i'
#define tokStatic     'j'
#define tokInline     'k'
#define tokExtern     'l'
#define tokAuto       'm'
#define tokRegister   'n'
#define tokTypedef    'o'
#define tokEnum       'p'
#define tokStruct     'q'
#define tokUnion      'r'
#define tokDo         's'
#define tokFor        't'
#define tokSwitch     'u'
#define tokCase       'v'
#define tokDefault    'w'
#define tok_Bool      'x'
#define tok_Complex   'y'
#define tok_Imagin    'z'

//#define tok?????      95
#define tok_Asm       '`'  // 96

/* Pseudo-tokens (converted from others or generated) */
#define tokURShift    28
#define tokUDiv       29
#define tokUMod       30
#define tokAssignURSh 31
#define tokAssignUDiv '@'
#define tokAssignUMod 'K'
#define tokComma      '0'

#define tokIfNot      'L'

#define tokUnaryAnd   'M'
#define tokUnaryStar  'N'
#define tokUnaryPlus  'O'
#define tokUnaryMinus 'P'

#define tokPostInc    'Q'
#define tokPostDec    'R'
#define tokPostAdd    'S'
#define tokPostSub    'T'

#define tokULess      'U'
#define tokUGreater   'V'
#define tokULEQ       'W'
#define tokUGEQ       'X'

#define tokLocalOfs   'Y'
#define tokShortCirc  'Z'

#define tokSChar      0x80
#define tokUChar      0x81
#define tokUShort     0x82
#define tokULong      0x83
//#define tokLongLong   0x84
//#define tokULongLong  0x85
//#define tokLongDbl    0x86
#define tokGotoLabel  0x8F
#define tokStructPtr  0x90
#define tokTag        0x91
#define tokMemberIdent 0x92
#define tokEnumPtr    0x93
#define tokIntr       0x94
#define tokNaked      0x95

///////////////////////////////
// Flags member
// bits 31:29 = far type token
// bits 28:10 = ??
// bits  9: 0 = reserved by normal tokens ????
#define tokFarMask    (7<<29)
#define tokFarC       (1<<29)
#define tokFarD       (2<<29)
#define tokFarE       (3<<29)
#define tokFarF       ((unsigned) (4<<29))  // new GCC assumes that 0x80000000 is a signed value
#define tokFarG       ((unsigned) (5<<29))  //  they should have a fix for it soon.

struct S_SYNTAX_STACK {
  int    tok;
  bit32u param;   // int or pointer (casted)
  bit32u flags;
};

#define FORMATFLAT      0
#define FORMATSEGMENTED 1

#define SECTION_IS_CODE 0
#define SECTION_IS_DATA 1

#define SymVoidSynPtr 0
#define SymIntSynPtr  1
#define SymUintSynPtr 2
#define SymFuncPtr    3

#define SymFxn       1
#define SymGlobalVar 2
#define SymGlobalArr 3
#define SymLocalVar  4
#define SymLocalArr  5

struct S_BRKCNTSWCHTARGET {
  int   Break;
  int   Continue;
  int   Default;
  int   NextCase;
};

struct S_CASES {
  int  Value;  // is case constant
  int  Label;  // is case label number
};

// all public prototypes
unsigned truncUint(unsigned);
int truncInt(int);

bool DoIfElse(char *, char *);
int GetToken(void);
char* GetTokenName(int token);
int exprUnary(int tok, int *gotUnary, int commaSeparator, int argOfSizeOf);

void DumpMacroTable(void);
void DumpSynDecls(void);

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Macros
#define MAX_MACRO_TABLE_LEN   (2 * 1024 * 1024)
#define MACRO_DEFAULT_EX_LEN   63
#define MACRO_TABLE_NEXT(m)   (struct S_MACRO_TABLE *) ((bit8u *) (m) + (m)->length)
#define MACRO_TABLE_ID_LEN(m) (m)->idlen
#define MACRO_TABLE_EX_LEN(m) (m)->exlen
#define MACRO_TABLE_ID(m)     (char *) ((bit8u *)(m) + sizeof(struct S_MACRO_TABLE))
#define MACRO_TABLE_EX(m)     (char *) ((bit8u *)(m) + sizeof(struct S_MACRO_TABLE) + (m)->idlen)
struct S_MACRO_TABLE {
  unsigned int length;   // total length of this entry
  unsigned int idlen;    // identifier length
  unsigned int exlen;    // length of what the identifier expands into
//char id[];    // identifier (ASCIIZ) (fixed size once set)
//char ex[];    // what the identifier expands into (ASCII) (variable size)
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Strings
#define STRING_IS_WIDE (1<<0)

#define MAX_STRING_TABLE_LEN (2 * 1024 * 1024)
#define STRING_TABLE_NEXT(x, l) (struct S_STRING_TABLE *) ((bit8u *) (x) + sizeof(struct S_STRING_TABLE) + (l))
struct S_STRING_TABLE {
           int label;  // temporary identifier's (char *) label number (Lxxxx ?)
  unsigned int len;    // string length
  unsigned int flags;  // flags  (bit 0 = 1 = is wchar_t, else is asciiz)
   //     char str[];  // string (ASCII)
};
int AddString(const int label, const char *str, const int len, unsigned int flags);
struct S_STRING_TABLE *FindString(const int label);
void PurgeStringTable(void);

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Ident Table
#define IDENT_FLAGS_ASSIGNED   (1 <<  0)   // was it assigned
#define IDENT_FLAGS_USED       (1 <<  1)   // was it used

#define IDENT_FLAGS_ISLOCAL    (1 << 16)   // is a local variable
#define IDENT_FLAGS_NOUNDERSCR (1 << 17)   // Don't add an underscore


#define IDENT_FLAGS_ISCONST    (1 << 26)
#define IDENT_FLAGS_ISREGISTER (1 << 27)
#define IDENT_FLAGS_ISVOLATILE (1 << 28)
#define IDENT_FLAGS_ISAUTO     (1 << 29)
#define IDENT_FLAGS_ISRESTRICT (1 << 30)
#define IDENT_FLAGS_ISINLINE   (1 << 31)

struct S_IDENT_PREFIX {
  struct S_IDENT_PREFIX *prev;  // pointer to previous (NULL if first)
  bit32u flags;                 // see IDENT_FLAGS_x
     int slen;                  // length of this string including null char (not more than MAX_IDENT_NAME_LEN + 1)
     int line_no;               // line number of this ident
//char  id[];
};

#define MAX_IDENT_NAME_LEN   127   // doesn't really matter as long as it is less than 65536, though let's keep it sane
#define MAX_IDENT_TABLE_LEN  (2 * 1024 * 1024)
#define IDENT_STR(x)   ((char *) (x) + sizeof(struct S_IDENT_PREFIX))
#define IDENT_NO_US(x) ((((struct S_IDENT_PREFIX *) (x))->flags & IDENT_FLAGS_NOUNDERSCR) == IDENT_FLAGS_NOUNDERSCR)

struct S_IDENT_PREFIX *FindIdent(const char *name);
struct S_IDENT_PREFIX *AddIdent(const char *name, const bit32u ident_flags);
void DumpIdentTable(void);

char *lab2str(char* p, int n);

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Goto Table
struct S_GOTO_LABELS {
  struct S_IDENT_PREFIX *ident;
    int label_num;
  bit8u stat;  // bit 1 = used (by "goto label;"), bit 0 = defined (with "label:")
};





struct S_STRUCTINFO {
  unsigned int type;       // type:
  unsigned int alignment;  // initial member alignment
  unsigned int offset;     // initial member offset
  unsigned int max_size;   // initial max member size (for unions)
};

#pragma pack(pop)

void SwitchSection(int section);
void GenInit(void);
void GenFin(void);
void GenInitFinalize(void);
void GenWordAlignment(const int);
void GenLabel(const char *Label, const bool NoUnderScore, const bool Static);
void GenLabelIP(const bit32u ptr, const bool Static);
void GenNumLabel(int Label);
void GenZeroData(unsigned Size);
void GenIntData(int Size, int Val);
void GenStartAsciiString(bool wide);
void GenAddrData(int Size, const bit32u label, int ofs);

void GenJumpUncond(int Label);
void GenJumpIfZero(int Label);
void GenJumpIfNotZero(int Label);
void GenJumpIfNotEqual(int val, int Label);
void GenJumpIfEqual(int val, int Label);

void GenFxnProlog(void);
void GenFxnEpilog(void);
void GenIsrProlog(void);
void GenIsrEpilog(void);

void GenLocalAlloc(int Size);

int GenStrData(char *str, int len, int tok);
void GenExpr(void);

void PushSyntax(int t);
void PushSyntax2(int t, bit32u v);
void PushSyntax3(int t, bit32u v, bit32u f);

void push2(int t, bit32u v);
void ins3(int pos, int t, bit32u v, bit32u f);
void ins2(int pos, int t, bit32u v);
void ins(int pos, int t);
void del(int pos, int cnt);

int TokenStartsDeclaration(int t, int params);
int ParseDecl(int tok, struct S_STRUCTINFO *structInfo, const bool cast, int label);

void ShiftChar(void);
int puts_out(const char *);
int printf_out(const char *, ...);

int InitScalar(int synPtr, int tok, const unsigned, const bool);
int InitArray(int synPtr, int tok);
int InitStruct(int synPtr, int tok);

int expr(int tok, int *gotUnary, int commaSeparator);

#ifdef GIVE_ERROR_LINES
  void error(const int, const int, const bool, ...);
#else
  void error(const int, const bool, ...);
#endif
void warning(const int, const int, ...);

void ParseFxnParams(int tok);
int ParseBlock(struct S_BRKCNTSWCHTARGET *BrkCntTarget, int casesIndex);
void AddFxnParamSymbols(int SyntaxPtr);

int FindSymbol(char* s);
int SymType(int SynPtr);
int FindTaggedDecl(char* s, int start, bool *CurScope);
int FindTypedef(char* s, bool *CurScope, const bool forUse);
int GetDeclSize(int SyntaxPtr, int SizeForDeref);

int ParseExpr(int tok, int *GotUnary, int *ExprTypeSynPtr, int *ConstExpr, int *ConstVal, int option, int option2);
int GetFxnInfo(int ExprTypeSynPtr, int *MinParams, int *MaxParams, int *ReturnExprTypeSynPtr, int *FirstParamSynPtr);

int AllocLocal(unsigned size);
int GenMaxLocalsSize(void);


#endif // NBC_H
