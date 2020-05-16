/////////////////////////////////////////////////////////////////////////
// Filename:    trf_gen.h
//
// Description: This file contains declarations for
//              class Packet 
//              class Stream
//              class StreamPareto
//              class StreamExpon
//              class StreamCBR
//              class StreamVideo
//
//              class PacketGenerator
//              class PacketGeneratorDist
//
//
// Author:      Glen Kramer (kramer@cs.ucdavis.edu)
//              University of California, Davis
//
// Copyright (c) 2000-2005, Glen Kramer. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person 
// obtaining a copy of this software and associated documentation 
// files (the "Software"), to deal in the Software without restriction, 
// including without limitation the rights to use, copy, modify, merge, 
// publish, distribute, sublicense, and/or sell copies of the Software, 
// and to permit persons to whom the Software is furnished to do so, 
// subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be 
// included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY 
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
/////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////
// 12/2/2005 - Changed Stream::Reset() function to prevent alignment of 
//             OFF periods upon reset.
// ----------------------------------------------------------------------
// 
//
/////////////////////////////////////////////////////////////////////////




#ifndef _TRF_GEN_H_V003_INCLUDED_
#define _TRF_GEN_H_V003_INCLUDED_


#include "_types.h"
#include "_rand_MT.h"
#include "avltree.h"

template < class T > inline T SetInRange( T x, T y, T z ) 
{ 
    if( x < y ) return y;
    if( x > z ) return z;
    return x;
}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
///
/// namespace PACKET_GENERATOR_environment
///
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
namespace GEN
{

    typedef  int32u         pause_size_t;
    typedef  int32u         burst_size_t;
    
    typedef  int64u         bytestamp_t;
    typedef  int16u         pckt_size_t;
    typedef  int16s         source_id_t;

    typedef  float          load_t;
    typedef  float          shape_t;

    typedef  pckt_size_t   (*PF_PCKT_SIZE)( void ); 

    const shape_t           MIN_ALPHA   = 1.001F;
    const shape_t           MAX_ALPHA   = 1.999F;

