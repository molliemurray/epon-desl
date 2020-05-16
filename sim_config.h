/**********************************************************
 * Filename:    sim_config.h
 *
 * Description: Common Configuration Parameters
 *
 * --------------------------------------------------------
 * Date:  
 * --------------------------------------------------------
 * Changes: 
 *********************************************************/
#ifndef _SIMULATION_H_INCLUDED_ 
#define _SIMULATION_H_INCLUDED_ 


///////////////////////////////////////////////////////////
//  Output options
///////////////////////////////////////////////////////////
#define STOP_ON_WARNING

#define WARNING_OUTPUT_FILE 
#define WARNING_OUTPUT_SCREEN

#define CONFIGURATION_OUTPUT_FILE 
#define CONFIGURATION_OUTPUT_SCREEN

#define INFORMATION_OUTPUT_FILE 
#define INFORMATION_OUTPUT_SCREEN

#define RESULT_OUTPUT_FILE 
//#define RESULT_OUTPUT_SCREEN

///////////////////////////////////////////////////////////
//  Network Device Identification Constants 
///////////////////////////////////////////////////////////
const int16s  OLT_BASE_ID               = 0x0100;
const int16s  ONU_BASE_ID               = 0x1000;
const int16s  LNK_BASE_ID               = 0x2000;
const int16s  SRC_BASE_ID               = 0x4000;


///////////////////////////////////////////////////////////
//  Event Constants 
///////////////////////////////////////////////////////////
const int8s  EV_PCKT_ARRIVAL                = 0x01;
const int8s  EV_PCKT_ENQUE                  = 0x03;
const int8s  EV_PCKT_DEQUE                  = 0x04;
const int8s  EV_PCKT_DROP                   = 0x05;

const int8s  EV_MPCP_GATE                   = 0x10;
const int8s  EV_MPCP_REPORT                 = 0x11;

const int8s  EV_TIMER_NEXT_PACKET           = 0x20;
const int8s  EV_TIMER_GRANT_REPORT          = 0x21;
const int8s  EV_TIMER_GRANT_DATA            = 0x22;


#include "sim_output.h"
#include "trf_gen_v3.h"
#include "desl.h"

/////////////////////////////////////////////////////////////////////
// Format of a data packet
/////////////////////////////////////////////////////////////////////
struct Pckt_Data_t
{
    int64s           PcktTime;
    GEN::pckt_size_t PcktSize;
    GEN::source_id_t SourceId;
};

/////////////////////////////////////////////////////////////////////
// Format of the GATE message
/////////////////////////////////////////////////////////////////////
struct GATE_Data_t
{
    int64s      Timestamp;
    int64s      StartTime;
    int32s      Length;
};

/////////////////////////////////////////////////////////////////////
// Format of the REPORT message
/////////////////////////////////////////////////////////////////////
struct RPRT_Data_t
{
    int64s      Timestamp;
    int32s      Length;
};


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
// Instantiate environment
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
struct EventData
{
    int8s Type;
    union
    {
        Pckt_Data_t     Pckt;           /* data associated with a data packet       */
        GATE_Data_t     GATE;           /* data associated with a GATE message      */
        RPRT_Data_t     RPRT;           /* data associated with a REPORT message    */
    };
};

// Disable W4-level warning that class 'DESL' can never be instantiated -
// this behavior is by design
#pragma warning( disable : 4610 )

class DESL : public DESL_environment< int64s, EventData > {};



///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
// Create base class for simulation objects
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
#include "clock.h"
#include "mport.h"


template < int16u PORTS = 1 > class SimBase : 
    public CClockSync, 
    public MultiPort< PORTS >
{
public:
    SimBase( DESL::obid_t id = 0 ): CClockSync( id ), MultiPort< PORTS >() {} 
};



///////////////////////////////////////////////////////////
//  Timing Constants 
///////////////////////////////////////////////////////////
const DESL::time_t      UNITS_PER_SEC       = 1000000000;  // time unit is 1 ns
const int16s            BYTE_SIZE           = 8;           // 8 bits in one byte

