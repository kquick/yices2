/*
 * Concrete values = constants of different types.
 * This is used to build models: a model is a mapping from terms to concrete values.
 *
 * The table is divided into two parts:
 * - permanent objects = objects that must be kept in the model
 * - temporary objects = objects created when evaluating the value of a non-atomic term.
 * The temporary objects can be deleted.
 *
 * The implementation works in two modes:
 * - default mode: create permanent objects
 * - tmp mode: all objects created are temporary and are deleted when
 *   tmp_mode is exited.
 *
 * We attempt to ensure that different objects in the table actually
 * represent different values. But this is hard to ensure for
 * functions. So we attach a "canonical flag" to each object:
 * - if the bit is 1 for object i then i is in a canonical representation.
 *   An object j with a different descriptor cannot be equal to i.
 * - if the bit is 0, then i is not in a canonical form.
 *
 *
 * For printing/pretty printing, we keep track of function objects
 * whose map must be printed. We store them in a queue + add a mark.
 */

#ifndef __CONCRETE_VALUES_H
#define __CONCRETE_VALUES_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>

#include "terms/types.h"
#include "terms/rationals.h"
#include "utils/bitvectors.h"
#include "terms/bv_constants.h"
#include "utils/int_queues.h"
#include "utils/int_vectors.h"
#include "utils/int_hash_tables.h"


/*
 * Each concrete value is identified by an integer index.
 * A type and descriptor for each value is stored in a table.
 * We use hash consing.
 *
 * The concrete types are
 *  UNKNOWN_VALUE
 *  BOOLEAN_VALUE
 *  RATIONAL_VALUE
 *  BITVECTOR_VALUE
 *  TUPLE_VALUE
 *  UNINTERPRETED_VALUE
 *  FUNCTION_VALUE
 *  FUNCTION_UPDATE
 *
 * A function value is a finite set of mappings + a default value.
 * The mappings are stored as MAP_VALUE objects in the table.
 * A mapping is of the form [a_0 ... a_{n-1} -> val].
 * If such a mapping belongs to f then (f a_0 ... a_{n-1}) = val.
 * To save memory, we also allow functions to be constructed as
 * updates of the form (update f_0 k) where k is a mapping and f_0
 * is another function.
 */

/*
 * Value indices = signed integers
 */
typedef int32_t value_t;


/*
 * null_value means no value assigned yet.
 * This is different from UNKNOWN_VALUE.
 */
enum {
  null_value = -1,
};


/*
 * Types
 */
typedef enum {
  UNKNOWN_VALUE,
  BOOLEAN_VALUE,
  RATIONAL_VALUE,
  BITVECTOR_VALUE,
  TUPLE_VALUE,
  UNINTERPRETED_VALUE,
  FUNCTION_VALUE,
  MAP_VALUE,
  UPDATE_VALUE,
} value_kind_t;

#define NUM_VALUE_KIND ((uint32_t) (UPDATE_VALUE + 1))


/*
 * Value stored in the table:
 * - either an integer or a rational or a pointer to an object descriptor
 */
typedef union value_desc_u {
  int32_t integer;
  rational_t rational;
  void *ptr;
} value_desc_t;


/*
 * Descriptors: encode the actual value
 */
// bitvector constant
typedef struct value_bv_s {
  uint32_t nbits;
  uint32_t width;   // size in words = ceil(nbits/32)
  uint32_t data[0]; // real size = width
} value_bv_t;

// tuple = array of values
typedef struct value_tuple_s {
  uint32_t nelems;
  value_t elem[0]; // real size = arity
} value_tuple_t;

// uninterpreted constant
typedef struct value_unint_s {
  type_t type;
  int32_t index;  // id = same as in constant_terms in term table
  char *name;
} value_unint_t;

// mapping object: arg[0] ... arg[n-1] |-> val
typedef struct value_map_s {
  uint32_t arity;
  value_t val;
  value_t arg[0]; // real size = arity
} value_map_t;

