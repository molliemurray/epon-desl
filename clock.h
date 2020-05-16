/////////////////////////////////////////////////////////////////////
// Filename:    clock.h
//
// Description: This file contains declarations for
//                  class CClock
//                  class CClockSync
//
// Author: Glen Kramer (kramer@cs.ucdavis.edu)
//         University of California @ Davis
/////////////////////////////////////////////////////////////////////

#ifndef _CLOCK_H_INCLUDED_
#define _CLOCK_H_INCLUDED_

/////////////////////////////////////////////////////////////////////
// CLASS:        class CClock 
// PURPOSE:      Represents a base class with a local clock having 
//               both offset and drift compared to the global 
//               (simulation) clock
/////////////////////////////////////////////////////////////////////
#define DRIFT_PERIOD  1000000

class CClock : public DESL::CBase
{
private:
    DESL::time_t timeOffset;
    int32s       clockDrift;   // clock drift (local ticks per 1,000,000 global ticks)

    
    /////////////////////////////////////////////////////////////////
    inline DESL::time_t local_to_global( DESL::time_t lt ) const
    {
        return ( lt * DRIFT_PERIOD ) / clockDrift;
    }
    /////////////////////////////////////////////////////////////////
    inline DESL::time_t global_to_local( DESL::time_t gt ) const
    {
         return ( gt * clockDrift ) / DRIFT_PERIOD;   
    }
    /////////////////////////////////////////////////////////////////
    inline DESL::time_t global_to_local( void ) const
    {
         return ( DESL::GlobalTime() * clockDrift ) / DRIFT_PERIOD;   
    }



protected:
    /////////////////////////////////////////////////////////////////
    // METHOD:       void RegisterEvent( evnt_t* ptr, time_t interval )
    // PURPOSE:      
    // ARGUMENTS:    ptr      - pointer to CEvent
    //               interval - time interval to event's occurence
    // RETURN VALUE: 
    /////////////////////////////////////////////////////////////////
    inline void RegisterEvent( DESL::evnt_t* ptr, DESL::time_t interval = 0 ) 
    {   
        DESL::RegisterEvent( ptr, local_to_global( interval ), this );
    }
    
    /////////////////////////////////////////////////////////////////
    // METHOD:       void RegisterEventAbs( evnt_t* ptr, time_t localtime )
    // PURPOSE:      
    // ARGUMENTS:    ptr       - pointer to CEvent
    //               localtime - future local time when event will occur
    // RETURN VALUE: 
    /////////////////////////////////////////////////////////////////
    inline void RegisterEventAbs( DESL::evnt_t* ptr, DESL::time_t localtime ) 
    {   
        RegisterEvent( ptr, localtime - LocalTime() );
    }
    
public:
    /////////////////////////////////////////////////////////////////
    CClock( int16s clk_drift = 0, DESL::obid_t id = 0 ) : CBase( id )
    { 
        timeOffset = 0;                          // local time is same as global time 
        clockDrift = clk_drift + DRIFT_PERIOD;   // from ppm to ticks per 1,000,000
    }

    virtual~ CClock()         {}
    
    /////////////////////////////////////////////////////////////////
    // METHOD:       inline DESL::time_t LocalTime( void ) const
    // PURPOSE:      claculates and returns local time
    // ARGUMENTS:    
    // RETURN VALUE: 
    /////////////////////////////////////////////////////////////////
    inline DESL::time_t LocalTime( void ) const    
    { 
        return global_to_local() + timeOffset; 
    }

    /////////////////////////////////////////////////////////////////
    // METHOD:       inline void LocalTime( DESL::time_t tm ) 
    // PURPOSE:      claculates and sets timeOffset, s.t. 
    //               current local time is equal tm 
    // ARGUMENTS:    
    // RETURN VALUE: none
    /////////////////////////////////////////////////////////////////
    inline void LocalTime( DESL::time_t tm )  
    { 
        timeOffset = tm - global_to_local();   
    }
};




////////////////////////////////////////////////////////////////////
// CLASS:        class CClockSync 
// PURPOSE:      Represents a base class with a local clock 
//               synchronized with the global clock (i.e., clock 
//               drift = 0). The local clock may have non-zero offset  
//               compared to the global (simulation) clock
/////////////////////////////////////////////////////////////////////
class CClockSync : public DESL::CBase
{
private:
    DESL::time_t  timeOffset;

protected:
    /////////////////////////////////////////////////////////////////
    // METHOD:       void RegisterEvent( evnt_t* ptr, time_t interval )
    // PURPOSE:      
    // ARGUMENTS:    ptr      - pointer to CEvent
    //               interval - time interval to event's occurence
    // RETURN VALUE: 
    /////////////////////////////////////////////////////////////////
    inline void RegisterEvent( DESL::evnt_t* ptr, DESL::time_t interval = 0 ) 
    {   
        DESL::RegisterEvent( ptr, interval, this );
    }
    
    /////////////////////////////////////////////////////////////////
    // METHOD:       void RegisterEventAbs( evnt_t* ptr, time_t localtime )
    // PURPOSE:      
    // ARGUMENTS:    ptr       - pointer to CEvent
    //               localtime - future local time when event will occur
    // RETURN VALUE: 
    /////////////////////////////////////////////////////////////////
    inline void RegisterEventAbs( DESL::evnt_t* ptr, DESL::time_t localtime ) 
    {   
        RegisterEvent( ptr, localtime - LocalTime() );
    }
    
public:
    /////////////////////////////////////////////////////////////////
    CClockSync( DESL::obid_t id = 0 ) : CBase( id )  
    { 
        timeOffset = 0;           // local time is the same as global time 
    }
    virtual~ CClockSync()         {}

    inline DESL::time_t LocalTime( void ) const      { return DESL::GlobalTime() + timeOffset; }
    inline void         LocalTime( DESL::time_t tm ) { timeOffset = tm - DESL::GlobalTime();   }
};


#endif // _CLOCK_H_INCLUDED_ 




