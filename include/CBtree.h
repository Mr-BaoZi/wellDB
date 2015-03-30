#ifndef CBTREE_H
#define CBTREE_H

#include <sys/types.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include <algorithm>
#include <time.h>
#include "CFile.h"

//btree in disk
//begin from 2015/3/22
namespace wellDB
{

const off_t nDEFAULT_POS = -1;
const int nMAGIC_NUM = 19900110;
const size_t nORDER_NUM = 256;
const size_t nNODE_BUFFER = 2;
const size_t nTestNum   = 1000000;

typedef
enum enum_BTREE_NODE_TYPE
{
      NO_LEAF,
      LEAF,
      FREE_BLOCK //when node delete , market it as free block the nextPos turn to header 's nFirstFreeBlockPos
}BTREE_NODE_TYPE;

typedef
struct tag_BTREE_HEADER
{
      int nMagicNum;//in case the fake;
      size_t nOrderNum;
      size_t nKeyNum;//the num of key
      size_t nNodeNum;
      size_t nHeight; // the height of BTree
      // read pos
      off_t nRootPos;
      off_t nStartLeafPos;
      //freespace
      size_t nFreeBlockNum;
      off_t nFirstFreeBlockPos;
}BTREE_HEADER;

typedef int KEY_TYPE;

typedef
struct tag_POS_AND_KEY
{
      tag_POS_AND_KEY() : nPos(nDEFAULT_POS), nKey(-1){   }
      tag_POS_AND_KEY(KEY_TYPE key) : nPos(nDEFAULT_POS), nKey(key){    }
      off_t nPos;
      KEY_TYPE nKey;
}POS_AND_KEY;

inline bool operator == (const POS_AND_KEY &lhs , const POS_AND_KEY &rhs )
{     return lhs.nKey == rhs.nKey ; }
inline bool operator != (const POS_AND_KEY &lhs , const POS_AND_KEY &rhs)
{     return ! (lhs==rhs); }
inline bool operator > (const POS_AND_KEY &lhs , const POS_AND_KEY &rhs)
{     return lhs.nKey > rhs.nKey; }
inline bool operator < (const POS_AND_KEY &lhs , const POS_AND_KEY &rhs )
{     return  ! (lhs>rhs); }

typedef
struct tag_BTREE_NODE
{
      BTREE_NODE_TYPE eType;
      size_t nBusyKey;//number of key used
      size_t nIdleKey;//numer of key not used
      off_t nSelfPos;
      off_t nNextPos;//if leaf, point to next  leaf
      POS_AND_KEY gPosAndKey[0];
}BTREE_NODE, *PBTREE_NODE;


const size_t nSIZEOF_POS_AND_KEY = sizeof(POS_AND_KEY);
const size_t nSIZEOF_BTREE_HEADER = sizeof(BTREE_HEADER);
const size_t nSIZEOF_BTREE_NODE = sizeof(BTREE_NODE);

const BTREE_HEADER DEFAULT_BTREE_HEADER = {nMAGIC_NUM, /*num*/nORDER_NUM , 0 , 1 ,1 , /*pos*/nSIZEOF_BTREE_HEADER , nSIZEOF_BTREE_HEADER, /*free space*/0 ,nDEFAULT_POS};
const BTREE_NODE DEFAULT_ROOT_NODE ={ LEAF , 0 , nORDER_NUM ,  nSIZEOF_BTREE_HEADER  , nDEFAULT_POS  };
const POS_AND_KEY DEFAULT_POS_AND_KEY = POS_AND_KEY();

class CBtree
{
      public:
            CBtree( const char* cPath );
            ~CBtree();
            bool Insert( const POS_AND_KEY &pPosAndKeyToInsert  );
            size_t SizeofBTreeNode( size_t nOrderNum  ) const
            {     return nSIZEOF_BTREE_NODE + nOrderNum * nSIZEOF_POS_AND_KEY;  }
            size_t SizeofBTreeNode() const
            {     return nSIZEOF_BTREE_NODE + m_pHeader->nOrderNum * nSIZEOF_POS_AND_KEY;  }
            size_t GetOrderNum()const
            {     return m_pHeader->nOrderNum;    }
            void Show();

      //protected:
      private:
            BTREE_HEADER* m_pHeader;
            BTREE_NODE* m_pRootNode;
            BTREE_NODE* m_pNodeBuffer[nNODE_BUFFER];
            int m_fdBTreeFile;


            bool __WriteHeader();// if header change;
            bool __WriteNode( BTREE_NODE* pNodeToWrite ) const ;
            bool __ReadNode( BTREE_NODE* pNodeToWrite , off_t nPos );
            bool __SplitNode(BTREE_NODE* pParentNode, BTREE_NODE* pChildNode , KEY_TYPE kKey);//normal node spilte
            bool __NodeIsLeaf(  BTREE_NODE *pBtreeNode)const
            {     return pBtreeNode->eType == LEAF; }
            bool __NodeIsRoot(BTREE_NODE *pBtreeNode)const
            {     return pBtreeNode->nSelfPos == m_pHeader->nRootPos; }
            bool __LeftRotate( POS_AND_KEY* pArray, size_t nArrayLen,  size_t nRotateNum );
            bool __SearchPosByKey (BTREE_NODE* pNodeToSearch ,KEY_TYPE kKey ,size_t &nIndex , off_t &nPos ) const;
            bool __InsertKeyIntoNode( BTREE_NODE* pNodeToInsert, const POS_AND_KEY &pPosAndKey );
            bool __InsertNodeNonFull(BTREE_NODE* gpBtreeNode[] , size_t nArray,  POS_AND_KEY pPosAndKeyToInsert );
            void __ShowNode(BTREE_NODE * pBtreeNode);
            void __ShowHeader();

            CBtree(const CBtree& other);
            CBtree& operator=(const CBtree& other);
};

}
#endif // CBTREE_H
