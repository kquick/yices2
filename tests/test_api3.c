/*
 * TEST BOOLEAN AND BIT-VECTOR API FUNCTIONS
 */

/*
 * Force assert to work even if compiled with debug disabled
 */
#ifdef NDEBUG
# undef NDEBUG
#endif


#include <assert.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <gmp.h>

#include "int_vectors.h"
#include "memalloc.h"
#include "bitvectors.h"

#include "rationals.h"
#include "bv64_constants.h"
#include "bv_constants.h"

#include "yices.h"
#include "yices_globals.h"
#include "type_printer.h"
#include "term_printer.h"

#include "stores.h"


#ifdef MINGW
static inline long int random(void) {
  return rand();
}
#endif

/* dials for the numbers */
#if 0
#define BVNUM 200
#define ITENUM 5000
#define BINUM 6000
#else
#define BVNUM 200
#define ITENUM 200
#define BINUM 200
#endif

/*
 * GLOBAL STORE:
 * - a store for all the types and another one for the terms
 */
static type_store_t all_types;
static term_store_t all_terms;


/*
 * Initialize both
 */
static void init_store(void) {
  init_type_store(&all_types);
  init_term_store(&all_terms);
}


/*
 * Delete both
 */
static void delete_store(void) {
  delete_type_store(&all_types);
  delete_term_store(&all_terms);
}


/*
 * Add term t to both
 * - do nothing if t is already present
 */
static void add_term(term_t t) {
  if (! term_store_contains_term(&all_terms, t)) {
    term_store_add_term(&all_terms, t);
    type_store_add_term(&all_types, t);
  }
}




/*
 * BASE TYPES
 */
static type_t boolean, bv1, bv2, bv12, bv32, bv64, bv65, bv100;

static void init_base_types(void) {
  boolean = yices_bool_type();
  bv1 = yices_bv_type(1);
  bv2 = yices_bv_type(2);
  bv12 = yices_bv_type(12);
  bv32 = yices_bv_type(32);
  bv64 = yices_bv_type(64);
  bv65 = yices_bv_type(65);
  bv100 = yices_bv_type(100);
}


/*
 * BASE TERMS
 */

/*
 * Add constants and uninterpreted terms of base types
 */
