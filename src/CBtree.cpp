#include "CBtree.h"

namespace wellDB
{

CBtree::CBtree( const char* cPath  )
{
      //ctor
      m_fdBTreeFile = open(cPath,O_RDWR |O_CREAT,0666 );
      m_pHeader = (BTREE_HEADER *)malloc(nSIZEOF_BTREE_HEADER);
      //if file is fake
      if ( nSIZEOF_BTREE_HEADER != read(m_fdBTreeFile, m_pHeader,nSIZEOF_BTREE_HEADER ) &&
      m_pHeader ->nMagicNum != nMAGIC_NUM )
      {
            //clean the fake file
            lseek(m_fdBTreeFile, 0 , SEEK_SET);
            ftruncate( m_fdBTreeFile , 0 );
            //write default header
            write(m_fdBTreeFile,&DEFAULT_BTREE_HEADER, nSIZEOF_BTREE_HEADER);
            //write default root node
            write(m_fdBTreeFile,&DEFAULT_ROOT_NODE,nSIZEOF_BTREE_NODE );
            for( size_t i = 0; i < DEFAULT_BTREE_HEADER.nOrderNum ; i++ )
            {
                  write( m_fdBTreeFile ,&DEFAULT_POS_AND_KEY ,nSIZEOF_POS_AND_KEY );
            }
            fsync(m_fdBTreeFile);
            memcpy(m_pHeader, &DEFAULT_BTREE_HEADER ,nSIZEOF_BTREE_HEADER );
      }
      size_t nNodeByte = SizeofBTreeNode();
      m_pRootNode = (BTREE_NODE* )malloc( nNodeByte );
      lseek(m_fdBTreeFile , nSIZEOF_BTREE_HEADER,SEEK_SET);
      read(m_fdBTreeFile, m_pRootNode,nNodeByte );

      for (  size_t i = 0 ; i< nNODE_BUFFER ; i++)
            m_pNodeBuffer[i] =  (BTREE_NODE* )malloc( nNodeByte );
}

CBtree::~CBtree()
{
      //dtor
      close(m_fdBTreeFile);

      if( m_pHeader )
            free(m_pHeader);

      if(m_pRootNode)
            free(m_pRootNode);

      for (  size_t i = 0 ; i< nNODE_BUFFER ; i++)
      {
            if( m_pNodeBuffer[i]  )
                  free(m_pNodeBuffer[i] );
      }
}

CBtree::CBtree(const CBtree& other)
{
      //copy ctor
}

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
/*
      if( !m_pHeader) return false;
      lseek(m_fdBTreeFile,0,SEEK_SET);
      write(m_fdBTreeFile,m_pHeader,nSIZEOF_BTREE_HEADER);
      fsync(m_fdBTreeFile);
      return true;
      */
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
}

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
}

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

void CBtree::__ShowNode(BTREE_NODE * pBtreeNode)
{
/*
      BTREE_NODE* p = pBtreeNode;
      printf("busy = %d idle = %d selfPos = %d nextPos = %d \n",p->nBusyKey,p->nIdleKey, p->nSelfPos,p->nNextPos);
      for ( size_t i = 0 ; i < p->nBusyKey ; i++ )
      {
            printf("%d:  key = %d  pos = %d \n" ,  i ,p->gPosAndKey[i].nKey , p->gPosAndKey[i].nPos );
      }
*/
}

void CBtree::__ShowHeader()
{
/*
      BTREE_HEADER *p = m_pHeader;
      printf("header: \n");
      printf("keyNum = %d nodeNum = %d height = %d rootPos = %d startLeafPos = %d\n" ,p->nKeyNum ,p->nNodeNum , p->nHeight , p->nRootPos ,p->nStartLeafPos);
*/
}

}
