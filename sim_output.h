/**********************************************************
 * Filename:    sim_output.h
 *
 * Description: This file contains routines for output to 
 *              screen and file
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

#ifndef _SIM_OUTPUT_INCLUDED_
#define _SIM_OUTPUT_INCLUDED_


//#include <fstream.h>  // old style for VC++ 6.0
#include <fstream>      // new style for VC++.NET
#include <conio.h>

using namespace std;

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
// Output routines
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
#define REAL_STREAM( n )                                        \
ofstream LOG_##n ;                                              \
inline void OPEN_##n##_STREAM( char* d, size_t sz )             \
{ strcat_s( d, sz, "_" #n ".csv" );                             \
  LOG_##n.open( d );                                            \
  LOG_##n.precision( 12 );                                      \
  LOG_##n << d << endl; }                                       \
inline void CLOSE_##n##_STREAM( void )   { LOG_##n.close(); } 

#define DUMMY_STREAM( n )                                       \
inline void OPEN_##n##_STREAM( char* )   {}                     \
inline void CLOSE_##n##_STREAM( void )   {}


////////////////////////////////////////////////////////////////////////
// Protocol warnings output
////////////////////////////////////////////////////////////////////////
#if defined ( WARNING_OUTPUT_FILE )
    REAL_STREAM( WARN );
    #define WARN_FILE_OUT( msg )    LOG_WARN << "WARNING: " << msg << endl
#else
    DUMMY_STREAM( WARN );
    #define WARN_FILE_OUT( msg )           
#endif
    
#if defined ( WARNING_OUTPUT_SCREEN )
    #define WARN_SCREEN_OUT( msg )  cerr << "WARNING: " << msg << endl 
#else
    #define WARN_SCREEN_OUT( msg )           
#endif

#if defined ( STOP_ON_WARNING )
    #include <signal.h>
    #define STOP_WARN          { clog << "Press any key to continue ..." << endl; if( _getch() == 0x03 ) raise(SIGINT); } 
#else
    #define STOP_WARN           
#endif

#define MSG_WARN( msg )  { WARN_SCREEN_OUT( msg ); WARN_FILE_OUT( msg ); STOP_WARN; }  


////////////////////////////////////////////////////////////////////////
// Configuration output
////////////////////////////////////////////////////////////////////////
#if defined ( CONFIGURATION_OUTPUT_FILE )
    REAL_STREAM( CONF );
    #define CONF_FILE_OUT( msg )    LOG_CONF << msg << endl
#else
    DUMMY_STREAM( CONF );
    #define CONF_FILE_OUT( msg )           
#endif
    
#if defined ( CONFIGURATION_OUTPUT_SCREEN )
    #define CONF_SCREEN_OUT( msg )  clog << msg << endl 
#else
    #define CONF_SCREEN_OUT( msg )           
#endif

#define MSG_CONF( msg )     { CONF_SCREEN_OUT( msg );  CONF_FILE_OUT( msg ); }  

////////////////////////////////////////////////////////////////////////
// Information output
////////////////////////////////////////////////////////////////////////
#if defined ( INFORMATION_OUTPUT_FILE )
    REAL_STREAM( INFO );
    #define INFO_FILE_OUT( msg )    LOG_INFO << "INFO: " << msg << endl
#else
    DUMMY_STREAM( INFO );
    #define INFO_FILE_OUT( msg )           
#endif
    
#if defined ( INFORMATION_OUTPUT_SCREEN )
    #define INFO_SCREEN_OUT( msg )  clog << "INFO: " << msg << endl 
#else
    #define INFO_SCREEN_OUT( msg )           
#endif

#define MSG_INFO( msg )     { INFO_SCREEN_OUT( msg );  INFO_FILE_OUT( msg ); }  


////////////////////////////////////////////////////////////////////////
// Results output
////////////////////////////////////////////////////////////////////////
#if defined ( RESULT_OUTPUT_FILE )
    REAL_STREAM( RSLT );
    #define RSLT_FILE_OUT( msg )        LOG_RSLT << msg
#else
    DUMMY_STREAM( RSLT );
    #define RSLT_FILE_OUT( msg )           
#endif
    
#if defined ( RESULT_OUTPUT_SCREEN )
    #define RSLT_SCREEN_OUT( msg )      cout << msg 
#else
    #define RSLT_SCREEN_OUT( msg )           
#endif

#define MSG_RSLT( msg )     { RSLT_SCREEN_OUT( msg );  RSLT_FILE_OUT( msg ); } 



////////////////////////////////////////////////////////////////////////
// Macro _FILE_ATTRIBUTES( file_type, ver ) outputs file attributes.  
// Example:
//      _FILE_ATTRIBUTES( Configuration, 001 );
//      _FILE_ATTRIBUTES( Simulation, 040 );
////////////////////////////////////////////////////////////////////////
#define _FILE_ATTRIBUTES( file_type, ver )                  \
inline void   file_type##_FileAttributes( void )            \
{                                                           \
    MSG_CONF( "===============================" );          \
    MSG_CONF( #file_type "," #ver );                        \
    MSG_CONF( "File," __FILE__ );                           \
    MSG_CONF( "Last modified," __TIMESTAMP__ );             \
    MSG_CONF( "Last compiled," __DATE__ " " __TIME__ );     \
    MSG_CONF( "===============================" );          \
}

#endif