static void init_base_terms(void) {
  uint32_t x;
  term_t t;

  // boolean terms
  add_term(yices_true());
  add_term(yices_false());
  t = yices_new_uninterpreted_term(boolean);
  yices_set_term_name(t, "p0");
  add_term(t);
  add_term(yices_not(t));
  t = yices_new_uninterpreted_term(boolean);
  yices_set_term_name(t, "p1");
  add_term(t);
  add_term(yices_not(t));
  t = yices_new_uninterpreted_term(boolean);
  yices_set_term_name(t, "p2");
  add_term(t);
  add_term(yices_not(t));
  t = yices_new_uninterpreted_term(boolean);
  yices_set_term_name(t, "p3");
  add_term(t);
  add_term(yices_not(t));
  t = yices_new_uninterpreted_term(boolean);
  yices_set_term_name(t, "p4");
  add_term(t);
  add_term(yices_not(t));


  // bitvector constants
  add_term(yices_bvconst_zero(1));
  add_term(yices_bvconst_one(1));

  add_term(yices_bvconst_zero(2));
  add_term(yices_bvconst_one(2));
  add_term(yices_bvconst_uint32(2, 2));
  add_term(yices_bvconst_minus_one(2));

  add_term(yices_bvconst_zero(12));
  add_term(yices_bvconst_zero(32));
  add_term(yices_bvconst_zero(64));
  add_term(yices_bvconst_zero(65));
  add_term(yices_bvconst_zero(100));

  add_term(yices_bvconst_one(12));
  add_term(yices_bvconst_one(32));
  add_term(yices_bvconst_one(64));
  add_term(yices_bvconst_one(65));
  add_term(yices_bvconst_one(100));

  add_term(yices_bvconst_minus_one(12));
  add_term(yices_bvconst_minus_one(32));
  add_term(yices_bvconst_minus_one(64));
  add_term(yices_bvconst_minus_one(65));
  add_term(yices_bvconst_minus_one(100));

  // more random constants
  x = (uint32_t) random();
  add_term(yices_bvconst_uint32(12, x));
  add_term(yices_bvconst_uint32(32, x));
  add_term(yices_bvconst_uint32(64, x));
  add_term(yices_bvconst_uint32(65, x));
  add_term(yices_bvconst_uint32(100, x));

  x = (uint32_t) random();
  add_term(yices_bvconst_uint32(12, x));
  add_term(yices_bvconst_uint32(32, x));
  add_term(yices_bvconst_uint32(64, x));
  add_term(yices_bvconst_uint32(65, x));
  add_term(yices_bvconst_uint32(100, x));

  x = (uint32_t) random();
  add_term(yices_bvconst_uint32(12, x));
  add_term(yices_bvconst_uint32(32, x));
  add_term(yices_bvconst_uint32(64, x));
  add_term(yices_bvconst_uint32(65, x));
  add_term(yices_bvconst_uint32(100, x));

  // uninterpreted bitvectors
  t = yices_new_uninterpreted_term(bv1);
  yices_set_term_name(t, "a0");
  add_term(t);
  t = yices_new_uninterpreted_term(bv1);
  yices_set_term_name(t, "a1");
  add_term(t);
  t = yices_new_uninterpreted_term(bv1);
  yices_set_term_name(t, "a2");
  add_term(t);
  t = yices_new_uninterpreted_term(bv1);
  yices_set_term_name(t, "a3");
  add_term(t);
  t = yices_new_uninterpreted_term(bv1);
  yices_set_term_name(t, "a4");
  add_term(t);

  t = yices_new_uninterpreted_term(bv2);
  yices_set_term_name(t, "b0");
  add_term(t);
  t = yices_new_uninterpreted_term(bv2);
  yices_set_term_name(t, "b1");
  add_term(t);
  t = yices_new_uninterpreted_term(bv2);
  yices_set_term_name(t, "b2");
  add_term(t);
  t = yices_new_uninterpreted_term(bv2);
  yices_set_term_name(t, "b3");
  add_term(t);
  t = yices_new_uninterpreted_term(bv2);
  yices_set_term_name(t, "b4");
  add_term(t);

  t = yices_new_uninterpreted_term(bv12);
  yices_set_term_name(t, "c0");
  add_term(t);
  t = yices_new_uninterpreted_term(bv12);
  yices_set_term_name(t, "c1");
  add_term(t);
  t = yices_new_uninterpreted_term(bv12);
  yices_set_term_name(t, "c2");
  add_term(t);
  t = yices_new_uninterpreted_term(bv12);
  yices_set_term_name(t, "c3");
  add_term(t);
  t = yices_new_uninterpreted_term(bv12);
  yices_set_term_name(t, "c4");
  add_term(t);

  t = yices_new_uninterpreted_term(bv32);
  yices_set_term_name(t, "d0");
  add_term(t);
  t = yices_new_uninterpreted_term(bv32);
  yices_set_term_name(t, "d1");
  add_term(t);
  t = yices_new_uninterpreted_term(bv32);
  yices_set_term_name(t, "d2");
  add_term(t);
  t = yices_new_uninterpreted_term(bv32);
  yices_set_term_name(t, "d3");
  add_term(t);
  t = yices_new_uninterpreted_term(bv32);
  yices_set_term_name(t, "d4");
  add_term(t);

  t = yices_new_uninterpreted_term(bv64);
  yices_set_term_name(t, "e0");
  add_term(t);
  t = yices_new_uninterpreted_term(bv64);
  yices_set_term_name(t, "e1");
  add_term(t);
  t = yices_new_uninterpreted_term(bv64);
  yices_set_term_name(t, "e2");
  add_term(t);
  t = yices_new_uninterpreted_term(bv64);
  yices_set_term_name(t, "e3");
  add_term(t);
  t = yices_new_uninterpreted_term(bv64);
  yices_set_term_name(t, "e4");
  add_term(t);

  t = yices_new_uninterpreted_term(bv65);
  yices_set_term_name(t, "f0");
  add_term(t);
  t = yices_new_uninterpreted_term(bv65);
  yices_set_term_name(t, "f1");
  add_term(t);
  t = yices_new_uninterpreted_term(bv65);
  yices_set_term_name(t, "f2");
  add_term(t);
  t = yices_new_uninterpreted_term(bv65);
  yices_set_term_name(t, "f3");
  add_term(t);
  t = yices_new_uninterpreted_term(bv65);
  yices_set_term_name(t, "f4");
  add_term(t);

  t = yices_new_uninterpreted_term(bv100);
  yices_set_term_name(t, "g0");
  add_term(t);
  t = yices_new_uninterpreted_term(bv100);
  yices_set_term_name(t, "g1");
  add_term(t);
  t = yices_new_uninterpreted_term(bv100);
  yices_set_term_name(t, "g2");
  add_term(t);
  t = yices_new_uninterpreted_term(bv100);
  yices_set_term_name(t, "g3");
  add_term(t);
  t = yices_new_uninterpreted_term(bv100);
  yices_set_term_name(t, "g4");
  add_term(t);

}



