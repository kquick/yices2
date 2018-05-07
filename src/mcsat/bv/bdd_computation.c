/*
 * The Yices SMT Solver. Copyright 2015 SRI International.
 *
 * This program may only be used subject to the noncommercial end user
 * license agreement which is downloadable along with this program.
 */

#include "bdd_computation.h"
#include "bv_utils.h"

//#define DEBUG_PRINT(x, n) fprintf(stderr, #x" = "); bdds_print(cudd, x, n, stderr); fprintf(stderr, "\n");
#define DEBUG_PRINT(x, n)

static inline
void bdds_reverse(BDD** bdds, uint32_t n) {
  assert(n > 0);
  BDD** p = bdds;
  BDD** q = bdds + n - 1;
  BDD* tmp;
  while (p < q) {
    tmp = *p; *p = *q; *q = tmp;
    p ++; q --;
  }
}

CUDD* bdds_new() {
  CUDD* cudd = (CUDD*) safe_malloc(sizeof(CUDD));
  cudd->cudd = Cudd_Init(0, 0, CUDD_UNIQUE_SLOTS, CUDD_CACHE_SLOTS,0);
  cudd->tmp_alloc_size = 0;
  cudd->tmp_inputs = NULL;
  cudd->tmp_model = NULL;
  return cudd;
}

void bdds_delete(CUDD* cudd) {
  int leaks = Cudd_CheckZeroRef(cudd->cudd);
  (void) leaks;
  assert(leaks == 0);
  Cudd_Quit(cudd->cudd);
  safe_free(cudd->tmp_inputs);
  safe_free(cudd->tmp_model);
}

void bdds_init(BDD** a, uint32_t n) {
  for(uint32_t i = 0; i < n; ++ i) {
    a[i] = NULL;
  }
}

void bdds_clear(CUDD* cudd, BDD** a, uint32_t n) {
  for(uint32_t i = 0; i < n; ++ i) {
    if (a[i] != NULL) { Cudd_IterDerefBdd(cudd->cudd, a[i]); }
    a[i] = NULL;
  }
}

void bdds_attach(BDD** a, uint32_t n) {
  for(uint32_t i = 0; i < n; ++ i) {
    assert(a[i] != NULL);
    Cudd_Ref(a[i]);
  }
}

bool bdds_eq(BDD** a, BDD** b, uint32_t n) {
  for(uint32_t i = 0; i < n; ++ i) {
    if (a[i] != b[i]) {
      return false;
    }
  }
  return true;
}

void bdds_print(CUDD* cudd, BDD** a, uint32_t n, FILE* out) {
  Cudd_DumpFactoredForm(cudd->cudd, n, a, NULL, NULL, out);
}

void bdds_mk_variable(CUDD* cudd, BDD** out, uint32_t n) {
  BDD* bdd_i = NULL;
  for (uint32_t i = 0; i < n; ++i) {
    bdd_i = Cudd_bddNewVar(cudd->cudd);
    out[i] = bdd_i;
    // We do increase the reference count so that we are uniform when dereferencing
    Cudd_Ref(bdd_i);
  }
  if (bdd_i) {
    // Max index: last allocated variable
    unsigned int needed_size = Cudd_NodeReadIndex(bdd_i) + 1;
    if (needed_size > cudd->tmp_alloc_size) {
      if (cudd->tmp_alloc_size == 0) {
        cudd->tmp_alloc_size = 10;
      }
      while (needed_size > cudd->tmp_alloc_size) {
        cudd->tmp_alloc_size += cudd->tmp_alloc_size >> 1;
      }
      cudd->tmp_inputs = (int*) safe_realloc(cudd->tmp_inputs, sizeof(int)*cudd->tmp_alloc_size);
      cudd->tmp_model = (char*) safe_realloc(cudd->tmp_model, sizeof(char)*cudd->tmp_alloc_size);
    }
  }
}

void bdds_mk_zero(CUDD* cudd, BDD** out, uint32_t n) {
  for(uint32_t i = 0; i < n; ++ i) {
    assert(out[i] == NULL);
    out[i] = Cudd_ReadLogicZero(cudd->cudd);
    Cudd_Ref(out[i]);
  }
}

