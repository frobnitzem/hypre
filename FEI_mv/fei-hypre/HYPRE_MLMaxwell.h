/*BHEADER**********************************************************************
 * (c) 2006   The Regents of the University of California
 *
 * See the file COPYRIGHT_and_DISCLAIMER for a complete copyright
 * notice, contact person, and disclaimer.
 *
 *********************************************************************EHEADER*/

/****************************************************************************/ 
/* data structures for local matrix and ml object                           */
/*--------------------------------------------------------------------------*/

#ifndef _MLMAXWELL_
#define _MLMAXWELL_

#ifdef HAVE_MLMAXWELL
#include "ml_include.h"
#endif

typedef struct
{
    HYPRE_ML_Matrix *Amat;
    MPI_Comm comm;
    int globalEqns;
    int *partition;
}
MLMaxwell_Context;
    
typedef struct
{
    MPI_Comm comm;
#ifdef HAVE_MLMAXWELL
    ML       *ml_ee;
    ML       *ml_nn;
#endif
    int      nlevels;
    int      smoothP_flag;
    double   ag_threshold;
    void     *edge_smoother;
    void     *node_smoother;
#ifdef HAVE_MLMAXWELL
    ML_Aggregate *ml_ag;
    ML_Operator  *Gmat;
    ML_Operator  *GTmat;
    ML_Operator  **Gmat_array;
    ML_Operator  **GTmat_array;

#endif
    MLMaxwell_Context *Aee_contxt;
    MLMaxwell_Context *Amm_contxt;
    MLMaxwell_Context *Ann_contxt;
    user_Gmat_data    *Gmat_data;
    void              **node_args;
    void              **edge_args;
} 
MLMaxwell_Link;

#endif