    const load_t            MIN_LOAD    = 1.0E-10F;
    const load_t            MAX_LOAD    = 0.99999F;

    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    ///
    /// struct Packet
    ///
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    struct Packet
    {
        source_id_t  SourceId;
        pckt_size_t  PcktSize;
        pause_size_t Interval;

    };    

  
    
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    ///
    /// class Stream
    ///
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    class Stream: public AVL::AVLNode< bytestamp_t > 
    {

    #define BurstTime AVL::AVLNode<bytestamp_t>::NodeKey

    friend class PacketGenerator;

    private:
        burst_size_t  BurstSize;    // number of bytes in current burst
        
        /////////////////////////////////////////////////////////////////    
        virtual inline burst_size_t  NextBurstSize(void)  = 0;
        virtual inline pause_size_t  NextPauseSize(void)  = 0;
        

    public:
        Stream(): AVL::AVLNode< bytestamp_t >( 0 ) 
        {
            BurstSize = 0;
        }

        virtual ~Stream() {}
 
        /////////////////////////////////////////////////////////////////
        virtual inline void SetLoad( load_t ) = 0;

        /////////////////////////////////////////////////////////////////
        inline void SetLoadRecursive( load_t load )
        {
            SetLoad( load );
            if( LChild )  ((Stream*)LChild)->SetLoadRecursive( load );
            if( RChild )  ((Stream*)RChild)->SetLoadRecursive( load );
        }
       
        /////////////////////////////////////////////////////////////////
        inline void Reset(void)
        {
            BurstSize = NextBurstSize();
            BurstTime = NextPauseSize() + BurstSize;

            // quick start: simulate start at random time during ON- or OFF-period
            bytestamp_t start_time = _uniform_int_( 0, (rnd_int_t)BurstTime );

            if( start_time < BurstSize )  // zero time fell on ON period 
            {
                BurstSize -= (burst_size_t)start_time;
                BurstTime = 0;
            }
            else  // zero time fell on OFF period
            {
                BurstSize = NextBurstSize(); 
                BurstTime -= start_time;
            }
        }

        /////////////////////////////////////////////////////////////////
        inline bytestamp_t   GetArrival(void)   const  { return BurstTime; }
        inline burst_size_t  GetBurstSize(void) const  { return BurstSize; }

        /////////////////////////////////////////////////////////////////
        // FUNCTION:    void ExtractPacket(void)
        // DESCRIPTION: Generates new burst
        // NOTES:       
        /////////////////////////////////////////////////////////////////
        inline void ExtractBurst(void)
        {
            BurstTime += BurstSize + NextPauseSize(); // Update BurstTime to point to 
                                                      // PauseSize after the end of burst.

            BurstSize  = NextBurstSize();             // Get next burst size.
                                                      // This burst starts at BurstTime.
        }
        /////////////////////////////////////////////////////////////////
    };  // class Stream



    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    ///
    /// StreamPareto
    ///
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    class StreamPareto : public Stream
    {
    private:
        float    MinBurst;    // minimum burst length (in bytes)
        float    MinPause;    // minimum inter-burst gap value (in bytes) 
        shape_t  Shape;       // shape parameter for burst distribution   

        virtual inline burst_size_t NextBurstSize(void) { return round<burst_size_t>(_pareto_(Shape) * MinBurst ); }
        virtual inline pause_size_t NextPauseSize(void) { return round<pause_size_t>(_pareto_(Shape) * MinPause ); }

        /////////////////////////////////////////////////////////////////

    public:
        StreamPareto( load_t ld, float mean_burst, shape_t shape ) : Stream()
        { 
            Shape = SetInRange<shape_t>( shape,  MIN_ALPHA, MAX_ALPHA );
            MinBurst = mean_burst * ( 1.0F - 1.0F / Shape );

            SetLoad( ld );
            Reset();
        }

        virtual ~StreamPareto()       {}

        /////////////////////////////////////////////////////////////////
        virtual inline void SetLoad( load_t load )
        {
            MinPause = MinBurst * ( 1.0F / SetInRange(load, MIN_LOAD, MAX_LOAD) - 1.0F );
        }
    };  // class StreamPareto



    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    ///
    /// class StreamExpon
    ///
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    class StreamExpon : public Stream
    {
    private:
        float    MeanPause;      // mean inter-burst gap value (in bytes)
        float    MeanBurst;      // mean burst size (in bytes)

        virtual  inline burst_size_t NextBurstSize(void) { return round<burst_size_t>(_exponent_() * MeanBurst); }
        virtual  inline pause_size_t NextPauseSize(void) { return round<pause_size_t>(_exponent_() * MeanPause); }

        /////////////////////////////////////////////////////////////////
                
    public:

        StreamExpon( load_t ld, float mean_burst ) : Stream()
        { 
            MeanBurst = mean_burst;
            SetLoad( ld );
            Reset();
        }
        /////////////////////////////////////////////////////////////////
        virtual ~StreamExpon()       {}
        /////////////////////////////////////////////////////////////////
       
        virtual inline void SetLoad( load_t load )
        {
            MeanPause = MeanBurst * ( 1.0F / SetInRange(load, MIN_LOAD, MAX_LOAD) - 1.0F );
        }
        /////////////////////////////////////////////////////////////////

    };  // class StreamExpon


    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    ///
    /// class StreamCBR
    ///
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    class StreamCBR : public Stream
    {
    private:
        burst_size_t BurstSize;        // burst length (in bytes)
        pause_size_t PauseSize;        // inter-burst gap value (in bytes)

        virtual  inline burst_size_t  NextBurstSize(void) { return BurstSize; }
        virtual  inline pause_size_t  NextPauseSize(void) { return PauseSize; }

        /////////////////////////////////////////////////////////////////
        
    public:

        StreamCBR( load_t ld, float mean_burst ) : Stream()
        { 
            BurstSize = round<burst_size_t>( mean_burst );
            SetLoad( ld );
            Reset();
        }
        /////////////////////////////////////////////////////////////////
        virtual ~StreamCBR()       {}
        /////////////////////////////////////////////////////////////////

        virtual inline void SetLoad( load_t load )
        {
            PauseSize = round<pause_size_t>(BurstSize * (1.0F / SetInRange(load, MIN_LOAD, MAX_LOAD) - 1.0F));
        }
        /////////////////////////////////////////////////////////////////

    };  // class StreamCBR


    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    ///
    /// class StreamVideo
    ///
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    class StreamVideo : public Stream
    {
    private:
        burst_size_t    Tokens;
        burst_size_t    LastBurst;
        pause_size_t    BurstPrd;      // burst period
        burst_size_t    MinBurst;      // min burst size (in bytes)
        burst_size_t    MaxBurst;      // max burst size (in bytes)
        shape_t         Shape;         // shape parameter for burst distribution

        virtual  inline burst_size_t NextBurstSize(void) 
        { 
            Tokens   += round<burst_size_t>(_pareto_(Shape) * MinBurst ); 
            LastBurst = MIN( Tokens, MaxBurst );
            Tokens   -= LastBurst;

            return LastBurst;
        }

        virtual  inline pause_size_t NextPauseSize(void) 
        { 
            return round<pause_size_t>( BurstPrd - LastBurst ); 
        }
        /////////////////////////////////////////////////////////////////

       
    public:
        //StreamVideo(load_t ld, burst_size_t max_burst, pause_size_t burst_period, shape_t shape) : Stream()

        StreamVideo( load_t ld, float max_burst, pause_size_t burst_period, shape_t shape ) : Stream()
        { 
            Shape = SetInRange<shape_t>( shape,  MIN_ALPHA, MAX_ALPHA );
            MaxBurst = max_burst;
            //MaxBurst = round<burst_size_t>( max_burst );
            BurstPrd = burst_period;

            SetLoad( ld );
            Reset();
        }
        /////////////////////////////////////////////////////////////////
        virtual ~StreamVideo()       {}
        /////////////////////////////////////////////////////////////////
                
        virtual inline void SetLoad( load_t load )
        {
            MinBurst = round<burst_size_t>( (1.0 - 1.0/Shape) * SetInRange(load, MIN_LOAD, MAX_LOAD) * BurstPrd );
        }
    };  // class StreamVideo



    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    ///
    /// class PacketGenerator
    ///
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    typedef Stream*  (*PF_STREAM_CTOR)( load_t, float );

