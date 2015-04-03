#include "CBtree.h"

namespace wellDB
{

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

tag_BTREE_HEADER::tag_BTREE_HEADER():
nMagicNum(nMAGIC_NUM) , nOrderNum(nORDER_NUM)   ,nKeyNum (0) , nNodeNum(1)   ,  nHeight(1),
nRootPos (nSIZEOF_BTREE_HEADER),    nStartLeafPos(nSIZEOF_BTREE_HEADER),
nFreeBlockNum(0),  nFirstFreeBlockPos (nDEFAULT_POS)  {     }

tag_BTREE_HEADER::tag_BTREE_HEADER(size_t nOrder):
nMagicNum(nMAGIC_NUM) , nOrderNum(nOrder)   ,nKeyNum (0) , nNodeNum(1)   ,  nHeight(1),
nRootPos (nSIZEOF_BTREE_HEADER),    nStartLeafPos(nSIZEOF_BTREE_HEADER),
nFreeBlockNum(0),  nFirstFreeBlockPos (nDEFAULT_POS)  {     }

tag_BTREE_NODE::tag_BTREE_NODE(BTREE_NODE_TYPE eType ,  size_t nBusy , size_t nIdle , off_t nSelf , off_t nNext  ):
eNodeType(eType) , nBusyKey(nBusy) , nIdleKey(nIdle) , nSelfPos(nSelf) , nNextPos(nNext)  {     }

CBtree::CBtree():m_pFileOp(NULL)
{

}

CBtree::~CBtree()
{
      //dtor
      // code is incomplete , write header must be added
      __RegularWrite();

      if(m_pFileOp)
            delete m_pFileOp;

      for (  size_t i = 0 ; i < m_vcNodeBuffer.size() ; i++)
            free( m_vcNodeBuffer[i].pNode );
}

CBtree::CBtree(const CBtree& other)
{
      //copy ctor
}

bool CBtree::Init( const char* cPath , CFileBase* pFileOp , size_t nOrderNum)
{
      if ( !( m_pFileOp = pFileOp) ) return false;
      if ( !m_pFileOp->Open(cPath) )  return false;

      if( m_pFileOp->Read( &m_bhHeader ,nSIZEOF_BTREE_HEADER) < nSIZEOF_BTREE_HEADER )
      {
            __InitNewHeader(nOrderNum);
      }
      __ShowHeader();

      if( m_bhHeader.nMagicNum != nMAGIC_NUM )  return false;

      __InitNodeBuffer();
      __ShowNode(m_vcNodeBuffer[0].pNode);
      return true;
}

void CBtree::__InitNewHeader(size_t nOrderNum)
{
      //file is too short
      if(nOrderNum < 4 ) nOrderNum = 4;
      BTREE_HEADER tempDefaultHeader(nOrderNum);
      BTREE_NODE tempFirstNode( LEAF , 0 , nOrderNum ,nSIZEOF_BTREE_HEADER , nDEFAULT_POS  ) ;
      POS_AND_KEY tempPAK;

      memcpy( &m_bhHeader , &tempDefaultHeader , nSIZEOF_BTREE_HEADER);

      m_pFileOp ->Seek( 0 ,SEEK_SET);
      m_pFileOp ->Write( &tempDefaultHeader, nSIZEOF_BTREE_HEADER );
      m_pFileOp ->Write( &tempFirstNode, nSIZEOF_BTREE_NODE);
      for( size_t i = 0 ; i < nOrderNum ; i++ )
            m_pFileOp->Write( &tempPAK, nSIZEOF_POS_AND_KEY );
}

void CBtree::__InitNodeBuffer()
{
      size_t nNodeByte = SizeofBTreeNode();
      for( size_t i = 0 ; i < m_bhHeader.nHeight ; i++ )
      {
            NODE_BUFFER temp(BTREE_NODE::CreateBtreeNode(nNodeByte)) ;
            off_t nDataPos = nDEFAULT_POS;
            if( i == 0 )
                  nDataPos = m_bhHeader.nRootPos;
            else
                  nDataPos = m_vcNodeBuffer[i-1].pNode->gPosAndKey[0].nPos;

            m_pFileOp->Seek(nDataPos, SEEK_SET);
            m_pFileOp->Read( temp.pNode, nNodeByte );
            m_vcNodeBuffer.push_back(temp);
      }
}

void CBtree::__RegularWrite()
{
      __WriteHeader();
      __ShowHeader();
      for (size_t i = 0 ; i < m_vcNodeBuffer.size(); i++)
            __WriteDirtyNode( m_vcNodeBuffer[i] );
}

void CBtree::__WriteDirtyNode(NODE_BUFFER &nbBuffer)
{
      if( nbBuffer.IsDirty()  )
      {
            __WriteNode( nbBuffer.pNode );
            nbBuffer.Clear();
      }
}

void CBtree::__ShowNode(BTREE_NODE * pBtreeNode)const
{
#ifdef SHOWNODE
      BTREE_NODE* p = pBtreeNode;
      printf("node:\n");
      printf("busy = %d idle = %d selfPos = %ld nextPos = %ld \n",p->nBusyKey,p->nIdleKey, p->nSelfPos,p->nNextPos);
      for ( size_t i = 0 ; i < p->nBusyKey ; i++ )
      {
            if( pBtreeNode->nSelfPos == m_bhHeader.nRootPos)
            printf("%d:  key = %d  pos = %ld \n" ,  i ,p->gPosAndKey[i].kKey , p->gPosAndKey[i].nPos );
      }

#endif // SHOWNODE
}
void CBtree::__ShowHeader()
{
#ifdef SHOWNODE
      BTREE_HEADER* p = &m_bhHeader;
      printf("header: \n");
      printf("oder = %d key = %d node = %d height = %d rootPos = %ld startLeafPos = %ld\n" , p->nOrderNum , p->nKeyNum ,p->nNodeNum , p->nHeight , p->nRootPos ,p->nStartLeafPos);
#endif // SHOWNODE
}

bool CBtree::__WriteNode( BTREE_NODE* pNodeToWrite )
{
      if( ! pNodeToWrite ) return false;

      off_t nPos = pNodeToWrite->nSelfPos;
      m_pFileOp->Seek(nPos, SEEK_SET);
      m_pFileOp->Write( pNodeToWrite ,SizeofBTreeNode()  );
      return true;
}

bool CBtree::__ReadNode( BTREE_NODE* pNodeToRead , off_t nPos )
{
      if(!pNodeToRead) return false;

      m_pFileOp->Seek (nPos , SEEK_SET);
      m_pFileOp->Read( pNodeToRead ,SizeofBTreeNode() );

      if( pNodeToRead->nSelfPos != nPos) return false;
      return true;
}

bool CBtree::__InsertKeyIntoNode( NODE_BUFFER &nbBuffer, const POS_AND_KEY &pPosAndKey)
{
      BTREE_NODE *pNodeToInsert = nbBuffer.pNode;
      if ( !pNodeToInsert ||pNodeToInsert->nIdleKey == 0 || pNodeToInsert->nBusyKey == GetOrderNum()   )
            return false;
      size_t nIndex = pNodeToInsert->nBusyKey ;
      for( ;  nIndex > 0 ; nIndex--  )
      {
            if( pPosAndKey.kKey > pNodeToInsert->gPosAndKey[nIndex-1].kKey )
                  break;
            pNodeToInsert->gPosAndKey[nIndex] = pNodeToInsert->gPosAndKey[nIndex-1];
      }
      pNodeToInsert->gPosAndKey[nIndex]  = pPosAndKey;
      ++pNodeToInsert->nBusyKey;
       --pNodeToInsert->nIdleKey;
       nbBuffer.SetDirty();
       return true;
}

off_t CBtree::__SearchPosByKey (const NODE_BUFFER &nbBuffer  ,KEY_TYPE kKey)const
{
      BTREE_NODE* pNode = nbBuffer.pNode;
      POS_AND_KEY* pStart = pNode->gPosAndKey;
      POS_AND_KEY* pEnd = pStart + pNode->nBusyKey;
      POS_AND_KEY* pFind = std::upper_bound( pStart, pEnd , kKey )  ;

      if(  pFind == pStart ) return nDEFAULT_POS;
      if( __NodeIsLeaf(nbBuffer) )
      {
            if((pFind - 1)->kKey != kKey )
                  return nDEFAULT_POS;
      }

      return ( pFind - 1 )->nPos;
}

void CBtree::__LeftRotate( NODE_BUFFER &nbNode  )
{
      nbNode.SetDirty();
      BTREE_NODE *pNodeToLeftRotate = nbNode.pNode;
      size_t nArrayNum = pNodeToLeftRotate->nBusyKey + pNodeToLeftRotate->nIdleKey;

      POS_AND_KEY *pFirstStart = pNodeToLeftRotate->gPosAndKey;
      POS_AND_KEY *pFirstEnd =pFirstStart + pNodeToLeftRotate->nBusyKey;
      POS_AND_KEY *pSecondStart = pFirstEnd;
      POS_AND_KEY *pSecondEnd = pFirstStart + nArrayNum;
      std::reverse(pFirstStart , pFirstEnd);
      std::reverse(pSecondStart , pSecondEnd);
      std::reverse(pFirstStart , pSecondEnd);
      std::swap( pNodeToLeftRotate->nBusyKey , pNodeToLeftRotate->nIdleKey );
}

bool  CBtree::__SplitNode( NODE_BUFFER &nbParentNode, NODE_BUFFER &nbChildNode , KEY_TYPE kKey)
{
      BTREE_NODE  *pParentNode = nbParentNode.pNode;
      BTREE_NODE  *pChildNode = nbChildNode.pNode;
      if(  !pParentNode || !pChildNode )  return false;

      off_t nOldSelfPos= pChildNode->nSelfPos;
      off_t nOldNextPos = pChildNode->nNextPos;
      off_t nNewNodePos = __AllocNewNodePos();

      size_t nMoveOutNum = (m_bhHeader.nOrderNum)>>1;// /2 , the strategy may be changed
      size_t nLeftRotateNum = GetOrderNum() - nMoveOutNum;
      POS_AND_KEY pPosAndKeyToInsert ;
      pPosAndKeyToInsert.nPos = nNewNodePos;// the pos of second node
      pPosAndKeyToInsert.kKey = pChildNode->gPosAndKey[nLeftRotateNum].kKey;//first entry in second node
      //change parent node
      if ( !__InsertKeyIntoNode( nbParentNode ,pPosAndKeyToInsert) ) return false;

      __ShowNode(nbParentNode.pNode);

      pChildNode->nBusyKey = nLeftRotateNum;
      pChildNode->nIdleKey = GetOrderNum() - pChildNode ->nBusyKey;

      off_t nKeyPos  = __SearchPosByKey(pParentNode , kKey);
      if( nKeyPos == nNewNodePos  )
      {
            ProcessFirstNode:
            pChildNode ->nSelfPos =nOldSelfPos;
            pChildNode ->nNextPos = nNewNodePos;
            //import bug appears,use __WriteDirtyNode,but didn't set dirty,so it never be write back.
            nbChildNode.SetDirty();
            __ShowNode(pChildNode);
            if( nKeyPos == nNewNodePos  )
            {
                  __WriteDirtyNode(nbChildNode);
                  goto ProcessSecondNode;
            }
      }
      else if ( nKeyPos == nOldSelfPos )
      {
            ProcessSecondNode:
            pChildNode ->nSelfPos = nNewNodePos;
            pChildNode ->nNextPos = nOldNextPos;
            __LeftRotate(nbChildNode);
            __WriteDirtyNode(nbChildNode);//must write back immidiately
            m_bhHeader.IncreaseNumOf(NODE);
            __ShowNode(pChildNode);

            if( nKeyPos == nOldSelfPos )
            {
                  __LeftRotate(nbChildNode);
                  goto ProcessFirstNode;
            }
      }
      else
      {
            return false;
      }

      nbParentNode.SetDirty();
      nbChildNode.SetDirty();
      return true;
}

bool CBtree::Insert( const POS_AND_KEY &pPosAndKeyToInsert  )
{
      if( m_vcNodeBuffer[0].pNode->nIdleKey == 0 )
      {
            //increase a new root node
            NODE_BUFFER nbRootNode( BTREE_NODE::CreateBtreeNode(SizeofBTreeNode()) ) ;
            BTREE_NODE tempRootNode( NO_LEAF , 0 , m_bhHeader.nOrderNum ,__AllocNewNodePos() , nDEFAULT_POS  ) ;
            memcpy( nbRootNode.pNode,  &tempRootNode , nSIZEOF_BTREE_NODE );
            //create a new key into root node
            KEY_TYPE kKey  =  m_vcNodeBuffer[0].pNode->gPosAndKey[0].kKey;
            off_t nPos  = m_vcNodeBuffer[0].pNode->nSelfPos;
            POS_AND_KEY pakKey(nPos,kKey);

            __InsertKeyIntoNode(nbRootNode ,pakKey);
            __WriteDirtyNode(nbRootNode );
            //cahnge root pos
            m_bhHeader.nRootPos =  nbRootNode.pNode->nSelfPos ;
            m_bhHeader.IncreaseNumOf(HEIGHT);
            m_bhHeader.IncreaseNumOf(NODE);

            m_vcNodeBuffer.insert( m_vcNodeBuffer.begin() , nbRootNode );

            if ( !__SplitNode(m_vcNodeBuffer[0],m_vcNodeBuffer[1],pPosAndKeyToInsert.kKey) ) return false;
            return __InsertNodeNonFull( m_vcNodeBuffer ,  0  , pPosAndKeyToInsert );
      }
      else
      {
            return  __InsertNodeNonFull(m_vcNodeBuffer ,  0 , pPosAndKeyToInsert);
      }
}

bool CBtree::__InsertNodeNonFull(  std::vector<NODE_BUFFER> &vcNodeBuffer , size_t nLevel , POS_AND_KEY pPosAndKeyToInsert )
{
      //
      if ( __NodeIsLeaf(vcNodeBuffer[nLevel]) )
      {
            if( nDEFAULT_POS == __SearchPosByKey(  vcNodeBuffer[nLevel].pNode , pPosAndKeyToInsert.kKey) )
            {
                  __InsertKeyIntoNode( vcNodeBuffer[nLevel] ,pPosAndKeyToInsert);//maybe need to add the pos of entry
                  m_bhHeader.IncreaseNumOf(KEY);
                  //__WriteHeader();
                  __ShowNode(vcNodeBuffer[nLevel].pNode);
                  __ShowHeader();
                  return true;
            }
            else
            {
                  return false;
            }
      }
      else
      {
            off_t nPos = __SearchPosByKey(m_vcNodeBuffer[nLevel],pPosAndKeyToInsert.kKey) ;
            if( nDEFAULT_POS == nPos  )
            {
                  m_vcNodeBuffer[nLevel].pNode->gPosAndKey[0].kKey = pPosAndKeyToInsert.kKey;
                  nPos = m_vcNodeBuffer[nLevel].pNode->gPosAndKey[0].nPos;
                  m_vcNodeBuffer[nLevel].SetDirty();
            }

            if(  nPos != m_vcNodeBuffer[ nLevel +1 ].pNode->nSelfPos )
            {
                  __WriteDirtyNode(m_vcNodeBuffer[nLevel +1 ]);
                  __ReadNode( m_vcNodeBuffer[ nLevel +1 ].pNode ,nPos );
            }

            if( m_vcNodeBuffer[nLevel +1 ].pNode ->nIdleKey == 0 )
                  __SplitNode(m_vcNodeBuffer[nLevel] ,m_vcNodeBuffer[nLevel + 1] , pPosAndKeyToInsert.kKey );

            __InsertNodeNonFull( m_vcNodeBuffer , ++nLevel , pPosAndKeyToInsert);
      }
      return true;
}

off_t CBtree::__AllocNewNodePos()
{
      if ( m_bhHeader.nFirstFreeBlockPos == nDEFAULT_POS )
      {
            return   m_pFileOp->Seek(0,SEEK_END);
      }
      //reserve other condition
      return nDEFAULT_POS;
}

void CBtree::__WriteHeader() const
{
      m_pFileOp->Seek( 0 ,SEEK_SET);
      m_pFileOp->Write(& m_bhHeader , nSIZEOF_BTREE_HEADER);
}

void CBtree::Traversal()
{
      BTREE_NODE *p =BTREE_NODE::CreateBtreeNode(SizeofBTreeNode());
      off_t nPos= m_bhHeader.nStartLeafPos;
      while(nPos != nDEFAULT_POS)
      {
            __ReadNode(p,nPos);
            __ShowNode(p);
            nPos= p->nNextPos;
      }
      free(p);
}


CBtree& CBtree::operator=(const CBtree& rhs)
{
      if (this == &rhs) return *this; // handle self assignment
      //assignment operator
      return *this;
}



void CBtree::Show()
{
      srand( (unsigned)time(NULL) );
      for( size_t nInput = 0 ; nInput < nTestNum ; nInput++ )
      {
            //getchar();
            size_t nPos =0;
            KEY_TYPE kKey = rand();
            POS_AND_KEY pPAK(nPos, kKey);
            //std::cout<<"nPos = "<<nPos <<" key = "<<kKey<<std::endl;
            Insert( pPAK );

            size_t nOutputOpint = nTestNum /20;
            if( (nInput +1) %(nOutputOpint) == 0 )
            {
                  printf("%d past\n",nOutputOpint);
            }
      }
      printf("game over!\n");
}

}
