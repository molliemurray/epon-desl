/**********************************************************
 * Filename:    ONU.h
 *
 * Description: This file contains declarations for
 *              class ONU
 *
 * Author: Glen Kramer (kramer@cs.ucdavis.edu)
 *         University of California @ Davis
 *********************************************************/
#ifndef _ONU_H_INCLUDED_
#define _ONU_H_INCLUDED_


const int16s NOT_SENDING  = -1;

class ONU : public SimBase<>, public PacketPool
{
private:
    PDList< Packet >  FIFO;                // FIFO queue implemented as linked list
    int32s            QueueBytes;          // number of bytes in the queue

    DESL::time_t      LastSent;            // transmission timestamp of the last packet
    DESL::time_t      SlotEnd;             // time when current slot ends

    BOOL              Sending;             // indication whether a queue is currently transmitting 

    ////////////////////////////////////////////////////////////////////////////////
    // FUNCTION:    void EnqueuePacket( const Pckt_Data_t& pckt )
    // DESCRIPTION: Adds packet to FIFO queue, updates counters
    // NOTES:
    ////////////////////////////////////////////////////////////////////////////////
    inline void EnqueuePacket( const Pckt_Data_t& pckt )
    {
        Packet* ptr = AllocatePacket();
        *ptr = pckt;
        FIFO.Append( ptr ); 
        QueueBytes += pckt.PcktSize;
    }

    ////////////////////////////////////////////////////////////////////////////////
    // FUNCTION:    Pckt_Data_t DequeuePacket( void )
    // DESCRIPTION: Removes packet from FIFO queue, updates counters
    // NOTES:
    ////////////////////////////////////////////////////////////////////////////////
    inline Pckt_Data_t DequeuePacket( void )
    {
        Pckt_Data_t pckt = { 0, 0, 0 };
        Packet*     ptr = FIFO.RemoveHead();
        if( ptr ) 
        {
            pckt = *ptr;
            DestroyPacket( ptr );

            QueueBytes -= pckt.PcktSize;
        }
        return pckt; 
    }

   
    ////////////////////////////////////////////////////////////////////////////////
    // FUNCTION:    BOOL ReceiveDataPacket( DESL::evnt_t* pEvent )
    // DESCRIPTION: If packet fits into a queue, generate EV_PCKT_ENQUE event
    //                 else generate EV_PCKT_DROP event
    // NOTES:
    ////////////////////////////////////////////////////////////////////////////////
    inline void ReceiveDataPacket( DESL::evnt_t* pEvent )
     {
        if( QueueBytes + pEvent->Pckt.PcktSize <= BUFFER_SIZE ) // if enough space in buffer
        {
            EnqueuePacket( pEvent->Pckt );    // add packet to the queue 
            pEvent->Type = EV_PCKT_ENQUE;     // create EV_PCKT_ENQUE event    
        }
        else
        {
            pEvent->Type  = EV_PCKT_DROP;     // create EV_PCKT_ENQUE event
        }

        pEvent->Consumer = NULL;        
        RegisterEvent( pEvent );              // register an immediate event
    }

    ////////////////////////////////////////////////////////////////////////////////
    // FUNCTION:    BOOL ProcessGATE( DESL::evnt_t* pEvent )
    // DESCRIPTION: Receives GATE message, sets local time, and processes grants
    // NOTES:
    ////////////////////////////////////////////////////////////////////////////////
    inline void ProcessGATE( DESL::evnt_t* pEvent )
    {
        DESL::evnt_t* ptr;
        int32s        length = pEvent->GATE.Length;

        //////////////////////////////////////////////////////////////
        // update local time 
        //////////////////////////////////////////////////////////////
        LocalTime( pEvent->GATE.Timestamp ); 

        //////////////////////////////////////////////////////////////
        // make sure the grant is in the future
        //////////////////////////////////////////////////////////////
        if( pEvent->GATE.StartTime < LocalTime() + ONU_HW_PROCESS_DELAY )
        {
            MSG_WARN( "Late Grant for LLID " << _ONU_ID( ID ) );
            return;
        }

        //////////////////////////////////////////////////////////////
        // allocate space for Report message
        //////////////////////////////////////////////////////////////
        if( length >= _OVERHEAD( MPCP_PACKET_SIZE ))
        {
            length -= _OVERHEAD( MPCP_PACKET_SIZE );

            //////////////////////////////////////////////////////////////
            // Set timer to to pReport message
            //////////////////////////////////////////////////////////////
            ptr                   = DESL::AllocateEvent();
            ptr->Consumer         = this;
            ptr->Type             = EV_TIMER_GRANT_REPORT;
            RegisterEventAbs( ptr, pEvent->GATE.StartTime + _PON_TIME( length ));
        }
        else
            MSG_WARN( "Grant at ONU " << _ONU_ID( ID ) << " is too small for Report " );

        //////////////////////////////////////////////////////////////
        // Set timer to 'Grant for data'
        //////////////////////////////////////////////////////////////
        if( length >= _OVERHEAD( MIN_PACKET_SIZE ))
        {
            ptr                   = DESL::AllocateEvent();
            ptr->Consumer         = this;
            ptr->Type             = EV_TIMER_GRANT_DATA;
            ptr->GATE.Length      = length;
            RegisterEventAbs( ptr, pEvent->GATE.StartTime );
        }

        DESL::DestroyEvent( pEvent );
    }

     
    ////////////////////////////////////////////////////////////////////////////////
    // FUNCTION:    void StartSendingPacket( void )
    // DESCRIPTION: Starts packet transmission
    // NOTES:
    ////////////////////////////////////////////////////////////////////////////////
    inline void StartSendingPacket( void )
    {
        if( Sending == FALSE && FIFO.GetCount() > 0 )
        {
            // if currently not sending and a new packet available ... 
            if( LocalTime() + _PON_PCKT_TIME( FIFO.GetHead()->PcktSize ) <= SlotEnd )
            {
                Sending  = TRUE;

                // Generate timer event when packet finishes transmission 
                DESL::evnt_t* ptr = DESL::AllocateEvent();
                ptr->Consumer     = this;
                ptr->Type         = EV_PCKT_DEQUE;
                ptr->Pckt         = DequeuePacket();
                RegisterEvent( ptr, _PON_PCKT_TIME( ptr->Pckt.PcktSize ));
            }
        }
    }
   