    class PacketGenerator
    {
        //////////////////////////////////////////////////////////////////
        // Nested class StreamPool
        //////////////////////////////////////////////////////////////////
        class StreamPool: public AVL::AVLTree< bytestamp_t >
        {
        public:
            inline Stream* GetRoot( void )  { return static_cast<Stream*>( pRoot ); }
        };


    protected:
        
        StreamPool      Pool1, Pool2;
        StreamPool*     BusyPool;          
        StreamPool*     IdlePool;

        Packet          NextPacket;
        bytestamp_t     Elapsed;     // Time elapsed since last reset
        pckt_size_t     MinIFG;      // Minimum inter-packet gap
        burst_size_t    Tokens;


        PF_PCKT_SIZE    pfPcktSize;


        /////////////////////////////////////////////////////////////////

    public:
        /////////////////////////////////////////////////////////////////
        // FUNCTION:    PacketGenerator( pckt_size_t IFG )
        // DESCRIPTION: Constructor
        /////////////////////////////////////////////////////////////////
        PacketGenerator( source_id_t source_id, pckt_size_t inter_packet_gap )                   
        { 
            MinIFG    = inter_packet_gap;
            BusyPool  = &Pool1;
            IdlePool  = &Pool2;

            Tokens = 0;
            Elapsed   = 0;

            NextPacket.SourceId = source_id;
            NextPacket.PcktSize = 0;
            NextPacket.Interval = NextPacket.PcktSize + MinIFG;
        }

        /////////////////////////////////////////////////////////////////
        PacketGenerator( source_id_t     source_id,
                         pckt_size_t     inter_packet_gap, 
                         float           mean_burst,
                         PF_STREAM_CTOR  pf_strm,
                         PF_PCKT_SIZE    pf_size,
                         int16s          pool_size,
                         load_t          load )                   
        {
            MinIFG     = inter_packet_gap;
            BusyPool   = &Pool1;
            IdlePool   = &Pool2;
            pfPcktSize = pf_size;
            Tokens  = 0;
            Elapsed    = 0;
            
            NextPacket.SourceId = source_id;
            NextPacket.PcktSize = pfPcktSize();
            NextPacket.Interval = NextPacket.PcktSize + MinIFG;

            for( int16s s = 0; s < pool_size; s++ )
            {
                AddStream( pf_strm( load / pool_size, mean_burst ) );
            }

        }

        /////////////////////////////////////////////////////////////////
        // FUNCTION:    ~PacketGenerator()
        // DESCRIPTION: Destructor
        // NOTES:
        /////////////////////////////////////////////////////////////////
        virtual ~PacketGenerator()        { Clear(); }

        /////////////////////////////////////////////////////////////////
        // FUNCTION:    void Reset( void )
        // DESCRIPTION: resets all statistic variables and elapsed time
        // NOTES:       Reset should be called to generate multiple traces with same 
        //              set of sources 
        /////////////////////////////////////////////////////////////////
        void Reset( void )
        {
            for( Stream* pNode = RemoveStream(); pNode; pNode = RemoveStream() )
            {
                pNode->Reset();
                IdlePool->AddNode( pNode );
            }
        
            SWAP<StreamPool*>( BusyPool, IdlePool );
            Elapsed = 0;
        }

        /////////////////////////////////////////////////////////////////

        inline int32s GetStreams(void)  const { return BusyPool->GetCount(); }

        /////////////////////////////////////////////////////////////////
        // FUNCTION:    void AddSource( Source* )
        // DESCRIPTION: Adds a new source of to the PacketGenerator.
        /////////////////////////////////////////////////////////////////
        inline void AddStream( Stream* pSrc )         { BusyPool->AddNode( pSrc ); }

        /////////////////////////////////////////////////////////////////
        // FUNCTION:    RemoveSource( Source* )  
        // DESCRIPTION: Removes source
        // NOTES:
        /////////////////////////////////////////////////////////////////
        inline void     RemoveStream( Stream* pSrc )  { BusyPool->RemoveNode( pSrc ); }
        inline Stream*  RemoveStream( void )          { return (Stream*)BusyPool->RemoveHead(); }

