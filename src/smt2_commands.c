/*
 * ALL SMT-LIB 2 COMMANDS
 */

#if defined(CYGWIN) || defined(MINGW)
#define EXPORTED __declspec(dllexport)
#define __YICES_DLLSPEC__ EXPORTED
#else
#define EXPORTED __attribute__((visibility("default")))
#endif


#include <stdio.h>
#include <inttypes.h>
#include <assert.h>

#include "attribute_values.h"
#include "smt_logic_codes.h"
#include "smt2_lexer.h"
#include "smt2_commands.h"

#include "yices.h"
#include "yices_exit_codes.h"
#include "yices_extensions.h"


/*
 * GLOBAL OBJECTS
 */
static bool done;    // set to true on exit
static attr_vtbl_t avtbl; // attribute values


// exported globals
smt2_globals_t __smt2_globals;


/*
 * ERROR REPORTS
 */

/*
 * bug report
 */
static void report_bug(FILE *f) {
  fprintf(f, "\n*************************************************************\n");
  fprintf(f, "FATAL ERROR\n\n");
  fprintf(f, "Please report this bug to yices-bugs@csl.sri.com.\n");
  fprintf(f, "To help us diagnose this problem, please include the\n"
                  "following information in your bug report:\n\n");
  fprintf(f, "  Yices version: %s\n", yices_version);
  fprintf(f, "  Build date: %s\n", yices_build_date);
  fprintf(f, "  Platform: %s (%s)\n", yices_build_arch, yices_build_mode);
  fprintf(f, "\n");
  fprintf(f, "Thank you for your help.\n");
  fprintf(f, "*************************************************************\n\n");
  fflush(f);

  exit(YICES_EXIT_INTERNAL_ERROR);
}


/*
 * Syntax error (reported by tstack)
 * - lex = lexer 
 * - expected_token = either an smt2_token or -1
 *
 * lex is as follows: 
 * - current_token(lex) = token that caused the error
 * - current_token_value(lex) = corresponding string
 * - current_token_length(lex) = length of that string
 * - lex->tk_line and lex->tk_column = start of the token in the input
 * - lex->reader.name  = name of the input file (NULL means input is stdin)
 */
static inline char *tkval(lexer_t *lex) {
  return current_token_value(lex);
}

void smt2_syntax_error(lexer_t *lex, int32_t expected_token) {
  reader_t *rd;
  smt2_token_t tk;

  tk = current_token(lex);
  rd = &lex->reader;

  fprintf(__smt2_globals.out, "(error on line %"PRId32", column %"PRId32": ", rd->line, rd->column);

  switch (tk) {
  case SMT2_TK_INVALID_STRING:
    fprintf(__smt2_globals.out, "missing string terminator");
    break;

  case SMT2_TK_INVALID_NUMERAL:
    fprintf(__smt2_globals.out, "invalid numeral %s", tkval(lex));
    break;

  case SMT2_TK_INVALID_DECIMAL:
    fprintf(__smt2_globals.out, "invalid decimal %s", tkval(lex));
    break;

  case SMT2_TK_INVALID_HEXADECIMAL:
    fprintf(__smt2_globals.out, "invalid hexadecimal constant %s", tkval(lex));
    break;

  case SMT2_TK_INVALID_BINARY:
    fprintf(__smt2_globals.out, "invalid binary constant %s", tkval(lex));
    break;

  case SMT2_TK_INVALID_SYMBOL:
    fprintf(__smt2_globals.out, "invalid symbol");
    break;

  case SMT2_TK_INVALID_KEYWORD:
    fprintf(__smt2_globals.out, "invalid keyword");
    break;

  case SMT2_TK_ERROR:
    fprintf(__smt2_globals.out, "invalid token %s", tkval(lex));
    break;
    
  default:
    if (expected_token >= 0) {
      fprintf(__smt2_globals.out, "syntax error: %s expected", smt2_token_to_string(expected_token));
    } else {
      fprintf(__smt2_globals.out, "syntax error");
    }
    break;
  }
  fprintf(__smt2_globals.out, ")\n" );
  fflush(__smt2_globals.out);
}


