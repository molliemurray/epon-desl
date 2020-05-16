/**********************************************************
 * Filename:    _stack.h
 *
 * Description: This file contains declaration for class Stack
 * 
 * Author: Glen Kramer (kramer@cs.ucdavis.edu)
 *         University of California @ Davis
 *********************************************************/

#ifndef _STACK_H_V001_INCLUDED_
#define _STACK_H_V001_INCLUDED_

#include "_types.h"

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

template < class T > class Stack
/* class T must have the following public methods:
	T*	  GetNext( void );
    void  SetNext( T*   );
*/
{
private:
    T*		pTop;
    T*		pBtm;
    int32s  Count;

public:
    Stack()                                 { Clear(); }
    virtual ~Stack()                        {}

    ///////////////////////////////////////////////////////////////////////////
	inline void     Clear(void)             { pTop = pBtm = NULL;  Count = 0; }
    inline int32s   GetCount(void)  const   { return Count; }
    inline T*		GetTop(void)    const   { return pTop;  }
    inline T*		GetBottom(void) const   { return pBtm;  }

    ///////////////////////////////////////////////////////////////////////////
    inline void Push( T* ptr )
    {
        if( ptr )   
        { 
            if( !pBtm ) pBtm = ptr;
            ptr->SetNext( pTop ); 
            pTop = ptr; 
            Count ++;
        }
    }
    ///////////////////////////////////////////////////////////////////////////
    inline T* Pop( void )
    {
        T* ret = pTop;
        if( pTop )  
        {
            pTop = pTop->GetNext();
            Count--;
            if( !pTop )  pBtm = NULL;
        }
        return ret;
    }
    ///////////////////////////////////////////////////////////////////////////
    inline void Combine( Stack<T>* pStack ) /* push 'pStack' on top of 'this' */
    {
        if( pStack && pStack->pBtm )             /* if pStack is not empty ... */   
        {
            pStack->pBtm->SetNext( pTop );
            pTop = pStack->pTop;

            if( !pBtm ) pBtm = pStack->pBtm;    /* if 'this' stack was empty ... */ 
            Count += pStack->Count;

            pStack->Clear();                    /* must clear pStack, otherwise,
                                                   poping an item from pStack will 
                                                   break this Stack */
        }
    }
    
};
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////


#endif /* _STACK_H_V001_INCLUDED_ */
