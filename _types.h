/********************************************************************
 * Filename:    _types.h
 *
 * Description: This file contains type definitions and
 *              general macros
 *
 * Author:      Glen Kramer (kramer@cs.ucdavis.edu)
 *              University of California, Davis
 *
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
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY 
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 ********************************************************************/

#ifndef _TYPES_H_V001_INCLUDED_
#define _TYPES_H_V001_INCLUDED_

#ifndef NULL
//#define NULL ((void*) 0L)
#define NULL 0
#endif

//#define _USE_MSVC_SPECIFIC_TYPES_

/* Datatype abstractions. */

#if defined _USE_MSVC_SPECIFIC_TYPES_

    typedef unsigned __int8     int8u;
    typedef signed   __int8     int8s;

    typedef unsigned __int16    int16u;
    typedef signed   __int16    int16s;

    typedef unsigned __int32    int32u;
    typedef signed   __int32    int32s;

    typedef unsigned __int64    int64u;
    typedef signed   __int64    int64s;

#else  

/* use generic types */

    typedef unsigned char       int8u;
    typedef signed   char       int8s;

    typedef unsigned short int  int16u;
    typedef signed   short int  int16s;

    typedef unsigned long       int32u;
    typedef signed   long       int32s;

	/* this is only MS compatible. To port to other platforms, 
	 * define a class of large integers and typedef it to int64x
	 */
	typedef unsigned __int64    int64u;
    typedef signed   __int64    int64s;

#endif

typedef int8s               BOOL;
typedef int8u               BYTE;
typedef char                CHAR;
typedef unsigned int        WORD;

typedef float               FLOAT;
typedef double              DOUBLE;



template < class T > inline T round( DOUBLE val )    { return (T)( val + 0.5 ); }
template < class T > inline T MAX( T x, T y )        { return x > y? x : y;     }
template < class T > inline T MIN( T x, T y )        { return x < y? x : y;     }

template < class T > inline void SWAP( T& x, T& y )  { T z = x; x = y; y = z;   }

const BOOL TRUE  = 1;
const BOOL FALSE = 0;

#endif /* _TYPES_H_V001_INCLUDED_ */