/*
 * ERROR FROM YICES (in yices_error_report)
 */

/*
 * If full is true: print (error <message>)
 * Otherwise: print <message>
 */
static void print_yices_error(bool full) {
  FILE *out;
  error_report_t *error;

  out = __smt2_globals.out;
  if (full) fputs("(error: ", out);

  error = yices_error_report();
  switch (error->code) {
  case INVALID_BITSHIFT:
    fputs("invalid index in rotate", out);
    break;
  case INVALID_BVEXTRACT:
    fputs("invalid indices in bit-vector extract", out);
    break;
  case TOO_MANY_ARGUMENTS:
    fprintf(out, "too many arguments. Function arity is at most %"PRIu32, YICES_MAX_ARITY);
    break;
  case TOO_MANY_VARS:
    fprintf(out, "too many variables in quantifier. Max is %"PRIu32, YICES_MAX_VARS);
    break;
  case MAX_BVSIZE_EXCEEDED:
    fprintf(out, "bit-vector size too large. Max is %"PRIu32, YICES_MAX_BVSIZE);
    break;
  case DEGREE_OVERFLOW:
    fputs("maximal polynomial degree exceeeded", out);
    break;
  case DIVISION_BY_ZERO:
    fputs("division by zero", out);    
    break;
  case POS_INT_REQUIRED:
    fputs("integer argument must be positive", out);
    break;
  case NONNEG_INT_REQUIRED:
    fputs("integer argument must be non-negative", out);
    break;
  case FUNCTION_REQUIRED:
    fputs("argument is not a function", out);
    break;
  case ARITHTERM_REQUIRED:
    fputs("argument is not an arithmetic term", out);
    break;
  case BITVECTOR_REQUIRED:
    fputs("argument is not a bit-vector term", out);
    break;
  case WRONG_NUMBER_OF_ARGUMENTS:
    fputs("wrong number of arguments", out);
    break;
  case TYPE_MISMATCH:
    fputs("type error: invalid arguments", out);
    break;
  case INCOMPATIBLE_TYPES:
    fputs("incomaptible types", out);
    break;
  case INCOMPATIBLE_BVSIZES:
    fputs("arguments do not have the same number of bits", out);
    break;
  case EMPTY_BITVECTOR:
    fputs("bit-vectors can't have 0 bits", out);
    break;
  case ARITHCONSTANT_REQUIRED:
    fputs("argument is not an arithmetic constant", out);
    break;
  case TOO_MANY_MACRO_PARAMS:
    fprintf(out, "too many arguments in sort constructor. Max is %"PRIu32, TYPE_MACRO_MAX_ARITY);
    break;

  case CTX_FREE_VAR_IN_FORMULA:
  case CTX_LOGIC_NOT_SUPPORTED:
  case CTX_UF_NOT_SUPPORTED:
  case CTX_ARITH_NOT_SUPPORTED:
  case CTX_BV_NOT_SUPPORTED:
  case CTX_ARRAYS_NOT_SUPPORTED:
  case CTX_QUANTIFIERS_NOT_SUPPORTED:
  case CTX_NONLINEAR_ARITH_NOT_SUPPORTED:
  case CTX_FORMULA_NOT_IDL:
  case CTX_FORMULA_NOT_RDL:
  case CTX_TOO_MANY_ARITH_VARS:
  case CTX_TOO_MANY_ARITH_ATOMS:
  case CTX_TOO_MANY_BV_VARS:
  case CTX_TOO_MANY_BV_ATOMS:
  case CTX_ARITH_SOLVER_EXCEPTION:
  case CTX_BV_SOLVER_EXCEPTION:
  case CTX_ARRAY_SOLVER_EXCEPTION:
  case CTX_OPERATION_NOT_SUPPORTED:
  case CTX_INVALID_CONFIG:
  case CTX_UNKNOWN_PARAMETER:
  case CTX_INVALID_PARAMETER_VALUE:
  case CTX_UNKNOWN_LOGIC:
    fputs("context exception", out); // expand
    break;

  case EVAL_UNKNOWN_TERM:
  case EVAL_FREEVAR_IN_TERM:
  case EVAL_QUANTIFIER:
  case EVAL_LAMBDA:
  case EVAL_OVERFLOW:
  case EVAL_FAILED:
    fputs("can't evaluate term value", out); // expand
    break;

  case OUTPUT_ERROR:
    fputs(" IO error", out);
    break;

  default:
    fputs("BUG detected", out);
    if (full) fputc(')', out);
    fflush(out);
    report_bug(__smt2_globals.err);
    break;
  }

  if (full) fputs(")\n", out);
  fflush(out);  
}


