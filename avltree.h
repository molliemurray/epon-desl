/**********************************************************
 * Filename:    avltree.h
 *
 * Description: This file contains declarations of the
 *              following classes:
 *              class AVLNode
 *              class AVLTree
 * 
 * Author:      Glen Kramer (kramer@cs.ucdavis.edu)
 *              University of California @ Davis
 *
 * AVL tree structure is discovered by Adelson-Velsky and Landis   
 *      G.M. Adelson-Velskii and E.M. Landis. 
 *      An algorithm for the organization of information. 
 *      Soviet Math. Dokl., 3:1259--1262, 1962
 *
 * Date:        June 2001
 *********************************************************/

#ifndef _AVL_AVLTREE_H_V001_INCLUDED_
#define _AVL_AVLTREE_H_V001_INCLUDED_

#include "_types.h"

namespace AVL_tree_namespace {

template < class avlkey_t > class AVLNode;
template < class avlkey_t > class AVLTree;

#pragma inline_depth( 128 )
#pragma inline_recursion( on )



/*******************************************************************
** CLASS:        class AVLNode
** PURPOSE:      Represents a node in an AVL tree
********************************************************************/
template < class avlkey_t > class AVLNode
{
    typedef AVLNode< avlkey_t >*  pnode_t;
    typedef int16s                height_t;

    friend class AVLTree< avlkey_t >;

/*******************************************************************/
private:
/*******************************************************************/

    int8s       Balance;    /* Node balance: height of right subtree 
                               minus height of left subtree */

    height_t    Height;     /* maximum distance to any leave */

    /***************************************************************/ 
    inline height_t GetHeight( void )    { return Height; }
    /***************************************************************/
    inline pnode_t Initialize(void)
    {
        LChild  = NULL;
        RChild  = NULL; 
        Balance = 0;
        Height  = 0;
        return this;
    }

    /***************************************************************
    ** METHOD:       void UpdateHeight( void )
    ** PURPOSE:      Recalcilates height and balance of given node
    ** RETURN VALUE: 
    ****************************************************************/
    inline void UpdateHeight( void )
    {
        height_t LHeight = LChild? LChild->Height : (height_t) -1;
        height_t RHeight = RChild? RChild->Height : (height_t) -1;

        Height  = MAX<height_t>( LHeight, RHeight ) + 1;
        Balance = (int8s)(RHeight - LHeight);
    }

    /***************************************************************
    ** METHOD:       pnode_t PromoteRight( void ) 
    ** PURPOSE:      Makes right child the parent (ratation left)
    ** RETURN VALUE: new root of the subtree
    ****************************************************************/
    inline pnode_t PromoteRight( void ) 
    {
        pnode_t  pNode = RChild;
        RChild = pNode->LChild;
        pNode->LChild  = this;

        this ->UpdateHeight();
        pNode->UpdateHeight();

        return pNode;
    }

    /***************************************************************
    ** METHOD:       pnode_t PromoteLeft( void ) 
    ** PURPOSE:      Makes left child the parent (ratation right)
    ** RETURN VALUE: new root of the subtree
    ****************************************************************/
    inline pnode_t PromoteLeft( void ) 
    {
        pnode_t pNode  = LChild;
        LChild = pNode->RChild;
        pNode->RChild  = this;

        this ->UpdateHeight();
        pNode->UpdateHeight();

        return pNode;
    }

    /***************************************************************
    ** METHOD:       pnode_t RepairBalance( void )
    ** PURPOSE:      Updates node's height and performs balancing, 
    **               if necessary
    ** RETURN VALUE: new root of the subtree
    ****************************************************************/
    inline pnode_t RepairBalance( void )
    {
        UpdateHeight();

        if( Balance < -1 )
        {
            /* if imbalance is due to internal branch, do double promotion */
            if( LChild->Balance > 0 )   LChild = LChild->PromoteRight();
            return PromoteLeft();
        }

        if( Balance > 1 )       
        {
            /* if imbalance is due to internal branch, do double promotion */
            if( RChild->Balance < 0 )   RChild = RChild->PromoteLeft();
            return PromoteRight();
        }

        return this;
    }

    /***************************************************************
    ** METHOD:       pnode_t InsertNode( pnode_t pNode )
    ** PURPOSE:      Inserts node into a subtree rooted at the current node
    ** ARGUMENTS:    pNode  - node to insert
    ** RETURN VALUE: new root of the subtree
    ****************************************************************/
    inline pnode_t InsertNode( pnode_t pNode )
    {
        if( pNode->NodeKey > NodeKey )
            RChild = RChild? RChild->InsertNode( pNode ): pNode->Initialize();

        else //if( pNode->NodeKey <= NodeKey )
            LChild = LChild? LChild->InsertNode( pNode ): pNode->Initialize();

        return RepairBalance();
    }

    /***************************************************************
    ** METHOD:       pnode_t RemoveNode( pnode_t pNode, BOOL* result )
    ** PURPOSE:      Removes node from a subtree rooted at the current node
    **
    ** ARGUMENTS:    pNode  - node to remove (never NULL)
    **               result - placeholder for the boolean indicating 
    **                        whether given node was found in the tree 
    **                        and removed or not
    **
    ** RETURN VALUE: new root of the subtree
    ****************************************************************/
    inline pnode_t RemoveNode( pnode_t pNode, BOOL* result )
    {
        if( pNode == this )
        {
            if( !LChild )   return RChild;
            if( !RChild )   return LChild;

            /* swap pNode with the right end of its left sub-AVLTree */
            pnode_t ptr, left_subroot;

            left_subroot = LChild->RemoveRightEnd( &ptr );
            ptr->LChild  = left_subroot;
            ptr->RChild  = RChild;

            *result = TRUE;
            return ptr->RepairBalance();
        }

        if( pNode->NodeKey > NodeKey && RChild )
        {
            RChild = RChild->RemoveNode( pNode, result );
            return RepairBalance();
        }

        if( pNode->NodeKey <= NodeKey && LChild )
        {
            LChild = LChild->RemoveNode( pNode, result );
            return RepairBalance();
        }
    }

    /***************************************************************
    ** METHOD:       pnode_t RemoveLeftEnd( pnode_t* ppNode )
    ** PURPOSE:      Removes the leftmost (smallest) node from a 
    **               subtree rooted at the current node
    **
    ** ARGUMENTS:    pNode - placeholder for the address of the removed node
    **
    ** RETURN VALUE: new root of the subtree
    ****************************************************************/
    inline pnode_t RemoveLeftEnd( pnode_t* ppNode )
    {
        if( !LChild )  /* end of recursion */
        {
            *ppNode = this;
            return RChild;
        }
        LChild = LChild->RemoveLeftEnd( ppNode );
        return RepairBalance();
    }

    /***************************************************************
    ** METHOD:       pnode_t RemoveRightEnd( pnode_t* ppNode )
    ** PURPOSE:      Removes the rightmost (largest) node from a 
    **               subtree rooted at the current node
    **
    ** ARGUMENTS:    pNode - placeholder for the address of the removed node
    **
    ** RETURN VALUE: new root of the subtree
    ****************************************************************/
    inline pnode_t RemoveRightEnd( pnode_t* ppNode )
    {
        if( !RChild )  /* end of recursion */
        {
            *ppNode = this;
            return LChild;
        }
        RChild = RChild->RemoveRightEnd( ppNode );
        return RepairBalance();
    }


/*******************************************************************/
protected:
/*******************************************************************/

    pnode_t     LChild;     /* Left  child */
    pnode_t     RChild;     /* Right child */
    avlkey_t    NodeKey;    /*             */

/*******************************************************************/
public:
/*******************************************************************/
    AVLNode( avlkey_t key_val )     { Initialize(); NodeKey = key_val; }
    /* virtual */ ~AVLNode()        {}

};    





/*******************************************************************
** CLASS:        class AVLTree
** PURPOSE:      
********************************************************************/
template < class avlkey_t > class AVLTree
{
    typedef AVLNode< avlkey_t >* pnode_t;

/*******************************************************************/
protected:
/*******************************************************************/
    pnode_t pRoot;  /* root node of the tree       */
    int32u  Count;  /* number of nodes in the tree */

/*******************************************************************/
public:
/*******************************************************************/

    AVLTree()   
    { 
        pRoot = NULL; 
        Count = 0;
    }

    virtual ~AVLTree()  {}

    /***************************************************************/
    inline int32u GetCount(void) const   { return Count; }

    /***************************************************************/
    inline void AddNode( pnode_t pNode )
    {
		if( pNode )
		{
			pRoot = ( pRoot )? pRoot->InsertNode( pNode ): pNode->Initialize();
			Count++;
		}
    }
    /***************************************************************/
    inline BOOL RemoveNode( pnode_t pNode )
    {
        BOOL result = FALSE;
        if( pRoot && pNode ) pRoot = pRoot->RemoveNode( pNode, &result );
        if( result ) Count--;
        return result;
    }
    /***************************************************************/
    inline pnode_t RemoveHead( void )
    {
        pnode_t pNode = NULL;
        if( pRoot ) pRoot = pRoot->RemoveLeftEnd( &pNode );
        if( pNode ) Count--;
        return pNode;
    }
    /***************************************************************/
    inline pnode_t RemoveTail( void )
    {
        //NODE_t* pNode = NULL;
        pnode_t pNode = NULL;
        if( pRoot ) pRoot = pRoot->RemoveRightEnd( &pNode );
        if( pNode ) Count--;
        return pNode;
    }
};




}; /* namespace AVL_tree_namespace */

namespace AVL     = AVL_tree_namespace;
namespace AVLTREE = AVL_tree_namespace;

#endif   /* _AVL_AVLTREE_H_V001_INCLUDED_ */