// function: default value + an array of mapping objects
typedef struct value_fun_s {
  char *name;
  type_t type;        // function type
  uint32_t arity;     // number of parameters
  value_t def;        // default value
  uint32_t map_size;  // size of array map
  value_t map[0];     // array of mapping object of size = map_size
} value_fun_t;


// function update = (update fun map)
typedef struct value_update_s {
  uint32_t arity;
  value_t fun; // function
  value_t map; // mapping
} value_update_t;


// Limits on the size of maps and tuple that can be represented
#define VTBL_MAX_MAP_SIZE   ((UINT32_MAX-sizeof(value_fun_t))/sizeof(value_t))
#define VTBL_MAX_TUPLE_SIZE ((UINT32_MAX-sizeof(value_tuple_t))/sizeof(value_t))
#define VTBL_MAX_MAP_ARITY  ((UINT32_MAX-sizeof(value_map_t))/sizeof(value_t))


/*
 * To accelerate function evaluation, we store pairs
 * <function, map> into an auxiliary hash table.
 * - function is a function object (not an update)
 * - map must be a mapping object that belongs to function
 */
typedef struct map_pair_s {
  value_t function;
  value_t map;
} map_pair_t;

typedef struct map_htbl_s {
  map_pair_t *data;  // hash table proper
  uint32_t size;     // its size (must be a power of 2)
  uint32_t nelems;
  uint32_t resize_threshold;
} map_htbl_t;


/*
 * Default initial size of a map table + maximal size
 */
#define MAP_HTBL_DEFAULT_SIZE 64
#define MAP_HTBL_MAX_SIZE (UINT32_MAX/sizeof(map_pair_t))

/*
 * Resize ratio: the table size is doubled when nelems >= size * resize ratio
 */
#define MAP_HTBL_RESIZE_RATIO 0.7



/*
 * Hash set used to compute the normal form of update objects
 * - a function is represented as a finite set of mapping objects
 * - normalizing an update object converts it to a finite set of
 *   mappings.
 * This set is represented as a hash-set
 */
typedef struct map_hset_s {
  value_t *data;     // set elements
  uint32_t size;     // size of the data array
  uint32_t nelems;   // number of elements in the array
  uint32_t resize_threshold;
} map_hset_t;


/*
 * Default initial size + maximal size of an hset
 */
#define MAP_HSET_DEFAULT_SIZE 32
#define MAP_HSET_MAX_SIZE (UINT32_MAX/sizeof(value_t))


/*
 * Resize ratio
 */
#define MAP_HSET_RESIZE_RATIO 0.7


/*
 * Reduce threshold: in reset, if the hset size is more than this
 * threshold then the data array is reduced to the default size.
 */
#define MAP_HSET_REDUCE_THRESHOLD 256


/*
 * Queue + bitvector for functions whose map must be printed.
 */
typedef struct vtbl_queue_s {
  int_queue_t queue;
  byte_t *mark;
  uint32_t size; // size of the mark vector
} vtbl_queue_t;

#define DEF_VTBL_QUEUE_SIZE 2048


/*
 * Optional function to name uninterpreted constants
 * - the table contains a pointer to a function 'unint_namer'
 *   + an opaque pointer aux_namer.
 * - when an uninterpreted value is printed, the value's name
 *   (stored in the value_unint_t descriptor d) is used.
 * - if d->name is NULL and name_of_unint is non NULL, then
 *      unint_namer(aux, d) is called
 *   if this function returns a non-NULL string, that's used
 *   as the name.
 * - otherwise, the modules uses a name 'const!k' for some k
 */
typedef const char *(*unint_namer_fun_t)(void *aux, value_unint_t *d);