/*
 * EXCEPTIONS
 */

/*
 * Error messages for tstack exceptions
 * NULL means that this should never occur (i.e., fatal exception)
 */
static const char * const exception_string[NUM_SMT2_EXCEPTIONS] = {
  NULL,                                 // TSTACK_NO_ERROR
  NULL,                                 // TSTACK_INTERNAL_ERROR
  "operation not implemented",          // TSTACK_OP_NOT_IMPLEMENTED
  "undefinedd term",                    // TSTACK_UNDEF_TERM
  "undefined sort",                     // TSTACK_UNDEF_TYPE
  "undefined sort constructor",         // TSTACK_UNDEF_MACRO,
  "invalid numeral",                    // TSTACK_RATIONAL_FORMAT
  "invalid decimal'",                   // TSTACK_FLOAT_FORMAT
  "invalid binary",                     // TSTACK_BVBIN_FORMAT
  "invalid hexadecimal",                // TSTACK_BVHEX_FORMAT
  "can't redefine sort",                // TSTACK_TYPENAME_REDEF
  "can't redefine term",                // TSTACK_TERMNAME_REDEF
  "can't redefine sort constructor",    // TSTACK_MACRO_REDEF
  NULL,                                 // TSTACK_DUPLICATE_SCALAR_NAME
  "duplicate variable name",            // TSTACK_DUPLICATE_VAR_NAME
  "duplicate variable name",            // TSTACK_DUPLICATE_TYPE_VAR_NAME
  NULL,                                 // TSTACK_INVALID_OP
  "wrong number of arguments",          // TSTACK_INVALID_FRAME
  "constant too large",                 // TSTACK_INTEGER_OVERFLOW
  NULL,                                 // TSTACK_NEGATIVE_EXPONENT
  "integer required",                   // TSTACK_NOT_AN_INTEGER
  "string required",                    // TSTACK_NOT_A_STRING
  "symbol required",                    // TSTACK_NOT_A_SYMBOL 
  "numeral required",                   // TSTACK_NOT_A_RATIONAL
  "sort required",                      // TSTACK_NOT_A_TYPE
  "error in arithmetic operation",      // TSTACK_ARITH_ERROR
  "division by zero",                   // TSTACK_DIVIDE_BY_ZERO
  "divisor must be constant",           // TSTACK_NON_CONSTANT_DIVISOR
  "size must be positive",              // TSTACK_NONPOSITIVE_BVSIZE
  "bitvectors have incompatible sizes", // TSTACK_INCOMPATIBLE_BVSIZES
  "number can't be converted to a bitvector constant", // TSTACK_INVALID_BVCONSTANT
  "error in bitvector arithmetic operation",  //TSTACK_BVARITH_ERROR
  "error in bitvector operation",       // TSTACK_BVLOGIC_ERROR
  "incompatible sort in definition",    // TSTACK_TYPE_ERROR_IN_DEFTERM
  NULL,                                 // TSTACK_YICES_ERROR
  "missing symbol in :named attribute", // SMT2_MISSING_NAME
  "no pattern given",                   // SMT2_MISSING_PATTERN
  "not a sort identifier",              // SMT2_SYMBOL_NOT_SORT
  "not an indexed sort identifier",     // SMT2_SYMBOL_NOT_IDX_SORT
  "not a sort constructor",             // SMT2_SYMBOL_NOT_SORT_OP
  "not an indexed sort constructor",    // SMT2_SYMBOL_NOT_IDX_SORT_OP
  "not a term identifier",              // SMT2_SYMBOL_NOT_TERM
  "not an indexed term identifier",     // SMT2_SYMBOL_NOT_IDX_TERM
  "not a function identifier",          // SMT2_SYMBOL_NOT_FUNCTION
  "not an indexed function identifier", // SMT2_SYMBOL_NOT_IDX_FUNCTION
  "undefined identifier",               // SMT2_UNDEF_IDX_SORT
  "undefined identifier",               // SMT2_UNDEF_IDX_SORT_OP
  "undefined identifier",               // SMT2_UNDEF_IDX_TERM
  "undefined identifier",               // SMT2_UNDEF_IDX_FUNCTION
  "invalid bitvector constant",         // SMT2_INVALID_IDX_BV
};


