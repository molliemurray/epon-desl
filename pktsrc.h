/**********************************************************
 * Filename:    pktsrc.h
 *
 * Description: This file contains declarations for
 *              class Packet: public GEN::Packet
 *              class PacketPool
 *              class PacketSource
 *
 * Author: Glen Kramer (kramer@cs.ucdavis.edu)
 *         University of California @ Davis
 *********************************************************/

#if !defined(_PACKET_SOURCE_H_INCLUDED_)
#define _PACKET_SOURCE_H_INCLUDED_

#include "broadcom_pdf.h"
#include "trf_gen_v3.h"

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

GEN::Stream* CreateParetoStream( GEN::load_t load, float mean_burst )
{
    return new GEN::StreamPareto( load, mean_burst, 1.4F );
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

GEN::Stream* CreateExponStream(  GEN::load_t load, float mean_burst )
{
    return new GEN::StreamExpon( load, mean_burst );
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

GEN::Stream* CreateCBRStream(  GEN::load_t load, float mean_burst )
{
    return new GEN::StreamCBR( load, mean_burst );
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

GEN::Stream* CreateVideoStream( GEN::load_t load, float max_burst)
{
    return new GEN::StreamVideo( load, max_burst, 10000, 1.4F );
}


//class GEN 
//:public PACKET_GENERATOR_environment<DESL::time_t,int16s,int32s,float,int16s> {};

////////////////////////////////////////////////////////////////////////////////////
class Packet: public Pckt_Data_t 
{ 
    friend class PacketPool;

private:
    ///////////////////////////////////////////////////////
    Packet* pPrev;  /* pointer to a previous packet */
    Packet* pNext;  /* pointer to a next packet     */

    ///////////////////////////////////////////////////////
    Packet()    { pPrev = pNext = NULL; }
    ~Packet()   {}

public:
    

    ///////////////////////////////////////////////////////
    inline Packet*  GetPrev( void )         { return pPrev; }
    inline Packet*  GetNext( void )         { return pNext; }
    inline void     SetPrev( Packet* prv )  { pPrev = prv;  }
    inline void     SetNext( Packet* nxt )  { pNext = nxt;  }

    Packet& operator= ( const Pckt_Data_t& pckt )
    {
        *(Pckt_Data_t*)this = pckt;
        return *this;
    }
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class PacketPool 
{
private:
    static PDList< Packet > ppPool;

protected:
public:
    ///////////////////////////////////////////////////////////////////////////
    // STATIC METHODS
    ///////////////////////////////////////////////////////////////////////////
    inline static void RecycleAllPackets( PDList< Packet > *ptr )
    {
        ppPool.Combine( ptr );
    }
    ///////////////////////////////////////////////////////////////////////////
    inline static void ReleaseAllPackets( void )  
    { 
        while( ppPool.GetCount() > 0 ) delete ppPool.RemoveHead(); 
    }
    ///////////////////////////////////////////////////////////////////////////
    inline static Packet* AllocatePacket(void)
    { 
        return ppPool.GetCount() > 0 ? ppPool.RemoveHead(): new Packet; 
    }
    ///////////////////////////////////////////////////////////////////////////
    inline static void DestroyPacket( Packet* pPckt )  
    { 
        ppPool.InsertHead( pPckt ); 
    }
    ///////////////////////////////////////////////////////////////////////////
};

/** Initialize static members **************************************/
PDList< Packet >     PacketPool::ppPool;
/*******************************************************************/



///////////////////////////////////////////////////////////////////////////
// Callback function
///////////////////////////////////////////////////////////////////////////
int32s broadcom_frequency( int32s n )
{
    return round<int32u>( upstrm_size_pdf[n] * 0x7FFFFFFFL );
}



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////
//// Class PacketSource generates stream of packets with a given size distribution
////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define PACKET_GEN GEN::PacketGeneratorDist<int32s, MAX_PACKET_SIZE + 1, broadcom_frequency>

class PacketSource : public SimBase<>, public PACKET_GEN
{
private:    
        DESL::evnt_t*    SClock;
        int32u           ByteTime;

protected:
    inline void SetNextPacketTimer( void )
    {
        GEN::Packet nxt_pckt  = GetNextPacket();
        SClock                = DESL::AllocateEvent();
        SClock->Consumer      = this;
        SClock->Type          = EV_TIMER_NEXT_PACKET;
        SClock->Pckt.PcktTime = DESL::GlobalTime() + nxt_pckt.Interval * ByteTime;
        SClock->Pckt.PcktSize = nxt_pckt.PcktSize;
        SClock->Pckt.SourceId = nxt_pckt.SourceId;

        RegisterEvent( SClock, nxt_pckt.Interval * ByteTime );
    }

public:

    PacketSource( int16s                byte_time, 
                  GEN::pckt_size_t      ifg, 
                  float                 mean_burst,
                  GEN::PF_STREAM_CTOR   pf_strm, 
                  int16s                pool_size,
                  GEN::load_t           load, 
                  GEN::source_id_t      src_id,
                  DESL::obid_t          id = 0 ) 
    : SimBase<>( id ), PACKET_GEN( src_id, ifg, mean_burst, pf_strm, pool_size, load )
    {
        SClock    = NULL;
        ByteTime  = byte_time;
        SetNextPacketTimer();   /* set timer to next packet */
    }


    virtual ~PacketSource()    {}
    ///////////////////////////////////////////////////////////////////////////
    virtual void Free( void ) 
    { 
        PACKET_GEN::Clear();
    }
    ///////////////////////////////////////////////////////////////////////////
    void Reset( void )
    {
        SClock = NULL;
        PACKET_GEN::Reset();
        SetNextPacketTimer();   /* set timer to next packet */
    }

    ///////////////////////////////////////////////////////////////////////////
    void SetLoad( GEN::load_t load )
    {
        DESL::CancelEvent( SClock );
        SetLoadReset( load );
        SetNextPacketTimer();   /* set timer to next packet */
    }
    
    ///////////////////////////////////////////////////////////////////////////
    inline void OutputPacket( DESL::evnt_t* pEvent ) 
    {
        if( pEvent == SClock ) //&& pEvent->Type == EV_TIMER_NEXT_PACKET )
        {
            pEvent->Type     = EV_PCKT_ARRIVAL;   /* change pEvent into 'arrival' event */
            pEvent->Consumer = OutPort[0];

            RegisterEvent( pEvent, 0 );           /* send immediate arrival event */ 
            SetNextPacketTimer();                 /* set timer to next packet */                           
        }
    }    
    ////////////////////////////////////////////////////////////////////////////////

    virtual void ProcessEvent( DESL::evnt_t* pEvent ) 
    {
        if( pEvent->Type == EV_TIMER_NEXT_PACKET )
            OutputPacket( pEvent );
        else
            MSG_WARN( "Unhandled event in class PacketSource (Type = " << pEvent->Type << " )" );
    }    
    ///////////////////////////////////////////////////////////////////////////
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
////
//// Class CBRSource generates CBR stream of packets of constant size
////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class CBRSource: public SimBase<>, public GEN::StreamCBR
{
private:    
    int32u              ByteTime;
    GEN::pckt_size_t    PcktSize;
    GEN::source_id_t    SourceId;

protected:
    inline void SetNextPacketTimer( void )
    {
        ExtractBurst();

        DESL::evnt_t* ptr = DESL::AllocateEvent();
        ptr->Type         = EV_TIMER_NEXT_PACKET;
        ptr->Consumer     = this;

        RegisterEventAbs( ptr, (GetArrival() + _OVERHEAD( PcktSize )) * ByteTime );
    }

public:
    CBRSource(  int32u           byte_time, 
                GEN::pckt_size_t pckt_size,
                GEN::load_t      ld, 
                GEN::source_id_t source_id, 
                DESL::obid_t     id = 0 ) : SimBase<>( id ), GEN::StreamCBR( ld, pckt_size ) // burst = 1 packet
    {
        ByteTime = byte_time;
        PcktSize = pckt_size;
        SourceId = source_id;
        Reset();
    } 

    virtual ~CBRSource()    {}
    
    ///////////////////////////////////////////////////////////////////////////
    virtual void Free( void ) 
    { 
    }
    ///////////////////////////////////////////////////////////////////////////
    void Reset( void )
    {
        StreamCBR::Reset();
        SetNextPacketTimer();
    }
    
    ///////////////////////////////////////////////////////////////////////////
    inline void OutputPacket( DESL::evnt_t* pEvent ) 
    {
        pEvent->Type          = EV_PCKT_ARRIVAL;   
        pEvent->Consumer      = OutPort[0];
        pEvent->Pckt.PcktTime = DESL::GlobalTime();
        pEvent->Pckt.PcktSize = PcktSize;
        pEvent->Pckt.SourceId = SourceId;

        RegisterEvent( pEvent, 0 );   /* send immediate arrival event */ 
        SetNextPacketTimer();         /* generate self-clock event */       
    }    
    ////////////////////////////////////////////////////////////////////////////////

    virtual void ProcessEvent( DESL::evnt_t* pEvent ) 
    {
        if( pEvent->Type == EV_TIMER_NEXT_PACKET )
            OutputPacket( pEvent );
        else
            MSG_WARN( "Unhandled event in class CBRSource (Type = " << pEvent->Type << " )" );
    }    
    ///////////////////////////////////////////////////////////////////////////
};



#endif // _PACKET_SOURCE_H_INCLUDED_
