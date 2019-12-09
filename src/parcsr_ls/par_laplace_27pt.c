/******************************************************************************
 * Copyright 1998-2019 Lawrence Livermore National Security, LLC and other
 * HYPRE Project Developers. See the top-level COPYRIGHT file for details.
 *
 * SPDX-License-Identifier: (Apache-2.0 OR MIT)
 ******************************************************************************/

#include "_hypre_parcsr_ls.h"

/*--------------------------------------------------------------------------
 * hypre_GenerateLaplacian27pt
 *--------------------------------------------------------------------------*/

HYPRE_ParCSRMatrix
GenerateLaplacian27pt(MPI_Comm comm,
                      HYPRE_BigInt   nx,
                      HYPRE_BigInt   ny,
                      HYPRE_BigInt   nz,
                      HYPRE_Int      P,
                      HYPRE_Int      Q,
                      HYPRE_Int      R,
                      HYPRE_Int      p,
                      HYPRE_Int      q,
                      HYPRE_Int      r,
                      HYPRE_Real  *value )
{
   hypre_ParCSRMatrix *A;
   hypre_CSRMatrix *diag;
   hypre_CSRMatrix *offd;

   HYPRE_Int    *diag_i;
   HYPRE_Int    *diag_j;
   HYPRE_Real *diag_data;

   HYPRE_Int    *offd_i;
   HYPRE_Int    *offd_j;
   HYPRE_BigInt *big_offd_j = NULL;
   HYPRE_Real *offd_data;

   HYPRE_BigInt *global_part;
   HYPRE_BigInt ix, iy, iz;
   HYPRE_Int cnt, o_cnt;
   HYPRE_Int local_num_rows;
   HYPRE_BigInt *col_map_offd;
   HYPRE_BigInt *work;
   HYPRE_Int row_index;
   HYPRE_Int i;

   HYPRE_Int nx_local, ny_local, nz_local;
   HYPRE_Int num_cols_offd;
   HYPRE_Int nxy;
   HYPRE_BigInt grid_size;

   HYPRE_BigInt *nx_part;
   HYPRE_BigInt *ny_part;
   HYPRE_BigInt *nz_part;

   HYPRE_Int num_procs;
   HYPRE_Int P_busy, Q_busy, R_busy;

   hypre_MPI_Comm_size(comm,&num_procs);

   grid_size = nx*ny*nz;

   hypre_GeneratePartitioning(nx,P,&nx_part);
   hypre_GeneratePartitioning(ny,Q,&ny_part);
   hypre_GeneratePartitioning(nz,R,&nz_part);

   nx_local = (HYPRE_Int)(nx_part[p+1] - nx_part[p]);
   ny_local = (HYPRE_Int)(ny_part[q+1] - ny_part[q]);
   nz_local = (HYPRE_Int)(nz_part[r+1] - nz_part[r]);

   local_num_rows = nx_local*ny_local*nz_local;

#ifdef HYPRE_NO_GLOBAL_PARTITION

   global_part = hypre_CTAlloc(HYPRE_BigInt, 2, HYPRE_MEMORY_HOST);
   global_part[0] = nz_part[r]*nx*ny+(ny_part[q]*nx+nx_part[p]*ny_local)*nz_local;
   global_part[1] = global_part[0] + (HYPRE_BigInt)local_num_rows;

#else
   HYPRE_Int nx_size, ny_size, nz_size;
   HYPRE_Int jx, jy, jz;

   global_part = hypre_CTAlloc(HYPRE_BigInt, P*Q*R+1, HYPRE_MEMORY_HOST);

   global_part[0] = 0;
   cnt = 1;
   for (jz = 0; jz < R; jz++)
   {
      nz_size = (HYPRE_Int)(nz_part[jz+1]-nz_part[jz]);
      for (jy = 0; jy < Q; jy++)
      {
         ny_size = (HYPRE_Int)(ny_part[jy+1]-ny_part[jy])*nz_size;
         for (jx = 0; jx < P; jx++)
         {
            nx_size = (HYPRE_Int)(nx_part[jx+1] - nx_part[jx]);
            global_part[cnt] = global_part[cnt-1];
            global_part[cnt++] += (HYPRE_BigInt)(nx_size*ny_size);
         }
      }
   }

#endif

   diag_i = hypre_CTAlloc(HYPRE_Int,  local_num_rows+1, HYPRE_MEMORY_HOST);
   offd_i = hypre_CTAlloc(HYPRE_Int,  local_num_rows+1, HYPRE_MEMORY_HOST);

   P_busy = hypre_min(nx,P);
   Q_busy = hypre_min(ny,Q);
   R_busy = hypre_min(nz,R);

   num_cols_offd = 0;
   if (p) num_cols_offd += ny_local*nz_local;
   if (p < P_busy-1) num_cols_offd += ny_local*nz_local;
   if (q) num_cols_offd += nx_local*nz_local;
   if (q < Q_busy-1) num_cols_offd += nx_local*nz_local;
   if (r) num_cols_offd += nx_local*ny_local;
   if (r < R_busy-1) num_cols_offd += nx_local*ny_local;
   if (p && q) num_cols_offd += nz_local;
   if (p && q < Q_busy-1 ) num_cols_offd += nz_local;
   if (p < P_busy-1 && q ) num_cols_offd += nz_local;
   if (p < P_busy-1 && q < Q_busy-1 ) num_cols_offd += nz_local;
   if (p && r) num_cols_offd += ny_local;
   if (p && r < R_busy-1 ) num_cols_offd += ny_local;
   if (p < P_busy-1 && r ) num_cols_offd += ny_local;
   if (p < P_busy-1 && r < R_busy-1 ) num_cols_offd += ny_local;
   if (q && r) num_cols_offd += nx_local;
   if (q && r < R_busy-1 ) num_cols_offd += nx_local;
   if (q < Q_busy-1 && r ) num_cols_offd += nx_local;
   if (q < Q_busy-1 && r < R_busy-1 ) num_cols_offd += nx_local;
   if (p && q && r) num_cols_offd++;
   if (p && q && r < R_busy-1) num_cols_offd++;
   if (p && q < Q_busy-1 && r) num_cols_offd++;
   if (p && q < Q_busy-1 && r < R_busy-1) num_cols_offd++;
   if (p < P_busy-1 && q && r) num_cols_offd++;
   if (p < P_busy-1 && q && r < R_busy-1 ) num_cols_offd++;
   if (p < P_busy-1 && q < Q_busy-1 && r ) num_cols_offd++;
   if (p < P_busy-1 && q < Q_busy-1 && r < R_busy-1) num_cols_offd++;

   if (!local_num_rows) num_cols_offd = 0;

   col_map_offd = hypre_CTAlloc(HYPRE_BigInt, num_cols_offd, HYPRE_MEMORY_HOST);

   cnt = 0;
   o_cnt = 0;
   diag_i[0] = 0;
   offd_i[0] = 0;
   for (iz = nz_part[r];  iz < nz_part[r+1]; iz++)
   {
      for (iy = ny_part[q];  iy < ny_part[q+1]; iy++)
      {
         for (ix = nx_part[p]; ix < nx_part[p+1]; ix++)
         {
            cnt++;
            o_cnt++;
            diag_i[cnt] = diag_i[cnt-1];
            offd_i[o_cnt] = offd_i[o_cnt-1];
            diag_i[cnt]++;
            if (iz > nz_part[r])
            {
               diag_i[cnt]++;
               if (iy > ny_part[q])
               {
                  diag_i[cnt]++;
                  if (ix > nx_part[p])
                  {
                     diag_i[cnt]++;
                  }
                  else
                  {
                     if (ix)
                        offd_i[o_cnt]++;
                  }
                  if (ix < nx_part[p+1]-1)
                  {
                     diag_i[cnt]++;
                  }
                  else
                  {
                     if (ix+1 < nx)
                        offd_i[o_cnt]++;
                  }
               }
               else
               {
                  if (iy)
                  {
                     offd_i[o_cnt]++;
                     if (ix > nx_part[p])
                     {
                        offd_i[o_cnt]++;
                     }
                     else if (ix)
                     {
                        offd_i[o_cnt]++;
                     }
                     if (ix < nx_part[p+1]-1)
                     {
                        offd_i[o_cnt]++;
                     }
                     else if (ix < nx-1)
                     {
                        offd_i[o_cnt]++;
                     }
                  }
               }
               if (ix > nx_part[p])
                  diag_i[cnt]++;
               else
               {
                  if (ix)
                  {
                     offd_i[o_cnt]++;
                  }
               }
               if (ix+1 < nx_part[p+1])
                  diag_i[cnt]++;
               else
               {
                  if (ix+1 < nx)
                  {
                     offd_i[o_cnt]++;
                  }
               }
               if (iy+1 < ny_part[q+1])
               {
                  diag_i[cnt]++;
                  if (ix > nx_part[p])
                  {
                     diag_i[cnt]++;
                  }
                  else
                  {
                     if (ix)
                        offd_i[o_cnt]++;
                  }
                  if (ix < nx_part[p+1]-1)
                  {
                     diag_i[cnt]++;
                  }
                  else
                  {
                     if (ix+1 < nx)
                        offd_i[o_cnt]++;
                  }
               }
               else
               {
                  if (iy+1 < ny)
                  {
                     offd_i[o_cnt]++;
                     if (ix > nx_part[p])
                     {
                        offd_i[o_cnt]++;
                     }
                     else if (ix)
                     {
                        offd_i[o_cnt]++;
                     }
                     if (ix < nx_part[p+1]-1)
                     {
                        offd_i[o_cnt]++;
                     }
                     else if (ix < nx-1)
                     {
                        offd_i[o_cnt]++;
                     }
                  }
               }
            }
            else
            {
               if (iz)
               {
                  offd_i[o_cnt]++;
                  if (iy > ny_part[q])
                  {
                     offd_i[o_cnt]++;
                     if (ix > nx_part[p])
                     {
                        offd_i[o_cnt]++;
                     }
                     else
                     {
                        if (ix)
                           offd_i[o_cnt]++;
                     }
                     if (ix < nx_part[p+1]-1)
                     {
                        offd_i[o_cnt]++;
                     }
                     else
                     {
                        if (ix+1 < nx)
                           offd_i[o_cnt]++;
                     }
                  }
                  else
                  {
                     if (iy)
                     {
                        offd_i[o_cnt]++;
                        if (ix > nx_part[p])
                        {
                           offd_i[o_cnt]++;
                        }
                        else if (ix)
                        {
                           offd_i[o_cnt]++;
                        }
                        if (ix < nx_part[p+1]-1)
                        {
                           offd_i[o_cnt]++;
                        }
                        else if (ix < nx-1)
                        {
                           offd_i[o_cnt]++;
                        }
                     }
                  }
                  if (ix > nx_part[p])
                     offd_i[o_cnt]++;
                  else
                  {
                     if (ix)
                     {
                        offd_i[o_cnt]++;
                     }
                  }
                  if (ix+1 < nx_part[p+1])
                     offd_i[o_cnt]++;
                  else
                  {
                     if (ix+1 < nx)
                     {
                        offd_i[o_cnt]++;
                     }
                  }
                  if (iy+1 < ny_part[q+1])
                  {
                     offd_i[o_cnt]++;
                     if (ix > nx_part[p])
                     {
                        offd_i[o_cnt]++;
                     }
                     else
                     {
                        if (ix)
                           offd_i[o_cnt]++;
                     }
                     if (ix < nx_part[p+1]-1)
                     {
                        offd_i[o_cnt]++;
                     }
                     else
                     {
                        if (ix+1 < nx)
                           offd_i[o_cnt]++;
                     }
                  }
                  else
                  {
                     if (iy+1 < ny)
                     {
                        offd_i[o_cnt]++;
                        if (ix > nx_part[p])
                        {
                           offd_i[o_cnt]++;
                        }
                        else if (ix)
                        {
                           offd_i[o_cnt]++;
                        }
                        if (ix < nx_part[p+1]-1)
                        {
                           offd_i[o_cnt]++;
                        }
                        else if (ix < nx-1)
                        {
                           offd_i[o_cnt]++;
                        }
                     }
                  }
               }
            }
            if (iy > ny_part[q])
            {
               diag_i[cnt]++;
               if (ix > nx_part[p])
               {
                  diag_i[cnt]++;
               }
               else
               {
                  if (ix)
                     offd_i[o_cnt]++;
               }
               if (ix < nx_part[p+1]-1)
               {
                  diag_i[cnt]++;
               }
               else
               {
                  if (ix+1 < nx)
                     offd_i[o_cnt]++;
               }
            }
            else
            {
               if (iy)
               {
                  offd_i[o_cnt]++;
                  if (ix > nx_part[p])
                  {
                     offd_i[o_cnt]++;
                  }
                  else if (ix)
                  {
                     offd_i[o_cnt]++;
                  }
                  if (ix < nx_part[p+1]-1)
                  {
                     offd_i[o_cnt]++;
                  }
                  else if (ix < nx-1)
                  {
                     offd_i[o_cnt]++;
                  }
               }
            }
            if (ix > nx_part[p])
               diag_i[cnt]++;
            else
            {
               if (ix)
               {
                  offd_i[o_cnt]++;
               }
            }
            if (ix+1 < nx_part[p+1])
               diag_i[cnt]++;
            else
            {
               if (ix+1 < nx)
               {
                  offd_i[o_cnt]++;
               }
            }
            if (iy+1 < ny_part[q+1])
            {
               diag_i[cnt]++;
               if (ix > nx_part[p])
               {
                  diag_i[cnt]++;
               }
               else
               {
                  if (ix)
                     offd_i[o_cnt]++;
               }
               if (ix < nx_part[p+1]-1)
               {
                  diag_i[cnt]++;
               }
               else
               {
                  if (ix+1 < nx)
                     offd_i[o_cnt]++;
               }
            }
            else
            {
               if (iy+1 < ny)
               {
                  offd_i[o_cnt]++;
                  if (ix > nx_part[p])
                  {
                     offd_i[o_cnt]++;
                  }
                  else if (ix)
                  {
                     offd_i[o_cnt]++;
                  }
                  if (ix < nx_part[p+1]-1)
                  {
                     offd_i[o_cnt]++;
                  }
                  else if (ix < nx-1)
                  {
                     offd_i[o_cnt]++;
                  }
               }
            }
            if (iz+1 < nz_part[r+1])
            {
               diag_i[cnt]++;
               if (iy > ny_part[q])
               {
                  diag_i[cnt]++;
                  if (ix > nx_part[p])
                  {
                     diag_i[cnt]++;
                  }
                  else
                  {
                     if (ix)
                        offd_i[o_cnt]++;
                  }
                  if (ix < nx_part[p+1]-1)
                  {
                     diag_i[cnt]++;
                  }
                  else
                  {
                     if (ix+1 < nx)
                        offd_i[o_cnt]++;
                  }
               }
               else
               {
                  if (iy)
                  {
                     offd_i[o_cnt]++;
                     if (ix > nx_part[p])
                     {
                        offd_i[o_cnt]++;
                     }
                     else if (ix)
                     {
                        offd_i[o_cnt]++;
                     }
                     if (ix < nx_part[p+1]-1)
                     {
                        offd_i[o_cnt]++;
                     }
                     else if (ix < nx-1)
                     {
                        offd_i[o_cnt]++;
                     }
                  }
               }
               if (ix > nx_part[p])
                  diag_i[cnt]++;
               else
               {
                  if (ix)
                  {
                     offd_i[o_cnt]++;
                  }
               }
               if (ix+1 < nx_part[p+1])
                  diag_i[cnt]++;
               else
               {
                  if (ix+1 < nx)
                  {
                     offd_i[o_cnt]++;
                  }
               }
               if (iy+1 < ny_part[q+1])
               {
                  diag_i[cnt]++;
                  if (ix > nx_part[p])
                  {
                     diag_i[cnt]++;
                  }
                  else
                  {
                     if (ix)
                        offd_i[o_cnt]++;
                  }
                  if (ix < nx_part[p+1]-1)
                  {
                     diag_i[cnt]++;
                  }
                  else
                  {
                     if (ix+1 < nx)
                        offd_i[o_cnt]++;
                  }
               }
               else
               {
                  if (iy+1 < ny)
                  {
                     offd_i[o_cnt]++;
                     if (ix > nx_part[p])
                     {
                        offd_i[o_cnt]++;
                     }
                     else if (ix)
                     {
                        offd_i[o_cnt]++;
                     }
                     if (ix < nx_part[p+1]-1)
                     {
                        offd_i[o_cnt]++;
                     }
                     else if (ix < nx-1)
                     {
                        offd_i[o_cnt]++;
                     }
                  }
               }
            }
            else
            {
               if (iz+1 < nz)
               {
                  offd_i[o_cnt]++;
                  if (iy > ny_part[q])
                  {
                     offd_i[o_cnt]++;
                     if (ix > nx_part[p])
                     {
                        offd_i[o_cnt]++;
                     }
                     else
                     {
                        if (ix)
                           offd_i[o_cnt]++;
                     }
                     if (ix < nx_part[p+1]-1)
                     {
                        offd_i[o_cnt]++;
                     }
                     else
                     {
                        if (ix+1 < nx)
                           offd_i[o_cnt]++;
                     }
                  }
                  else
                  {
                     if (iy)
                     {
                        offd_i[o_cnt]++;
                        if (ix > nx_part[p])
                        {
                           offd_i[o_cnt]++;
                        }
                        else if (ix)
                        {
                           offd_i[o_cnt]++;
                        }
                        if (ix < nx_part[p+1]-1)
                        {
                           offd_i[o_cnt]++;
                        }
                        else if (ix < nx-1)
                        {
                           offd_i[o_cnt]++;
                        }
                     }
                  }
                  if (ix > nx_part[p])
                     offd_i[o_cnt]++;
                  else
                  {
                     if (ix)
                     {
                        offd_i[o_cnt]++;
                     }
                  }
                  if (ix+1 < nx_part[p+1])
                     offd_i[o_cnt]++;
                  else
                  {
                     if (ix+1 < nx)
                     {
                        offd_i[o_cnt]++;
                     }
                  }
                  if (iy+1 < ny_part[q+1])
                  {
                     offd_i[o_cnt]++;
                     if (ix > nx_part[p])
                     {
                        offd_i[o_cnt]++;
                     }
                     else
                     {
                        if (ix)
                           offd_i[o_cnt]++;
                     }
                     if (ix < nx_part[p+1]-1)
                     {
                        offd_i[o_cnt]++;
                     }
                     else
                     {
                        if (ix+1 < nx)
                           offd_i[o_cnt]++;
                     }
                  }
                  else
                  {
                     if (iy+1 < ny)
                     {
                        offd_i[o_cnt]++;
                        if (ix > nx_part[p])
                        {
                           offd_i[o_cnt]++;
                        }
                        else if (ix)
                        {
                           offd_i[o_cnt]++;
                        }
                        if (ix < nx_part[p+1]-1)
                        {
                           offd_i[o_cnt]++;
                        }
                        else if (ix < nx-1)
                        {
                           offd_i[o_cnt]++;
                        }
                     }
                  }
               }
            }
         }
      }
   }

   diag_j = hypre_CTAlloc(HYPRE_Int,  diag_i[local_num_rows], HYPRE_MEMORY_HOST);
   diag_data = hypre_CTAlloc(HYPRE_Real,  diag_i[local_num_rows], HYPRE_MEMORY_HOST);

   if (num_procs > 1)
   {
      big_offd_j = hypre_CTAlloc(HYPRE_BigInt, offd_i[local_num_rows], HYPRE_MEMORY_HOST);
      offd_j = hypre_CTAlloc(HYPRE_Int,  offd_i[local_num_rows], HYPRE_MEMORY_HOST);
      offd_data = hypre_CTAlloc(HYPRE_Real,  offd_i[local_num_rows], HYPRE_MEMORY_HOST);
   }

   nxy = nx_local*ny_local;
   row_index = 0;
   cnt = 0;
   o_cnt = 0;
   for (iz = nz_part[r];  iz < nz_part[r+1]; iz++)
   {
      for (iy = ny_part[q];  iy < ny_part[q+1]; iy++)
      {
         for (ix = nx_part[p]; ix < nx_part[p+1]; ix++)
         {
            diag_j[cnt] = row_index;
            diag_data[cnt++] = value[0];
            if (iz > nz_part[r])
            {
               if (iy > ny_part[q])
               {
                  if (ix > nx_part[p])
                  {
                     diag_j[cnt] = row_index-nxy-nx_local-1;
                     diag_data[cnt++] = value[1];
                  }
                  else
                  {
                     if (ix)
                     {
                        big_offd_j[o_cnt] = hypre_map(ix-1,iy-1,iz-1,p-1,q,r,nx,ny,
                              nx_part,ny_part,nz_part);
                        offd_data[o_cnt++] = value[1];
                     }
                  }
                  diag_j[cnt] = row_index-nxy-nx_local;
                  diag_data[cnt++] = value[1];
                  if (ix < nx_part[p+1]-1)
                  {
                     diag_j[cnt] = row_index-nxy-nx_local+1;
                     diag_data[cnt++] = value[1];
                  }
                  else
                  {
                     if (ix+1 < nx)
                     {
                        big_offd_j[o_cnt] = hypre_map(ix+1,iy-1,iz-1,p+1,q,r,nx,ny,
                              nx_part,ny_part,nz_part);
                        offd_data[o_cnt++] = value[1];
                     }
                  }
               }
               else
               {
                  if (iy)
                  {
                     if (ix > nx_part[p])
                     {
                        big_offd_j[o_cnt] = hypre_map(ix-1,iy-1,iz-1,p,q-1,r,nx,ny,
                              nx_part,ny_part,nz_part);
                        offd_data[o_cnt++] = value[1];
                     }
                     else if (ix)
                     {
                        big_offd_j[o_cnt] = hypre_map(ix-1,iy-1,iz-1,p-1,q-1,r,nx,ny,
                              nx_part,ny_part,nz_part);
                        offd_data[o_cnt++] = value[1];
                     }
                     big_offd_j[o_cnt] = hypre_map(ix,iy-1,iz-1,p,q-1,r,nx,ny,
                           nx_part,ny_part,nz_part);
                     offd_data[o_cnt++] = value[1];
                     if (ix < nx_part[p+1]-1)
                     {
                        big_offd_j[o_cnt] = hypre_map(ix+1,iy-1,iz-1,p,q-1,r,nx,ny,
                              nx_part,ny_part,nz_part);
                        offd_data[o_cnt++] = value[1];
                     }
                     else if (ix < nx-1)
                     {
                        big_offd_j[o_cnt] = hypre_map(ix+1,iy-1,iz-1,p+1,q-1,r,nx,ny,
                              nx_part,ny_part,nz_part);
                        offd_data[o_cnt++] = value[1];
                     }
                  }
               }
               if (ix > nx_part[p])
               {
                  diag_j[cnt] = row_index-nxy-1;
                  diag_data[cnt++] = value[1];
               }
               else
               {
                  if (ix)
                  {
                     big_offd_j[o_cnt] = hypre_map(ix-1,iy,iz-1,p-1,q,r,nx,ny,
                           nx_part,ny_part,nz_part);
                     offd_data[o_cnt++] = value[1];
                  }
               }
               diag_j[cnt] = row_index-nxy;
               diag_data[cnt++] = value[1];
               if (ix+1 < nx_part[p+1])
               {
                  diag_j[cnt] = row_index-nxy+1;
                  diag_data[cnt++] = value[1];
               }
               else
               {
                  if (ix+1 < nx)
                  {
                     big_offd_j[o_cnt] = hypre_map(ix+1,iy,iz-1,p+1,q,r,nx,ny,
                           nx_part,ny_part,nz_part);
                     offd_data[o_cnt++] = value[1];
                  }
               }
               if (iy+1 < ny_part[q+1])
               {
                  if (ix > nx_part[p])
                  {
                     diag_j[cnt] = row_index-nxy+nx_local-1;
                     diag_data[cnt++] = value[1];
                  }
                  else
                  {
                     if (ix)
                     {
                        big_offd_j[o_cnt] = hypre_map(ix-1,iy+1,iz-1,p-1,q,r,nx,ny,
                              nx_part,ny_part,nz_part);
                        offd_data[o_cnt++] = value[1];
                     }
                  }
                  diag_j[cnt] = row_index-nxy+nx_local;
                  diag_data[cnt++] = value[1];
                  if (ix < nx_part[p+1]-1)
                  {
                     diag_j[cnt] = row_index-nxy+nx_local+1;
                     diag_data[cnt++] = value[1];
                  }
                  else
                  {
                     if (ix+1 < nx)
                     {
                        big_offd_j[o_cnt] = hypre_map(ix+1,iy+1,iz-1,p+1,q,r,nx,ny,
                              nx_part,ny_part,nz_part);
                        offd_data[o_cnt++] = value[1];
                     }
                  }
               }
               else
               {
                  if (iy+1 < ny)
                  {
                     if (ix > nx_part[p])
                     {
                        big_offd_j[o_cnt] = hypre_map(ix-1,iy+1,iz-1,p,q+1,r,nx,ny,
                              nx_part,ny_part,nz_part);
                        offd_data[o_cnt++] = value[1];
                     }
                     else if (ix)
                     {
                        big_offd_j[o_cnt] = hypre_map(ix-1,iy+1,iz-1,p-1,q+1,r,nx,ny,
                              nx_part,ny_part,nz_part);
                        offd_data[o_cnt++] = value[1];
                     }
                     big_offd_j[o_cnt] = hypre_map(ix,iy+1,iz-1,p,q+1,r,nx,ny,
                           nx_part,ny_part,nz_part);
                     offd_data[o_cnt++] = value[1];
                     if (ix < nx_part[p+1]-1)
                     {
                        big_offd_j[o_cnt] = hypre_map(ix+1,iy+1,iz-1,p,q+1,r,nx,ny,
                              nx_part,ny_part,nz_part);
                        offd_data[o_cnt++] = value[1];
                     }
                     else if (ix < nx-1)
                     {
                        big_offd_j[o_cnt] = hypre_map(ix+1,iy+1,iz-1,p+1,q+1,r,nx,ny,
                              nx_part,ny_part,nz_part);
                        offd_data[o_cnt++] = value[1];
                     }
                  }
               }
            }
            else
            {
               if (iz)
               {
                  if (iy > ny_part[q])
                  {
                     if (ix > nx_part[p])
                     {
                        big_offd_j[o_cnt] = hypre_map(ix-1,iy-1,iz-1,p,q,r-1,nx,ny,
                              nx_part,ny_part,nz_part);
                        offd_data[o_cnt++] = value[1];
                     }
                     else
                     {
                        if (ix)
                        {
                           big_offd_j[o_cnt] = hypre_map(ix-1,iy-1,iz-1,p-1,q,r-1,nx,ny,
                                 nx_part,ny_part,nz_part);
                           offd_data[o_cnt++] = value[1];
                        }
                     }
                     big_offd_j[o_cnt] = hypre_map(ix,iy-1,iz-1,p,q,r-1,nx,ny,
                           nx_part,ny_part,nz_part);
                     offd_data[o_cnt++] = value[1];
                     if (ix < nx_part[p+1]-1)
                     {
                        big_offd_j[o_cnt] = hypre_map(ix+1,iy-1,iz-1,p,q,r-1,nx,ny,
                              nx_part,ny_part,nz_part);
                        offd_data[o_cnt++] = value[1];
                     }
                     else
                     {
                        if (ix+1 < nx)
                        {
                           big_offd_j[o_cnt] = hypre_map(ix+1,iy-1,iz-1,p+1,q,r-1,nx,ny,
                                 nx_part,ny_part,nz_part);
                           offd_data[o_cnt++] = value[1];
                        }
                     }
                  }
                  else
                  {
                     if (iy)
                     {
                        if (ix > nx_part[p])
                        {
                           big_offd_j[o_cnt] = hypre_map(ix-1,iy-1,iz-1,p,q-1,r-1,nx,ny,
                                 nx_part,ny_part,nz_part);
                           offd_data[o_cnt++] = value[1];
                        }
                        else if (ix)
                        {
                           big_offd_j[o_cnt] =hypre_map(ix-1,iy-1,iz-1,p-1,q-1,r-1,nx,ny,
                                 nx_part,ny_part,nz_part);
                           offd_data[o_cnt++] = value[1];
                        }
                        big_offd_j[o_cnt] = hypre_map(ix,iy-1,iz-1,p,q-1,r-1,nx,ny,
                              nx_part,ny_part,nz_part);
                        offd_data[o_cnt++] = value[1];
                        if (ix < nx_part[p+1]-1)
                        {
                           big_offd_j[o_cnt] = hypre_map(ix+1,iy-1,iz-1,p,q-1,r-1,nx,ny,
                                 nx_part,ny_part,nz_part);
                           offd_data[o_cnt++] = value[1];
                        }
                        else if (ix < nx-1)
                        {
                           big_offd_j[o_cnt] =hypre_map(ix+1,iy-1,iz-1,p+1,q-1,r-1,nx,ny,
                                 nx_part,ny_part,nz_part);
                           offd_data[o_cnt++] = value[1];
                        }
                     }
                  }
                  if (ix > nx_part[p])
                  {
                     big_offd_j[o_cnt] =hypre_map(ix-1,iy,iz-1,p,q,r-1,nx,ny,
                           nx_part,ny_part,nz_part);
                     offd_data[o_cnt++] = value[1];
                  }
                  else
                  {
                     if (ix)
                     {
                        big_offd_j[o_cnt] =hypre_map(ix-1,iy,iz-1,p-1,q,r-1,nx,ny,
                              nx_part,ny_part,nz_part);
                        offd_data[o_cnt++] = value[1];
                     }
                  }
                  big_offd_j[o_cnt] =hypre_map(ix,iy,iz-1,p,q,r-1,nx,ny,
                        nx_part,ny_part,nz_part);
                  offd_data[o_cnt++] = value[1];
                  if (ix+1 < nx_part[p+1])
                  {
                     big_offd_j[o_cnt] =hypre_map(ix+1,iy,iz-1,p,q,r-1,nx,ny,
                           nx_part,ny_part,nz_part);
                     offd_data[o_cnt++] = value[1];
                  }
                  else
                  {
                     if (ix+1 < nx)
                     {
                        big_offd_j[o_cnt] =hypre_map(ix+1,iy,iz-1,p+1,q,r-1,nx,ny,
                              nx_part,ny_part,nz_part);
                        offd_data[o_cnt++] = value[1];
                     }
                  }
                  if (iy+1 < ny_part[q+1])
                  {
                     if (ix > nx_part[p])
                     {
                        big_offd_j[o_cnt] =hypre_map(ix-1,iy+1,iz-1,p,q,r-1,nx,ny,
                              nx_part,ny_part,nz_part);
                        offd_data[o_cnt++] = value[1];
                     }
                     else
                     {
                        if (ix)
                        {
                           big_offd_j[o_cnt] =hypre_map(ix-1,iy+1,iz-1,p-1,q,r-1,nx,ny,
                                 nx_part,ny_part,nz_part);
                           offd_data[o_cnt++] = value[1];
                        }
                     }
                     big_offd_j[o_cnt] =hypre_map(ix,iy+1,iz-1,p,q,r-1,nx,ny,
                           nx_part,ny_part,nz_part);
                     offd_data[o_cnt++] = value[1];
                     if (ix < nx_part[p+1]-1)
                     {
                        big_offd_j[o_cnt] =hypre_map(ix+1,iy+1,iz-1,p,q,r-1,nx,ny,
                              nx_part,ny_part,nz_part);
                        offd_data[o_cnt++] = value[1];
                     }
                     else
                     {
                        if (ix+1 < nx)
                        {
                           big_offd_j[o_cnt] =hypre_map(ix+1,iy+1,iz-1,p+1,q,r-1,nx,ny,
                                 nx_part,ny_part,nz_part);
                           offd_data[o_cnt++] = value[1];
                        }
                     }
                  }
                  else
                  {
                     if (iy+1 < ny)
                     {
                        if (ix > nx_part[p])
                        {
                           big_offd_j[o_cnt] =hypre_map(ix-1,iy+1,iz-1,p,q+1,r-1,nx,ny,
                                 nx_part,ny_part,nz_part);
                           offd_data[o_cnt++] = value[1];
                        }
                        else if (ix)
                        {
                           big_offd_j[o_cnt] =hypre_map(ix-1,iy+1,iz-1,p-1,q+1,r-1,nx,ny,
                                 nx_part,ny_part,nz_part);
                           offd_data[o_cnt++] = value[1];
                        }
                        big_offd_j[o_cnt] =hypre_map(ix,iy+1,iz-1,p,q+1,r-1,nx,ny,
                              nx_part,ny_part,nz_part);
                        offd_data[o_cnt++] = value[1];
                        if (ix < nx_part[p+1]-1)
                        {
                           big_offd_j[o_cnt] =hypre_map(ix+1,iy+1,iz-1,p,q+1,r-1,nx,ny,
                                 nx_part,ny_part,nz_part);
                           offd_data[o_cnt++] = value[1];
                        }
                        else if (ix < nx-1)
                        {
                           big_offd_j[o_cnt] =hypre_map(ix+1,iy+1,iz-1,p+1,q+1,r-1,nx,ny,
                                 nx_part,ny_part,nz_part);
                           offd_data[o_cnt++] = value[1];
                        }
                     }
                  }
               }
            }
            if (iy > ny_part[q])
            {
               if (ix > nx_part[p])
               {
                  diag_j[cnt] = row_index-nx_local-1;
                  diag_data[cnt++] = value[1];
               }
               else
               {
                  if (ix)
                  {
                     big_offd_j[o_cnt] =hypre_map(ix-1,iy-1,iz,p-1,q,r,nx,ny,
                           nx_part,ny_part,nz_part);
                     offd_data[o_cnt++] = value[1];
                  }
               }
               diag_j[cnt] = row_index-nx_local;
               diag_data[cnt++] = value[1];
               if (ix < nx_part[p+1]-1)
               {
                  diag_j[cnt] = row_index-nx_local+1;
                  diag_data[cnt++] = value[1];
               }
               else
               {
                  if (ix+1 < nx)
                  {
                     big_offd_j[o_cnt] =hypre_map(ix+1,iy-1,iz,p+1,q,r,nx,ny,
                           nx_part,ny_part,nz_part);
                     offd_data[o_cnt++] = value[1];
                  }
               }
            }
            else
            {
               if (iy)
               {
                  if (ix > nx_part[p])
                  {
                     big_offd_j[o_cnt] =hypre_map(ix-1,iy-1,iz,p,q-1,r,nx,ny,
                           nx_part,ny_part,nz_part);
                     offd_data[o_cnt++] = value[1];
                  }
                  else if (ix)
                  {
                     big_offd_j[o_cnt] =hypre_map(ix-1,iy-1,iz,p-1,q-1,r,nx,ny,
                           nx_part,ny_part,nz_part);
                     offd_data[o_cnt++] = value[1];
                  }
                  big_offd_j[o_cnt] =hypre_map(ix,iy-1,iz,p,q-1,r,nx,ny,
                        nx_part,ny_part,nz_part);
                  offd_data[o_cnt++] = value[1];
                  if (ix < nx_part[p+1]-1)
                  {
                     big_offd_j[o_cnt] =hypre_map(ix+1,iy-1,iz,p,q-1,r,nx,ny,
                           nx_part,ny_part,nz_part);
                     offd_data[o_cnt++] = value[1];
                  }
                  else if (ix < nx-1)
                  {
                     big_offd_j[o_cnt] =hypre_map(ix+1,iy-1,iz,p+1,q-1,r,nx,ny,
                           nx_part,ny_part,nz_part);
                     offd_data[o_cnt++] = value[1];
                  }
               }
            }
            if (ix > nx_part[p])
            {
               diag_j[cnt] = row_index-1;
               diag_data[cnt++] = value[1];
            }
            else
            {
               if (ix)
               {
                  big_offd_j[o_cnt] =hypre_map(ix-1,iy,iz,p-1,q,r,nx,ny,
                        nx_part,ny_part,nz_part);
                  offd_data[o_cnt++] = value[1];
               }
            }
            if (ix+1 < nx_part[p+1])
            {
               diag_j[cnt] = row_index+1;
               diag_data[cnt++] = value[1];
            }
            else
            {
               if (ix+1 < nx)
               {
                  big_offd_j[o_cnt] =hypre_map(ix+1,iy,iz,p+1,q,r,nx,ny,
                        nx_part,ny_part,nz_part);
                  offd_data[o_cnt++] = value[1];
               }
            }
            if (iy+1 < ny_part[q+1])
            {
               if (ix > nx_part[p])
               {
                  diag_j[cnt] = row_index+nx_local-1;
                  diag_data[cnt++] = value[1];
               }
               else
               {
                  if (ix)
                  {
                     big_offd_j[o_cnt] =hypre_map(ix-1,iy+1,iz,p-1,q,r,nx,ny,
                           nx_part,ny_part,nz_part);
                     offd_data[o_cnt++] = value[1];
                  }
               }
               diag_j[cnt] = row_index+nx_local;
               diag_data[cnt++] = value[1];
               if (ix < nx_part[p+1]-1)
               {
                  diag_j[cnt] = row_index+nx_local+1;
                  diag_data[cnt++] = value[1];
               }
               else
               {
                  if (ix+1 < nx)
                  {
                     big_offd_j[o_cnt] =hypre_map(ix+1,iy+1,iz,p+1,q,r,nx,ny,
                           nx_part,ny_part,nz_part);
                     offd_data[o_cnt++] = value[1];
                  }
               }
            }
            else
            {
               if (iy+1 < ny)
               {
                  if (ix > nx_part[p])
                  {
                     big_offd_j[o_cnt] =hypre_map(ix-1,iy+1,iz,p,q+1,r,nx,ny,
                           nx_part,ny_part,nz_part);
                     offd_data[o_cnt++] = value[1];
                  }
                  else if (ix)
                  {
                     big_offd_j[o_cnt] =hypre_map(ix-1,iy+1,iz,p-1,q+1,r,nx,ny,
                           nx_part,ny_part,nz_part);
                     offd_data[o_cnt++] = value[1];
                  }
                  big_offd_j[o_cnt] =hypre_map(ix,iy+1,iz,p,q+1,r,nx,ny,
                        nx_part,ny_part,nz_part);
                  offd_data[o_cnt++] = value[1];
                  if (ix < nx_part[p+1]-1)
                  {
                     big_offd_j[o_cnt] =hypre_map(ix+1,iy+1,iz,p,q+1,r,nx,ny,
                           nx_part,ny_part,nz_part);
                     offd_data[o_cnt++] = value[1];
                  }
                  else if (ix < nx-1)
                  {
                     big_offd_j[o_cnt] =hypre_map(ix+1,iy+1,iz,p+1,q+1,r,nx,ny,
                           nx_part,ny_part,nz_part);
                     offd_data[o_cnt++] = value[1];
                  }
               }
            }
            if (iz+1 < nz_part[r+1])
            {
               if (iy > ny_part[q])
               {
                  if (ix > nx_part[p])
                  {
                     diag_j[cnt] = row_index+nxy-nx_local-1;
                     diag_data[cnt++] = value[1];
                  }
                  else
                  {
                     if (ix)
                     {
                        big_offd_j[o_cnt] =hypre_map(ix-1,iy-1,iz+1,p-1,q,r,nx,ny,
                              nx_part,ny_part,nz_part);
                        offd_data[o_cnt++] = value[1];
                     }
                  }
                  diag_j[cnt] = row_index+nxy-nx_local;
                  diag_data[cnt++] = value[1];
                  if (ix < nx_part[p+1]-1)
                  {
                     diag_j[cnt] = row_index+nxy-nx_local+1;
                     diag_data[cnt++] = value[1];
                  }
                  else
                  {
                     if (ix+1 < nx)
                     {
                        big_offd_j[o_cnt] =hypre_map(ix+1,iy-1,iz+1,p+1,q,r,nx,ny,
                              nx_part,ny_part,nz_part);
                        offd_data[o_cnt++] = value[1];
                     }
                  }
               }
               else
               {
                  if (iy)
                  {
                     if (ix > nx_part[p])
                     {
                        big_offd_j[o_cnt] =hypre_map(ix-1,iy-1,iz+1,p,q-1,r,nx,ny,
                              nx_part,ny_part,nz_part);
                        offd_data[o_cnt++] = value[1];
                     }
                     else if (ix)
                     {
                        big_offd_j[o_cnt] =hypre_map(ix-1,iy-1,iz+1,p-1,q-1,r,nx,ny,
                              nx_part,ny_part,nz_part);
                        offd_data[o_cnt++] = value[1];
                     }
                     big_offd_j[o_cnt] =hypre_map(ix,iy-1,iz+1,p,q-1,r,nx,ny,
                           nx_part,ny_part,nz_part);
                     offd_data[o_cnt++] = value[1];
                     if (ix < nx_part[p+1]-1)
                     {
                        big_offd_j[o_cnt] =hypre_map(ix+1,iy-1,iz+1,p,q-1,r,nx,ny,
                              nx_part,ny_part,nz_part);
                        offd_data[o_cnt++] = value[1];
                     }
                     else if (ix < nx-1)
                     {
                        big_offd_j[o_cnt] =hypre_map(ix+1,iy-1,iz+1,p+1,q-1,r,nx,ny,
                              nx_part,ny_part,nz_part);
                        offd_data[o_cnt++] = value[1];
                     }
                  }
               }
               if (ix > nx_part[p])
               {
                  diag_j[cnt] = row_index+nxy-1;
                  diag_data[cnt++] = value[1];
               }
               else
               {
                  if (ix)
                  {
                     big_offd_j[o_cnt] =hypre_map(ix-1,iy,iz+1,p-1,q,r,nx,ny,
                           nx_part,ny_part,nz_part);
                     offd_data[o_cnt++] = value[1];
                  }
               }
               diag_j[cnt] = row_index+nxy;
               diag_data[cnt++] = value[1];
               if (ix+1 < nx_part[p+1])
               {
                  diag_j[cnt] = row_index+nxy+1;
                  diag_data[cnt++] = value[1];
               }
               else
               {
                  if (ix+1 < nx)
                  {
                     big_offd_j[o_cnt] =hypre_map(ix+1,iy,iz+1,p+1,q,r,nx,ny,
                           nx_part,ny_part,nz_part);
                     offd_data[o_cnt++] = value[1];
                  }
               }
               if (iy+1 < ny_part[q+1])
               {
                  if (ix > nx_part[p])
                  {
                     diag_j[cnt] = row_index+nxy+nx_local-1;
                     diag_data[cnt++] = value[1];
                  }
                  else
                  {
                     if (ix)
                     {
                        big_offd_j[o_cnt] =hypre_map(ix-1,iy+1,iz+1,p-1,q,r,nx,ny,
                              nx_part,ny_part,nz_part);
                        offd_data[o_cnt++] = value[1];
                     }
                  }
                  diag_j[cnt] = row_index+nxy+nx_local;
                  diag_data[cnt++] = value[1];
                  if (ix < nx_part[p+1]-1)
                  {
                     diag_j[cnt] = row_index+nxy+nx_local+1;
                     diag_data[cnt++] = value[1];
                  }
                  else
                  {
                     if (ix+1 < nx)
                     {
                        big_offd_j[o_cnt] =hypre_map(ix+1,iy+1,iz+1,p+1,q,r,nx,ny,
                              nx_part,ny_part,nz_part);
                        offd_data[o_cnt++] = value[1];
                     }
                  }
               }
               else
               {
                  if (iy+1 < ny)
                  {
                     if (ix > nx_part[p])
                     {
                        big_offd_j[o_cnt] =hypre_map(ix-1,iy+1,iz+1,p,q+1,r,nx,ny,
                              nx_part,ny_part,nz_part);
                        offd_data[o_cnt++] = value[1];
                     }
                     else if (ix)
                     {
                        big_offd_j[o_cnt] =hypre_map(ix-1,iy+1,iz+1,p-1,q+1,r,nx,ny,
                              nx_part,ny_part,nz_part);
                        offd_data[o_cnt++] = value[1];
                     }
                     big_offd_j[o_cnt] =hypre_map(ix,iy+1,iz+1,p,q+1,r,nx,ny,
                           nx_part,ny_part,nz_part);
                     offd_data[o_cnt++] = value[1];
                     if (ix < nx_part[p+1]-1)
                     {
                        big_offd_j[o_cnt] =hypre_map(ix+1,iy+1,iz+1,p,q+1,r,nx,ny,
                              nx_part,ny_part,nz_part);
                        offd_data[o_cnt++] = value[1];
                     }
                     else if (ix < nx-1)
                     {
                        big_offd_j[o_cnt] =hypre_map(ix+1,iy+1,iz+1,p+1,q+1,r,nx,ny,
                              nx_part,ny_part,nz_part);
                        offd_data[o_cnt++] = value[1];
                     }
                  }
               }
            }
            else
            {
               if (iz+1 < nz)
               {
                  if (iy > ny_part[q])
                  {
                     if (ix > nx_part[p])
                     {
                        big_offd_j[o_cnt] =hypre_map(ix-1,iy-1,iz+1,p,q,r+1,nx,ny,
                              nx_part,ny_part,nz_part);
                        offd_data[o_cnt++] = value[1];
                     }
                     else
                     {
                        if (ix)
                        {
                           big_offd_j[o_cnt] =hypre_map(ix-1,iy-1,iz+1,p-1,q,r+1,nx,ny,
                                 nx_part,ny_part,nz_part);
                           offd_data[o_cnt++] = value[1];
                        }
                     }
                     big_offd_j[o_cnt] =hypre_map(ix,iy-1,iz+1,p,q,r+1,nx,ny,
                           nx_part,ny_part,nz_part);
                     offd_data[o_cnt++] = value[1];
                     if (ix < nx_part[p+1]-1)
                     {
                        big_offd_j[o_cnt] =hypre_map(ix+1,iy-1,iz+1,p,q,r+1,nx,ny,
                              nx_part,ny_part,nz_part);
                        offd_data[o_cnt++] = value[1];
                     }
                     else
                     {
                        if (ix+1 < nx)
                        {
                           big_offd_j[o_cnt] =hypre_map(ix+1,iy-1,iz+1,p+1,q,r+1,nx,ny,
                                 nx_part,ny_part,nz_part);
                           offd_data[o_cnt++] = value[1];
                        }
                     }
                  }
                  else
                  {
                     if (iy)
                     {
                        if (ix > nx_part[p])
                        {
                           big_offd_j[o_cnt] =hypre_map(ix-1,iy-1,iz+1,p,q-1,r+1,nx,ny,
                                 nx_part,ny_part,nz_part);
                           offd_data[o_cnt++] = value[1];
                        }
                        else if (ix)
                        {
                           big_offd_j[o_cnt] =hypre_map(ix-1,iy-1,iz+1,p-1,q-1,r+1,nx,ny,
                                 nx_part,ny_part,nz_part);
                           offd_data[o_cnt++] = value[1];
                        }
                        big_offd_j[o_cnt] =hypre_map(ix,iy-1,iz+1,p,q-1,r+1,nx,ny,
                              nx_part,ny_part,nz_part);
                        offd_data[o_cnt++] = value[1];
                        if (ix < nx_part[p+1]-1)
                        {
                           big_offd_j[o_cnt] =hypre_map(ix+1,iy-1,iz+1,p,q-1,r+1,nx,ny,
                                 nx_part,ny_part,nz_part);
                           offd_data[o_cnt++] = value[1];
                        }
                        else if (ix < nx-1)
                        {
                           big_offd_j[o_cnt] =hypre_map(ix+1,iy-1,iz+1,p+1,q-1,r+1,nx,ny,
                                 nx_part,ny_part,nz_part);
                           offd_data[o_cnt++] = value[1];
                        }
                     }
                  }
                  if (ix > nx_part[p])
                  {
                     big_offd_j[o_cnt] =hypre_map(ix-1,iy,iz+1,p,q,r+1,nx,ny,
                           nx_part,ny_part,nz_part);
                     offd_data[o_cnt++] = value[1];
                  }
                  else
                  {
                     if (ix)
                     {
                        big_offd_j[o_cnt] =hypre_map(ix-1,iy,iz+1,p-1,q,r+1,nx,ny,
                              nx_part,ny_part,nz_part);
                        offd_data[o_cnt++] = value[1];
                     }
                  }
                  big_offd_j[o_cnt] =hypre_map(ix,iy,iz+1,p,q,r+1,nx,ny,
                        nx_part,ny_part,nz_part);
                  offd_data[o_cnt++] = value[1];
                  if (ix+1 < nx_part[p+1])
                  {
                     big_offd_j[o_cnt] =hypre_map(ix+1,iy,iz+1,p,q,r+1,nx,ny,
                           nx_part,ny_part,nz_part);
                     offd_data[o_cnt++] = value[1];
                  }
                  else
                  {
                     if (ix+1 < nx)
                     {
                        big_offd_j[o_cnt] =hypre_map(ix+1,iy,iz+1,p+1,q,r+1,nx,ny,
                              nx_part,ny_part,nz_part);
                        offd_data[o_cnt++] = value[1];
                     }
                  }
                  if (iy+1 < ny_part[q+1])
                  {
                     if (ix > nx_part[p])
                     {
                        big_offd_j[o_cnt] =hypre_map(ix-1,iy+1,iz+1,p,q,r+1,nx,ny,
                              nx_part,ny_part,nz_part);
                        offd_data[o_cnt++] = value[1];
                     }
                     else
                     {
                        if (ix)
                        {
                           big_offd_j[o_cnt] =hypre_map(ix-1,iy+1,iz+1,p-1,q,r+1,nx,ny,
                                 nx_part,ny_part,nz_part);
                           offd_data[o_cnt++] = value[1];
                        }
                     }
                     big_offd_j[o_cnt] =hypre_map(ix,iy+1,iz+1,p,q,r+1,nx,ny,
                           nx_part,ny_part,nz_part);
                     offd_data[o_cnt++] = value[1];
                     if (ix < nx_part[p+1]-1)
                     {
                        big_offd_j[o_cnt] =hypre_map(ix+1,iy+1,iz+1,p,q,r+1,nx,ny,
                              nx_part,ny_part,nz_part);
                        offd_data[o_cnt++] = value[1];
                     }
                     else
                     {
                        if (ix+1 < nx)
                        {
                           big_offd_j[o_cnt] =hypre_map(ix+1,iy+1,iz+1,p+1,q,r+1,nx,ny,
                                 nx_part,ny_part,nz_part);
                           offd_data[o_cnt++] = value[1];
                        }
                     }
                  }
                  else
                  {
                     if (iy+1 < ny)
                     {
                        if (ix > nx_part[p])
                        {
                           big_offd_j[o_cnt] =hypre_map(ix-1,iy+1,iz+1,p,q+1,r+1,nx,ny,
                                 nx_part,ny_part,nz_part);
                           offd_data[o_cnt++] = value[1];
                        }
                        else if (ix)
                        {
                           big_offd_j[o_cnt] =hypre_map(ix-1,iy+1,iz+1,p-1,q+1,r+1,nx,ny,
                                 nx_part,ny_part,nz_part);
                           offd_data[o_cnt++] = value[1];
                        }
                        big_offd_j[o_cnt] =hypre_map(ix,iy+1,iz+1,p,q+1,r+1,nx,ny,
                              nx_part,ny_part,nz_part);
                        offd_data[o_cnt++] = value[1];
                        if (ix < nx_part[p+1]-1)
                        {
                           big_offd_j[o_cnt] =hypre_map(ix+1,iy+1,iz+1,p,q+1,r+1,nx,ny,
                                 nx_part,ny_part,nz_part);
                           offd_data[o_cnt++] = value[1];
                        }
                        else if (ix < nx-1)
                        {
                           big_offd_j[o_cnt] =hypre_map(ix+1,iy+1,iz+1,p+1,q+1,r+1,nx,ny,
                                 nx_part,ny_part,nz_part);
                           offd_data[o_cnt++] = value[1];
                        }
                     }
                  }
               }
            }
            row_index++;
         }
      }
   }

   if (num_procs > 1)
   {
      work = hypre_CTAlloc(HYPRE_BigInt, o_cnt, HYPRE_MEMORY_HOST);

      for (i=0; i < o_cnt; i++)
         work[i] = big_offd_j[i];

      hypre_BigQsort0(work, 0, o_cnt-1);

      col_map_offd[0] = work[0];
      cnt = 0;
      for (i=0; i < o_cnt; i++)
      {
         if (work[i] > col_map_offd[cnt])
         {
            cnt++;
            col_map_offd[cnt] = work[i];
         }
      }

      for (i=0; i < o_cnt; i++)
      {
         offd_j[i] = hypre_BigBinarySearch(col_map_offd,big_offd_j[i],num_cols_offd);
      }

      hypre_TFree(work, HYPRE_MEMORY_HOST);
   }

   A = hypre_ParCSRMatrixCreate(comm, grid_size, grid_size,
                                global_part, global_part, num_cols_offd,
                                diag_i[local_num_rows],
                                offd_i[local_num_rows]);

   hypre_ParCSRMatrixColMapOffd(A) = col_map_offd;

   diag = hypre_ParCSRMatrixDiag(A);
   hypre_CSRMatrixI(diag) = diag_i;
   hypre_CSRMatrixJ(diag) = diag_j;
   hypre_CSRMatrixData(diag) = diag_data;

   offd = hypre_ParCSRMatrixOffd(A);
   hypre_CSRMatrixI(offd) = offd_i;
   if (num_cols_offd)
   {
      hypre_CSRMatrixJ(offd) = offd_j;
      hypre_CSRMatrixData(offd) = offd_data;
   }

   hypre_CSRMatrixMemoryLocation(diag) = HYPRE_MEMORY_HOST;
   hypre_CSRMatrixMemoryLocation(offd) = HYPRE_MEMORY_HOST;

#if defined(HYPRE_USING_CUDA) || defined(HYPRE_USING_DEVICE_OPENMP)
   HYPRE_Int memory_location = hypre_handle->no_cuda_um ? HYPRE_MEMORY_DEVICE : HYPRE_MEMORY_SHARED;

   hypre_CSRMatrix *A_diag = hypre_CSRMatrixClone_v2(hypre_ParCSRMatrixDiag(A), 1, memory_location);
   hypre_CSRMatrixDestroy(hypre_ParCSRMatrixDiag(A));
   hypre_ParCSRMatrixDiag(A) = A_diag;

   hypre_CSRMatrix *A_offd = hypre_CSRMatrixClone_v2(hypre_ParCSRMatrixOffd(A), 1, memory_location);
   hypre_CSRMatrixDestroy(hypre_ParCSRMatrixOffd(A));
   hypre_ParCSRMatrixOffd(A) = A_offd;
#endif

   hypre_TFree(nx_part,    HYPRE_MEMORY_HOST);
   hypre_TFree(ny_part,    HYPRE_MEMORY_HOST);
   hypre_TFree(nz_part,    HYPRE_MEMORY_HOST);
   hypre_TFree(big_offd_j, HYPRE_MEMORY_HOST);

   return (HYPRE_ParCSRMatrix) A;
}

