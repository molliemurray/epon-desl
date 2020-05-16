
#ifndef __OLT_H_INCLUDED__ 
#define __OLT_H_INCLUDED__ 

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
class OLT : public SimBase< NUM_LLID >
{

private:
    DESL::time_t  ScheduleEnd;           // future time up to which the schedule exists
    DESL::time_t  LastPacketArrival;     // arrival time of the last packet
    int32s        MaxSlot;               // Maximum slot size


    ////////////////////////////////////////////////////////////////////////////////
    // FUNCTION:    void CheckPacketCollision( GEN::pckt_size_t pckt_size )
    // DESCRIPTION: 
    // NOTES:
    ////////////////////////////////////////////////////////////////////////////////
    inline void CheckPacketCollision( GEN::pckt_size_t pckt_size )
    {
        if( LastPacketArrival + _PON_PCKT_TIME( pckt_size ) > LocalTime() )
            MSG_WARN( "OLT detected collided packets" );

        LastPacketArrival = LocalTime();
    }    

    ////////////////////////////////////////////////////////////////////////////////
    // FUNCTION:    void ReceiveDataPacket( DESL::evnt_t* pEvent )
    // DESCRIPTION: 
    // NOTES:
    ////////////////////////////////////////////////////////////////////////////////
    inline void ReceiveDataPacket( DESL::evnt_t* pEvent )
    {
        CheckPacketCollision( pEvent->Pckt.PcktSize );

        /////////////////////////////////////////////////////////
        // Keep track of the bytes received by each LLID
        /////////////////////////////////////////////////////////
        //int16s llid = _LNK_ID( pEvent->Producer->ID );
        //...ReceivedBytes[ llid ] += _OVERHEAD( pEvent->Pckt.PcktSize );

        DESL::DestroyEvent( pEvent );
    }

    ////////////////////////////////////////////////////////////////////////////////
    // FUNCTION:    void ReceiveREPORTPacket( DESL::evnt_t* pEvent )
    // DESCRIPTION: 
    // NOTES:
    ////////////////////////////////////////////////////////////////////////////////
    inline void ReceiveREPORTPacket( DESL::evnt_t* pEvent )
    {
        CheckPacketCollision( MPCP_PACKET_SIZE );

        //////////////////////////////////////////////////////////
        // measure RTT
        //////////////////////////////////////////////////////////
        DESL::time_t rtt    = LocalTime() - pEvent->RPRT.Timestamp;

        DESL::evnt_t* ptr   = DESL::AllocateEvent();
        ptr->Type           = EV_MPCP_GATE;
        ptr->Consumer       = pEvent->Producer;
        ptr->GATE.Timestamp = LocalTime() + _PON_PCKT_TIME( MPCP_PACKET_SIZE ) + OLT_HW_PROCESS_DELAY; 
        ptr->GATE.StartTime = MAX( ptr->GATE.Timestamp + ONU_HW_PROCESS_DELAY, ScheduleEnd - rtt );
        
        //////////////////////////////////////////////////////////
        // scheduling disciplines
        //////////////////////////////////////////////////////////

        // a. Fixed service
        //ptr->GATE.Length = MaxSlot; 

        // b. Limited service
        ptr->GATE.Length = MIN<int32s>( pEvent->RPRT.Length + _OVERHEAD( MPCP_PACKET_SIZE ), MaxSlot ); 

        // c. Gated service
        //ptr->GATE.Length = pEvent->RPRT.Length + _OVERHEAD( MPCP_PACKET_SIZE ); 

        // d. Constant Credit service
        //ptr->GATE.Length = MIN<int32s>( pEvent->RPRT.Length + _OVERHEAD(MPCP_PACKET_SIZE) + _OVERHEAD(MAX_PACKET_SIZE), MaxSlot );  

        // e. Linear Credit service
        //ptr->GATE.Length = MIN<int32s>( pEvent->RPRT.Length * 1.2 + _OVERHEAD(MPCP_PACKET_SIZE), MaxSlot ); 

        // f. Elastic service
        //static int32s last_grant[NUM_LLID];
        //int32s total_granted = 0;

        //for( int16s ndx = 0; ndx < NUM_LLID; ndx++ )
        //    total_granted += last_grant[ndx];

        //ptr->GATE.Length = MIN<int32s>( pEvent->RPRT.Length + _OVERHEAD(MPCP_PACKET_SIZE), MAX<int32s>( NUM_LLID * MaxSlot - total_granted, 0 ));
        //last_grant[ _LNK_ID( pEvent->Producer->ID ) ] = ptr->GATE.Length;


        //////////////////////////////////////////////////////////
        // Issue GATE message
        //////////////////////////////////////////////////////////
        RegisterEventAbs( ptr, ptr->GATE.Timestamp );
        ScheduleEnd = ptr->GATE.StartTime + rtt + _PON_TIME( ptr->GATE.Length ) + GUARD_BAND_TIME;
    }