/*
 * Conversion of opcodes to strings
 */
static const char * const opcode_string[NUM_SMT2_OPCODES] = {
  NULL,                   // NO_OP
  "sort definition",      // DEFINE_TYPE (should not occur?)
  "term definition",      // DEFINE_TERM (should not occur?)
  "binding",              // BIND?
  "variable declaration", // DECLARE_VAR
  "sort-variable declaration", // DECLARE_TYPE_VAR
  "let",                  // LET
  "BitVec",               // MK_BV_TYPE
  NULL,                   // MK_SCALAR_TYPE
  NULL,                   // MK_TUPLE_TYPE
  "function type",        // MK_FUN_TYPE
  "type constructor",     // MK_APP_TYPE
  "function application", // MK_APPLY
  "ite",                  // MK_ITE
  "equality",             // MK_EQ
  "disequality",          // MK_DISEQ
  "distinct",             // MK_DISTINCT
  "not",                  // MK_NOT
  "or",                   // MK_OR
  "and",                  // MK_AND
  "xor",                  // MK_XOR
  "iff",                  // MK_IFF (not in SMT2?)
  "=>",                   // MK_IMPLIES
  NULL,                   // MK_TUPLE
  NULL,                   // MK_SELECT
  NULL,                   // MK_TUPLE_UPDATE
  NULL,                   // MK_UPDATE (replaced by SMT2_MK_STORE)
  "forall",               // MK_FORALL
  "exists",               // MK_EXISTS
  "lambda",               // MK_LAMBDA (not in SMT2)
  "addition",             // MK_ADD
  "subtraction",          // MK_SUB
  "negation",             // MK_NEG
  "multiplication",       // MK_MUL
  "division",             // MK_DIVISION
  "expponetiation",       // MK_POW
  "inequality",           // MK_GE
  "inequality",           // MK_GT
  "inequality",           // MK_LE
  "inequality",           // MK_LT
  "bitvector constant",   // MK_BV_CONST
  "bvadd",                // MK_BV_ADD
  "bvsub",                // MK_BV_SUB
  "bvmul",                // MK_BV_MUL
  "bvneg",                // MK_BV_NEG
  "bvpow",                // MK_BV_POW (not in SMT2)
  "bvudiv",               // MK_BV_DIV
  "bvurem",               // MK_BV_REM
  "bvsdiv",               // MK_BV_SDIV
  "bvsrme",               // MK_BV_SREM
  "bvsmod",               // MK_BV_SMOD
  "bvnot",                // MK_BV_NOT
  "bvand",                // MK_BV_AND
  "bvor",                 // MK_BV_OR
  "bvxor",                // MK_BV_XOR
  "bvnand",               // MK_BV_NAND
  "bvnor",                // MK_BV_NOR
  "bvxnor",               // MK_BV_XNOR
  NULL,                   // MK_BV_SHIFT_LEFT0
  NULL,                   // MK_BV_SHIFT_LEFT1
  NULL,                   // MK_BV_SHIFT_RIGHT0
  NULL,                   // MK_BV_SHIFT_RIGHT1
  NULL,                   // MK_BV_ASHIFT_RIGHT
  "rotate_left",          // MK_BV_ROTATE_LEFT
  "rotate_right",         // MK_BV_ROTATE_RIGHT
  "bvshl",                // MK_BV_SHL
  "bvlshr",               // MK_BV_LSHR
  "bvashr",               // MK_BV_ASHR
  "extract",              // MK_BV_EXTRACT
  "concat",               // MK_BV_CONCAT
  "repeat",               // MK_BV_REPEAT
  "sign_extenrd",         // MK_BV_SIGN_EXTEND
  "zero_extend",          // MK_BV_ZERO_EXTEND
  "bvredand",             // MK_BV_REDAND (not in SMT2)
  "bvredor",              // MK_BV_REDOR (not in SMT2)
  "bvomp",                // MK_BV_COMP
  "bvuge",                // MK_BV_GE,
  "bvugt",                // MK_BV_GT
  "bvule",                // MK_BV_LE
  "bvult",                // MK_BV_LT
  "bvsge",                // MK_BV_SGE
  "bvsgt",                // MK_BV_SGT
  "bvsle",                // MK_BV_SLE
  "bvslt",                // MK_BV_SLT
  "build term",           // BUILD_TERM
  "build_type",           // BUILD_TYPE
  // 
  "exit",                 // SMT2_EXIT
  "get_assertions",       // SMT2_GET_ASSERTIONS
  "get_assignment",       // SMT2_GET_ASSIGNMENT
  "get_proof",            // SMT2_GET_PROOF
  "get_unsat_CORE",       // SMT2_GET_UNSAT_CORE
  "get_value",            // SMT2_GET_VALUE
  "get_option",           // SMT2_GET_OPTION
  "get_info",             // SMT2_GET_INFO
  "set_option",           // SMT2_SET_OPTION
  "set_info",             // SMT2_SET_INFO
  "set_logic",            // SMT2_SET_LOGIC
  "push",                 // SMT2_PUSH
  "pop",                  // SMT2_POP
  "assert",               // SMT2_ASSERT,
  "check_sat",            // SMT2_CHECK_SAT,
  "declare_sort",         // SMT2_DECLARE_SORT
  "define_sort",          // SMT2_DEFINE_SORT
  "declare_fun",          // SMT2_DECLARE_FUN
  "define_fun",           // SMT2_DEFINE_FUN
  "attributes",           // SMT2_MAKE_ATTR_LIST
  "term annotation",      // SMT2_ADD_ATTRIBUTES
  "Array",                // SMT2_MK_ARRAY
  "select",               // SMT2_MK_SELECT
  "store",                // SMT2_MK_STORE
  "indexed_sort",         // SMT2_INDEXED_SORT
  "sort expression",      // SMT2_APP_INDEXED_SORT
  "indexed identifier",   // SMT2_INDEXED_TERM
  "as",                   // SMT2_SORTED_TERM
  "as",                   // SMT2_SORTED_INDEXED_TERM
  "function application", // SMT2_INDEXED_APPLY
  "function application", // SMT2_SORTED_APPLY
  "function application", // SMT2_SORTED_INDEXED_APPLY
};


