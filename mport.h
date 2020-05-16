/**********************************************************
 * Filename:    mport.h
 *
 * Description: This file contains declarations for
 *              class MultiPort
 *
 * Author: Glen Kramer (kramer@cs.ucdavis.edu)
 *         University of California @ Davis
 *********************************************************/

#ifndef _MPORT_H_INCLUDED_
#define _MPORT_H_INCLUDED_

#include <string.h>     /* needed for memset(...) in class MultiPort */

/*******************************************************************
** CLASS:        class MultiPort 
** PURPOSE:      Class of all network (system) elements 
**               with one or more output port/interface, i.e.,  
**               switches, splitters, etc. 
**
** PARAMETERS:   PORTS - number of output ports/interfaces
**
** NOTES:      
********************************************************************/

template < int16u PORTS = 1 > class MultiPort 
{

/*******************************************************************/
protected:
/*******************************************************************/
    DESL::base_t* OutPort[ PORTS ];

/*******************************************************************/
public:
/*******************************************************************/
    MultiPort()
    {
        memset( OutPort, 0, PORTS * sizeof( DESL::base_t* ));
    }

    /****************************************************************/
    inline int16u GetPortCount( void ) const       { return PORTS; }
    /****************************************************************/
    inline void SetPort( DESL::base_t* dst_node, int16u src_port = 0 ) 
    { 
        _ASSERT( src_port < PORTS );
        OutPort[ src_port ] = dst_node;
    }
    /****************************************************************/
    inline DESL::base_t* GetPort( int16u src_port = 0 ) const 
    { 
        _ASSERT( src_port < PORTS );
        return OutPort[ src_port ];
    }
};



#endif /* _MPORT_H_INCLUDED_ */