/*
 * FUNCTIONS TO TEST
 */

/*
 * Binary functions that require two bitvector of the same size
 * - for each function, we store: name + function pointer in a global
 *   array
 */
typedef struct bv_binop_s {
  char *name;
  term_t (*fun)(term_t, term_t);
} bv_binop_t;

#define NUM_BINOPS 18

static bv_binop_t binop_array[NUM_BINOPS] = {
  { "bvadd", yices_bvadd },
  { "bvsub", yices_bvsub },
  { "bvmul", yices_bvmul },
  { "bvdiv", yices_bvdiv },
  { "bvrem", yices_bvrem },
  { "bvsdiv", yices_bvsdiv },
  { "bvsrem", yices_bvsrem },
  { "bvsmod", yices_bvsmod },
  { "bvand", yices_bvand },
  { "bvor", yices_bvor },
  { "bvxor", yices_bvxor },
  { "bvnand", yices_bvnand },
  { "bvnor", yices_bvnor },
  { "bvxnor", yices_bvxnor },
  { "bvshl", yices_bvshl },
  { "bvlshr", yices_bvlshr },
  { "bvashr", yices_bvashr },
  { "redcomp", yices_redcomp },
};


/*
 * Functions that take one bitvector argument
 */
typedef struct bv_unary_op_s {
  char *name;
  term_t (*fun)(term_t);
} bv_unary_op_t;

#define NUM_UNARY_OPS 5

static bv_unary_op_t unop_array[NUM_UNARY_OPS] = {
  { "bvneg", yices_bvneg },
  { "bvsquare", yices_bvsquare },
  { "bvnot", yices_bvnot },
  { "redand", yices_redand },
  { "redor", yices_redor },
};


/*
 * Atom constructors: two bitvector arguments
 */
#define NUM_PREDS 10

static bv_binop_t pred_array[NUM_PREDS] = {
  { "bveq", yices_bveq_atom },
  { "bvneq", yices_bvneq_atom },
  { "bvge", yices_bvge_atom },
  { "bvgt", yices_bvgt_atom },
  { "bvle", yices_bvle_atom },
  { "bvlt", yices_bvlt_atom },
  { "bvsge", yices_bvsge_atom },
  { "bvsgt", yices_bvsgt_atom },
  { "bvsle", yices_bvsle_atom },
  { "bvslt", yices_bvslt_atom },
};


/*
 * Shift and rotate operations: a bitvector + an integer constant
 */
typedef struct bv_shift_op_s {
  char *name;
  term_t (*fun)(term_t, uint32_t);
} bv_shift_op_t;

#define NUM_SHIFT_OPS 7

static bv_shift_op_t shift_array[NUM_SHIFT_OPS] = {
  { "shift_left0", yices_shift_left0 },
  { "shift_left1", yices_shift_left1 },
  { "shift_right0", yices_shift_right0 },
  { "shift_right1", yices_shift_right1 },
  { "ashift_right", yices_ashift_right },
  { "rotate_left", yices_rotate_left },
  { "rotate_right", yices_rotate_right },
};


/*
 * Zero-extend, sign-extend, repeat concat: bitvector + integer
 */
#define NUM_EXTEND_OPS 3

