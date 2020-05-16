/**********************************************************
 * Filename:    list.h
 *
 * Description: This file contains declaration for class ...
 * 
 * Author: Glen Kramer (kramer@cs.ucdavis.edu)
 *         University of California @ Davis
 *********************************************************/

#ifndef _LIST_H_V001_INCLUDED_
#define _LIST_H_V001_INCLUDED_

#include "_types.h"

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
class DLinkable 
{
  private:
    DLinkable* pPrev;
    DLinkable* pNext;

  public:
    //T Item;     // item being linked      

    DLinkable( DLinkable* prev = NULL, DLinkable* next = NULL )   { pPrev = prev; pNext = next; }
    virtual ~DLinkable()                 {}

    inline DLinkable* GetNext(void)                    { return pNext; }
    inline DLinkable* GetPrev(void)                    { return pPrev; }
    inline void       InsertAfter( DLinkable* ptr )    { Insert( ptr, ptr? ptr->pNext : NULL ); }
    inline void       InsertBefore( DLinkable* ptr )   { Insert( ptr? ptr->pNext : NULL, ptr ); }

    inline void       Insert(DLinkable* prv, DLinkable* nxt) 
    { 
        if(( pPrev = prv )) pPrev->pNext = this; 
        if(( pNext = nxt )) pNext->pPrev = this; 
    }

    inline void       Remove(void)                     
    { 
        if( pPrev ) pPrev->pNext = pNext; 
        if( pNext ) pNext->pPrev = pPrev; 
    }
};

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

template < class T > class PDList
/* class T must have the following public methods:
	T*	  GetNext( void );
    T*	  GetPrev( void );
	void  SetPrev( T*   );
	void  SetNext( T*   );
*/
{
private:
    T*		pHead;
    T*		pTail;
    int32s  Count;

public:
    PDList()                                 { Clear(); }
    virtual ~PDList()                        {}

 	///////////////////////////////////////////////////////////////////////////   
    inline void Insert( T* prv, T* ptr, T* nxt )    
    { 
        if( ptr ) 
        { 
			ptr->SetPrev( prv );
			ptr->SetNext( nxt );

            if( prv == NULL ) pHead = ptr; else prv->SetNext( ptr );
            if( nxt == NULL ) pTail = ptr; else nxt->SetPrev( ptr );

            Count++; 
        }
    }

	///////////////////////////////////////////////////////////////////////////
	inline T* Remove( T* ptr )  
    { 
		T *prv, *nxt;

        if( ptr ) 
        { 
			prv = ptr->GetPrev();
			nxt = ptr->GetNext();

            if( ptr == pTail ) pTail = prv;
			if( ptr == pHead ) pHead = nxt;

			if( prv ) prv->SetNext( nxt );
			if( nxt ) nxt->SetPrev( prv );

            Count--; 
        } 
        return ptr;
    }
    ///////////////////////////////////////////////////////////////////////////
	inline void     Clear(void)             { pHead = pTail = NULL;  Count = 0; }
    inline int32s   GetCount(void) const    { return Count; }
    inline T*		GetHead(void)  const    { return pHead; }
    inline T*		GetTail(void)  const    { return pTail; }

    inline void     Append( T* ptr )        { Insert( pTail, ptr, NULL ); }
    inline void     InsertHead( T* ptr )    { Insert( NULL, ptr, pHead ); }
    inline T*       RemoveHead(void)        { return Remove( pHead );     }
    inline T*       RemoveTail(void)        { return Remove( pTail );     }
    ///////////////////////////////////////////////////////////////////////////
    inline void Combine( PDList<T>* ptr ) 
    {
        if( ptr && ptr->GetCount() ) 
        { 
            if( pTail == NULL ) 
            {
                pHead = ptr->pHead;
                pTail = ptr->pTail;
            }
            else
            {
				pTail->SetNext( ptr->pHead );
				ptr->pHead->SetPrev( pTail );
                pTail = ptr->pTail;
            }

            Count += ptr->Count;
            ptr->Clear(); 
        }
    }
    
};
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////


#endif /* _LIST_H_V001_INCLUDED_ */
