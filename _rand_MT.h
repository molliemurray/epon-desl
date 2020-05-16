/**********************************************************
 * Filename:    _rand_MT.h
 *
 * Description: This file contains routines for generating 
 *              random values
 * 
 * Author: Glen Kramer (kramer@cs.ucdavis.edu)
 *         University of California @ Davis
 *
 * Copyright (c) 2000-2004, Glen Kramer
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person 
 * obtaining a copy of this software and associated documentation 
 * files (the "Software"), to deal in the Software without restriction, 
 * including without limitation the rights to use, copy, modify, merge, 
 * publish, distribute, sublicense, and/or sell copies of the Software, 
 * and to permit persons to whom the Software is furnished to do so, 
 * subject to the following conditions:
 * 
 *     The above copyright notice and this permission notice shall be 
 *     included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY 
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 ********************************************************************/

#ifndef _RAND_MT_H_V002_INCLUDED_
#define _RAND_MT_H_V002_INCLUDED_

#include <math.h>

#include "_types.h"
#include "MersenneTwister.h"

//extern MTRand RND;

typedef DOUBLE  rnd_real_t;
typedef int32s  rnd_int_t;

const rnd_real_t  SMALL_VAL = 1.0 / 0xFFFFFFFFUL;

static MTRand RND;

inline void       _seed(void)                            { RND.seed(); }
inline rnd_real_t _uniform_real_0_1(void)   /* [0,1] */  { return RND.rand(); }  
inline rnd_real_t _uniform_real_0_X1(void)  /* [0,1) */  { return RND.randExc(1.0); }  
inline rnd_real_t _uniform_real_X0_1(void)  /* (0,1] */  { return 1.0 - _uniform_real_0_X1(); }  

inline rnd_real_t _uniform_real_(rnd_real_t low, rnd_real_t hi) { return RND.rand( hi - low ) + low;    }
inline rnd_int_t  _uniform_int_ (rnd_int_t low,  rnd_int_t hi)  { return RND.randInt( hi - low ) + low; }

inline rnd_real_t _exponent_(void)                       { return -log( _uniform_real_X0_1() );            }
inline rnd_real_t _pareto_(rnd_real_t shape)             { return  pow( _uniform_real_X0_1(), -1.0/shape); }




/////////////////////////////////////////////////////////////////////
// CLASS:        class GenericDistribByIndex
// PURPOSE:      Generates random values in the range from 0 to N-1 
//               such that probability of returning value i is 
//               PF_FREQUENCY(i) / SUM_all{ PF_FREQUENCY(n) }
// TEMPLATE 
// PARAMETERS:   T - type of elements representing probabilities
//                   or frequencies (counts). To improve the performance 
//                   it is desirable to make T an integer.
//                   
//               N - number of elements in the distribution.
//
//               PF_FREQUENCY - a pointer to callback function that returns 
//                   probability or frequency of an element with index i
/////////////////////////////////////////////////////////////////////
template< class T, int32s N, T (*PF_FREQUENCY)(int32s) > class GenericDistribByIndex
{
private:
    static T cdf[ N ];  // cummulative distribution

public:

    /////////////////////////////////////////////////////////////////
    // FUNCTION:    GenericDistribByIndex()
    // DESCRIPTION: constructor
    // ARGUMENTS:   
    // NOTES:       
    /////////////////////////////////////////////////////////////////
    GenericDistribByIndex()
    {
        cdf[0] = PF_FREQUENCY( 0 );

        for( int32s ndx = 1; ndx < N; ndx ++ )
            cdf[ndx] = cdf[ndx-1] + PF_FREQUENCY( ndx );
    }

    /////////////////////////////////////////////////////////////////
    // FUNCTION:    int32s GetIndex( void )
    // DESCRIPTION: returns next random value
    // NOTES:       does binary search over the array cdf[] 
    //              representing cummulative distribution function
    /////////////////////////////////////////////////////////////////
    static int32s GetIndex( void )   
    { 
        int32s lo = - 1;
        int32s hi = N - 1;
        int32s md;

        T val = static_cast< T >( _uniform_real_0_1() * cdf[ N-1 ] );

        while( hi - lo > 1 )
        {
            md = ( lo + hi ) / 2;

            if( val < cdf[md] )
                hi = md;
            else 
                lo = md;
        }
        return hi; 
    }
};

/////////////////////////////////////////////////////////////////////
// Declaration for static array VAL_T cdf[ N ]
/////////////////////////////////////////////////////////////////////
template< class T, int32s N, T (*PF_FREQUENCY)(int32s) > 
T GenericDistribByIndex< T, N, PF_FREQUENCY >::cdf[N] = {0};



/////////////////////////////////////////////////////////////////////
// CLASS:        class GenericDistribution
// PURPOSE:      Generates random values such that probability of 
//               returning value elem[i] is 
//               histogram[i] / SUM_j{ histogram[j] }
// TEMPLATE 
// PARAMETERS:   E - type of elements being generated
//               T, N, PF_FREQUENCY - see class GenericDistribByIndex
//
// NOTES:        This class is usefull to select a random element from 
//               a set of non-sequential or non-numeric elements
/////////////////////////////////////////////////////////////////////
template< class E, class T, int32s N, T (*PF_FREQUENCY)(int32s) >
class GenericDistribution : public GenericDistribByIndex< T, N, PF_FREQUENCY >
{
private:
    const E* Elements;

public:
    /////////////////////////////////////////////////////////////////
    // FUNCTION:    GenericDistribution( const ELM_T elements[],
    //                                   const PDF_T histogram[], 
    //                                   PF_SCALE scale )
    // DESCRIPTION: constructor
    // ARGUMENTS:   E elements[] - array containing sample space 
    //                  from which a random element is selected 
    // NOTES:       
    /////////////////////////////////////////////////////////////////
    GenericDistribution( const E elements[] ) : GenericDistribByIndex< T, N, PF_FREQUENCY >()
    {
        Elements = &elements[0];
    }
    
    /////////////////////////////////////////////////////////////////
    // FUNCTION:    ELM_T GetElement( void )
    // DESCRIPTION: returns next random value
    // NOTES:       
    /////////////////////////////////////////////////////////////////
    static inline E GetElement( void ) { return Elements[ GenericDistribByIndex<T, N, PF_FREQUENCY>.GetIndex() ]; }
};


#endif /* _RAND_MT_H_V002_INCLUDED_ */