static bv_shift_op_t extend_array[NUM_EXTEND_OPS] = {
  { "bvrepeat", yices_bvrepeat },
  { "sign_extend", yices_sign_extend },
  { "zero_extend", yices_zero_extend },
};



/*
 * Test a binary operation
 * - i = index in binop_array
 * - t1, t2 = arguments
 */
static term_t test_binop(uint32_t i, term_t t1, term_t t2) {
  term_t t;

  assert(i < NUM_BINOPS);

  printf("test: (%s ", binop_array[i].name);
  print_term(stdout, __yices_globals.terms, t1);
  printf(" ");
  print_term(stdout, __yices_globals.terms, t2);
  printf(") --> ");
  t = binop_array[i].fun(t1, t2);
  print_term(stdout, __yices_globals.terms, t);
  printf("\n");

  fflush(stdout);

  return t;
}

// same thing for unary operations
static term_t test_unop(uint32_t i, term_t t1) {
  term_t t;

  assert(i < NUM_UNARY_OPS);

  printf("test: (%s ", unop_array[i].name);
  print_term(stdout, __yices_globals.terms, t1);
  printf(") --> ");
  t = unop_array[i].fun(t1);
  print_term(stdout, __yices_globals.terms, t);
  printf("\n");

  fflush(stdout);

  return t;
}


// predicate
static term_t test_pred(uint32_t i, term_t t1, term_t t2) {
  term_t t;

  assert(i < NUM_PREDS);

  printf("test: (%s ", pred_array[i].name);
  print_term(stdout, __yices_globals.terms, t1);
  printf(" ");
  print_term(stdout, __yices_globals.terms, t2);
  printf(") --> ");
  t = pred_array[i].fun(t1, t2);
  print_term(stdout, __yices_globals.terms, t);
  printf("\n");

  fflush(stdout);

  return t;
}


// shift/rotate operations
static term_t test_shift(uint32_t i, term_t t1, uint32_t n) {
  term_t t;

  assert(i < NUM_SHIFT_OPS);

  printf("test: (%s ", shift_array[i].name);
  print_term(stdout, __yices_globals.terms, t1);
  printf(" %"PRIu32") --> ", n);
  t = shift_array[i].fun(t1, n);
  print_term(stdout, __yices_globals.terms, t);
  printf("\n");

  fflush(stdout);

  return t;
}

// extend/repeat operations
static term_t test_extend(uint32_t i, term_t t1, uint32_t n) {
  term_t t;

  assert(i < NUM_EXTEND_OPS);

  printf("test: (%s ", extend_array[i].name);
  print_term(stdout, __yices_globals.terms, t1);
  printf(" %"PRIu32") --> ", n);
  t = extend_array[i].fun(t1, n);
  print_term(stdout, __yices_globals.terms, t);
  printf("\n");

  fflush(stdout);

  return t;
}



/*
 * Test bvconcat
 */
static term_t test_bvconcat(term_t t1, term_t t2) {
  term_t t;

  printf("test: (bvconcat ");
  print_term(stdout, __yices_globals.terms, t1);
  printf(" ");
  print_term(stdout, __yices_globals.terms, t2);
  printf(") ---> ");
  t = yices_bvconcat(t1, t2);
  print_term(stdout, __yices_globals.terms, t);
  printf("\n");

  fflush(stdout);

  return t;
}


/*
 * Test bvextract
 */
static term_t test_bvextract(term_t t1, uint32_t i, uint32_t j) {
  term_t t;

  printf("test: (bvextract ");
  print_term(stdout, __yices_globals.terms, t1);
  printf(" %"PRIu32" %"PRIu32") --> ", i, j);
  t = yices_bvextract(t1, i, j);
  print_term(stdout, __yices_globals.terms, t);
  printf("\n");

  fflush(stdout);

  return t;
}


/*
 * Test of bvarray construct
 */
static term_t bvarray[100];