/*
 * Exception raised by tstack
 * - tstack = term stack
 * - exception = error code (defined in term_stack2.h)
 *
 * Error location in the input file is given by 
 * - tstack->error_loc.line
 * - tstack->error_loc.column
 *
 * Extra fields (depending on the exception)
 * - tstack->error_string = erroneous input
 * - tstack->error_op = erroneous operation
 */
void smt2_tstack_error(tstack_t *tstack, int32_t exception) {
  FILE *out;

  out = __smt2_globals.out;

  fprintf(out, "(error at line %"PRId32", column %"PRId32": ", 
	  tstack->error_loc.line, tstack->error_loc.column);

  switch (exception) {
  case TSTACK_OP_NOT_IMPLEMENTED:
    fprintf(out, "operation %s not implemented)\n", opcode_string[tstack->error_op]);
    break;
    
  case TSTACK_UNDEF_TERM:
  case TSTACK_UNDEF_TYPE:
  case TSTACK_UNDEF_MACRO:
  case TSTACK_DUPLICATE_VAR_NAME:
  case TSTACK_DUPLICATE_TYPE_VAR_NAME:
  case TSTACK_RATIONAL_FORMAT:
  case TSTACK_FLOAT_FORMAT:
  case TSTACK_BVBIN_FORMAT:
  case TSTACK_BVHEX_FORMAT:
  case TSTACK_TYPENAME_REDEF:
  case TSTACK_TERMNAME_REDEF:
  case TSTACK_MACRO_REDEF:
  case SMT2_SYMBOL_NOT_SORT:
  case SMT2_SYMBOL_NOT_IDX_SORT:
  case SMT2_SYMBOL_NOT_SORT_OP:
  case SMT2_SYMBOL_NOT_IDX_SORT_OP:
  case SMT2_SYMBOL_NOT_TERM: 
  case SMT2_SYMBOL_NOT_IDX_TERM:
  case SMT2_SYMBOL_NOT_FUNCTION: 
  case SMT2_SYMBOL_NOT_IDX_FUNCTION:
  case SMT2_UNDEF_IDX_SORT:
  case SMT2_UNDEF_IDX_SORT_OP:
  case SMT2_UNDEF_IDX_TERM:
  case SMT2_UNDEF_IDX_FUNCTION:
    fprintf(out, "%s: %s)\n", exception_string[exception], tstack->error_string);
    break;

  case TSTACK_INVALID_FRAME:
  case TSTACK_NONPOSITIVE_BVSIZE:
    fprintf(out, "%s in %s)\n", exception_string[exception], opcode_string[tstack->error_op]);
    break;

  case TSTACK_INTEGER_OVERFLOW:
  case TSTACK_NOT_AN_INTEGER:
  case TSTACK_NOT_A_STRING:
  case TSTACK_NOT_A_SYMBOL:
  case TSTACK_NOT_A_RATIONAL:
  case TSTACK_NOT_A_TYPE:
  case TSTACK_ARITH_ERROR:
  case TSTACK_DIVIDE_BY_ZERO:
  case TSTACK_NON_CONSTANT_DIVISOR:
  case TSTACK_INCOMPATIBLE_BVSIZES:
  case TSTACK_INVALID_BVCONSTANT:
  case TSTACK_BVARITH_ERROR:
  case TSTACK_BVLOGIC_ERROR:
  case TSTACK_TYPE_ERROR_IN_DEFTERM:
  case SMT2_MISSING_NAME:
  case SMT2_MISSING_PATTERN:
    fprintf(out, "%s)\n", exception_string[exception]);
    break;

  case TSTACK_YICES_ERROR:
    // TODO: extract mode information from yices_error_report();
    fprintf(out, "in %s: ", opcode_string[tstack->error_op]);
    print_yices_error(false);
    fprintf(out, ")\n");
    break;

  case SMT2_INVALID_IDX_BV:
    fprintf(out, "%s)\n", exception_string[exception]);
    break;

  case TSTACK_NO_ERROR:
  case TSTACK_INTERNAL_ERROR:
  case TSTACK_DUPLICATE_SCALAR_NAME:
  case TSTACK_INVALID_OP:
  case TSTACK_NEGATIVE_EXPONENT:
  default:
    fprintf(out, ")\n");
    fflush(out);
    report_bug(__smt2_globals.err);
    break;
  }

  fflush(out);
}




