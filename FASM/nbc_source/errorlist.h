#ifndef NBC_ERROR_LIST_H
#define NBC_ERROR_LIST_H

// The list of warnings and errors
struct S_WARNING_LIST {
   int number;   // the warning number
   int level;    // the level (1,2,3,4)
  char name[80]; // the error text;
} WarningList[] = {
  { 0x0001, 1, "Shift count out of range." },
  { 0x0002, 1, "Division by 0 or division overflow." },
  { 0x0003, 3, "Call to undeclared function '%s()'." },
  { 0x0004, 1, "String literal truncated." },

  { 0x0100, 2, "local variable '%s' used w/o being initialized" },      // *
  { 0x0101, 2, "unreferenced local variable: '%s'" },                   // *

  { 0x0179, 3, "Expected %spointer in argument %d" },                   // *

  { 0x0300, 1, "EOF found before #endif" },

  { 0xFFFF, 0, "" }  // last one
};

struct S_ERROR_LIST {
   int number;   // the warning number
   int level;    // the level (1,2,3,4)
  char name[80]; // the error text;      // *  == has parameter(s) to string
} ErrorList[] = {
  // compiler structures
  { 0x0001, 1, "Table of globals exhausted" },
  { 0x0002, 1, "Macro table exhausted" },
  { 0x0003, 1, "String table exhausted" },
  { 0x0004, 1, "Identifier table exhausted" },
  { 0x0005, 1, "Goto table exhausted" },
  { 0x0006, 1, "Symbol table exhausted" },
  
  { 0x0010, 1, "Too long expansion of macro '%s'" },                    // *
  { 0x0011, 1, "Invalid or unsupported preprocessor directive" },
  { 0x0012, 1, "Invalid line number in preprocessor output" },
  { 0x0013, 1, "#pragma pack stack overflow" },
  { 0x0014, 1, "#pragma pack stack underflow" },
  { 0x0015, 1, "Invalid alignment value" },
  { 0x0016, 1, "Redefinition of macro '%s'" },                          // *
  { 0x0017, 1, "Unsupported type of macro '%s'" },                      // *
  { 0x0018, 1, "sizeof of incomplete type" },
  { 0x0019, 1, "Unknown 'extern' parameter '%s'" },                     // *
  { 0x001A, 1, "'long' parameter used without 386 or above #pragma" },
  
  // files and filenames
  { 0x0020, 1, "Too many include files (max = %i)" },                   // *
  { 0x0021, 1, "Invalid or too long file name or path name" },
  { 0x0022, 1, "Unable to open, read, write or close file \"%s\"" },    // *
//  { 0x0023, 1, "Invalid or unsupported command line option" },
//  { 0x0024, 1, "Input file not specified" },
    
  // Identifiers
  { 0x0100, 1, "Identifier too long: '%s'" },                           // *
  { 0x0101, 1, "Redefinition of label: '%s'"  },                        // *
  { 0x0102, 1, "Identifier expected"  },
  { 0x0103, 1, "Too long expression" },
  { 0x0104, 1, "Undeclared label: '%s'" },                              // *
  { 0x0105, 1, "Undeclared identifier: '%s'" },                         // *
  
  { 0x010F, 1, "unexpected token: '%s'" },                              // *
  { 0x0110, 1, "primary expression expected after token: '%s'" },       // *
  { 0x0111, 1, " ')' expected, unexpected token: '%s'" },               // *
  { 0x0112, 1, "primary expression expected after '(type)'" },
  { 0x0113, 1, "primary expression expected in '()'" },
  { 0x0114, 1, "primary expression (fxn argument) expected before ','" },
  { 0x0115, 1, "primary expression (fxn argument) expected between ',' and ')'" },
  { 0x0116, 1, "unexpected token: '%s'.  ',' or ')' expected" },       // *
  { 0x0117, 1, "primary expression expected in '[]'" },
  { 0x0118, 1, "unexpected token: '%s'. ']' expected" },               // *
  { 0x0119, 1, "Unsupported or invalid array dimension (token %s)" },  // *
  { 0x011A, 1, "missing array dimension" },
  { 0x011B, 1, " '}' expected, unexpected token: '%s'" },              // *
  { 0x011C, 1, "'...' unexpected here" },
  { 0x011D, 1, "'...' must be the last in the parameter list" },
  { 0x011E, 1, "missing return value" },
  { 0x011F, 1, "cannot return a value from a function returning 'void'" },
  { 0x0120, 1, "';' expected" },
  { 0x0121, 1, "'(' expected after 'while'" },
  { 0x0122, 1, "')' expected after 'while ( expression'" },
  { 0x0123, 1, "expression expected in 'while ( expression )'" },
  { 0x0124, 1, "unexpected 'void' expression in 'while ( expression )'" },
  { 0x0125, 1, "'while' expected after 'do statement'" },
  { 0x0126, 1, "'(' expected after 'while'" },
  { 0x0127, 1, "')' expected after 'while ( expression'" },
  { 0x0128, 1, "';' expected after 'do statement while ( expression )'" },
  { 0x0129, 1, "'(' expected after 'if'" },
  { 0x012A, 1, "')' expected after 'if ( expression'" },
  { 0x012B, 1, "expression expected in 'if ( expression )'" },
  { 0x012C, 1, "'(' expected after 'for'" },
  { 0x012D, 1, "';' expected after 'for ( expression'" },
  { 0x012E, 1, "';' expected after 'for ( expression ; expression'" },
  { 0x012F, 1, "')' expected after 'for ( expression ; expression ; expression'" },
  { 0x0130, 1, "'(' expected after 'switch'" },
  { 0x0131, 1, "')' expected after 'switch ( expression'" },
  { 0x0132, 1, "expression expected in 'switch ( expression )'" },
  { 0x0133, 1, "'{' expected after 'switch ( expression )'" },
  { 0x0134, 1, "':' expected after 'case expression'" },
  { 0x0135, 1, "':' expected after 'default'" },
  { 0x0136, 1, "only one 'default' allowed in 'switch'" },
  { 0x0137, 1, "'(' expected after '_asm'" },
  { 0x0138, 1, "string literal expression expected in '_asm ( expression )'" },
  { 0x0139, 1, "')' expected after '_asm ( expression'" },
  { 0x013A, 1, "Pointer to or structure or union expected" },
  { 0x013B, 1, "Undefined structure or union member '%s'" },            // *
  { 0x013C, 1, "'break' must be within statement" },
  { 0x013D, 1, "'continue' must be within statement" },
  { 0x013E, 1, "'case' must be within 'switch' statement" },
  { 0x013F, 1, "'default' must be within 'switch' statement" },
  { 0x0140, 1, "nesting too deep for '%s' statement" },                 // *
  { 0x0141, 1, "case table exhausted" },
  { 0x0142, 1, "':' expected after 'case expression'/'default expression'" },
  { 0x0143, 1, "constant integer expression expected in 'case expression'" },
  { 0x0144, 1, "duplicate case value found: %i" },                      // *
  { 0x0145, 1, "only one 'default' allowed in switch" },
  
  // declarations
  { 0x0160, 1, "Invalid or unsupported declaration" },
  { 0x0161, 1, "cannot define a nested function" },
  { 0x0162, 1, "cannot declare a static function in this scope" },
  { 0x0163, 1, "cannot define an array of incomplete type" },
  { 0x0164, 1, "Non-constant expression" },
  { 0x0165, 1, "non-constant array dimension" },
  { 0x0166, 1, "cannot initialize a global variable with a non-constant expression" },
  { 0x0167, 1, "constant integer expression expected in 'case expression :" },
  { 0x0168, 1, "Identifier expected in declaration" },
  { 0x0169, 1, "Identifier unexpected in declaration" },
  { 0x016A, 1, "illegal use of 'far' keyword" },
  { 0x016B, 1, "use of 'far' keyword on non-pointer is not allowed" },
  { 0x016C, 1, "Initialization of a CONST variable '%s'" },             // *
  { 0x016D, 1, "use of 'far' keyword on Enum ignored" },
  
  { 0x0170, 1, "Redefinition of type tagged '%s'" },                    // *
  { 0x0171, 1, "Redefinition of identifier '%s'" },                     // *
  { 0x0172, 1, "Variable(s) take(s) too much space" },
  { 0x0173, 1, "Local variables take too much space" },
  { 0x0174, 1, "Invalid or unsupported initialization" },
  { 0x0175, 1, "cannot initialize a function" },
  { 0x0176, 1, "cannot initialize an external variable" },
  { 0x0177, 1, "Array dimension less than 1" },
  { 0x0178, 1, "Too many array initializers" },

  // operand types
  { 0x0200, 1, "cannot do pointer arithmetic on a pointer to a function" },
  { 0x0201, 1, "cannot do arithmetic on a function" },
  { 0x0202, 1, "unexpected operand type for operator '%s', numeric type expected" }, // *
  { 0x0203, 1, "Unexpected operand type" },
  { 0x0204, 1, "pointer/array expected after '*' / before '[]'" },
  { 0x0205, 1, "incompatible pointers" },
  { 0x0206, 1, "invalid combination of operands for '+' or '-'" },
  { 0x0207, 1, "Invalid/unsupported combination of compared operands" },
  { 0x0208, 1, "function or function pointer expected" },
  { 0x0209, 1, "invalid combination of operands for %s" },              // *
  { 0x020A, 1, "function parameters cannot be of type 'void'" },
  { 0x020B, 1, "cannot return a value of type 'void'" },
  { 0x020C, 1, "unexpected 'void' expression in 'for ( ; expression ; )'" },
  { 0x020D, 1, "unexpected 'void' expression in 'switch ( expression )'" },
  { 0x020E, 1, "unexpected 'void' expression in 'if ( expression )'" },
  { 0x020F, 1, "unexpected 'void' expression in 'while ( expression )'" },
  { 0x0210, 1, "Unexpected declaration or expression of type void" },
  { 0x0211, 1, "cannot do pointer arithmetic on a pointer to 'void'" },
  { 0x0212, 1, "unexpected operand type 'void' for operator '%s'" },    // *
  { 0x0213, 1, "Cannot declare a variable ('%s') of type 'void'" },     // *
  { 0x0214, 1, "Invalid or unsupported type" },
  
  // expresions
  { 0x0280, 1, "lvalue expected after '&'" },
  { 0x0281, 1, "lvalue expected for '++' or '--'" },
  { 0x0282, 1, "lvalue expected before '='" },
  { 0x0283, 1, "lvalue expected before %s" },                           // *
  
  { 0x02A0, 1, "Too many function parameters" },
  { 0x02A1, 1, "Too few function parameters" },

  // source code text (comments, etc.)
  { 0x0300, 1, "Invalid comment" },
  
  { 0x0310, 1, "Too many #if(n)def's" },
  { 0x0311, 1, "#elif, #else, or #endif without #if/#if(n)def" },
  
  { 0x0330, 1, "Constant too big" },
  { 0x0331, 1, "Invalid hexadecimal constant" },
  { 0x0332, 1, "Constant too big for %i-bit type" },                    // *
  { 0x0332, 1, "Constant too big for %i-bit signed type" },             // *
  { 0x0333, 1, "Invalid or unsupported character with code 0x%02X" },   // *

  // strings
  { 0x0400, 1, "Invalid or unsupported character constant or string literal" },
  { 0x0401, 1, "String literal too long" },
  { 0x0402, 1, "Wide characters and strings not supported" },

  // standards
  { 0x1000, 1, "Declarations in for() statement found.  Use '-allow-c99' command line switch" },
  
  
  // Internal Errors
  { 0xE000, 1, "Internal Error: GetTokenName(): Invalid token %i" },    // *
  { 0xE001, 1, "Internal Error: IncludeFile(): #include parsing error" },
  { 0xE002, 1, "Internal Error: stacktop(): expression stack underflow!" },
  { 0xE003, 1, "Internal Error: opstacktop(): operator stack underflow!" },
  { 0xE004, 1, "Internal Error: exprval(): idx < 0" },
  { 0xE005, 1, "Internal Error: exprval(): ??? " },
  { 0xE006, 1, "Internal Error: exprval(): ??? " },
  { 0xE007, 1, "Internal Error: exprval(): ??? " },
  { 0xE008, 1, "Internal Error: GetDeclSize(): SyntaxPtr < 0" },
  { 0xE009, 1, "Internal Error: GetDeclSize(): ??? " },
  { 0xE00A, 1, "Internal Error: GetDeclSize(): ???  (token = %i)" },       // *
  { 0xE00B, 1, "Internal Error: GetDeclSize(): ??? " },
  { 0xE00C, 1, "Internal Error: GetDeclAlignment(): SyntaxPtr < 0" },
  { 0xE00D, 1, "Internal Error: GetDeclAlignment(): ??? " },
  { 0xE00E, 1, "Internal Error: GetDeclAlignment(): ??? " },
  { 0xE00F, 1, "Internal Error: GetDeclAlignment(): ??? " },
  { 0xE010, 1, "Internal error: AddFxnParamSymbols(): a: Invalid input" },
  { 0xE011, 1, "Internal error: AddFxnParamSymbols(): b: Invalid input" },
  { 0xE012, 1, "Internal error: AddFxnParamSymbols(): GetDeclSize() = 0" },
  { 0xE013, 1, "Internal error: AddFxnParamSymbols(): Unexpected token %s" }, // *
  { 0xE014, 1, "Internal Error: GenPrintInstr2Operands(): ??? " },
  { 0xE015, 1, "Internal Error: GenFuse(): idx < 0" },
  { 0xE016, 1, "Internal Error: GenFuse(): unexpected token %s" },      // *
  { 0xE017, 1, "Internal Error: GenGetBinaryOperatorInstr(): Invalid operator" },
  { 0xE018, 1, "Internal Error: GenExpr1() a: unexpected token %s" }, // *
  { 0xE019, 1, "Internal Error: GenExpr1() b: unexpected token %s" }, // *
  
  { 0xFFFE, 0, "#error: %s" },   // Compiler defined                  // *
  
  { 0xFFFF, 0, "" }  // last one
};

#endif  // NBC_ERROR_LIST_H
