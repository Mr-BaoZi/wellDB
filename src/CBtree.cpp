#include "CBtree.h"

namespace wellDB
{

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

CBtree::CBtree():m_pFileOp(NULL),m_pNextNodeBuffer(NULL)
{

}

CBtree::~CBtree()
{
      //dtor
      // code is incomplete , write header must be added
      if(m_pFileOp)
            delete m_pFileOp;

      for (  size_t i = 0 ; i < m_vcNodeBuffer.size() ; i++)
      {
            if( m_vcNodeBuffer[i]  )
                  free(m_vcNodeBuffer[i] );
      }
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
            __InitHeader(nOrderNum);
            __ShowHeader();
      }

      if( m_bhHeader.nMagicNum != nMAGIC_NUM )  return false;

      __InitNodeBuffer();
      __ShowNode(m_vcNodeBuffer[0].pBuffer);
      return true;
}

void CBtree::__InitHeader(size_t nOrderNum)
{
      //file is too short
      if(nOrderNum < 4 ) nOrderNum = 4;
      BTREE_HEADER tempDefaultHeader(nOrderNum);
      BTREE_NODE tempFirstNode( LEAF , 0 , nOrderNum ,nSIZEOF_BTREE_HEADER , nDEFAULT_POS  ) ;
      POS_AND_KEY tempPAK;

      m_pFileOp ->Seek( 0 ,SEEK_SET);
      //Init header
      m_pFileOp ->Write( &tempDefaultHeader, nSIZEOF_BTREE_HEADER );
      memcpy( &m_bhHeader , &tempDefaultHeader , nSIZEOF_BTREE_HEADER);
      //init firstNode
      m_pFileOp ->Write( &tempFirstNode, nSIZEOF_BTREE_NODE);

      for( size_t i = 0 ; i < nOrderNum ; i++ )
            m_pFileOp->Write( &tempPAK, nSIZEOF_POS_AND_KEY );
}

void CBtree::__InitNodeBuffer()
{
      size_t nNodeByte = SizeofBTreeNode();
      m_pNextNodeBuffer = BTREE_NODE::CreateBtreeNode(nNodeByte);
      for( size_t i = 0 ; i < m_bhHeader.nHeight ; i++ )
      {
            BTREE_NODE* temp = BTREE_NODE::CreateBtreeNode(nNodeByte);
            m_vcNodeBuffer.push_back(temp);
            //read  data of every level in buffer
            off_t nDataPos = nDEFAULT_POS;

            if( i == 0 )
                  nDataPos = m_bhHeader.nRootPos;
            else
                  nDataPos = m_vcNodeBuffer[i-1]->gPosAndKey[0].nPos;

            m_pFileOp->Seek(nDataPos, SEEK_SET);
            m_pFileOp->Read( m_vcNodeBuffer[i], nNodeByte );
      }
}

void CBtree::__RegularWrite()
{
      __WriteHeader();
      for (size_t i = 0 ; i < m_vcNodeBuffer.size(); i++)
      {
            __WriteDirtyNode( m_vcNodeBuffer[i] );
      }
}

void CBtree::__WriteDirtyNode(const NODE_BUFFER &nbBuffer)
{
      if( nbBuffer.isDirty == true )
      {
            __WriteNode( nbBuffer.pBuffer );
            nbBuffer.isDirty = false;
      }
}

void CBtree::__ShowNode(BTREE_NODE * pBtreeNode)const
{
      BTREE_NODE* p = pBtreeNode;
      printf("node:\n");
      printf("busy = %d idle = %d selfPos = %ld nextPos = %ld \n",p->nBusyKey,p->nIdleKey, p->nSelfPos,p->nNextPos);
      for ( size_t i = 0 ; i < p->nBusyKey ; i++ )
      {
            printf("%d:  key = %d  pos = %ld \n" ,  i ,p->gPosAndKey[i].kKey , p->gPosAndKey[i].nPos );
      }
}