/*
 * Table of concrete objects
 * - valid objects have indices between 0 and nobjects - 1
 * - for each object,
 *   kind[i] = its type
 *   desc[i] = its descriptor
 *   canonical[i] = one bit: 1 means i is in a canonical form
 *                           0 means it's not
 * - other components:
 *   type_table = pointer to an associated type table
 *   htbl = hash table for hash consing
 *   buffer for building bitvector constants
 *   auxiliary vector
 *   mtbl = hash table of pairs (fun, map)
 *   hset1, hset2 = hash sets allocated on demand (used in
 *      hash consing of update objects)
 * - unknown_value = index of the unknown value
 * - true_value, false_value = indices of true/false values
 *
 * - first_tmp = index of the first temporary object
 *   if first_tmp = -1 then all objects are permanent.
 *   if first_tmp >= 0, then objects in [0 .. first_tmp -1] are permanent
 *   and objects in [first_tmp .. nobjects - 1] are temporary.
 *
 * - pointer for getting names of uninterpreted constants
 */
typedef struct value_table_s {
  uint32_t size;
  uint32_t nobjects;
  uint8_t *kind;
  value_desc_t *desc;
  byte_t *canonical; // bitvector

  type_table_t *type_table;
  int_htbl_t htbl;
  bvconstant_t buffer;
  ivector_t aux_vector;
  map_htbl_t mtbl;
  vtbl_queue_t queue;
  map_hset_t *hset1;
  map_hset_t *hset2;

  int32_t unknown_value;
  int32_t true_value;
  int32_t false_value;
  int32_t first_tmp;

  void *aux_namer;
  unint_namer_fun_t unint_namer;
} value_table_t;


#define DEF_VALUE_TABLE_SIZE 200
#define MAX_VALUE_TABLE_SIZE (UINT32_MAX/sizeof(value_desc_t))




/*
 * INITIALIZATION
 */

/*
 * Initialize a table;
 * - n = initial size. If n is zero, the default size is used.
 * - ttbl = attached type table.
 */
extern void init_value_table(value_table_t *table, uint32_t n, type_table_t *ttbl);

/*
 * Delete table: free all memory
 */
extern void delete_value_table(value_table_t *table);

/*
 * Reset: empty the table
 */
extern void reset_value_table(value_table_t *table);


/*
 * Assign aux and namer for dealing with uninterpreted values whose name is missing
 */
static inline void value_table_set_namer(value_table_t *table, void *aux, unint_namer_fun_t f) {
  assert(f != NULL);
  table->aux_namer = aux;
  table->unint_namer = f;
}


/*
 * OBJECT CONSTRUCTORS
 */

/*
 * Undefined value
 */
extern value_t vtbl_mk_unknown(value_table_t *table);

/*
 * Boolean constant: val = 0 means false, val != 0 means true
 */
extern value_t vtbl_mk_bool(value_table_t *table, int32_t val);
extern value_t vtbl_mk_true(value_table_t *table);
extern value_t vtbl_mk_false(value_table_t *table);

/*
 * Negate v (v must be either true or false)
 */
extern value_t vtbl_mk_not(value_table_t *table, value_t v);


/*
 * Rational (or integer) constant (make a copy)
 */
extern value_t vtbl_mk_rational(value_table_t *table, rational_t *v);
extern value_t vtbl_mk_int32(value_table_t *table, int32_t x);


/*
 * Bit-vector constant: input is an array of n integers
 * - bit i is 0 if a[i] == 0
 * - bit i is 1 if a[i] != 0
 * So a[0] = low order bit, a[n-1] = high order bit
 */
extern value_t vtbl_mk_bv(value_table_t *table, uint32_t n, int32_t *a);

/*
 * Variant: the input is an array of 32bit words
 * - n = number of bits
 * - a = array of w words where w = ceil(n/32)
 */
extern value_t vtbl_mk_bv_from_bv(value_table_t *table, uint32_t n, uint32_t *a);

/*
 * Variant: input is a bvconstant object
 * - b->bitsize = number of bits
 * - b->data = bits (stored as an array of uint32_t integers)
 */