/*
 * Print
 */
static void report_success(void) {
  if (__smt2_globals.print_success) {
    fprintf(__smt2_globals.out, "success\n");
    fflush(__smt2_globals.out);
  }
}


/*
 * MAIN CONTROL FUNCTIONS
 */

/*
 * Initialize g to defaults
 */
static void init_smt2_globals(smt2_globals_t *g) {
  g->logic_code = SMT_UNKNOWN;
  g->benchmark = false;
  g->out = stdout;
  g->err = stderr;
  g->print_success = true;
  g->expand_definitions = false;
  g->interactive_mode = false;
  g->produce_proofs = false;
  g->produce_unsat_core = false;
  g->produce_models = false;
  g->produce_assignments = false;
  g->random_seed = 0;
  g->verbosity = 0;
  g->avtbl = NULL;
  g->ctx = NULL;
  g->model = NULL;
}

/*
 * Initialize all internal structures
 * - benchmark: if true, the input is assumed to be an SMT-LIB 2.0 benchmark
 *  (i.e., a set of assertions followed by a single call to check-sat)
 *  In this mode, destructive simplifications are allowed.
 * - this is called after yices_init so all Yices internals are ready
 */
void init_smt2(bool benchmark) {
  done = false;
  init_smt2_globals(&__smt2_globals);
  init_attr_vtbl(&avtbl);
  __smt2_globals.benchmark = benchmark;
  __smt2_globals.avtbl = &avtbl;
}