// array of constants + t1 + (not t1)
static term_t test_bvarray1(uint32_t n, term_t t1) {
  term_t t;
  uint32_t i;

  assert(n <= 100);
  for (i=0; i<n; i++) {
    switch (random() % 4) {
    case 0:
      bvarray[i] = false_term;
      break;

    case 1:
      bvarray[i] = true_term;
      break;

    case 2:
      bvarray[i] = t1;
      break;

    default:
      bvarray[i] = yices_not(t1);
      break;
    }
  }

  printf("test: (bvarray");
  for (i=0; i<n; i++) {
    printf(" ");
    print_term(stdout, __yices_globals.terms, bvarray[i]);
  }
  printf(") --> ");
  t = yices_bvarray(n, bvarray);
  print_term(stdout,__yices_globals.terms, t);
  printf("\n");

  fflush(stdout);

  return t;
}

// array of constants + t1/t2 + (not t1) + (not t2)
static term_t test_bvarray2(uint32_t n, term_t t1, term_t t2) {
  term_t t;
  uint32_t i;

  assert(n <= 100);
  for (i=0; i<n; i++) {
    switch (random() % 6) {
    case 0:
      bvarray[i] = false_term;
      break;

    case 1:
      bvarray[i] = true_term;
      break;

    case 2:
      bvarray[i] = t1;
      break;

    case 3:
      bvarray[i] = yices_not(t1);
      break;

    case 4:
      bvarray[i] = t2;
      break;

    default:
      bvarray[i] = yices_not(t2);
      break;
    }
  }

  printf("test: (bvarray");
  for (i=0; i<n; i++) {
    printf(" ");
    print_term(stdout, __yices_globals.terms, bvarray[i]);
  }
  printf(") --> ");
  t = yices_bvarray(n, bvarray);
  print_term(stdout,__yices_globals.terms, t);
  printf("\n");

  fflush(stdout);

  return t;
}


/*
 * If-then-else
 */
static term_t test_ite(term_t c, term_t left, term_t right) {
  term_t t;

  printf("test: (ite ");
  print_term(stdout, __yices_globals.terms, c);
  printf(" ");
  print_term(stdout, __yices_globals.terms, left);
  printf(" ");
  print_term(stdout, __yices_globals.terms, right);
  printf(") --> ");
  t = yices_ite(c, left, right);
  print_term(stdout, __yices_globals.terms, t);
  printf("\n");

  fflush(stdout);

  return t;
}


/*
 * Test bit_extract
 */
static term_t test_bitextract(term_t t, uint32_t i) {
  term_t b;

  printf("test: (bit-extract ");
  print_term(stdout, __yices_globals.terms, t);
  printf(" %"PRIu32") --> ", i);
  b = yices_bitextract(t, i);
  print_term(stdout, __yices_globals.terms, b);
  printf("\n");

  fflush(stdout);

  return b;
}



/*
 * Run all possible tests with terms t1 and t2 (equal size)
 */
static void full_binary_tests(term_t t1, term_t t2) {
  uint32_t i, n;

  for (i=0; i<NUM_BINOPS; i++) {
    test_binop(i, t1, t2);
    test_binop(i, t2, t1);
  }

  for (i=0; i<NUM_UNARY_OPS; i++) {
    test_unop(i, t1);
    test_unop(i, t2);
  }

  for (i=0; i<NUM_PREDS; i++) {
    test_pred(i, t1, t2);
  }

  n = term_bitsize(__yices_globals.terms, t1);
  for (i=0; i<NUM_SHIFT_OPS; i++) {
    test_shift(i, t1, 0);
    test_shift(i, t2, 0);
    test_shift(i, t1, 1);
    test_shift(i, t2, 1);
    test_shift(i, t1, n-1);
    test_shift(i, t2, n-1);
    test_shift(i, t1, n);
    test_shift(i, t2, n);
  }

  for (i=0; i<NUM_EXTEND_OPS; i++) {
    if (i > 0) {
      // repeat concat does not allow n=0
      test_extend(i, t1, 0);
      test_extend(i, t2, 0);
    }
    test_extend(i, t1, 1);
    test_extend(i, t2, 1);
    test_extend(i, t1, 4);
    test_extend(i, t2, 4);
  }

  test_bvconcat(t1, t2);
  test_bvconcat(t2, t1);
  test_bvconcat(t1, t1);
  test_bvconcat(t2, t2);

  test_bvextract(t1, 0, n-1);
  test_bvextract(t2, 0, n-1);
  for (i=0; i+2<n; i++) {
    test_bvextract(t1, i, i+2);
    test_bvextract(t2, i, i+2);
  }

  for (i=0; i<n; i++) {
    test_bvextract(t1, i, i);
    test_bvextract(t2, i, i);
  }

  for (i=0; i<n; i++) {
    test_bitextract(t1, i);
    test_bitextract(t2, i);
  }
}