bool CBtree::__WriteNode( BTREE_NODE* pNodeToWrite )
{
      if( ! pNodeToWrite ) return false;

      off_t nPos = pNodeToWrite->nSelfPos;
      m_pFileOp->Seek(nPos, SEEK_CUR);
      m_pFileOp->Write( pNodeToWrite ,SizeofBTreeNode()  );
      __ShowNode(pNodeToWrite);
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

void CBtree::__ShowHeader()
{
      BTREE_HEADER* p = &m_bhHeader;
      printf("header: \n");
      printf("oder = %d key = %d node = %d height = %d rootPos = %ld startLeafPos = %ld\n" , p->nOrderNum , p->nKeyNum ,p->nNodeNum , p->nHeight , p->nRootPos ,p->nStartLeafPos);
}

bool CBtree::__InsertKeyIntoNode( BTREE_NODE* pNodeToInsert, const POS_AND_KEY &pPosAndKey)
{
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

       __WriteNode(pNodeToInsert);
       return true;
}

off_t CBtree::__SearchPosByKey (BTREE_NODE* pNodeToSearch ,KEY_TYPE kKey )const
{
      if(!pNodeToSearch) return false;

      POS_AND_KEY* pStart = pNodeToSearch->gPosAndKey;
      POS_AND_KEY* pEnd = pStart + pNodeToSearch->nBusyKey;
      POS_AND_KEY* pFind = std::upper_bound( pStart, pEnd , kKey )  ;

      if(  pFind == pStart ) return nDEFAULT_POS;
      if( __NodeIsLeaf(pNodeToSearch) )
      {
            if((pFind - 1)->kKey != kKey )
                  return nDEFAULT_POS;
      }
      return ( pFind - 1 )->nPos;
}

bool __LeftRotate( BTREE_NODE* pNodeToLeftRotate )
{
      if(  !pNodeToLeftRotate || nRotateNum <0   )
            return false;

      size_t nArrayNum = pNodeToLeftRotate->nBusyKey + pNodeToLeftRotate->nIdleKey;
       nRotateNum %= nArrayNum;
      if( nRotateNum == 0 )
            return true;

      POS_AND_KEY *pFirstStart = pNodeToLeftRotate->gPosAndKey;
      POS_AND_KEY *pFirstEnd =pFirstStart + pNodeToLeftRotate->nBusyKey;
      POS_AND_KEY *pSecondStart = pFirstEnd;
      POS_AND_KEY *pSecondEnd = pFirstStart + nArrayNum;
      std::reverse(pFirstStart , pFirstEnd);
      std::reverse(pSecondStart , pSecondEnd);
      std::reverse(pFirstStart , pSecondEnd);
      std::swap( pNodeToLeftRotate->nBusyKey , pNodeToLeftRotate->nIdleKey );
      return true;
}

bool  CBtree::__SplitNode( BTREE_NODE* pParentNode, BTREE_NODE* pChildNode ,KEY_TYPE kKey)
{
      if(  !pParentNode || !pChildNode )  return false;

      off_t nOldSelfPos= pChildNode->nSelfPos;
      off_t nOldNextPos = pChildNode->nNextPos;
      off_t nNewNodePos = m_pFileOp->Seek(0,SEEK_END);

      size_t nMoveOutNum = (m_pHeader ->nOrderNum)>>1;// /2 , the strategy may be changed
      size_t nLeftRotateNum = GetOrderNum() - nMoveOutNum;
      POS_AND_KEY pPosAndKeyToInsert ;
      pPosAndKeyToInsert.nPos = nNewNodePos;// the pos of second node
      pPosAndKeyToInsert.kKey = pChildNode->gPosAndKey[nLeftRotateNum].kKey;//first entry in second node
      //change parent node
      __InsertKeyIntoNode(pParentNode,pPosAndKeyToInsert);
      __ShowNode(pParentNode);
      //please wait , in turn write disk
      //But before it ,we must init the node
      pChildNode->nBusyKey = nLeftRotateNum;
      pChildNode->nIdleKey = GetOrderNum() - pChildNode ->nBusyKey
      off_t nKeyPos  = __SearchPosByKey(pParentNode , kKey);

      if( nKeyPos == nNewNodePos  )
      {
            ProcessFirstNode:

            pChildNode ->nSelfPos =nOldSelfPos;
            pChildNode ->nNextPos = nNewNodePos;
            __WriteNode(pChildNode);

            if( nKeyPos == nNewNodePos  )
                        goto ProcessSecondNode;
      }
      else if ( nKeyPos == nOldSelfPos )
      {
            ProcessSecondNode:

            pChildNode ->nSelfPos = nNewNodePos;
            pChildNode ->nNextPos = nOldNextPos;
            __LeftRotate(pChildNode->gPosAndKey,GetOrderNum(),nLeftRotateNum);
            __WriteNode(pChildNode);

            m_pHeader->nNodeNum++;//increase a sibling

            if( nKeyPos == nOldSelfPos )
            {
                  __LeftRotate(pChildNode->gPosAndKey,GetOrderNum(),nMoveOutNum);
                  goto ProcessFirstNode;
            }
      }
      else
      {
            return false;
      }
      return true;
}

bool CBtree::Insert( const POS_AND_KEY &pPosAndKeyToInsert  )
{
      if( m_pNodeBuffer[0]->nIdleKey == 0 )
      {
            //increase a new root node
            BTREE_NODE tempRootNode( NO_LEAF , 0 , nOrderNum ,m_pFileOp->Seek(0,SEEK_END) , nDEFAULT_POS  ) ;
            BTREE_NODE* pNewRootNode =  BTREE_NODE::CreateBtreeNode(SizeofBTreeNode());
            m_vcNodeBuffer.insert(m_vcNodeBuffer.begin() , pNewRootNode );
            memcpy( m_vcNodeBuffer[0] ,  tempRootNode , nSIZEOF_BTREE_NODE );

            POS_AND_KEY pakKey;
            pakKey.kKey =  m_vcNodeBuffer[1]->gPosAndKey[0].kKey;
            pakKey.nPos = m_vcNodeBuffer[1]->nSelfPos;
            __InsertKeyIntoNode(m_vcNodeBuffer[0],pakKey);

            //cahnge root pos
            m_pHeader->nRootPos =  m_vcNodeBuffer[0]->nSelfPos ;
            __WriteHeader();

            if ( !__SplitNode(m_vcNodeBuffer[0],m_vcNodeBuffer[1],pPosAndKeyToInsert.nKey) ) return false;
            return __InsertNodeNonFull( m_vcNodeBuffer ,  0  , pPosAndKeyToInsert );
      }
      else
      {
            return  __InsertNodeNonFull(m_vcNodeBuffer ,  0 , pPosAndKeyToInsert);
      }
}

bool CBtree::__InsertNodeNonFull(  std::vector<BTREE_NODE*> &vcNodeBuffer , size_t nLevel , POS_AND_KEY pPosAndKeyToInsert )
{
      //
      if ( __NodeIsLeaf(pBtreeNode[nLevel]) )
      {
            //if ( __SearchPosByKey( pBtreeNode[0] ,pPosAndKeyToInsert.nKey,nIndex,nPos) )  return false;
            if( nDEFAULT_POS != __SearchPosByKey(pPosAndKeyToInsert.kKey) )
            {
                  __InsertKeyIntoNode(pBtreeNode[0] ,pPosAndKeyToInsert);//maybe need to add the pos of entry
                  ++m_pHeader->nKeyNum;
                  __WriteHeader();
                  return true;
            }
            else  return false;
      }
      else
      {
            off_t nPos = __SearchPosByKey(m_vcNodeBuffer[nLevel],pPosAndKeyToInsert.kKey) ;
            if( nDEFAULT_POS == nPos  )
            {
                  m_vcNodeBuffer[nLevel]->gPosAndKey[0].kKey = pPosAndKeyToInsert.kKey;
                  nPos = m_vcNodeBuffer[nLevel]->gPosAndKey[0].nPos;
            }

            if(  nPos != m_vcNodeBuffer[ nLevel +1 ]->nSelfPos )
                  __ReadNode( m_vcNodeBuffer[ nLevel +1 ] ,nPos );

            if( m_vcNodeBuffer[nLevel +1 ] ->nIdleKey == 0 )
                  __SplitNode(m_vcNodeBuffer[nLevel] ,m_vcNodeBuffer[nLevel + 1] , pPosAndKeyToInsert.kKey );

            __InsertNodeNonFull( m_vcNodeBuffer , ++nLevel , pPosAndKeyToInsert);
      }
      return true;
}

/*
CBtree& CBtree::operator=(const CBtree& rhs)
{
      if (this == &rhs) return *this; // handle self assignment
      //assignment operator
      return *this;
}

bool  CBtree::__WriteHeader()
{
      if( !m_pHeader) return false;
      lseek(m_fdBTreeFile,0,SEEK_SET);
      write(m_fdBTreeFile,m_pHeader,nSIZEOF_BTREE_HEADER);
      fsync(m_fdBTreeFile);
      return true;
}

*/





/*
void CBtree::Show()
{
      BTREE_NODE *temp = ( BTREE_NODE *)malloc(SizeofBTreeNode());


      for( size_t nInput = 0 ; nInput < nTestNum ; nInput++ )
      {
            POS_AND_KEY pPAK;
            //srand( (unsigned)time(NULL) );
            //pPAK.nPos =rand();
            //pPAK.nKey = rand();
            pPAK.nPos = 0;
            pPAK.nKey = nInput;
            Insert( pPAK );
            //getchar();
            size_t nOutputOpint = nTestNum /20;
            if( nInput %(nOutputOpint) == 0 )
            {
                  printf("%d past\n",nOutputOpint);
            }
      }
      printf("game over!\n");
      free(temp);
}

*/

}
