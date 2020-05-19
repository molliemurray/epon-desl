/**********************************************************
 * Filename:    test_001.h
 *
 * Description: This file contains the simulation scenario 001.
 *              
 *              In this scenario 16 LLIDs are used.
 *              Different parameters are measured as the load
 *              of all LLIDs changes from MIN_LOAD to MAX_LOAD.  
 *              
 * Instruction: Set following simulation parameters.
 *
 *              1. MIN_LOAD:              Minimum load
 *              2. MAX_LOAD:              Maximum load
 *
 *              3. NUM_TEST:              Measurements will be performed 
 *                                        at NUM_TEST different load 
 *                                        uniformly spaced between 
 *                                        MIN_LOAD and MAX_LOAD..
 *
 *              4. PACKET_LIMIT:          Number of packtes for which 
 *                                        simulation will run.
 *
 *              5. WARMUP_TIME:           A time after which statistic collection starts 
 *
 *              
 * Note:        For each load the simulation will run until PACKET_LIMIT packets 
 *              are transmitted by all LLIDs together.
 * 
 * Result Format: Below is sample result output. 
 *
 *                 file_name.csv                                                                                
 *                 TARGET LOAD              load1  ;  load2 ;  ... ; loadN    
 *                 SIM TIME (sec)     
 *                 ONU LOAD
 *                 OFFERED LOAD                                                                                  
 *                 CARRIED LOAD 
 *                 AVG DELAY (us)                                                                                
 *                 MAX DELAY (us)     
 *                 AVG QUEUE LENGTH (bytes)                                                                                
 *                 RECV PACKETS                                                                                  
 *                 SENT PACKETS                                                                                  
 *                 DROP PACKETS                                                                                  
 *                 RECV BYTES                                                                                
 *                 SENT BYTES                                                                                
 *                 DROP BYTES 
 *                 PACKET LOSS RATIO                                                                                  
 *                 BYTE LOSS RATIO  
 *                 AVG CYCLE TIME
 *                 MAX CYCLE TIME
 *                 TOTAL CYCLES
 *
 * 
 * Author:      Glen Kramer (kramer@cs.ucdavis.edu)
 *              University of California, Davis
 *
 * Copyright (c) 2000-2005, Glen Kramer. All rights reserved.
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
 *********************************************************/

_FILE_ATTRIBUTES( SIMULATION, 001 );

#pragma message( "Using SIMULATION file ......... " __FILE__ )

#include "stats.h"

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
// Simulation Parameters
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

const int32s PACKET_LIMIT   = 1000000;    
const int64s WARMUP_TIME    = 10 * UNITS_PER_SEC; // 10 seconds
const float  MIN_LOAD       = 0.05F;
const float  MAX_LOAD       = 0.90F;
const int16s NUM_TEST       = 18;   
const float  LOAD_STEP      = (MAX_LOAD - MIN_LOAD) / (NUM_TEST - 1);


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
// Macros
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

#define PER_PON( name, val )                              \
    MSG_RSLT( name << ",," );                             \
    for( t=0; t < NUM_TEST; t++ )                         \
        MSG_RSLT( (val) << "," );                         \
        MSG_RSLT( endl );


#define RATIO( val, port )    ( (DOUBLE)( val * ##port##_BYTE_TIME ) / RunTime[t] )
#define RATE_MBPS( val )      ( RATIO( val, PON ) * PON_RATE_MBPS )


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
// Pointers to simulation objects
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

OLT*            pOLT;
ONU*            pONU[ NUM_LLID ];
BiDirLink*      pLNK[ NUM_LLID ];
PacketSource*   pSRC[ NUM_LLID ];


int16s          NumTest = 0;
int32s          LastQueueLength = 0;
DESL::time_t    LastQueueChange = 0;
DESL::time_t    LastCycleStart = 0;


float           TargetLoad[NUM_TEST] = { 0.0F };   // Target ONU Load
DESL::time_t    RunTime[NUM_TEST]  = { 0 };        // Simulation Run Time
                    