extern value_t vtbl_mk_bv_from_constant(value_table_t *table, bvconstant_t *b);

/*
 * Variant: input is a 64bit unsigned integer
 * - n = number of bits to use (n <= 64)
 * - c = integer constant
 */
extern value_t vtbl_mk_bv_from_bv64(value_table_t *table, uint32_t n, uint64_t c);

/*
 * Variants:
 * - bitvector 0b0000...00 of n bits
 * - bitvector 0b0000...01 of n bits
 */
extern value_t vtbl_mk_bv_zero(value_table_t *table, uint32_t n);
extern value_t vtbl_mk_bv_one(value_table_t *table, uint32_t n);


/*
 * Tuple:
 * - n = arity
 * - e[0] ... e[n-1] = components
 * all components must be valid elements in table
 * n must be less than MAX_TERM_ARITY
 */
extern value_t vtbl_mk_tuple(value_table_t *table, uint32_t n, value_t *e);


#if 0

// NOT USED
/*
 * Fresh uninterpreted constant
 * - tau = its type (as an index in the type table)
 * - name = an optional name (NULL if no name is given)
 * - if name is given, a copy is made.
 * - the constant is assigned a default index = -1
 */
extern value_t vtbl_mk_unint(value_table_t *table, type_t tau, char *name);
#endif


/*
 * Uninterpreted constant of index id
 * - tau = its type (must be UNINTERPRETED or SCALAR type)
 * - name = optional name (NULL if no name is given)
 * - id = index (must be non-negative)
 *
 * If the constant already exists and has a name, it keeps
 * its current name. Otherwise, if name != NULL, then the constant
 * is given that name.
 */
extern value_t vtbl_mk_const(value_table_t *table, type_t tau, int32_t id, char *name);


/*
 * Mapping a[0 ... n-1] := v
 * - return a mapping object
 */
extern value_t vtbl_mk_map(value_table_t *table, uint32_t n, value_t *a, value_t v);


/*
 * Function defined by the array a[0...n] and default value def
 * - tau = its type
 * - a = array of n mapping objects.
 *   The array must not contain conflicting mappings and all elements in a
 *   must have the right arity (same as defined by type tau). It's allowed
 *   to have duplicate elements in a.
 * - def = default value (must be unknown if no default is given).
 *
 * NOTE: array a is modified.
 */
extern value_t vtbl_mk_function(value_table_t *table, type_t tau, uint32_t n, value_t *a, value_t def);


/*
 * Create (update f (a[0] ... a[n-1]) v)
 * - f must be a function of arity n (either a function object or another update)
 */
extern value_t vtbl_mk_update(value_table_t *table, value_t f, uint32_t n, value_t *a, value_t v);



/*
 * DEFAULT VALUES
 */

/*
 * Return an arbitrary value of type tau:
 * - this is deterministic: the same value is returned every time the function is called
 *   with the same type.
 * - tau must be a valid ground type defined in table->type_table
 */
extern value_t vtbl_make_object(value_table_t *table, type_t tau);


/*
 * Attempt to construct two distinct objects of type tau.
 * - return false if tau is a singleton type
 * - otherwise store the two objects in a[0] and a[1] and return true
 */
extern bool vtbl_make_two_objects(value_table_t *vtbl, type_t tau, value_t a[2]);




/*
 * CHECK WHETHER OBJECTS ARE PRESENT
 */

/*
 * All functions below check whether an object exists in table
 * - they return the object id (value_t) if it exists
 * - they return null_value otherwise (and don't construct anything
 *   in table).
 */

// rationals
extern value_t vtbl_find_rational(value_table_t *table, rational_t *v);
extern value_t vtbl_find_int32(value_table_t *table, int32_t x);

// constants of scalar or uninterpreted type
extern value_t vtbl_find_const(value_table_t *table, type_t tau, int32_t id);

