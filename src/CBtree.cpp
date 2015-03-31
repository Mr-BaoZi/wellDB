#include "CBtree.h"

namespace wellDB
{

tag_BTREE_HEADER::tag_BTREE_HEADER():
nMagicNum(nMAGIC_NUM) , nOrderNum(nORDER_NUM)   ,nKeyNum (0) , nNodeNum(1)   ,  nHeight(1),
nRootPos (nSIZEOF_BTREE_HEADER),    nStartLeafPos(nDEFAULT_POS),
nFreeBlockNum(0),  nFirstFreeBlockPos (nDEFAULT_POS)  {     }

tag_BTREE_HEADER::tag_BTREE_HEADER(size_t nOrder):
nMagicNum(nMAGIC_NUM) , nOrderNum(nOrder)   ,nKeyNum (0) , nNodeNum(1)   ,  nHeight(1),
nRootPos (nSIZEOF_BTREE_HEADER),    nStartLeafPos(nDEFAULT_POS),
nFreeBlockNum(0),  nFirstFreeBlockPos (nDEFAULT_POS)  {     }


tag_BTREE_NODE::tag_BTREE_NODE(BTREE_NODE_TYPE eType ,  size_t nBusy , size_t nIdle , off_t nSelf , off_t nNext  ):
eNodeType(eType) , nBusyKey(nBusy) , nIdleKey(nIdle) , nSelfPos(nSelf) , nNextPos(nNext)  {     }

CBtree::CBtree():m_pFileOp(NULL)
{

}

CBtree::~CBtree()
{
      //dtor
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
      __ShowNode(m_vcNodeBuffer[0]);
      return true;
}

void CBtree::__InitHeader(size_t nOrderNum)
{
      //file is too short
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
      for( size_t i = 0 ; i < m_bhHeader.nHeight ; i++ )
      {
            BTREE_NODE* temp = (BTREE_NODE* )malloc(nNodeByte);
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

void CBtree::__ShowNode(BTREE_NODE * pBtreeNode)
{
      BTREE_NODE* p = pBtreeNode;
      printf("node:\n");
      printf("busy = %d idle = %d selfPos = %d nextPos = %d \n",p->nBusyKey,p->nIdleKey, p->nSelfPos,p->nNextPos);
      for ( size_t i = 0 ; i < p->nBusyKey ; i++ )
      {
            printf("%d:  key = %d  pos = %d \n" ,  i ,p->gPosAndKey[i].kKey , p->gPosAndKey[i].nPos );
      }

}

void CBtree::__ShowHeader()
{
      BTREE_HEADER *p = &m_bhHeader;
      printf("header: \n");
      printf("oder = %d key = %d node = %d height = %d rootPos = %d startLeafPos = %d\n" , p->nOrderNum , p->nKeyNum ,p->nNodeNum , p->nHeight , p->nRootPos ,p->nStartLeafPos);
}


/*
CBtree& CBtree::operator=(const CBtree& rhs)
{
      if (this == &rhs) return *this; // handle self assignment
      //assignment operator
      return *this;
}



bool  CBtree::__SplitNode( BTREE_NODE* pParentNode, BTREE_NODE* pChildNode ,KEY_TYPE kKey)
{
      if(  !pParentNode || !pChildNode )  return false;

      size_t nMoveOutNum = (m_pHeader ->nOrderNum)>>1;// /2
      off_t nOldSelfPos= pChildNode->nSelfPos;
      off_t nOldNextPos = pChildNode->nNextPos;
      off_t nNewNodePos = lseek(m_fdBTreeFile,0,SEEK_END);
      size_t nLeftRotateNum = GetOrderNum() - nMoveOutNum;
      POS_AND_KEY pPosAndKeyToInsert ;

      pPosAndKeyToInsert.nPos = nNewNodePos;// the pos of second node
      pPosAndKeyToInsert.nKey = pChildNode->gPosAndKey[nLeftRotateNum].nKey;//first entry in second node
      //change parent node
      __InsertKeyIntoNode(pParentNode,pPosAndKeyToInsert);
      __ShowNode(pParentNode);
      //search pos of the key in the parent node;
      size_t nIndex = -1;
      off_t nKeyPos = nDEFAULT_POS;
      __SearchPosByKey(pParentNode,kKey,nIndex,nKeyPos);
      //please wait , in turn write disk
      if( nKeyPos == nNewNodePos  )
      {
            ProcessFirstNode:

            pChildNode ->nBusyKey = nLeftRotateNum;
            pChildNode ->nIdleKey = GetOrderNum() - pChildNode ->nBusyKey;
            pChildNode ->nSelfPos =nOldSelfPos;
            pChildNode ->nNextPos = nNewNodePos;
            __WriteNode(pChildNode);
            __ShowNode(pChildNode);
            if( nKeyPos == nNewNodePos  )
                        goto ProcessSecondNode;
      }
      else if ( nKeyPos == nOldSelfPos )
      {
            ProcessSecondNode:

            pChildNode->nBusyKey = nMoveOutNum;
            pChildNode->nIdleKey  = GetOrderNum() - pChildNode->nBusyKey;
            pChildNode ->nSelfPos = nNewNodePos;
            pChildNode ->nNextPos = nOldNextPos;
            __LeftRotate(pChildNode->gPosAndKey,GetOrderNum(),nLeftRotateNum);
            __WriteNode(pChildNode);
            __ShowNode(pChildNode);
            if( nKeyPos == nOldSelfPos )
            {
                  __LeftRotate(pChildNode->gPosAndKey,GetOrderNum(),nMoveOutNum);
                  goto ProcessFirstNode;
            }
      }
      else
      {
            printf("split false\n");
            return false;
      }
      //change header content
     m_pHeader->nNodeNum++;//increase a sibling
     __WriteHeader();
      return true;
}


bool CBtree::__LeftRotate( POS_AND_KEY* pArray, size_t nArrayLen,  size_t nRotateNum )
{
      if(  !pArray || nRotateNum <0 || nArrayLen <= 0  )
            return false;
      nRotateNum %= nArrayLen;
      if( nRotateNum == 0 )
            return true;

      std::reverse(pArray,pArray + nRotateNum);
      std::reverse(pArray+nRotateNum, pArray + nArrayLen  );
      std::reverse(pArray,pArray+nArrayLen );

      return true;
}

bool  CBtree::__WriteHeader()
{

      if( !m_pHeader) return false;
      lseek(m_fdBTreeFile,0,SEEK_SET);
      write(m_fdBTreeFile,m_pHeader,nSIZEOF_BTREE_HEADER);
      fsync(m_fdBTreeFile);
      return true;

}

bool CBtree::__WriteNode( BTREE_NODE* pNodeToWrite )const
{
      if( ! pNodeToWrite ) return false;

      off_t nPos = pNodeToWrite->nSelfPos;
      lseek(m_fdBTreeFile , nPos, SEEK_SET);
      write(m_fdBTreeFile,pNodeToWrite, SizeofBTreeNode());
      //fsync(m_fdBTreeFile);
      return true;
}


bool CBtree::__ReadNode( BTREE_NODE* pNodeToWrite , off_t nPos )
{
      if(!pNodeToWrite) return false;

      lseek(m_fdBTreeFile , nPos ,SEEK_SET);
      read(m_fdBTreeFile,pNodeToWrite,SizeofBTreeNode());

      if( pNodeToWrite->nSelfPos != nPos) return false;
      return true;
}*/

/*
bool CBtree::__SearchPosByKey (BTREE_NODE* pNodeToSearch ,KEY_TYPE kKey ,size_t &nIndex ,off_t &nPos )const
{
      if(!pNodeToSearch) return false;
      if ( __NodeIsLeaf( pNodeToSearch ) )
      {
            size_t nIndexTemp = -1;
            while( ++nIndexTemp < pNodeToSearch->nBusyKey  )
            {
                  if(pNodeToSearch->gPosAndKey[nIndexTemp].nKey==kKey)
                  {
                        nIndex = nIndexTemp ;
                        nPos = pNodeToSearch->gPosAndKey[nIndex].nPos;
                        return true;
                  }
             }
            return false;
      }
      else//for non-leaf,we use linear search
      {
            if( pNodeToSearch->nBusyKey <= 0  ) return false;
            if( kKey < pNodeToSearch->gPosAndKey[0].nKey ) return false;

            size_t nIndexTemp =pNodeToSearch->nBusyKey - 1 ;
            while(  nIndexTemp >= 0  && kKey < pNodeToSearch->gPosAndKey[nIndexTemp].nKey )
                  nIndexTemp--;
            nIndex=nIndexTemp;
            nPos = pNodeToSearch->gPosAndKey[nIndex].nPos;
            return true;
      }
      return false;
}

bool CBtree::__InsertKeyIntoNode( BTREE_NODE* pNodeToInsert, const POS_AND_KEY &pPosAndKey)
{
      if (!pNodeToInsert)      return false;
      if (pNodeToInsert->nIdleKey == 0 ) return false;
      if (pNodeToInsert->nBusyKey == GetOrderNum() ) return false;

      size_t nIndex = pNodeToInsert->nBusyKey;
      for(  ;  nIndex > 0 ; nIndex--  )
      {
            if( pPosAndKey.nKey > pNodeToInsert->gPosAndKey[nIndex-1].nKey )
                  break;
            pNodeToInsert->gPosAndKey[nIndex] = pNodeToInsert->gPosAndKey[nIndex-1];
      }

      pNodeToInsert->gPosAndKey[nIndex]  = pPosAndKey;
      ++pNodeToInsert->nBusyKey;
       --pNodeToInsert->nIdleKey;
      if( __NodeIsRoot(pNodeToInsert ))
      {
            memcpy(m_pRootNode , pNodeToInsert , SizeofBTreeNode());
      }
      //bug: maybe when m_pBtreeNode act as copy of m_pRootNode ,it change ,but it never write back
       __WriteNode(pNodeToInsert);
}

bool CBtree::__InsertNodeNonFull(BTREE_NODE* pBtreeNode[] , size_t nArray,  POS_AND_KEY pPosAndKeyToInsert )
{
      if( !pBtreeNode[0] || !pBtreeNode[1] )    return false;
      //pos 0 put parent node , pos 1 put child node ,
      //before recursing ,we drop parent ,as s result ,we exchange the point of both of them
      if ( __NodeIsLeaf(pBtreeNode[0]) )
      {
            size_t nIndex = -1;
            off_t nPos = nDEFAULT_POS;
            if ( __SearchPosByKey( pBtreeNode[0] ,pPosAndKeyToInsert.nKey,nIndex,nPos) )  return false;

            __InsertKeyIntoNode(pBtreeNode[0] ,pPosAndKeyToInsert);//maybe need to add the pos of entry
            ++m_pHeader->nKeyNum;
            __WriteHeader();
            if( __NodeIsRoot(pBtreeNode[0] ))
            {
                  memcpy(m_pRootNode , pBtreeNode[0] , SizeofBTreeNode());
            }
      }
      else
      {
            size_t nIndex = -1 ;
            off_t nPos =nDEFAULT_POS;
            if ( !__SearchPosByKey(pBtreeNode[0] ,pPosAndKeyToInsert.nKey ,nIndex , nPos) )
            {
                  pBtreeNode[0]->gPosAndKey[0].nKey = pPosAndKeyToInsert.nKey;
                  nPos = pBtreeNode[0]->gPosAndKey[0].nPos;
                  __WriteNode(pBtreeNode[0]);
            }
            __ReadNode(pBtreeNode[1],nPos);

            if(pBtreeNode[1]->nIdleKey == 0)
            {
                  __SplitNode(pBtreeNode[0],pBtreeNode[1], pPosAndKeyToInsert.nKey);
            }
            std::swap(pBtreeNode[0], pBtreeNode[1]);
            __InsertNodeNonFull( pBtreeNode , nArray , pPosAndKeyToInsert );
      }
      __ShowNode(pBtreeNode[0]);
      return true;
}
*/

/*
bool CBtree::Insert( const POS_AND_KEY &pPosAndKeyToInsert  )
{
      memcpy(m_pNodeBuffer[0], m_pRootNode , SizeofBTreeNode());
      if( m_pNodeBuffer[0]->nIdleKey == 0 )
      {
            //we create a new root node
            //in case of fault , we write node at first ,but it will increase one-time unnecessary write.
            memcpy(m_pRootNode,&DEFAULT_ROOT_NODE,nSIZEOF_BTREE_NODE );
            m_pRootNode->eType = NO_LEAF;
            m_pRootNode->nSelfPos = lseek(m_fdBTreeFile , 0, SEEK_END);
            m_pRootNode->gPosAndKey[0].nKey = m_pNodeBuffer[0]->gPosAndKey[0].nKey;
            m_pRootNode->gPosAndKey[0].nPos = m_pNodeBuffer[0]->nSelfPos;
            m_pRootNode->nBusyKey ++;
            m_pRootNode->nIdleKey-- ;
            __WriteNode(m_pRootNode);

             //increase a new root node
            ++m_pHeader->nHeight;
            ++m_pHeader->nNodeNum;
            m_pHeader->nRootPos =  m_pRootNode->nSelfPos ;
            __WriteHeader();

            __ShowNode(m_pRootNode);

            if ( !__SplitNode(m_pRootNode,m_pNodeBuffer[0],pPosAndKeyToInsert.nKey) ) return false;
            return __InsertNodeNonFull( m_pNodeBuffer ,nNODE_BUFFER , pPosAndKeyToInsert );
      }
      else
      {
            return  __InsertNodeNonFull(m_pNodeBuffer,nNODE_BUFFER,pPosAndKeyToInsert);
      }
}*/

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