    ////////////////////////////////////////////////////////////////////////////////
    // FUNCTION:    void SimplifiedDiscovery( void )
    // DESCRIPTION: send a unicast discovery GATE to eack logical port (each LLID)
    // NOTES:
    ////////////////////////////////////////////////////////////////////////////////
    inline void SimplifiedDiscovery( void )
    {

        DESL::evnt_t* ptr;
        DESL::time_t  timestamp = LocalTime();


        for( int16s ndx = 0; ndx < NUM_LLID; ndx++)
        {
            ptr                 = DESL::AllocateEvent();
            ptr->Type           = EV_MPCP_GATE;
            ptr->Consumer       = GetPort( ndx );
            ptr->GATE.Timestamp = timestamp; 
            ptr->GATE.Length    = _OVERHEAD( MPCP_PACKET_SIZE );  // only enough space to send one REPORT message
            ptr->GATE.StartTime = MAX( ptr->GATE.Timestamp + ONU_HW_PROCESS_DELAY, ScheduleEnd );

            RegisterEventAbs( ptr, ptr->GATE.Timestamp );

            ScheduleEnd = ptr->GATE.StartTime + 2 * PON_MAX_LINK_DISTANCE * FIBER_DELAY + GUARD_BAND_TIME;
            timestamp  += _PON_PCKT_TIME( MPCP_PACKET_SIZE ) + OLT_HW_PROCESS_DELAY;
        }
    }
   
        

public:
    ////////////////////////////////////////////////////////////////////////////////
    // FUNCTION:    OLT( DESL::obid_t id ) : MultiPort< NUM_LLID >( id )
    // DESCRIPTION: Constructor
    // NOTES:
    ////////////////////////////////////////////////////////////////////////////////
    OLT( DESL::obid_t id ) : SimBase< NUM_LLID >( id )
    {
        Reset();
        MaxSlot = MAX_SLOT;
    }

    ////////////////////////////////////////////////////////////////////////////////
    // FUNCTION:    void Free( void )
    // DESCRIPTION: 
    // NOTES:
    ////////////////////////////////////////////////////////////////////////////////
    inline void   SetMaxSlot( int32s slot ) { MaxSlot = slot; }
    inline int32s GetMaxSlot(void)  const   { return MaxSlot; }

    ////////////////////////////////////////////////////////////////////////////////
    // FUNCTION:    void Reset( void )
    // DESCRIPTION: 
    // NOTES:
    ////////////////////////////////////////////////////////////////////////////////
    virtual void Reset( void )
    {
        ScheduleEnd       = 
        LastPacketArrival = LocalTime();

        SimplifiedDiscovery();
    }

    ////////////////////////////////////////////////////////////////////////////////
    // FUNCTION:    void Free( void )
    // DESCRIPTION: 
    // NOTES:
    ////////////////////////////////////////////////////////////////////////////////
    virtual void Free( void ) {}

    ////////////////////////////////////////////////////////////////////////////////
    // FUNCTION:    void ProcessEvent( DESL::evnt_t* pEvent )
    // DESCRIPTION: Event Dispatcher
    // NOTES:       
    // RETURN:      
    ////////////////////////////////////////////////////////////////////////////////
    virtual void ProcessEvent( DESL::evnt_t* pEvent ) 
    {
        switch( pEvent->Type )
        {
            case EV_MPCP_REPORT:                ReceiveREPORTPacket( pEvent );      break;
            case EV_PCKT_ARRIVAL:               ReceiveDataPacket( pEvent );        break;
            default:  MSG_WARN( "Unhandled event in OLT (Type = " << pEvent->Type << " )" );
        }
    }   
};

#endif  // __OLT_H_INCLUDED__

