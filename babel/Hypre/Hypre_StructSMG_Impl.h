/*
 * File:          Hypre_StructSMG_Impl.h
 * Symbol:        Hypre.StructSMG-v0.1.5
 * Symbol Type:   class
 * Babel Version: 0.7.4
 * SIDL Created:  20021101 15:14:28 PST
 * Generated:     20021101 15:14:37 PST
 * Description:   Server-side implementation for Hypre.StructSMG
 * 
 * WARNING: Automatically generated; only changes within splicers preserved
 * 
 * babel-version = 0.7.4
 * source-line   = 457
 * source-url    = file:/home/painter/linear_solvers/babel/Interfaces.idl
 */

#ifndef included_Hypre_StructSMG_Impl_h
#define included_Hypre_StructSMG_Impl_h

#ifndef included_SIDL_header_h
#include "SIDL_header.h"
#endif
#ifndef included_Hypre_Vector_h
#include "Hypre_Vector.h"
#endif
#ifndef included_Hypre_Operator_h
#include "Hypre_Operator.h"
#endif
#ifndef included_Hypre_StructSMG_h
#include "Hypre_StructSMG.h"
#endif

/* DO-NOT-DELETE splicer.begin(Hypre.StructSMG._includes) */
/* Put additional include files here... */
/* DO-NOT-DELETE splicer.end(Hypre.StructSMG._includes) */

/*
 * Private data for class Hypre.StructSMG
 */

struct Hypre_StructSMG__data {
  /* DO-NOT-DELETE splicer.begin(Hypre.StructSMG._data) */
  /* Put private data members here... */
  int ignore; /* dummy to force non-empty struct; remove if you add data */
  /* DO-NOT-DELETE splicer.end(Hypre.StructSMG._data) */
};

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Access functions for class private data and built-in methods
 */

extern struct Hypre_StructSMG__data*
Hypre_StructSMG__get_data(
  Hypre_StructSMG);

extern void
Hypre_StructSMG__set_data(
  Hypre_StructSMG,
  struct Hypre_StructSMG__data*);

extern void
impl_Hypre_StructSMG__ctor(
  Hypre_StructSMG);

extern void
impl_Hypre_StructSMG__dtor(
  Hypre_StructSMG);

/*
 * User-defined object methods
 */

extern int32_t
impl_Hypre_StructSMG_SetCommunicator(
  Hypre_StructSMG,
  void*);

extern int32_t
impl_Hypre_StructSMG_GetDoubleValue(
  Hypre_StructSMG,
  const char*,
  double*);

extern int32_t
impl_Hypre_StructSMG_GetIntValue(
  Hypre_StructSMG,
  const char*,
  int32_t*);

extern int32_t
impl_Hypre_StructSMG_SetDoubleParameter(
  Hypre_StructSMG,
  const char*,
  double);

extern int32_t
impl_Hypre_StructSMG_SetIntParameter(
  Hypre_StructSMG,
  const char*,
  int32_t);

extern int32_t
impl_Hypre_StructSMG_SetStringParameter(
  Hypre_StructSMG,
  const char*,
  const char*);

extern int32_t
impl_Hypre_StructSMG_SetIntArrayParameter(
  Hypre_StructSMG,
  const char*,
  struct SIDL_int__array*);

extern int32_t
impl_Hypre_StructSMG_SetDoubleArrayParameter(
  Hypre_StructSMG,
  const char*,
  struct SIDL_double__array*);

extern int32_t
impl_Hypre_StructSMG_Setup(
  Hypre_StructSMG,
  Hypre_Vector,
  Hypre_Vector);

extern int32_t
impl_Hypre_StructSMG_Apply(
  Hypre_StructSMG,
  Hypre_Vector,
  Hypre_Vector*);

extern int32_t
impl_Hypre_StructSMG_SetOperator(
  Hypre_StructSMG,
  Hypre_Operator);

extern int32_t
impl_Hypre_StructSMG_GetResidual(
  Hypre_StructSMG,
  Hypre_Vector*);

extern int32_t
impl_Hypre_StructSMG_SetLogging(
  Hypre_StructSMG,
  int32_t);

extern int32_t
impl_Hypre_StructSMG_SetPrintLevel(
  Hypre_StructSMG,
  int32_t);

#ifdef __cplusplus
}
#endif
#endif