// tuple e[0] ... e[n-1]
extern value_t vtbl_find_tuple(value_table_t *table, uint32_t n, value_t *e);

// bitvector defined by a[0 ... n-1]
extern value_t vtbl_find_bv(value_table_t *table, uint32_t n, int32_t *a);

// bitvector defined by c. n = number of bits (must be <= 64)
extern value_t vtbl_find_bv64(value_table_t *table, uint32_t n, uint64_t c);

// bitvector defined by a bvconstant b
extern value_t vtbl_find_bvconstant(value_table_t *table, bvconstant_t *b);

// map object: a[0 ... n-1] := v
extern value_t vtbl_find_map(value_table_t *table, uint32_t n, value_t *a, value_t v);

// function defined by array of n maps + the default value
// array a is modified
extern value_t vtbl_find_function(value_table_t *table, type_t tau, uint32_t n, value_t *a, value_t def);


/*
 * TEST EXISTENCE
 */
static inline bool vtbl_test_rational(value_table_t *table, rational_t *v) {
  return vtbl_find_rational(table, v) >= 0;
}

static inline bool vtbl_test_int32(value_table_t *table, int32_t x) {
  return vtbl_find_int32(table, x) >= 0;
}

static inline bool vtbl_test_const(value_table_t *table, type_t tau, int32_t id) {
  return vtbl_find_const(table, tau, id) >= 0;
}

static inline bool vtbl_test_tuple(value_table_t *table, uint32_t n, value_t *e) {
  return vtbl_find_tuple(table, n, e) >= 0;
}

static inline bool vtbl_test_bv(value_table_t *table, uint32_t n, int32_t *a) {
  return vtbl_find_bv(table, n, a) >= 0;
}

static inline bool vtbl_test_bv64(value_table_t *table, uint32_t n, uint64_t c) {
  return vtbl_find_bv64(table, n, c) >= 0;
}

static inline bool vtbl_test_bvconstant(value_table_t *table, bvconstant_t *b) {
  return vtbl_find_bvconstant(table, b) >= 0;
}

static inline bool vtbl_test_map(value_table_t *table, uint32_t n, value_t *a, value_t v) {
  return vtbl_find_map(table, n, a, v) >= 0;
}

// warning: a is modified
static inline bool vtbl_test_function(value_table_t *table, type_t tau, uint32_t n, value_t *a, value_t def) {
  return vtbl_find_function(table, tau, n, a, def) >= 0;
}




/*
 * OBJECTS OF FINITE TYPE
 */

/*
 * If tau is a finite type, then we can enumerate its elements from
 * 0 to card(tau) - 1. The following function constructs an element
 * of finite type tau given an enumeration index i.
 * - tau must be finite
 * - i must be smaller than card(tau)
 */
extern value_t vtbl_gen_object(value_table_t *table, type_t tau, uint32_t i);

/*
 * Same thing for tuples:
 * - tau[0 ... n-1] must be n finite types
 * - i must be an index between 0 and card(tau[0]) x ... x card(tau[n-1])
 * - a must be large enough to store n elements
 * - this stores n objects in a[0 ... n-1] where a[k] has type tau[k]
 * - different indices i generate different tuples of n objects
 */
extern void vtbl_gen_object_tuple(value_table_t *table, uint32_t n, type_t *tau, uint32_t i, value_t *a);

/*
 * Same thing for a finite function type tau:
 * - the domain must be finite
 * - let D = cardinality of tau's domain and R = cardinality of tau's range
 * - a function of type tau is defined by D values a[0 ... D-1] where each a[k] is an
 *   object of tau's range.
 * - this function builds a[0 ... D-1] given an index i between 0 and R^D.
 * - a must be large enough to store D elements
 */
extern void vtbl_gen_function_map(value_table_t *table, type_t tau, uint32_t i, value_t *a);