/*
 * Delete all structures (close files too)
 */
void delete_smt2(void) {
  if (__smt2_globals.out != stdout) {
    fclose(__smt2_globals.out);
  }
  if (__smt2_globals.err != stderr) {
    fclose(__smt2_globals.err);
  }
  delete_attr_vtbl(&avtbl);
}


/*
 * Check whether the smt2 solver is ready
 * - this must be true after init_smt2()
 * - this must return false if smt2_exit has been called or after
 *   an unrecoverable error
 */
bool smt2_active(void) {
  return !done;
}


/*
 * TOP-LEVEL SMT2 COMMANDS
 */

/*
 * Exit function (also called on end-of-file)
 */
void smt2_exit(void) {
  done = true;
  report_success();
}


/*
 * Show all formulas asserted so far
 */
void smt2_get_assertions(void) {
  fprintf(__smt2_globals.out, "get_assertions: unsupported\n");
  fflush(__smt2_globals.out);
}


/*
 * Show the truth value of named Boolean terms 
 * (i.e., those that have a :named attribute)
 */
void smt2_get_assignment(void) {
  fprintf(__smt2_globals.out, "get_assignment: unsupported\n");  
  fflush(__smt2_globals.out);
}


/*
 * Show a proof when context is unsat
 */
void smt2_get_proof(void) {
  fprintf(__smt2_globals.out, "get_proof: unsupported\n");
  fflush(__smt2_globals.out);  
}


/*
 * Get the unsat core: subset of :named assertions that form an unsat core
 */
void smt2_get_unsat_core(void) {
  fprintf(__smt2_globals.out, "get_unsat_core: unsupported\n");  
  fflush(__smt2_globals.out);  
}


/*
 * Get the values of terms in the model
 * - the terms are listed in array a
 * - n = number of elements in the array
 */
void smt2_get_value(term_t *a, uint32_t n) {
  fprintf(__smt2_globals.out, "get_value: unsupported\n");  
  fflush(__smt2_globals.out);  
}


/*
 * Get the value of an option
 * - name = option name (a keyword)
 */
void smt2_get_option(const char *name) {
  fprintf(__smt2_globals.out, "get_option: unsupported\n");
  fflush(__smt2_globals.out);  
}


/*
 * Get some info
 * - name = keyword
 */
void smt2_get_info(const char *name) {
  fprintf(__smt2_globals.out, "get_info: unsupported\n");
  fflush(__smt2_globals.out);  
}


/*
 * Set an option:
 * - name = option name (keyword)
 * - value = value (stored in the attribute_value table)
 *
 * SMT2 allows the syntax (set-option :keyword). In such a case,
 * this function is called with value = NULL_VALUE (i.e., -1).
 */
void smt2_set_option(const char *name, aval_t value) {
  fprintf(__smt2_globals.out, "set_option: unsupported\n");
  fflush(__smt2_globals.out);  
}


/*
 * Set some info field
 * - same conventions as set_option
 */
void smt2_set_info(const char *name, aval_t value) {
  fprintf(__smt2_globals.out, "set_info: unsupported\n");
  fflush(__smt2_globals.out);  
}


/*
 * Set the logic:
 * - name = logic name (using the SMT-LIB conventions)
 */
void smt2_set_logic(const char *name) {
  smt_logic_t code;

  code = smt_logic_code(name);
  if (code != SMT_UNKNOWN) {
    smt2_lexer_activate_logic(code);
    __smt2_globals.logic_code = code;
    report_success();
  } else {
    fprintf(__smt2_globals.out, "(error: unknown logic %s)\n", name);
    fflush(__smt2_globals.out);
  }
}


