/////////////////////////////////////////////////////////////////////////
// Filename:    desl.h
//
// Description: This file contains declarations of the
//              following classes:
//              class DESL_environment<T>
//              class CBase (nested)
//              class CEvent (nested)
//              class CEventQueue (nested)
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

#ifndef _DESL_H_V003_INCLUDED_
#define _DESL_H_V003_INCLUDED_

#include <crtdbg.h>     // needed for _ASSERT() macro 

#include "_stack.h"
#include "_list.h"
#include "avltree.h"

/////////////////////////////////////////////////////////////////////////
// CLASS:        template < class TIME_T, class DATA_T > 
//               class DESL_environment 
//
// PURPOSE:      use as 'parameterized namespace'
/////////////////////////////////////////////////////////////////////////
#define DESL_QUALIFIER DESL_environment< TIME_T, DATA_T >

template < class TIME_T, class DATA_T > class DESL_environment 
{
/////////////////////////////////////////////////////////////////////////
private:
/////////////////////////////////////////////////////////////////////////
    
    // Make constructors private to prevent creation of objects 
    // of type DESL_environment< TIME_T, DATA_T > 
    DESL_environment()  {};
    DESL_environment( const DESL_QUALIFIER& )  {};

/////////////////////////////////////////////////////////////////////////
public:
/////////////////////////////////////////////////////////////////////////
#define ACTIVATION_TIME  NodeKey

    extern class CEvent;
    extern class CEventQueue;
    extern class CBase;

    friend class CBase;

    typedef TIME_T        time_t;     // event time 
    typedef DATA_T        data_t;     // data associated with each event 
    typedef int16s        obid_t;     // object id 
    typedef class CBase   base_t;     // base class for all DESL classes 
    typedef class CEvent  evnt_t;     // CEvent class  

    /////////////////////////////////////////////////////////////////////
    // CLASS:        class CEvent : public AVLNode
    // PURPOSE:      Represents an Event that can be stored in and 
    //               retrieved from the Event queue (CEventQueue) 
    /////////////////////////////////////////////////////////////////////
    class CEvent : private AVL::AVLNode< time_t >, public data_t
    {
        friend class CEventQueue;
        friend class Stack< evnt_t >;
    /////////////////////////////////////////////////////////////////////
    private:
    /////////////////////////////////////////////////////////////////////

        /////////////////////////////////////////////////////////////////
        // Make constructor and destructor private to ensure that 
        // events are created and destroyed only by CEventQueue class 
        /////////////////////////////////////////////////////////////////
        CEvent() : AVL::AVLNode< time_t >(0) 
        {
            Consumer = NULL;
            Producer = NULL;
            Activate();
        }

        CEvent( CEvent& event ) : AVL::AVLNode< time_t >(0) 
        {
            *this = event;
            Activate();
        }

        ~CEvent()    {}


        // the following methods are used to make class CEvent stackable
        inline void    SetNext( evnt_t* p )   { AVL::AVLNode<time_t>::RChild = p; }
        inline void    SetPrev( evnt_t* p )   { AVL::AVLNode<time_t>::LChild = p; }

        inline evnt_t* GetNext( void ) const  { return (evnt_t*) AVL::AVLNode<time_t>::RChild; }
        inline evnt_t* GetPrev( void ) const  { return (evnt_t*) AVL::AVLNode<time_t>::LChild; }

        /////////////////////////////////////////////////////////////////
        // When the event is stored in an event queue, it's right child 
        // and left child pointers can never point to itself.
        // If the pointers point to itself, that means the even is active 
        /////////////////////////////////////////////////////////////////
        inline void    Activate( void )       { AVL::AVLNode<time_t>::RChild = AVL::AVLNode<time_t>::LChild = this; }
        inline BOOL    IsActive( void ) const { return GetNext() == this  
                                                    && GetPrev() == this; }

        //inline time_t GetTime( void ) const { return ACTIVATION_TIME; }
    /////////////////////////////////////////////////////////////////////
    public:
    /////////////////////////////////////////////////////////////////////
        base_t*        Producer;   // pointer to producer of the event
        base_t*        Consumer;   // pointer to consumer of the event
    };




    /////////////////////////////////////////////////////////////////////
    // CLASS:        class CEventQueue : private AVLTree
    // PURPOSE:      
    /////////////////////////////////////////////////////////////////////
    class CEventQueue : private AVL::AVLTree< time_t >
    {
    /////////////////////////////////////////////////////////////////////
    private:
    /////////////////////////////////////////////////////////////////////
        time_t           eqCurrentTime;    // system time 
        Stack< evnt_t >  eqEventPool;      // pool of free events 
        Stack< evnt_t >  eqTopEvents;      // immediate events (all having
                                           // the timestamp same as 
                                           // current system time)

        /////////////////////////////////////////////////////////////////
        // METHOD:       void ClearEventChain( evnt_t* pEvent )
        // PURPOSE:      Recursively traverses (DFS) the AVL tree 
        //               and pushes the events onto the stack of 
        //               free events (EventPool)
        // ARGUMENTS:
        // RETURN VALUE:
        /////////////////////////////////////////////////////////////////
        void ClearEventChain( evnt_t* pEvent )
        {
            if( pEvent->LChild ) ClearEventChain( (evnt_t*)pEvent->LChild );
            if( pEvent->RChild ) ClearEventChain( (evnt_t*)pEvent->RChild );
            eqEventPool.Push( pEvent );
        }

    /////////////////////////////////////////////////////////////////////
    public:
    /////////////////////////////////////////////////////////////////////
        CEventQueue()                 { eqCurrentTime = 0; }
        /*virtual*/ ~CEventQueue()    { DeleteEvents();    }

        /////////////////////////////////////////////////////////////////
        // METHOD:       int32u GetCount( void )
        // PURPOSE:      
        // ARGUMENTS:
        // RETURN VALUE: Number of all events waiting in the AVL tree and 
        //               in eqTopEvents stack.
        /////////////////////////////////////////////////////////////////
        inline int32u GetCount( void ) const
        { 
            return AVL::AVLTree<time_t>::Count + eqTopEvents.GetCount(); 
        }

        /////////////////////////////////////////////////////////////////
        // METHOD:       void Reset( void )
        // PURPOSE:      Moves all events from AVL tree and from the stack 
        //               of immediate events (eqTopEvents) to the pool of 
        //               free events (eqEventPool).
        //               Also reset system time.
        // ARGUMENTS:
        // RETURN VALUE:
        /////////////////////////////////////////////////////////////////
        void Reset( void )
        {
            eqCurrentTime = 0;
            if( AVL::AVLTree<time_t>::pRoot )
            {
                // move Events from AVL tree to free Event pool 
                ClearEventChain( (evnt_t*)AVL::AVLTree<time_t>::pRoot );
                AVL::AVLTree<time_t>::pRoot = NULL;
            }

            // push TopEvents on top of EventPool */
            eqEventPool.Combine( &eqTopEvents );       
        }

        /////////////////////////////////////////////////////////////////
        // METHOD:       void DeleteEvents( void )
        // PURPOSE:      After moving all Events to the free pool, delete 
        //               them.  This is the only place where Events may 
        //               be deleted (memory deallocated)
        // ARGUMENTS:
        // RETURN VALUE:
        /////////////////////////////////////////////////////////////////
        void DeleteEvents( void )
        {
            Reset();
            while( eqEventPool.GetCount() > 0 )  delete eqEventPool.Pop();
        }
    
        /////////////////////////////////////////////////////////////////
        // METHOD:       evnt_t* AllocateEvent(void)
        // PURPOSE:      Get a pointer to a free Event.  If free Event 
        //               pool is not empty, get Event from there, 
        //               otherwise allocate new Event from the heap. 
        //               This is the only place where Events may be
        //               created (memory allocated)
        // ARGUMENTS:
        // RETURN VALUE: pointer to a free Event (CEvent*)
        /////////////////////////////////////////////////////////////////
        inline evnt_t* AllocateEvent(void)
        {  
            evnt_t* pEvent = eqEventPool.GetCount()? eqEventPool.Pop(): new evnt_t; 

            _ASSERT( pEvent != NULL ); 
            pEvent->Activate();
            return pEvent;
        }
    
        /////////////////////////////////////////////////////////////////
        // METHOD:       void RegisterEvent( evnt_t* pEvent, 
        //                                   time_t interval )
        // PURPOSE:      Assign timestamp to Event and register the Event 
        //               in the Event queue. If Event's timestamp is the 
        //               same as current system time, push it onto 
        //               eqTopEvents stack, otherwise insert it into AVL 
        //               tree.
        // ARGUMENTS:    pEvent     - pointer to the Event
        //               interval   - interval from the current time 
        //                            till Event's occurence
        // RETURN VALUE: 
        /////////////////////////////////////////////////////////////////
        inline void RegisterEvent( evnt_t* pEvent, time_t interval )  
        { 
            if( pEvent && pEvent->IsActive())
            {
                if( interval < 0 )
                    interval = 0;     /* no going back in time... */

                pEvent->ACTIVATION_TIME = eqCurrentTime + interval;

                if( interval == 0 ) eqTopEvents.Push( pEvent );
                else                AVL::AVLTree<time_t>::AddNode( pEvent );
            }
        }

        /////////////////////////////////////////////////////////////////
        // METHOD:       evnt_t* GetNextEvent( void )
        // PURPOSE:      Gets next Event from the eqTopEvents if it is 
        //               not empty, or from AVL tree otherwise.  Then  
        //               updates system time.
        // ARGUMENTS:    
        // RETURN VALUE: pointer to the next Event (CEvent*)
        /////////////////////////////////////////////////////////////////
        inline evnt_t* GetNextEvent( void )  
        { 
            evnt_t* pEvent = eqTopEvents.GetCount() ? eqTopEvents.Pop() : (evnt_t*) AVL::AVLTree<time_t>::RemoveHead();//RemoveHead();
        
            if( pEvent ) 
            {
                eqCurrentTime = pEvent->ACTIVATION_TIME;  // update global time
                pEvent->Activate();
            }
            return pEvent;
        }

        /////////////////////////////////////////////////////////////////
        // METHOD:       time_t GetCurrentTime( void )
        // PURPOSE:      
        // ARGUMENTS:    
        // RETURN VALUE: 
        /////////////////////////////////////////////////////////////////
        inline time_t GetCurrentTime( void ) const { return eqCurrentTime; }

        /////////////////////////////////////////////////////////////////
        // METHOD:       void CancelEvent( evnt_t* pEvent )
        // PURPOSE:      Will invalidate the Event by setting its 
        //               consumer to NULL.  This Event will not be 
        //               removed from the AVL tree. 
        // ARGUMENTS:    pEvent - Event to be canceled
        // RETURN VALUE: 
        /////////////////////////////////////////////////////////////////
        inline void CancelEvent( evnt_t* pEvent )   
        { 
            if( pEvent ) pEvent->Consumer = NULL; 
        }

        /////////////////////////////////////////////////////////////////
        // METHOD:       void DestroyEvent( evnt_t* pEvent )
        // PURPOSE:      Returns Event to the pool of free Events. It is 
        //               assumed that the Event was previously removed 
        //               from the Event queue, and from the eqEventPool 
        //               and eqTopEvents stacks
        // ARGUMENTS:    
        // RETURN VALUE: pointer to the next Event (CEvent*)
        /////////////////////////////////////////////////////////////////
        inline void DestroyEvent( evnt_t* pEvent )  
        { 
            if( pEvent && pEvent->IsActive())
                eqEventPool.Push( pEvent ); 
        }
    };




    ///////////////////////////////////////////////////////////////////*
    // CLASS:        class CBase 
    // PURPOSE:      Represents a base class from which all network 
    //               (system) elements must be derived
    /////////////////////////////////////////////////////////////////////
    class CBase
    {
        friend class PDList< CBase >;
        friend class DESL_QUALIFIER;
    /////////////////////////////////////////////////////////////////////
    private:
    /////////////////////////////////////////////////////////////////////
        base_t* pPrev;   // pointer to a previous base object
        base_t* pNext;   // pointer to a next base object

    /////////////////////////////////////////////////////////////////////
    protected:
    /////////////////////////////////////////////////////////////////////
        inline base_t* GetPrev( void ) const    { return pPrev; }
        inline base_t* GetNext( void ) const    { return pNext; }
        inline void    SetPrev( base_t* prv )   { pPrev = prv;  }
        inline void    SetNext( base_t* nxt )   { pNext = nxt;  }
       
    /////////////////////////////////////////////////////////////////////
    public:
    /////////////////////////////////////////////////////////////////////
        obid_t ID;   // Object id

        /////////////////////////////////////////////////////////////////
        CBase( obid_t id = 0 )   
        { 
            ID = id;
            pPrev = pNext = NULL; 
            DESL_OBJ.Append( this );  // regiser new object
        }
        /////////////////////////////////////////////////////////////////
        virtual~ CBase()         
        { 
            DESL_OBJ.Remove( this );  // unregiser object
        }
        /////////////////////////////////////////////////////////////////

        // The CBase is an abstract class.  The following are pure virtual 
        // functions that must be implemented in a derived class 
        virtual void  ProcessEvent( evnt_t* )  = 0; // { return FALSE;    }
        virtual void  Free( void )             = 0; // { /* do nothing */ }
        virtual void  Reset( void )            = 0; // { /* do nothing */ }
    };


/////////////////////////////////////////////////////////////////////////
private:
/////////////////////////////////////////////////////////////////////////
    
    /////////////////////////////////////////////////////////////////////
    // Declare private static members
    /////////////////////////////////////////////////////////////////////
    static CEventQueue      DESL_EQ;   // Event queue   
    static PDList< CBase >  DESL_OBJ;  // Doubly-linked list of all objects 
                                       // derived from CBase  

    /////////////////////////////////////////////////////////////////////
    // METHOD:       void ExecuteAllObjects( void (base_t::*pfun)(void) )
    // PURPOSE:      An iterator which will invoke pfun() method 
    //               for each object registered in SL_OBJ (that 
    //               includes every object derived from CBase)
    // ARGUMENTS:    pfun  - pointer to a method of class CBase
    // RETURN VALUE: 
    /////////////////////////////////////////////////////////////////////
    static void ExecuteAllObjects( void (base_t::*pfun)(void) )
    {
        for( base_t* ptr = DESL_OBJ.GetHead(); ptr; ptr = ptr->GetNext() )
            ( ptr->*pfun )();
    }

/////////////////////////////////////////////////////////////////////////
public:
/////////////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    //  
    //   public Static functions
    //
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////////
    // METHOD:       void GetObjCount( void )
    // PURPOSE:      
    // ARGUMENTS:    
    // RETURN VALUE: 
    /////////////////////////////////////////////////////////////////////
    static inline int32s GetObjCount( void ) { return DESL_OBJ.GetCount(); }

    /////////////////////////////////////////////////////////////////////
    // METHOD:       void GlobalReset( void )
    // PURPOSE:      Resets the Event queue and all registered objects
    // ARGUMENTS:    
    // RETURN VALUE: 
    /////////////////////////////////////////////////////////////////////
    static inline void GlobalReset( void )
    { 
        DESL_EQ.Reset(); 
        ExecuteAllObjects(&DESL_QUALIFIER::CBase::Reset);  // reset all objects 
    }

    /////////////////////////////////////////////////////////////////////
    // METHOD:       void GlobalFree( void )
    // PURPOSE:      Frees (deallocates) all Events in the Event queue 
    //               and all registered objects
    // ARGUMENTS:    
    // RETURN VALUE: 
    // NOTE:         Events can be deleted only by class CEventQueue
    /////////////////////////////////////////////////////////////////////
    static inline void GlobalFree( void )
    { 
        ExecuteAllObjects(&DESL_QUALIFIER::CBase::Free);    // free all objects 
        DESL_EQ.DeleteEvents();								// delete all Events
    }

    /////////////////////////////////////////////////////////////////////
    // PURPOSE:      Access methods for the Event queue   
    /////////////////////////////////////////////////////////////////////
    static inline time_t  GlobalTime( void )        { return DESL_EQ.GetCurrentTime(); }
    static inline evnt_t* AllocateEvent( void )     { return DESL_EQ.AllocateEvent();  }
    static inline void    DestroyEvent( evnt_t* p ) { DESL_EQ.DestroyEvent( p );       }
    static inline void    CancelEvent( evnt_t* p )  { DESL_EQ.CancelEvent( p );        }
    static inline evnt_t* GetNextEvent( void )      { return DESL_EQ.GetNextEvent();   }

    /////////////////////////////////////////////////////////////////////
    // METHOD:       void DispatchEvent( evnt_t* pEvent )
    // PURPOSE:      Dispatches the event to event's consumer 
    //               An event is considered consumed by the consumer
    //               if the consumer either Registers the event again, 
    //               or destroys it.
    //               if Event was not consumed by the consumer, 
    //               in will be destroyed by the DispatchEvent function
    /////////////////////////////////////////////////////////////////////
    static inline void DispatchEvent( evnt_t* p ) 
    { 
        if( p && p->Consumer )  
            p->Consumer->ProcessEvent( p );

        DestroyEvent( p );    
    }

    /////////////////////////////////////////////////////////////////////
    // METHOD:       RegisterEvent( evnt_t* ptr, 
    //                              time_t  interval = 0, 
    //                              base_t* producer = NULL )
    // PURPOSE:      Sets Event producer pointer 
    //               Registers Event in the Event queue
    /////////////////////////////////////////////////////////////////////
    static inline void RegisterEvent( evnt_t* ptr, 
                                      time_t  interval, 
                                      base_t* producer )
    {
        ptr->Producer = producer;           
        DESL_EQ.RegisterEvent( ptr, interval );
    }
   
};  // template < class TIME_T, class DATA_T > class DESL_environment 




/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
///  
///   Initialize static members
///
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

template<class TIME_T,class DATA_T> PDList<typename DESL_QUALIFIER::CBase> DESL_QUALIFIER::DESL_OBJ;
template<class TIME_T,class DATA_T> typename DESL_QUALIFIER::CEventQueue   DESL_QUALIFIER::DESL_EQ;
/////////////////////////////////////////////////////////////////////////

#endif   // _DESL_H_V003_INCLUDED_ 