/*
 * Check whether object of index i is present in the table
 * - tau must be finite
 * - i must be smaller than card(tau)
 * - if the object exists, it's returned
 * - otherwise, the function returns null_value
 */
extern value_t vtbl_find_object(value_table_t *table, type_t tau, uint32_t i);

static inline bool vtbl_test_object(value_table_t *table, type_t tau, uint32_t i) {
  return vtbl_find_object(table, tau, i) >= 0;
}


/*
 * Search for object tuple of index i and type tau[0] x ... x tau[n-1]
 * - return null_value if it's not present
 */
extern value_t vtbl_find_object_tuple(value_table_t *table, uint32_t n, type_t *tau, uint32_t i);

static inline bool vtbl_test_object_tuple(value_table_t *table, uint32_t n, type_t *tau, uint32_t i) {
  return vtbl_find_object_tuple(table, n, tau, i) >= 0;
}


/*
 * NAMES
 */

/*
 * Set or change the name of function f
 * - overwrite the current name if any
 * - make an internal copy of the string if name != NULL
 * - if name = NULL, this clears the name of f
 */
extern void vtbl_set_function_name(value_table_t *table, value_t f, char *name);


/*
 * Set or change the name of constant c
 * - same effect as the previous function
 */
extern void vtbl_set_constant_name(value_table_t *table, value_t c, char *name);




/*
 * TEMPORARY OBJECTS
 */

/*
 * Switch to temporary mode:
 * - all objects currently in the table are considered permanent.
 * - all terms created after this function is called are temporary.
 * - the table must not be in temporary mode already.
 *
 * Side effect: creates the unknown, true, and false values if they
 * don't exist already. These are always permanent terms.
 *
 * Warning: the permanent terms must not be modified after this.
 * So add_map and set_function_default should not be called with a permanent f.
 */
extern void value_table_start_tmp(value_table_t *table);


/*
 * Delete all temporary objects and return to permanent mode.
 * Do nothing if the table is not in temporary mode.
 */
extern void value_table_end_tmp(value_table_t *table);





/*
 * EVALUATION
 */

/*
 * Check whether a and b are equal
 * - return unknown if we can't tell
 */
extern value_t vtbl_eval_eq(value_table_t *table, value_t a, value_t b);


/*
 * Check whether arrays a[0 ... n-1] and b[0 ... n-1] are equal
 * - return unknown if we can't tell
 */
extern value_t vtbl_eval_array_eq(value_table_t *table, value_t *a, value_t *b, uint32_t n);


/*
 * Evaluate (f a[0] ... a[n-1])
 * - f must be a function or update object of arity n
 * - a[0] ... a[n-1] must be non-null values
 * Return unknown if the map is not defined for a[0 ... n-1]
 */
extern value_t vtbl_eval_application(value_table_t *table, value_t f, uint32_t n, value_t *a);





/*
 * ACCESS TO OBJECT REPRESENTATION
 */
static inline bool good_object(value_table_t *table, value_t v) {
  return 0 <= v && v < table->nobjects;
}

static inline value_kind_t object_kind(value_table_t *table, value_t v) {
  assert(good_object(table, v));
  return (value_kind_t) table->kind[v];
}

// check the type of v
static inline bool object_is_unknown(value_table_t *table, value_t v) {
  return object_kind(table, v) == UNKNOWN_VALUE;
}

static inline bool object_is_boolean(value_table_t *table, value_t v) {
  return object_kind(table, v) == BOOLEAN_VALUE;
}

static inline bool object_is_rational(value_table_t *table, value_t v) {
  return object_kind(table, v) == RATIONAL_VALUE;
}

static inline bool object_is_integer(value_table_t *table, value_t v) {
  return object_is_rational(table, v) && q_is_integer(&table->desc[v].rational);
}

static inline bool object_is_bitvector(value_table_t *table, value_t v) {
  return object_kind(table, v) == BITVECTOR_VALUE;
}