int32s          RcvdPckt[NUM_TEST] = { 0 };        // Total Number of Packets received 
int32s          DropPckt[NUM_TEST] = { 0 };        // Total Number of Packets dropped 
int32s          SentPckt[NUM_TEST] = { 0 };        // Total Number of Packets received at OLT
int32s          SchdPckt[NUM_TEST] = { 0 };        // Total Number of Packets scheduled by OLT

int64s          RcvdByte[NUM_TEST] = { 0 };        // Total Number of Bytes received 
int64s          DropByte[NUM_TEST] = { 0 };        // Total Number of Bytes dropped 
int64s          SentByte[NUM_TEST] = { 0 };        // Total Number of Bytes received at OLT 
int64s          SchdByte[NUM_TEST] = { 0 };        // Total Number of Bytes scheduled by OLT

Stats           DLY[NUM_TEST];                     // Delay statistics 
Stats           QUE[NUM_TEST];                     // Queue size statistics 
Stats           CYC[NUM_TEST];                     // Cycle length 

    
//////////////////////////////////////////////////////////////////
// FUNCTION:     void PrintResult( void )
// PURPOSE:      
// ARGUMENTS:    
// RETURN VALUE: 
//////////////////////////////////////////////////////////////////
void PrintResult( void )
{
    int16s t;
   
    PER_PON( "TARGET LOAD",            TargetLoad[t] );
    PER_PON( "SIM TIME (sec)",         ((DOUBLE)RunTime[t]) / UNITS_PER_SEC );
    PER_PON( "ONU LOAD",               RATIO( RcvdByte[t], UNI ) / NUM_LLID );
    PER_PON( "OFFERED LOAD",           RATIO( RcvdByte[t], PON ));
    PER_PON( "CARRIED LOAD",           RATIO( SentByte[t], PON ));
    PER_PON( "AVG DLY (ms)",           DLY[t].GetAvg() );
    PER_PON( "MAX DLY (ms)",           DLY[t].GetMax() );
    PER_PON( "AVG QUEUE (bytes)",      QUE[t].GetAvg() / NUM_LLID );
    PER_PON( "RECV PACKETS",           RcvdPckt[t] );
    PER_PON( "SENT PACKETS",           SentPckt[t] );
    PER_PON( "DROP PACKETS",           DropPckt[t] );
    PER_PON( "RECV BYTES",             RcvdByte[t] );
    PER_PON( "SENT BYTES",             SentByte[t] );
    PER_PON( "DROP BYTES",             DropByte[t] );
    PER_PON( "PACKET LOSS RATIO",      ((DOUBLE)DropPckt[t]) / RcvdPckt[t] );
    PER_PON( "BYTE LOSS RATIO",        ((DOUBLE)DropByte[t]) / RcvdByte[t] );
    PER_PON( "AVG CYCLE (ms)",         CYC[t].GetAvg() );
    PER_PON( "MAX CYCLE (ms)",         CYC[t].GetMax() );
    PER_PON( "CYCLES",                 CYC[t].GetCount() );
    PER_PON( "SCHD PACKETS",           SchdPckt[t] );
    PER_PON( "SCHD BYTES",             SchdByte[t] );
    //PER_PON( "UTIL",                   (SentByte[t]+RcvdByte[t]) / (1000000 * PON_RATE_MBPS  / 8 ) );

    MSG_RSLT( endl );
}



