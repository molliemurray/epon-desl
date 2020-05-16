/**********************************************************
 * Filename:    link.h
 *
 * Description: This file contains declarations for
 *              class LossLessLink
 *              class LossyLink
 *              class BiDirLink
 *              class JitterLink
 *
 * Author: Glen Kramer (kramer@cs.ucdavis.edu)
 *         University of California @ Davis
 *********************************************************/

#ifndef _LINK_H_INCLUDED_ 
#define _LINK_H_INCLUDED_ 

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class LossLessLink : public SimBase<>
{
private:
    DESL::time_t Delay; 

public:

    LossLessLink( DESL::time_t delay, DESL::obid_t id = 0 ) : SimBase<>( id )  
    {
        Delay = delay;
    } 

    ///////////////////////////////////////////////////////////////////////////
    virtual void  Free( void )      {}
    virtual void  Reset( void )     {}
    
    ///////////////////////////////////////////////////////////////////////////
    virtual void ProcessEvent( DESL::evnt_t* pEvent ) 
    {
        pEvent->Consumer = OutPort[0];        /* redirect the event    */
        RegisterEvent( pEvent, Delay );       /* register with 'Delay' */
    } 
    
    ///////////////////////////////////////////////////////////////////////////
    inline void          SetDelay( DESL::time_t dly )     { Delay = dly;  }
    inline DESL::time_t  GetDelay( void ) const           { return Delay; }
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class LossyLink : public LossLessLink
{
private:
    DOUBLE LossProb;

public:
    LossyLink( DESL::time_t delay, DOUBLE loss_prob, DESL::obid_t id = 0 ): LossLessLink( delay, id )   
    {
        LossProb = loss_prob;
    } 

    ///////////////////////////////////////////////////////////////////////////
    virtual void ProcessEvent( DESL::evnt_t* pEvent ) 
    {
        if( _uniform_real_0_1() > LossProb ) LossLessLink::ProcessEvent( pEvent );
    } 
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class BiDirLink : public SimBase< 2 >
{
private:
    DESL::time_t Delay; 

public:

    BiDirLink( DESL::time_t delay, DESL::obid_t id = 0 ) : SimBase< 2 >( id )  
    {
        Delay = delay;
    } 

    ///////////////////////////////////////////////////////////////////////////
    virtual void  Free( void )      {}
    virtual void  Reset( void )     {}
    
    ///////////////////////////////////////////////////////////////////////////
    virtual void ProcessEvent( DESL::evnt_t* pEvent ) 
    {
        /* redirect the event    */
        pEvent->Consumer = OutPort[ pEvent->Producer == OutPort[0]? 1: 0 ];
        RegisterEvent( pEvent, Delay );      /* register with 'Delay' */
    }    

    ///////////////////////////////////////////////////////////////////////////
    inline void          SetDelay( DESL::time_t dly )  { Delay = dly;  }
    inline DESL::time_t  GetDelay( void ) const        { return Delay; }
};


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
typedef DESL::time_t (*pf_jitter)(void);

class JitterLink : public SimBase<>
{
private:
    DESL::time_t Delay; 
    DESL::time_t LastEvent;
    pf_jitter    Jitter;
    

public:
    JitterLink( DESL::time_t delay, pf_jitter jitter_fun, DESL::obid_t id = 0 ) : SimBase<>(id) 
    {
        Delay     = delay;
        Jitter    = jitter_fun;
        LastEvent = 0;
    } 

    ///////////////////////////////////////////////////////////////////////////
    virtual void  Free( void )      {}
    virtual void  Reset( void )     {}
    
    ///////////////////////////////////////////////////////////////////////////
    inline void          SetDelay( DESL::time_t dly )     { Delay = dly;  }
    inline DESL::time_t  GetDelay( void ) const           { return Delay; }

    ///////////////////////////////////////////////////////////////////////////
    virtual void ProcessEvent( DESL::evnt_t* pEvent ) 
    {
        pEvent->Consumer = OutPort[0];              // redirect the event    
        RegisterEvent( pEvent, Delay + Jitter() );  // register with 'Delay' 
    } 
    
    ///////////////////////////////////////////////////////////////////////////
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif // _LINK_H_INCLUDED_