void bdds_mk_one(CUDD* cudd, BDD** out, uint32_t n) {
  for(uint32_t i = 0; i < n; ++ i) {
    assert(out[i] == NULL);
    out[i] = Cudd_ReadOne(cudd->cudd);
    Cudd_Ref(out[i]);
  }
}

void bdds_mk_constant(CUDD* cudd, BDD** out, uint32_t n, const bvconstant_t* c) {
  assert(n == c->bitsize);
  for(uint32_t i = 0; i < n; ++ i) {
    assert(out[i] == NULL);
    bool bit_i = bvconst_tst_bit(c->data, i);
    out[i] = bit_i ? Cudd_ReadOne(cudd->cudd) : Cudd_ReadLogicZero(cudd->cudd);
    Cudd_Ref(out[i]);
  }
}

void bdds_mk_neg(CUDD* cudd, BDD** out, BDD** a, uint32_t n) {
  for(uint32_t i = 0; i < n; ++ i) {
    assert(out[i] == NULL);
    out[i] = Cudd_Not(a[i]);
    Cudd_Ref(out[i]);
  }
}

void bdds_mk_and(CUDD* cudd, BDD** out, BDD** a, BDD** b, uint32_t n) {
  for(uint32_t i = 0; i < n; ++ i) {
    assert(out[i] == NULL);
    out[i] = Cudd_bddAnd(cudd->cudd, a[i], b[i]);
    Cudd_Ref(out[i]);
  }
}

void bdds_mk_or(CUDD* cudd, BDD** out, BDD** a, BDD** b, uint32_t n) {
  for(uint32_t i = 0; i < n; ++ i) {
    assert(out[i] == NULL);
    out[i] = Cudd_bddOr(cudd->cudd, a[i], b[i]);
    Cudd_Ref(out[i]);
  }
}

void bdds_mk_div(CUDD* cudd, BDD** out_bdds, BDD** a, BDD** b, uint32_t n) {
  assert(false);
}

void bdds_mk_rem(CUDD* cudd, BDD** out_bdds, BDD** a, BDD** b, uint32_t n) {
  assert(false);
}

void bdds_mk_sdiv(CUDD* cudd, BDD** out_bdds, BDD** a, BDD** b, uint32_t n) {
  assert(false);
}

void bdds_mk_srem(CUDD* cudd, BDD** out_bdds, BDD** a, BDD** b, uint32_t n) {
  assert(false);
}

void bdds_mk_smod(CUDD* cudd, BDD** out_bdds, BDD** a, BDD** b, uint32_t n) {
  assert(false);
}

void bdds_mk_shl(CUDD* cudd, BDD** out_bdds, BDD** a, BDD** b, uint32_t n) {
  assert(false);
}

void bdds_mk_lshr(CUDD* cudd, BDD** out_bdds, BDD** a, BDD** b, uint32_t n) {
  assert(false);
}

void bdds_mk_ashr(CUDD* cudd, BDD** out_bdds, BDD** a, BDD** b, uint32_t n) {
  assert(false);
}

void bdds_mk_bool_or(CUDD* cudd, BDD** out, const pvector_t* children_bdds) {
  uint32_t n = children_bdds->size;
  out[0] = Cudd_ReadLogicZero(cudd->cudd);
  Cudd_Ref(out[0]);
  for (uint32_t i = 0; i < n; i ++ ) {
    BDD* tmp = out[0];
    BDD** child_i = (BDD**) children_bdds->data[i];
    out[0] = Cudd_bddOr(cudd->cudd, tmp, child_i[0]);
    Cudd_Ref(out[0]);
    Cudd_IterDerefBdd(cudd->cudd, tmp);
  }
}

void bdds_mk_eq(CUDD* cudd, BDD** out, BDD** a, BDD** b, uint32_t n) {
  assert(n > 0);
  assert(out[0] == NULL);
  out[0] = Cudd_Xeqy(cudd->cudd, n, a, b);
  Cudd_Ref(out[0]);
}

