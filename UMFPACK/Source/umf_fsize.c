//------------------------------------------------------------------------------
// UMFPACK/Source/umf_fsize: determine largest front in each subtree
//------------------------------------------------------------------------------

// UMFPACK, Copyright (c) 2005-2022, Timothy A. Davis, All Rights Reserved.
// SPDX-License-Identifier: GPL-2.0+

//------------------------------------------------------------------------------

/* Determine the largest frontal matrix size for each subtree.   Called by
 * UMF_colamd and UMF_analyze.  Only required to sort the children of each
 * node prior to AMD_postorder. */

#include "umf_internal.h"
#include "umf_fsize.h"

GLOBAL void UMF_fsize
(
    Int nn,
    Int Fsize [ ],
    Int Fnrows [ ],
    Int Fncols [ ],
    Int Parent [ ],
    Int Npiv [ ]
)
{
    Int j, parent, frsize, r, c ;

    for (j = 0 ; j < nn ; j++)
    {
	Fsize [j] = EMPTY ;
    }

    /* ---------------------------------------------------------------------- */
    /* find max front size for tree rooted at node j, for each front j */
    /* ---------------------------------------------------------------------- */

    DEBUG1 (("\n\n========================================FRONTS:\n")) ;
    for (j = 0 ; j < nn ; j++)
    {
	if (Npiv [j] > 0)
	{
	    /* this is a frontal matrix */
	    parent = Parent [j] ;
	    r = Fnrows [j] ;
	    c = Fncols [j] ;
	    frsize = r * c ;
	    /* avoid integer overflow */
	    if (INT_OVERFLOW (((double) r) * ((double) c)))
	    {
		/* :: frsize int overflow :: */
		frsize = Int_MAX ;
	    }
	    DEBUG1 ((""ID" : npiv "ID" size "ID" parent "ID" ",
		j, Npiv [j], frsize, parent)) ;
	    Fsize [j] = MAX (Fsize [j], frsize) ;
	    DEBUG1 (("Fsize [j = "ID"] = "ID"\n", j, Fsize [j])) ;
	    if (parent != EMPTY)
	    {
		/* find the maximum frontsize of self and children */
		ASSERT (Npiv [parent] > 0) ;
		ASSERT (parent > j) ;
		Fsize [parent] = MAX (Fsize [parent], Fsize [j]) ;
		DEBUG1 (("Fsize [parent = "ID"] = "ID"\n",
		    parent, Fsize [parent]));
	    }
	}
    }
}
