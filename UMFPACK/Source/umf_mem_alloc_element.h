//------------------------------------------------------------------------------
// UMFPACK/Source/umf_mem_alloc_element.h
//------------------------------------------------------------------------------

// UMFPACK, Copyright (c) 2005-2022, Timothy A. Davis, All Rights Reserved.
// SPDX-License-Identifier: GPL-2.0+

//------------------------------------------------------------------------------

GLOBAL Int UMF_mem_alloc_element
(
    NumericType *Numeric,
    Int nrows,
    Int ncols,
    Int **Rows,
    Int **Cols,
    Entry **C,
    Int *size,
    Element **epout
) ;