void bdds_compute_bdds(CUDD* cudd, term_table_t* terms, term_t t,
    const pvector_t* children_bdds, BDD** out_bdds) {

  assert(bv_term_has_children(terms, t));

  BDD** t0;
  BDD** t1;

  uint32_t t_bitsize = bv_term_bitsize(terms, t);

  if (is_neg_term(t)) {
    // Negation
    assert(children_bdds->size == 1);
    t0 = (BDD**) children_bdds->data[0];
    bdds_mk_neg(cudd, out_bdds, t0, t_bitsize);
  } else {
    term_kind_t kind = term_kind(terms, t);
    switch (kind) {
    case BV_DIV:
      assert(children_bdds->size == 2);
      t0 = (BDD**) children_bdds->data[0];
      t1 = (BDD**) children_bdds->data[1];
      bdds_mk_div(cudd, out_bdds, t0, t1, t_bitsize);
      break;
    case BV_REM:
      assert(children_bdds->size == 2);
      t0 = (BDD**) children_bdds->data[0];
      t1 = (BDD**) children_bdds->data[1];
      bdds_mk_rem(cudd, out_bdds, t0, t1, t_bitsize);
      break;
    case BV_SDIV:
      assert(children_bdds->size == 2);
      t0 = (BDD**) children_bdds->data[0];
      t1 = (BDD**) children_bdds->data[1];
      bdds_mk_sdiv(cudd, out_bdds, t0, t1, t_bitsize);
      break;
    case BV_SREM:
      assert(children_bdds->size == 2);
      t0 = (BDD**) children_bdds->data[0];
      t1 = (BDD**) children_bdds->data[1];
      bdds_mk_srem(cudd, out_bdds, t0, t1, t_bitsize);
      break;
    case BV_SMOD:
      assert(children_bdds->size == 2);
      t0 = (BDD**) children_bdds->data[0];
      t1 = (BDD**) children_bdds->data[1];
      bdds_mk_smod(cudd, out_bdds, t0, t1, t_bitsize);
      break;
    case BV_SHL:
      assert(children_bdds->size == 2);
      t0 = (BDD**) children_bdds->data[0];
      t1 = (BDD**) children_bdds->data[1];
      bdds_mk_shl(cudd, out_bdds, t0, t1, t_bitsize);
      break;
    case BV_LSHR:
      assert(children_bdds->size == 2);
      t0 = (BDD**) children_bdds->data[0];
      t1 = (BDD**) children_bdds->data[1];
      bdds_mk_lshr(cudd, out_bdds, t0, t1, t_bitsize);
      break;
    case BV_ASHR:
      assert(children_bdds->size == 2);
      t0 = (BDD**) children_bdds->data[0];
      t1 = (BDD**) children_bdds->data[1];
      bdds_mk_ashr(cudd, out_bdds, t0, t1, t_bitsize);
      break;
    case BV_ARRAY: {
      assert(children_bdds->size == t_bitsize);
      for (uint32_t i = 0; i < children_bdds->size; ++ i) {
        assert(out_bdds[i] == NULL);
        out_bdds[i] = ((BDD**) children_bdds->data[i])[0];
        Cudd_Ref(out_bdds[i]);
      }
      break;
    }
    case BIT_TERM: {
      assert(t_bitsize == 1);
      assert(children_bdds->size == 1);
      select_term_t* desc = bit_term_desc(terms, t);
      uint32_t select_idx = desc->idx;
      BDD** child_bdds = ((BDD**)children_bdds->data[0]);
      BDD* bit_bdd = child_bdds[select_idx];
      assert(out_bdds[0] == NULL);
      out_bdds[0] = bit_bdd;
      Cudd_Ref(out_bdds[0]);
      break;
    }
    case BV_POLY:
      assert(false);
      break;
    case BV64_POLY:
      assert(false);
      break;
    case POWER_PRODUCT:
      assert(false);
      break;
    case OR_TERM: {
      composite_term_t* t_comp = or_term_desc(terms, t);
      assert(children_bdds->size == t_comp->arity);
      bdds_mk_bool_or(cudd, out_bdds, children_bdds);
      break;
    }
    case EQ_TERM: // Boolean equality
    case BV_EQ_ATOM: {
      assert(children_bdds->size == 2);
      composite_term_t* comp = composite_term_desc(terms, t);
      term_t child = comp->arg[0];
      uint32_t child_bitsize = bv_term_bitsize(terms, child);
      t0 = (BDD**) children_bdds->data[0];
      t1 = (BDD**) children_bdds->data[1];
      bdds_mk_eq(cudd, out_bdds, t0, t1, child_bitsize);
      break;
    }
    case BV_GE_ATOM: {
      assert(children_bdds->size == 2);
      composite_term_t* comp = composite_term_desc(terms, t);
      term_t child = comp->arg[0];
      uint32_t child_bitsize = bv_term_bitsize(terms, child);
      t0 = (BDD**) children_bdds->data[0];
      t1 = (BDD**) children_bdds->data[1];
      bdds_ge(cudd, out_bdds, t0, t1, child_bitsize);
      break;
    }
    case BV_SGE_ATOM: {
      assert(children_bdds->size == 2);
      composite_term_t* comp = composite_term_desc(terms, t);
      term_t child = comp->arg[0];
      uint32_t child_bitsize = bv_term_bitsize(terms, child);
      t0 = (BDD**) children_bdds->data[0];
      t1 = (BDD**) children_bdds->data[1];
      bdds_sge(cudd, out_bdds, t0, t1, child_bitsize);
      break;
    }
    default:
      // Not composite
      assert(false);
      break;
    }
  }
}