static inline bool object_is_tuple(value_table_t *table, value_t v) {
  return object_kind(table, v) == TUPLE_VALUE;
}

static inline bool object_is_unint(value_table_t *table, value_t v) {
  return object_kind(table, v) == UNINTERPRETED_VALUE;
}

static inline bool object_is_function(value_table_t *table, value_t v) {
  return object_kind(table, v) == FUNCTION_VALUE;
}

static inline bool object_is_map(value_table_t *table, value_t v) {
  return object_kind(table, v) == MAP_VALUE;
}

static inline bool object_is_update(value_table_t *table, value_t v) {
  return object_kind(table, v) == UPDATE_VALUE;
}


/*
 * Check the canonical bit
 */
static inline bool object_is_canonical(value_table_t *table, value_t v) {
  assert(good_object(table, v));
  return tst_bit(table->canonical, v);
}


// check value of v
static inline bool is_unknown(value_table_t *table, value_t v) {
  assert(good_object(table, v));
  return v == table->unknown_value;
}

static inline bool is_true(value_table_t *table, value_t v) {
  assert(good_object(table, v));
  return v == table->true_value;
}

static inline bool is_false(value_table_t *table, value_t v) {
  assert(good_object(table, v));
  return v == table->false_value;
}

static inline bool boolobj_value(value_table_t *table, value_t v) {
  assert(object_is_boolean(table, v));
  return (bool) table->desc[v].integer;
}


// get descriptor of v
static inline rational_t *vtbl_rational(value_table_t *table, value_t v) {
  assert(object_is_rational(table, v));
  return &table->desc[v].rational;
}

static inline value_bv_t *vtbl_bitvector(value_table_t *table, value_t v) {
  assert(object_is_bitvector(table, v));
  return (value_bv_t *) table->desc[v].ptr;
}

static inline value_tuple_t *vtbl_tuple(value_table_t *table, value_t v) {
  assert(object_is_tuple(table, v));
  return (value_tuple_t *) table->desc[v].ptr;
}

static inline value_unint_t *vtbl_unint(value_table_t *table, value_t v) {
  assert(object_is_unint(table, v));
  return (value_unint_t *) table->desc[v].ptr;
}

static inline value_fun_t *vtbl_function(value_table_t *table, value_t v) {
  assert(object_is_function(table, v));
  return (value_fun_t *) table->desc[v].ptr;
}

static inline value_map_t *vtbl_map(value_table_t *table, value_t v) {
  assert(object_is_map(table, v));
  return (value_map_t *) table->desc[v].ptr;
}

static inline value_t vtbl_map_result(value_table_t *table, value_t v) {
  return vtbl_map(table, v)->val;
}

static inline value_update_t *vtbl_update(value_table_t *table, value_t v) {
  assert(object_is_update(table, v));
  return (value_update_t *) table->desc[v].ptr;
}




/*
 * UTILITIES
 */

/*
 * Normalize an update object i
 * - this computes a set of mapping for object i
 * - the default value for i is stored in *def
 * - the type of i is stored in *tau (this is a function type)
 *
 * The set of mappings is stored in the internal hset1 table:
 * - hset1->data contains the set of mapping objects for i (without duplicates)
 * - hset1->nelems = number of mappings in hset1->data
 */
extern void vtbl_expand_update(value_table_t *table, value_t i, value_t *def, type_t *tau);

/*
 * Push v into the internal queue
 * - v must be a valid object
 * - do nothing if v is already in the queue
 */
extern void vtbl_push_object(value_table_t *table, value_t v);


/*
 * Check whether the queue is empty
 */
extern bool vtbl_queue_is_empty(value_table_t *table);

static inline bool vtbl_queue_is_nonempty(value_table_t *table) {
  return !vtbl_queue_is_empty(table);
}

/*
 * Empty the internal queue
 */
extern void vtbl_empty_queue(value_table_t *table);


#endif /* __CONCRETE_VALUES_H */