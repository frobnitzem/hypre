/*
 * File:          Hypre_PCG_Skel.c
 * Symbol:        Hypre.PCG-v0.1.5
 * Symbol Type:   class
 * Babel Version: 0.7.4
 * SIDL Created:  20021101 15:14:28 PST
 * Generated:     20021101 15:14:36 PST
 * Description:   Server-side glue code for Hypre.PCG
 * 
 * WARNING: Automatically generated; changes will be lost
 * 
 * babel-version = 0.7.4
 * source-line   = 463
 * source-url    = file:/home/painter/linear_solvers/babel/Interfaces.idl
 */

#include "Hypre_PCG_IOR.h"
#include "Hypre_PCG.h"
#include <stddef.h>

extern void
impl_Hypre_PCG__ctor(
  Hypre_PCG);

extern void
impl_Hypre_PCG__dtor(
  Hypre_PCG);

extern int32_t
impl_Hypre_PCG_SetCommunicator(
  Hypre_PCG,
  void*);

extern int32_t
impl_Hypre_PCG_GetDoubleValue(
  Hypre_PCG,
  const char*,
  double*);

extern int32_t
impl_Hypre_PCG_GetIntValue(
  Hypre_PCG,
  const char*,
  int32_t*);

extern int32_t
impl_Hypre_PCG_SetDoubleParameter(
  Hypre_PCG,
  const char*,
  double);

extern int32_t
impl_Hypre_PCG_SetIntParameter(
  Hypre_PCG,
  const char*,
  int32_t);

extern int32_t
impl_Hypre_PCG_SetStringParameter(
  Hypre_PCG,
  const char*,
  const char*);

extern int32_t
impl_Hypre_PCG_SetIntArrayParameter(
  Hypre_PCG,
  const char*,
  struct SIDL_int__array*);

extern int32_t
impl_Hypre_PCG_SetDoubleArrayParameter(
  Hypre_PCG,
  const char*,
  struct SIDL_double__array*);

extern int32_t
impl_Hypre_PCG_Setup(
  Hypre_PCG,
  Hypre_Vector,
  Hypre_Vector);

extern int32_t
impl_Hypre_PCG_Apply(
  Hypre_PCG,
  Hypre_Vector,
  Hypre_Vector*);

extern int32_t
impl_Hypre_PCG_SetOperator(
  Hypre_PCG,
  Hypre_Operator);

extern int32_t
impl_Hypre_PCG_GetResidual(
  Hypre_PCG,
  Hypre_Vector*);

extern int32_t
impl_Hypre_PCG_SetLogging(
  Hypre_PCG,
  int32_t);

extern int32_t
impl_Hypre_PCG_SetPrintLevel(
  Hypre_PCG,
  int32_t);

extern int32_t
impl_Hypre_PCG_SetPreconditioner(
  Hypre_PCG,
  Hypre_Solver);

extern int32_t
impl_Hypre_PCG_GetPreconditionedResidual(
  Hypre_PCG,
  Hypre_Vector*);

void
Hypre_PCG__set_epv(struct Hypre_PCG__epv *epv)
{
  epv->f__ctor = impl_Hypre_PCG__ctor;
  epv->f__dtor = impl_Hypre_PCG__dtor;
  epv->f_SetCommunicator = impl_Hypre_PCG_SetCommunicator;
  epv->f_GetDoubleValue = impl_Hypre_PCG_GetDoubleValue;
  epv->f_GetIntValue = impl_Hypre_PCG_GetIntValue;
  epv->f_SetDoubleParameter = impl_Hypre_PCG_SetDoubleParameter;
  epv->f_SetIntParameter = impl_Hypre_PCG_SetIntParameter;
  epv->f_SetStringParameter = impl_Hypre_PCG_SetStringParameter;
  epv->f_SetIntArrayParameter = impl_Hypre_PCG_SetIntArrayParameter;
  epv->f_SetDoubleArrayParameter = impl_Hypre_PCG_SetDoubleArrayParameter;
  epv->f_Setup = impl_Hypre_PCG_Setup;
  epv->f_Apply = impl_Hypre_PCG_Apply;
  epv->f_SetOperator = impl_Hypre_PCG_SetOperator;
  epv->f_GetResidual = impl_Hypre_PCG_GetResidual;
  epv->f_SetLogging = impl_Hypre_PCG_SetLogging;
  epv->f_SetPrintLevel = impl_Hypre_PCG_SetPrintLevel;
  epv->f_SetPreconditioner = impl_Hypre_PCG_SetPreconditioner;
  epv->f_GetPreconditionedResidual = impl_Hypre_PCG_GetPreconditionedResidual;
}

struct Hypre_PCG__data*
Hypre_PCG__get_data(Hypre_PCG self)
{
  return (struct Hypre_PCG__data*)(self ? self->d_data : NULL);
}

void Hypre_PCG__set_data(
  Hypre_PCG self,
  struct Hypre_PCG__data* data)
{
  if (self) {
    self->d_data = data;
  }
}