//////////////////////////////////////////////////////////////////
// FUNCTION:     void Monitor( DESL::evnt_t* pEvent )
// PURPOSE:      Monitor function looks into each Event
//               and collects statistics.      
// ARGUMENTS:    
// RETURN VALUE: 
//////////////////////////////////////////////////////////////////
void Monitor( DESL::evnt_t* pEvent )
{
    DOUBLE          pckt_dly;

    if( pEvent->Type == EV_PCKT_ARRIVAL && ( pEvent->Consumer->ID & ONU_BASE_ID ) )
    {
        ////////////////////////////////////////////////////////////
        // Calculate total number of packets and bytes received by all ONUs 
        ////////////////////////////////////////////////////////////
        RcvdPckt[NumTest] ++;
        RcvdByte[NumTest] += pEvent->Pckt.PcktSize;
    }

    else if( pEvent->Type == EV_PCKT_ARRIVAL && ( pEvent->Producer->ID & ONU_BASE_ID ) )
    {
        //////////////////////////////////////////////////////////// 
        //  Calculate packet delay (in us) as time difference between time when
        //  packet was generated and time when packet arrvies at OLT.
        ////////////////////////////////////////////////////////////
        pckt_dly = static_cast<DOUBLE>(DESL::GlobalTime() - pEvent->Pckt.PcktTime) / 1000000;


        DLY[NumTest].Sample( pckt_dly );

        ////////////////////////////////////////////////////////////
        // Calculate total number of packets and bytes sent by all ONUs 
        ////////////////////////////////////////////////////////////
        SentPckt[NumTest] ++; 
        SentByte[NumTest] += pEvent->Pckt.PcktSize;
    }

    else if( pEvent->Type == EV_PCKT_DROP )
    {
        ////////////////////////////////////////////////////////////
        // Calculate total number of packets and bytes dropped by all ONUs 
        ////////////////////////////////////////////////////////////
        DropPckt[NumTest] ++;
        DropByte[NumTest] += pEvent->Pckt.PcktSize;

    }

    else if( pEvent->Type == EV_PCKT_ENQUE || pEvent->Type == EV_PCKT_DEQUE )
    {
        
        if( LastQueueChange == 0 )
        {
            ////////////////////////////////////////////////////////////
            // First time, calculate the total queue length
            ////////////////////////////////////////////////////////////
            for( int16s onu_index = 0; onu_index < NUM_LLID; onu_index++ )
                LastQueueLength += pONU[onu_index]->GetQueueLength();
        }

        else
        {
            ////////////////////////////////////////////////////////////
            // Take queue length sample weighted by the time since the last change.
            // This will give the precise average-in-time
            ////////////////////////////////////////////////////////////
            QUE[NumTest].Sample( LastQueueLength, (DOUBLE)(DESL::GlobalTime() - LastQueueChange));

            ////////////////////////////////////////////////////////////
            // Calculate delta of queue length
            ////////////////////////////////////////////////////////////
            LastQueueLength += pEvent->Pckt.PcktSize * ( pEvent->Type == EV_PCKT_ENQUE ? 1 : -1 ); 
        }

        ////////////////////////////////////////////////////////////
        // Save last queue change time
        ////////////////////////////////////////////////////////////
        LastQueueChange  = DESL::GlobalTime();
    }

    else if( pEvent->Type == EV_MPCP_GATE && pEvent->Consumer->ID == ONU_BASE_ID )
    {
        if( LastCycleStart != 0 )
            CYC[NumTest].Sample( (DOUBLE)(pEvent->GATE.StartTime - LastCycleStart ) / 1000000 );

        ////////////////////////////////////////////////////////////
        // Save last cycle start time
        ////////////////////////////////////////////////////////////
        LastCycleStart = pEvent->GATE.StartTime;

        ////////////////////////////////////////////////////////////
        // Counts scheduled packets
        ////////////////////////////////////////////////////////////
        SchdByte[NumTest] += pEvent->GATE.Length;
    }
}


//////////////////////////////////////////////////////////////////
// FUNCTION:     void InitializeEPON( void )
// PURPOSE:      
// ARGUMENTS:    
// RETURN VALUE: 
//////////////////////////////////////////////////////////////////
void InitializeEPON( void )
{
    int32s delay;

    pOLT = new OLT( _OLT_ID( 2 ));

    for( int16s n = 0; n < NUM_LLID; n++ )
    {
        /* find prapagation delay to the ONU */
        delay   = _uniform_int_( PON_MIN_LINK_DISTANCE, PON_MAX_LINK_DISTANCE ) * FIBER_DELAY; 

        /* Create Network Elements */
        pSRC[n] = new SRC_CTOR( _SRC_ID( n ));
        pONU[n] = new ONU( _ONU_ID( n )); 
        pLNK[n] = new BiDirLink( delay, _LNK_ID( n ));

        /* downstream connection */
        pOLT   ->SetPort( pLNK[n], n );    /* connect OLT's port to a logical link */
        pLNK[n]->SetPort( pONU[n], 0 );    /* connect logical link to an ONU */

        /* upstream connection */
        pONU[n]->SetPort( pLNK[n]    );    /* connect ONU to a logical link   */
        pLNK[n]->SetPort( pOLT,    1 );    /* connect logical link to the OLT */
        pSRC[n]->SetPort( pONU[n]    );    /* connect packet source to ONU */
    }
    /**************************************************/
    MSG_INFO( "Created " << DESL::GetObjCount() << " objects" );
}


