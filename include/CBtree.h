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
#include <iostream>
#include "CFile.h"
//#define SHOWNODE

//btree in disk
//begin from 2015/3/22
namespace wellDB
{

const off_t nDEFAULT_POS = -1;
const int nMAGIC_NUM = 19900110;
const size_t nORDER_NUM = 256;
const size_t nNODE_BUFFER = 3;
const size_t nTestNum   = 10000000;

typedef
enum enum_BTREE_NODE_TYPE
{
      NO_LEAF,
      LEAF,
      FREE_BLOCK //when node delete , market it as free block the nextPos turn to header 's nFirstFreeBlockPos
}BTREE_NODE_TYPE;

typedef
enum enum_HEADER_FACTOR
{
      KEY,
      NODE,
      HEIGHT,
      //ROOT_POS,
      //START_LEAF_POS,
      //FREE_SAPCE_POS
}HEADER_FACTOR;

typedef
struct tag_BTREE_HEADER
{
      tag_BTREE_HEADER();
      tag_BTREE_HEADER(size_t nOrder);
      void IncreaseNumOf( HEADER_FACTOR eType )
      {
            switch(eType)
            {
                  case  KEY:  ++nKeyNum;    break;
                  case  NODE:    ++nNodeNum;   break;
                  case HEIGHT:    ++nHeight;  break;
            }
      }
      int nMagicNum;
      size_t nOrderNum;
      size_t nKeyNum;
      size_t nNodeNum;
      size_t nHeight;
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
      tag_POS_AND_KEY( off_t  nNewPos ,KEY_TYPE kNewKey ) :nPos(nNewPos) ,kKey(kNewKey)       {     }
      off_t nPos;
      KEY_TYPE kKey;
}POS_AND_KEY;

const size_t nSIZEOF_POS_AND_KEY = sizeof(struct tag_POS_AND_KEY);

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

typedef
struct tag_NODE_BUFFER
{
      tag_NODE_BUFFER (BTREE_NODE* pNew): isDirty(false) , pNode(pNew)
      {     }
      void SetDirty()
      {   isDirty = true;  }
      void Clear()
      {   isDirty = false;  }
      bool IsDirty()
      {     return isDirty ; }
      bool isDirty;
      BTREE_NODE *pNode;
}NODE_BUFFER;

class CBtree
{
      public:
            CBtree( );
            ~CBtree();
            bool Init(const char* cPath , CFileBase* pFileOp = new CStdFile() , size_t nOrderNum = nORDER_NUM);
            bool Insert( const POS_AND_KEY &pPosAndKeyToInsert  );
            size_t SizeofBTreeNode() const
            {     return nSIZEOF_BTREE_NODE + m_bhHeader.nOrderNum * nSIZEOF_POS_AND_KEY;  }
            size_t GetOrderNum() const
            {     return m_bhHeader.nOrderNum;    }
            void Show();
            void Traversal();
      private:
            BTREE_HEADER m_bhHeader;
            std::vector< NODE_BUFFER >  m_vcNodeBuffer;
            CFileBase* m_pFileOp;

      private:
            void __InitNodeBuffer();
            void __InitNewHeader(size_t nOrderNum);
            void __ShowNode(BTREE_NODE * pBtreeNode) const;
            void __ShowHeader();
            bool __WriteNode( BTREE_NODE* pNodeToWrite );
            void __WriteHeader() const;// if header change;
            bool __ReadNode( BTREE_NODE* pNodeToRead , off_t nPos );
            bool __NodeIsLeaf(  const NODE_BUFFER &nbNode )const
            {     return nbNode.pNode->eNodeType == LEAF; }
            bool __InsertKeyIntoNode( NODE_BUFFER &nbBuffer, const POS_AND_KEY &pPosAndKey );
            off_t __SearchPosByKey (const NODE_BUFFER &nbBuffer  ,KEY_TYPE kKey )const;
            void __LeftRotate( NODE_BUFFER &nbNode );
            bool __SplitNode( NODE_BUFFER &nbParentNode, NODE_BUFFER &nbChildNode , KEY_TYPE kKey);
            bool __InsertNodeNonFull( std::vector<NODE_BUFFER> &vcNodeBuffer , size_t nLevel , POS_AND_KEY pPosAndKeyToInsert );
            void __RegularWrite();
            void __WriteDirtyNode(NODE_BUFFER &nbBuffer);
            off_t __AllocNewNodePos();

            CBtree(const CBtree& other);
            CBtree& operator=(const CBtree& other);
};

}
#endif // CBTREE_H