/*
 * RANDOM TESTS
 */

// predicates used in sampling;
static bool is_bvtype(type_t tau) {
  return type_kind(__yices_globals.types, tau) == BITVECTOR_TYPE;
}

// term of type tau
static bool has_type(type_t tau, term_t t) {
  return term_type(__yices_globals.terms, t) == tau;
}

/*
 * Run n full tests on a pair of randomly selected bit-vector terms
 */
static void random_binary_tests(uint32_t n) {
  type_t tau;
  term_t t1, t2;

  while (n > 0) {
    tau = type_store_sample(&all_types, is_bvtype);
    assert(tau != NULL_TYPE);
    t1 = type_store_sample_terms(&all_types, tau);
    t2 = type_store_sample_terms(&all_types, tau);
    assert(t1 != NULL_TERM && t2 != NULL_TERM);
    printf("--- Test %"PRIu32"---\n", n);
    full_binary_tests(t1, t2);
    printf("\n\n");
    n --;
  }
}



/*
 * Random bitarrays n rounds
 */
static void random_bvarrays(uint32_t n) {
  type_t tau;
  term_t t1, t2, t;
  uint32_t k;

  printf("\n---- Random bitarrays ----\n");
  while (n > 0) {
    tau = type_store_sample(&all_types, is_bvtype);
    k = bv_type_size(__yices_globals.types, tau);
    t1 = term_store_sample(&all_terms, boolean, has_type);
    t2 = term_store_sample(&all_terms, boolean, has_type);

    t = test_bvarray1(k, t1);
    add_term(t);
    printf("\n");

    t = test_bvarray2(k, t1, t2);
    add_term(t);
    printf("\n");

    n --;
  }
}



/*
 * Random bv-extracts: n rounds
 */
static void random_bvextracts(uint32_t n) {
  type_t tau;
  term_t t1, t2, t3, t;
  uint32_t k, i;

  printf("\n---- Random bvextracts ----\n");
  while (n > 0) {
    tau = type_store_sample(&all_types, is_bvtype);
    k = bv_type_size(__yices_globals.types, tau);

    t1 = type_store_sample_terms(&all_types, tau);

    // split t1
    i = ((uint32_t) random()) % k;
    assert(0 <= i && i <= k-1);

    // right part: low order bits [0 ... i]
    t2 = test_bvextract(t1, 0, i);
    add_term(t2);

    if (i < k-1) {
      // left part: high-order bits [i+1 ... k-1]
      t3 = test_bvextract(t1, i+1, k-1);
      add_term(t3);

      // check concat: should get back t1
      t = test_bvconcat(t3, t2);
      assert(t == t1);

    } else {
      // the left part is empty
      assert(t1 == t2);
    }

    printf("\n");

    n --;
  }
}


/*
 * Random if-then-else test: n rounds
 */
static void random_ite(uint32_t n) {
  type_t tau;
  term_t t1, t2, c;

  printf("\n---- Test if-then-else ----\n");
  while (n > 0) {
    tau = type_store_sample(&all_types, is_bvtype);
    t1 = type_store_sample_terms(&all_types, tau);
    t2 = type_store_sample_terms(&all_types, tau);
    c = term_store_sample(&all_terms, boolean, has_type);

    test_ite(c, t1, t2);
    printf("\n");

    n --;
  }
}



int main(void) {
  yices_init();
  init_store();
  init_base_types();
  init_base_terms();

  show_types(stdout);
  show_terms(stdout);

  random_bvarrays(BVNUM);
  random_bvextracts(BVNUM);

  random_ite(ITENUM);
  random_binary_tests(BINUM);

  show_types(stdout);
  show_terms(stdout);

  delete_store();
  yices_exit();

  return 0;
}