//////////////////////////////////////////////////////////////////
// FUNCTION:     void DestroyEPON( void )
// PURPOSE:      
// ARGUMENTS:    
// RETURN VALUE: 
//////////////////////////////////////////////////////////////////
#define DELETE( p )     if( p ) { delete p; p = NULL; }
void DestroyEPON( void )
{
    DESL::GlobalFree();

    DELETE( pOLT ); 
    for( int16s n = 0; n < NUM_LLID; n++ )
    {
        DELETE( pONU[n] );
        DELETE( pLNK[n] );
        DELETE( pSRC[n] );
    }
}

//////////////////////////////////////////////////////////////////
// FUNCTION:     void Execute( void )
// PURPOSE:      
// ARGUMENTS:    
// RETURN VALUE: 
//////////////////////////////////////////////////////////////////
void Execute( void )
{
    DESL::evnt_t* pEvent;
    DESL::GlobalReset();

    MSG_INFO( "Warming-up ..." );

    ////////////////////////////////////////////////////////////    
    // Warmup the system 
    ////////////////////////////////////////////////////////////
    while( DESL::GlobalTime() < WARMUP_TIME ) 
    {
        pEvent = DESL::GetNextEvent();
        //Monitor(pEvent);
        DESL::DispatchEvent(pEvent);
    }

    MSG_INFO( "Warm-up completed" );

   
    ////////////////////////////////////////////////////////////
    //  Main loop
    ////////////////////////////////////////////////////////////
    for( NumTest = 0; NumTest < NUM_TEST; NumTest++ )
    {
        TargetLoad[NumTest] = MIN_LOAD + NumTest * LOAD_STEP;
        MSG_INFO( "load = " << TargetLoad[NumTest] );

        ////////////////////////////////////////////////////////////
        // Set Load
        ////////////////////////////////////////////////////////////
        for( int16s n =0; n < NUM_LLID; n++ )
            pSRC[n]->SetLoad( TargetLoad[NumTest] );

        ////////////////////////////////////////////////////////////
        // Remember test start time
        ////////////////////////////////////////////////////////////
        RunTime[NumTest] = DESL::GlobalTime();

        ////////////////////////////////////////////////////////////
        // Simulate until specified number of packets is received.
        //////////////////////////////////////////// ////////////////
        while( SentPckt[NumTest] < PACKET_LIMIT )
        {
            pEvent = DESL::GetNextEvent();
            Monitor( pEvent);
            DESL::DispatchEvent( pEvent );
        }

        ////////////////////////////////////////////////////////////
        // Calculate simulated time
        ////////////////////////////////////////////////////////////
        RunTime[NumTest] = DESL::GlobalTime() - RunTime[NumTest];
    }

    ////////////////////////////////////////////////////////////
    // Print Simulation Results
    ////////////////////////////////////////////////////////////
    MSG_INFO( "Simulation completed. Printing Results..." );

    PrintResult();

    ////////////////////////////////////////////////////////////
}

//////////////////////////////////////////////////////////////////
// FUNCTION:     void OutputConfiguration( void )
// PURPOSE:      
// ARGUMENTS:    
// RETURN VALUE: 
//////////////////////////////////////////////////////////////////
void OutputConfiguration( void )
{
    MSG_CONF( "Packet Limit,"               << PACKET_LIMIT );
    MSG_CONF( "Warm-up time (seconds),"     << WARMUP_TIME * 1.0 / UNITS_PER_SEC );
    MSG_CONF( "Minimum Load,"               << MIN_LOAD );
    MSG_CONF( "Maximum Load,"               << MAX_LOAD );
    MSG_CONF( "Number of Tests,"            << NUM_TEST );
    MSG_CONF( "-----------------------------" );
    OutputParameters();
}
