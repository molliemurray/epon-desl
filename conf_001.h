/**********************************************************
 * Filename:    conf_001.h
 *
 * Description: This file contains Configuration # 001.
 *
 * Author: 
 *********************************************************/

_FILE_ATTRIBUTES( CONFIGURATION, 001 );

#pragma message( "Using CONFIGURATION file ...... " __FILE__ )


///////////////////////////////////////////////////////////
//  Simulation Constants 
///////////////////////////////////////////////////////////
const float  LLID_LOAD       = 0.05F;

///////////////////////////////////////////////////////////
//  Timing Constants 
///////////////////////////////////////////////////////////
const int32s  ONU_HW_PROCESS_DELAY      = 16384;     // HW processing delay = 1000 TQ 
const int32s  OLT_HW_PROCESS_DELAY      = 16384;     // HW processing delay = 1000 TQ  
const int32s  GUARD_BAND_TIME           = 1000;      // guard band time = 1 us

///////////////////////////////////////////////////////////
//  PON Topology Constants 
///////////////////////////////////////////////////////////

const int32s  PON_MIN_LINK_DISTANCE     =  500;     //  0.5 km
const int32s  PON_MAX_LINK_DISTANCE     = 20000;    //  20   km 

///////////////////////////////////////////////////////////
//  Configuration Constants 
///////////////////////////////////////////////////////////
const int16s   NUM_LLID                 = 16 ;       // one LLID per ONU 
const int32s   BUFFER_SIZE              = 1024*1024; // ONU buffer size = 1 Mbyte
//const int32s   BUFFER_SIZE              = 1024 * 1024 * 10; // ONU buffer size = 10 Mbyte
const int16s   MAX_SLOT                 = 15500;

///////////////////////////////////////////////////////////
//  Traffic Profile Parameters 
///////////////////////////////////////////////////////////
const int16s  BURST_POOL_SIZE           = 128;       // number of On/Off sources 
const int16s  MEAN_BURST_SIZE           = 3200; 
const int16s  BURST_PERIOD              = 1;

///////////////////////////////////////////////////////////
//  Traffic Type
//      LRD - Long-Range Dependent (Bursty, Self-similar)
//      SRD - Short-Range Dependent (Bursty, not Self-similar)
//      CBR - Constant Bit Rate
///////////////////////////////////////////////////////////

#define TRAFFIC_TYPE   LRD
//#define TRAFFIC_TYPE   CBR
//#define TRAFFIC_TYPE   VST



#define LRD 1
#define SRD 2
#define CBR 3
#define VST 4


#if TRAFFIC_TYPE == LRD
    #define TRAFFIC_DESCRIPTOR     "Bursty (Self-similar)"
    #define SRC_CTOR( n ) PacketSource( UNI_BYTE_TIME,          \
                                        PACKET_OVERHEAD,        \
                                        MEAN_BURST_SIZE,        \
                                        CreateParetoStream,     \
                                        BURST_POOL_SIZE,        \
                                        LLID_LOAD,              \
                                        0,                      \
                                        n )

#elif TRAFFIC_TYPE == SRD 
    #define TRAFFIC_DESCRIPTOR     "Bursty (non-Self-similar)"
    #define SRC_CTOR( n ) PacketSource( UNI_BYTE_TIME,          \
                                        PACKET_OVERHEAD,        \
                                        MEAN_BURST_SIZE,        \
                                        CreateExponStream,      \
                                        BURST_POOL_SIZE,        \
                                        LLID_LOAD,              \
                                        0,                      \
                                        n )

#elif TRAFFIC_TYPE == CBR
    #define TRAFFIC_DESCRIPTOR      "Constant Bit Rate"
    #define SRC_CTOR( n ) PacketSource( UNI_BYTE_TIME,          \
                                        PACKET_OVERHEAD,        \
                                        MEAN_BURST_SIZE,        \
                                        CreateCBRStream,        \
                                        BURST_POOL_SIZE,        \
                                        LLID_LOAD,              \
                                        0,                      \
                                        n )
#elif TRAFFIC_TYPE == VST 
    #define TRAFFIC_DESCRIPTOR     "Video Stream"
    #define SRC_CTOR( n ) PacketSource( UNI_BYTE_TIME,          \
                                        PACKET_OVERHEAD,        \
                                        MEAN_BURST_SIZE,        \
                                        CreateVideoStream,      \
                                        BURST_POOL_SIZE,        \
                                        LLID_LOAD,              \
                                        0,                      \
                                        n )

#else

    #error TRAFFIC_TYPE should be defined.
#endif


///////////////////////////////////////////////////////////
//  Derived Constants 
///////////////////////////////////////////////////////////

const int32s  GUARD_BAND_BYTE           = GUARD_BAND_TIME / PON_BYTE_TIME;  // guard band (in bytes)

const int32s  PON_MIN_PROPAGATION_DLY   = PON_MIN_LINK_DISTANCE * FIBER_DELAY;
const int32s  PON_MAX_PROPAGATION_DLY   = PON_MAX_LINK_DISTANCE * FIBER_DELAY;

const int32s  PON_MAX_RTT               = PON_MAX_PROPAGATION_DLY * 2 /* round-trip */
                                        + ONU_HW_PROCESS_DELAY
                                        + _PON_PCKT_TIME( MPCP_PACKET_SIZE );


//////////////////////////////////////////////////////////////////
// FUNCTION:     void OutputParameters( void )
// PURPOSE:      
// ARGUMENTS:    
// RETURN VALUE: 
//////////////////////////////////////////////////////////////////
void OutputParameters( void )
{
    MSG_CONF( "Traffic Type,"               << TRAFFIC_DESCRIPTOR );

    MSG_CONF( "-------------------------------------------" );
    MSG_CONF( "OLT HW Delay (ns),"          << OLT_HW_PROCESS_DELAY );
    MSG_CONF( "ONU HW Delay (ns),"          << ONU_HW_PROCESS_DELAY );
    MSG_CONF( "Guard Band Time (ns),"       << GUARD_BAND_TIME );

    MSG_CONF( "-------------------------------------------" );
    MSG_CONF( "Number of LLIDs,"            << NUM_LLID );
    MSG_CONF( "ONU Buffer Size (bytes),"    << BUFFER_SIZE );
    MSG_CONF( "Minimum Link Distance (m),"  << PON_MIN_LINK_DISTANCE );
    MSG_CONF( "Maximum Link Distance (m),"  << PON_MAX_LINK_DISTANCE );

}