/*
 * Push 
 * - n = number of scopes to push
 * - if n = 0, nothing should be done
 */
void smt2_push(uint32_t n) {
  fprintf(__smt2_globals.out, "push: unsupported\n");
  fflush(__smt2_globals.out);  
}


/*
 * Pop:
 * - n = number of scopes to remove
 * - if n = 0 nothing should be done
 * - if n > total number of scopes then an error should be printed
 *   and nothing done
 */
void smt2_pop(uint32_t n) {
  fprintf(__smt2_globals.out, "pop: unsupported\n");
  fflush(__smt2_globals.out);
}


/*
 * Assert one formula t
 * - if t is a :named assertion then it should be recorded for unsat-core
 */
void smt2_assert(term_t t) {
  fprintf(__smt2_globals.out, "assert: unsupported\n");
  fflush(__smt2_globals.out);
}


/*
 * Check satsifiability of the current set of assertions
 */
void smt2_check_sat(void) {
  fprintf(__smt2_globals.out, "check_sat: unsupported\n");
  fflush(__smt2_globals.out);
}


/*
 * Declare a new sort:
 * - name = sort name
 * - arity = arity
 *
 * If arity is 0, this defines a new uninterpreted type.
 * Otherwise, this defines a new type constructor.
 */
void smt2_declare_sort(const char *name, uint32_t arity) {
  type_t tau;
  int32_t macro;

  if (arity == 0) {
    tau = yices_new_uninterpreted_type();
    yices_set_type_name(tau, name);
    report_success();
  } else {
    macro = yices_type_constructor(name, arity);
    if (macro < 0) {
      print_yices_error(true);
    } else {
      report_success();
    }
  }

}


/*
 * Define a new type macro
 * - name = macro name
 * - n = number of variables
 * - var = array of type variables
 * - body = type expressions
 */
void smt2_define_sort(const char *name, uint32_t n, type_t *var, type_t body) {
  int32_t macro;

  macro = yices_type_macro(name, n, var, body);
  if (macro < 0) {
    print_yices_error(true);
  } else {
    report_success();
  }
}


/*
 * Declare a new uninterpreted function symbol
 * - name = function name
 * - n = arity + 1
 * - tau = array of n types
 *
 * If n = 1, this creates an uninterpreted constant of type tau[0]
 * Otherwise, this creates an uninterpreted function of type tau[0] x ... x tau[n-1] to tau[n] 
 */
void smt2_declare_fun(const char *name, uint32_t n, type_t *tau) {
  term_t t;
  type_t sigma;
  
  assert(n > 0);
  n --;
  sigma = tau[n]; // range
  if (n > 0) {
    sigma = yices_function_type(n, tau, sigma);
  }
  assert(sigma != NULL_TYPE);

  t = yices_new_uninterpreted_term(sigma);
  assert(t != NULL_TERM);
  yices_set_term_name(t, name);

  report_success();
}


/*
 * Define a function
 * - name = function name
 * - n = arity
 * - var = array of n term variables
 * - body = term
 * - tau = expected type of body
 *
 * If n = 0, this is the same as (define <name> :: <type> <body> )
 * Otherwise, a lambda term is created.
 */
void smt2_define_fun(const char *name, uint32_t n, term_t *var, term_t body, type_t tau) {
  term_t t;

  if (! yices_check_term_type(body, tau)) {
    print_yices_error(true);
    return;
  } 

  t = body;
  if (n > 0) {
    t = yices_lambda(n, var, t);
    if (t < 0) {
      print_yices_error(true);
      return;
    }
  }
  yices_set_term_name(t, name);

  report_success();
}



/*
 * ATTRIBUTES
 */

/*
 * Add a :named attribute to term t
 */
void smt2_add_name(term_t t, const char *name) {
  // TBD
}


/*
 * Add a :pattern attribute to term t
 * - the pattern is a term p
 */
void smt2_add_pattern(term_t t, term_t p) {
  // TBD
}