    ////////////////////////////////////////////////////////////////////////////////
    // FUNCTION:    void FinishSendingPacket( DESL::evnt_t* pEvent )
    // DESCRIPTION: Finishes sending data packet and attempts to send a next one.
    // NOTES:
    ////////////////////////////////////////////////////////////////////////////////
    inline void FinishSendingPacket( DESL::evnt_t* pEvent )
    {
        pEvent->Type     = EV_PCKT_ARRIVAL;   // Generate immediate departure event 
        pEvent->Consumer = OutPort[0]; 
        RegisterEvent( pEvent );

        Sending  = FALSE;

        StartSendingPacket();                 // attempt to transmit next packet 
    }
    
    ////////////////////////////////////////////////////////////////////////////////
    // FUNCTION:    void OpenSlot( DESL::evnt_t* pEvent )
    // DESCRIPTION: Starts transmission of data frames
    // NOTES:
    ////////////////////////////////////////////////////////////////////////////////
    inline void OpenSlot( DESL::evnt_t* pEvent )
    {
        SlotEnd = LocalTime() + _PON_TIME( pEvent->GATE.Length );
        StartSendingPacket();
    }

   

    ////////////////////////////////////////////////////////////////////////////////
    // FUNCTION:    void SendpReport( DESL::evnt_t* pEvent )
    // DESCRIPTION: Specifies for each queue the queue sizes under the thresholds.
    //              Sends out the Report message
    // NOTES:
    ////////////////////////////////////////////////////////////////////////////////
    inline void SendREPORT( DESL::evnt_t* pEvent )
    {
        //////////////////////////////////////////////////////////
        // Create pReport message
        //////////////////////////////////////////////////////////
        pEvent->Consumer        = OutPort[0];
        pEvent->Type            = EV_MPCP_REPORT;
        //pEvent->RPRT.LLID       = _ONU_ID( ID );
        pEvent->RPRT.Timestamp  = LocalTime() + _PON_PCKT_TIME( MPCP_PACKET_SIZE );
        pEvent->RPRT.Length     = QueueBytes + FIFO.GetCount() * PACKET_OVERHEAD;

        RegisterEvent( pEvent, _PON_PCKT_TIME( MPCP_PACKET_SIZE ));
    }

public:
    ////////////////////////////////////////////////////////////////////////////////
    // FUNCTION:    ONU( DESL::obid_t id ): MultiPort<>( id )
    // DESCRIPTION: Constructor
    // NOTES:
    ////////////////////////////////////////////////////////////////////////////////
    ONU( DESL::obid_t id ) : SimBase<>( id )
    {
        Reset();
    }

    ////////////////////////////////////////////////////////////////////////////////
    // FUNCTION:    int32s GetQueueLength( void )
    // DESCRIPTION: Returns the total length of queues packets
    // NOTES:
    ////////////////////////////////////////////////////////////////////////////////
    int32s GetQueueLength( void )       { return QueueBytes; }

    ////////////////////////////////////////////////////////////////////////////////
    // FUNCTION:    void Reset( void )
    // DESCRIPTION: Clears all buffers in the ONU.
    // NOTES:
    ////////////////////////////////////////////////////////////////////////////////
    virtual void Reset( void )
    {
        Sending    = FALSE;
        SlotEnd    = 0;                // slot is closed at the beginning 
        QueueBytes = 0;
        RecycleAllPackets( &FIFO );
    }

    ////////////////////////////////////////////////////////////////////////////////
    // FUNCTION:    void Free( void )
    // DESCRIPTION: Frees all the resources allocated by 'this' object
    // NOTES:
    ////////////////////////////////////////////////////////////////////////////////
    virtual void Free( void ) 
    {
        Reset();
        ReleaseAllPackets();
    }

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
            // gate processing 
            case EV_MPCP_GATE:          ProcessGATE( pEvent );          break;
            case EV_TIMER_GRANT_REPORT: SendREPORT( pEvent );           break;
            case EV_TIMER_GRANT_DATA:   OpenSlot( pEvent );             break;
            
            // data processing 
            case EV_PCKT_ARRIVAL:       ReceiveDataPacket( pEvent );    break;
            case EV_PCKT_DEQUE:         FinishSendingPacket( pEvent );  break;
            default:                    MSG_WARN( "Unhandled event in ONU (Type = " << pEvent->Type << " )" );
        }
    }     
};


#endif /* _ONU_H_INCLUDED_ */