#ifndef CBTREE_H
#define CBTREE_H

#include <sys/types.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include <algorithm>
#include <vector>
#include <time.h>
#include "CFile.h"

//btree in disk
//begin from 2015/3/22
namespace wellDB
{

const off_t nDEFAULT_POS = -1;
const int nMAGIC_NUM = 19900110;
const size_t nORDER_NUM = 256;
const size_t nNODE_BUFFER = 3;
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
      tag_BTREE_HEADER();
      tag_BTREE_HEADER(size_t nOrder);
      int nMagicNum;//in case the fake;
      size_t nOrderNum;
      size_t nKeyNum;//the num of key
      size_t nNodeNum;
      size_t nHeight; // the height of BTree
      // read pos
      off_t nRootPos;
      off_t nStartLeafPos;
      //freespace admin
      size_t nFreeBlockNum;
      off_t nFirstFreeBlockPos;
}BTREE_HEADER;

const size_t nSIZEOF_BTREE_HEADER = sizeof(struct tag_BTREE_HEADER);

typedef int KEY_TYPE;

typedef
struct tag_POS_AND_KEY
{
      tag_POS_AND_KEY() : nPos(nDEFAULT_POS), kKey(-1)      {   }
      tag_POS_AND_KEY(KEY_TYPE key) : nPos(nDEFAULT_POS), kKey(key)      {    }
      off_t nPos;
      KEY_TYPE kKey;
}POS_AND_KEY;

const size_t nSIZEOF_POS_AND_KEY = sizeof(struct tag_POS_AND_KEY);

inline bool operator == (const POS_AND_KEY &lhs , const POS_AND_KEY &rhs )
{     return lhs.kKey == rhs.kKey ; }
inline bool operator != (const POS_AND_KEY &lhs , const POS_AND_KEY &rhs)
{     return ! (lhs==rhs); }
inline bool operator > (const POS_AND_KEY &lhs , const POS_AND_KEY &rhs)
{     return lhs.kKey > rhs.kKey; }
inline bool operator < (const POS_AND_KEY &lhs , const POS_AND_KEY &rhs )
{     return  lhs.kKey < rhs.kKey; }
inline bool operator <=  (const POS_AND_KEY &lhs , const POS_AND_KEY &rhs )
{    return  lhs.kKey <= rhs.kKey; }
inline bool operator >=  (const POS_AND_KEY &lhs , const POS_AND_KEY &rhs )
{    return  lhs.kKey >= rhs.kKey; }

typedef
struct tag_BTREE_NODE
{
      tag_BTREE_NODE(BTREE_NODE_TYPE ,  size_t nBusy , size_t nIdle , off_t nSelf , off_t nNext  );
      static struct tag_BTREE_NODE* CreateBtreeNode(size_t nSize)
      {     return ( struct tag_BTREE_NODE*)malloc(nSize) ;}
      BTREE_NODE_TYPE eNodeType;
      size_t nBusyKey;//number of key used
      size_t nIdleKey;//numer of key not used
      off_t nSelfPos;
      off_t nNextPos;//if leaf, point to next  leaf
      POS_AND_KEY gPosAndKey[0];
}BTREE_NODE, *PBTREE_NODE;

const size_t nSIZEOF_BTREE_NODE = sizeof(struct tag_BTREE_NODE);

class CBtree
{
      public:
            CBtree( );
            ~CBtree();
            bool Init(const char* cPath , CFileBase* pFileOp = new CStdFile() , size_t nOrderNum = nORDER_NUM);
            //bool Insert( const POS_AND_KEY &pPosAndKeyToInsert  )
            size_t SizeofBTreeNode() const
            {     return nSIZEOF_BTREE_NODE + m_bhHeader.nOrderNum * nSIZEOF_POS_AND_KEY;  }
            size_t GetOrderNum() const
            {     return m_bhHeader.nOrderNum;    }
            //void Show();
      private:
            BTREE_HEADER m_bhHeader;
            std::vector<BTREE_NODE* >  m_vcNodeBuffer;
            CFileBase* m_pFileOp;

      private:
            void __InitNodeBuffer();
            void __InitHeader(size_t nOrderNum);
            void __ShowNode(BTREE_NODE * pBtreeNode) const;
            void __ShowHeader();
            bool __WriteNode( BTREE_NODE* pNodeToWrite );
            bool __WriteHeader() const;// if header change;
            bool __ReadNode( BTREE_NODE* pNodeToRead , off_t nPos );
            bool __NodeIsLeaf(  BTREE_NODE *pBtreeNode)const
            {     return pBtreeNode->eNodeType == LEAF; }
            bool __NodeIsRoot(BTREE_NODE *pBtreeNode)const
            {     return pBtreeNode->nSelfPos == m_bhHeader.nRootPos; }
            bool __InsertKeyIntoNode( BTREE_NODE* pNodeToInsert, const POS_AND_KEY &pPosAndKey );
            off_t __SearchPosByKey (BTREE_NODE* pNodeToSearch ,KEY_TYPE kKey )const;
            bool __LeftRotate( BTREE_NODE* pNodeToLeftRotate );
            bool __SplitNode(BTREE_NODE* pParentNode, BTREE_NODE* pChildNode , KEY_TYPE kKey);
            /*
            bool __InsertNodeNonFull(BTREE_NODE* gpBtreeNode[] , size_t nArray,  POS_AND_KEY pPosAndKeyToInsert );
            */
            CBtree(const CBtree& other);
            CBtree& operator=(const CBtree& other);
};

}
#endif // CBTREE_H