        /////////////////////////////////////////////////////////////////
        // FUNCTION:    Packet PeekNextPacket(void)
        // DESCRIPTION: Peek at next packet before it arrives
        // NOTES:
        /////////////////////////////////////////////////////////////////
        inline Packet PeekNextPacket(void) const  { return NextPacket;  }


        /////////////////////////////////////////////////////////////////
        // FUNCTION:    Packet GetNextPacket(void)
        // DESCRIPTION: Return the next packet from the aggregated traffic
        // NOTES:
        /////////////////////////////////////////////////////////////////
        Packet GetNextPacket(void)
        {
            Stream*     pStrm;
            Packet      next_packet = NextPacket;
            pckt_size_t pckt_size   = pfPcktSize();
            bytestamp_t pckt_time   = Elapsed;

            // if the remaining burst size is less thn the packet size,
            // aggregate additional bursts
            while( Tokens < pckt_size && (pStrm = (Stream*)BusyPool->RemoveHead()) != NULL ) 
            {
                if( pStrm->GetArrival() > pckt_time + Tokens )
                    pckt_time = pStrm->GetArrival() - Tokens;

                Tokens += pStrm->GetBurstSize();
                
                pStrm->ExtractBurst();        // receive new burst
                BusyPool->AddNode( pStrm );   // place the stream in BusyPool
            }

            Tokens -= pckt_size;
            pckt_time += pckt_size + MinIFG;  // now pckt_time points to the last bit of the packet

            // update next packet
            NextPacket.PcktSize = pckt_size;
            NextPacket.Interval = (int32s)( pckt_time - Elapsed );

            Elapsed = pckt_time;

            return next_packet;
        }

      
        /////////////////////////////////////////////////////////////////
        // FUNCTION:    SetLoad( load_t load )
        // DESCRIPTION: Sets new load 
        // NOTES:
        /////////////////////////////////////////////////////////////////
        void SetLoad( load_t load )
        {
            if( BusyPool->GetRoot() )
                BusyPool->GetRoot()->SetLoadRecursive( load / GetStreams() );
        }
    
        /////////////////////////////////////////////////////////////////
        // FUNCTION:    void SetLoadReset( load_t load )
        // DESCRIPTION: Same as above, but calls Reset() to remove the long gaps 
        //              from the BusyPool
        // NOTES:       The new load is set to each stream after a burst extracted  
        //              from that stream. If current load is low, the next burst from  
        //              each stream will arrive too far in the future. That will delay  
        //              the assertion of the new load. To avoid this situation, Reset() 
        //              is called after setting a new load.
        /////////////////////////////////////////////////////////////////
        inline void SetLoadReset( load_t load )
        {
            load /= GetStreams();

            for( Stream* pNode = RemoveStream(); pNode; pNode = RemoveStream() )
            {
                pNode->SetLoad( load );
                pNode->Reset();
                IdlePool->AddNode( pNode );
            }
        
            SWAP<StreamPool*>( BusyPool, IdlePool );
            Elapsed = 0;
        }

        /////////////////////////////////////////////////////////////////
        // FUNCTION:    Clear( void )
        // DESCRIPTION: deletes all allocated streams
        // NOTES:
        /////////////////////////////////////////////////////////////////
        void Clear( void )
        {
            for( Stream*  pNode = RemoveStream(); pNode; pNode = RemoveStream() )
                delete pNode;

            Reset();
        }

    };  // class PacketGenerator



    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    ///
    /// class PacketGeneratorDist
    ///     Generates packets with a specified distribution of sizes
    ///
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////

    template< class T, int32s N, T (*PF_FREQUENCY)(int32s) > class PacketGeneratorDist : 
        public PacketGenerator, 
        public GenericDistribByIndex< T, N, PF_FREQUENCY >
    
    {
    private:
        static pckt_size_t GetPacketSize( void ) { return static_cast<pckt_size_t>( GenericDistribByIndex<T, N, PF_FREQUENCY>::GetIndex() ); }

    public:
        PacketGeneratorDist( source_id_t            source_id, 
                             pckt_size_t            inter_packet_gap, 
                             float                  mean_burst,
                             PF_STREAM_CTOR         pf_strm,
                             int16s                 pool_size,
                             load_t                 load ) : 
            PacketGenerator( source_id, 
                             inter_packet_gap, 
                             mean_burst, 
                             pf_strm,
                             PacketGeneratorDist< T, N, PF_FREQUENCY >::GetPacketSize,
                             pool_size,
                             load ), 
            GenericDistribByIndex< T, N, PF_FREQUENCY >() {}
        
    };  // class PacketGeneratorDist


   

};      // namespace GEN

#endif  // _TRF_GEN_H_V003_INCLUDED_