void bdds_ge(CUDD* cudd, BDD** out, BDD** a, BDD** b, uint32_t n) {
  assert(n > 0);
  assert(out[0] == NULL);
  // Reverse to satisfy CUDD
  bdds_reverse(a, n);
  bdds_reverse(b, n);
  // a < b
  out[0] = Cudd_Xgty(cudd->cudd, n, NULL, b, a);
  // a >= b
  out[0] = Cudd_Not(out[0]);
  Cudd_Ref(out[0]);
  // Undo reversal
  bdds_reverse(a, n);
  bdds_reverse(b, n);
}

void bdds_sge(CUDD* cudd, BDD** out, BDD** a, BDD** b, uint32_t n) {
  // signed comparison, just invert the first bits
  assert(n > 0);
  a[n-1] = Cudd_Not(a[n-1]);
  b[n-1] = Cudd_Not(b[n-1]);
  bdds_ge(cudd, out, a, b, n);
  a[n-1] = Cudd_Not(a[n-1]);
  b[n-1] = Cudd_Not(b[n-1]);
}

bool bdds_is_point(CUDD* cudd, BDD* a, uint32_t size) {
  bool is_cube = Cudd_CheckCube(cudd->cudd, a);
  if (!is_cube) { return false; }
  int dag_size = Cudd_DagSize(a);
  if (dag_size != size + 1) { return false; }
  return true;
}

bool bdds_is_model(CUDD* cudd, BDD** x, BDD* C_x, const bvconstant_t* out) {
  for (uint32_t i = 0; i < out->bitsize; ++ i) {
    unsigned int x_i = Cudd_NodeReadIndex(x[i]);
    bool bit_i_true = bvconst_tst_bit(out->data, i);
    cudd->tmp_inputs[x_i] = bit_i_true ? 1 : 0;
  }
  return Cudd_Eval(cudd->cudd, C_x, cudd->tmp_inputs) == Cudd_ReadOne(cudd->cudd);
}

void bdds_get_model(CUDD* cudd, BDD** x, BDD* C_x, bvconstant_t* out) {
  Cudd_bddPickOneCube(cudd->cudd, C_x, cudd->tmp_model);
  // Set the ones in the cube
  for (uint32_t i = 0; i < out->bitsize; ++ i) {
    unsigned int x_i = Cudd_NodeReadIndex(x[i]);
    char x_i_value = cudd->tmp_model[x_i];
    if (x_i_value == 1) {
      bvconst_set_bit(out->data, i);
    } else {
      // We just take 0 as the default (if x_i_value == 2, we can choose anything)
      bvconst_clr_bit(out->data, i);
    }
  }
}