const int32s            ___1_MBPS           =    1000000;
const int32s            __10_MBPS           =   10000000;
const int32s            _100_MBPS           =  100000000;
const int32s            ___1_GBPS           = 1000000000;
        
const int16s            BYTE_TIME_1_MBPS    = 8000;
const int16s            BYTE_TIME_10_MBPS   =  800; //(int16s)(UNITS_PER_SEC*BYTE_SIZE/_10_MBPS );
const int16s            BYTE_TIME_100_MBPS  =   80; //(int16s)(UNITS_PER_SEC*BYTE_SIZE/_100_MBPS);
const int16s            BYTE_TIME_1_GBPS    =    8; //(int16s)(UNITS_PER_SEC*BYTE_SIZE/_1_GBPS  );

const int32s            FIBER_DELAY         =   5; // signal delay in fiber = 5 ns / meter

///////////////////////////////////////////////////////////
//  Configuration Constants 
///////////////////////////////////////////////////////////
const int16s            UNI_BYTE_TIME       = BYTE_TIME_100_MBPS;   // user port rate = 100 Mbps
const int16s            PON_BYTE_TIME       = BYTE_TIME_1_GBPS;     // PON rate = 1 Gbps 
const int16s            PON_RATE_MBPS       = ___1_GBPS / ___1_MBPS;



///////////////////////////////////////////////////////////
//  Data Traffic Constants 
///////////////////////////////////////////////////////////
const GEN::pckt_size_t  MIN_PACKET_SIZE     = 64;
const GEN::pckt_size_t  MAX_PACKET_SIZE     = 1518;
const GEN::pckt_size_t  MPCP_PACKET_SIZE    = MIN_PACKET_SIZE;
const GEN::pckt_size_t  PREAMBLE_SIZE       = 8;
const GEN::pckt_size_t  MIN_IFG_SIZE        = 12;
const GEN::pckt_size_t  PACKET_OVERHEAD     = PREAMBLE_SIZE + MIN_IFG_SIZE;


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
// Miscellaneous Primitives
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

#define _PON_BYTE( X )           static_cast<int32s>( (X) / PON_BYTE_TIME   )
#define _UNI_BYTE( X )           static_cast<int32s>( (X) / UNI_BYTE_TIME   )
#define _PON_TIME( X )                              ( (X) * PON_BYTE_TIME   )
#define _UNI_TIME( X )                              ( (X) * UNI_BYTE_TIME   )
#define _OVERHEAD( X )                              ( (X) + PACKET_OVERHEAD )
#define _PON_PCKT_TIME( X )                         ( _PON_TIME( _OVERHEAD( X )))
#define _UNI_PCKT_TIME( X )                         ( _UNI_TIME( _OVERHEAD( X )))

#define _OLT_ID( N )                                ( (N) ^ OLT_BASE_ID )
#define _ONU_ID( N )                                ( (N) ^ ONU_BASE_ID )
#define _LNK_ID( N )                                ( (N) ^ LNK_BASE_ID )
#define _SRC_ID( N )                                ( (N) ^ SRC_BASE_ID )


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//  Include Congiguration file
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
#include "conf_001.h"


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
// Include Simulation objects
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
#include "link.h"
#include "pktsrc.h"
#include "ONU.h"
#include "OLT.h"


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//  Include Test Scenario file
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
#include "test_001.h"




////////////////////////////////////////////////////////////////
// FUNCTION:     int Simulation( int argc, char* argv[] )
// PURPOSE:      
// ARGUMENTS:    
// RETURN VALUE: 
////////////////////////////////////////////////////////////////
int Simulation( int, char* [] )
{
    _seed();

    ////////////////////////////////////////////////////////////
    // Output current configuration
    ////////////////////////////////////////////////////////////
    CONFIGURATION_FileAttributes();
    SIMULATION_FileAttributes();
    OutputConfiguration();

    ////////////////////////////////////////////////////////////
    // Create, execute, and destroy simulation
    ////////////////////////////////////////////////////////////
    InitializeEPON();
    Execute();
    DestroyEPON();

    return 0;
}

#endif _SIMULATION_H_INCLUDED_ 