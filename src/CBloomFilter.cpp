#include "CBloomFilter.h"
namespace wellDB
{

CBloomFilter::CBloomFilter( size_t nBitNum   )
{
      //ctor
      if( nBitNum <= 0)
            nBitNum = 1;
      size_t nArrNum =( ( nBitNum + nBIT_NUM_PER -1 )/(nBIT_NUM_PER) ) ;
      __MallocBitSet(nBitNum ,nArrNum , NULL);
}

CBloomFilter::~CBloomFilter()
{
      //dtor
       if(m_pBitSet)
            free ( m_pBitSet);
}

CBloomFilter::CBloomFilter(const CBloomFilter& other)
{
      //copy ctor
      BIT_SET * pBitSet = other.m_pBitSet;
      size_t nArrNum = pBitSet->nArrNum;
      size_t nBitNum = pBitSet->nBitNum;
      __MallocBitSet( nBitNum, nArrNum,pBitSet);
}

CBloomFilter& CBloomFilter::operator=(const CBloomFilter& rhs)
{
      if (this == &rhs) return *this; // handle self assignment
      //assignment operator
      BIT_SET * pBitSet = rhs.m_pBitSet;
      size_t nArrNum = pBitSet->nArrNum;
      size_t nBitNum = pBitSet->nBitNum;

      if( m_pBitSet ->nArrNum != nArrNum )
      {
            free(m_pBitSet);
            __MallocBitSet(nBitNum,nArrNum,pBitSet);
      }
      else
            __CopyBitSet(pBitSet);
      return *this;
}


void CBloomFilter::SetBitSeries( size_t gnPos[] ,size_t nNum )
{
      if( nNum <= 0 ) return ;
      for( size_t i = 0 ; i < nNum ; i++)
            SetBit(gnPos[i]);
}

bool CBloomFilter::GetBitSeries( size_t gnPos[] ,size_t nNum )
{
      if(nNum <= 0 ) return false;
      for(size_t i = 0 ; i < nNum ; i++ )
      {
            if( GetBit(gnPos[i]) == false )
                  return false;
      }
      return true;
}

void CBloomFilter::STest()
{
      CBloomFilter iBF1(65);
      printf("after iBF1(65)\n");
      iBF1.SetBit(44);
      iBF1.SetBit(33);
      iBF1.ShowBitSet();
      CBloomFilter iBF2(iBF1);
      printf("after iBF2(iBF1)\n");
      iBF2.ShowBitSet();
      CBloomFilter iBF3(33);
      printf("after iBF3(33)\n");
      iBF3.ShowBitSet();
      iBF2 = iBF2;
      printf("after iBF2 = iBF2\n");
      iBF2.ShowBitSet();
      iBF1 = iBF3;
      printf("after iBF1 = iBF3\n");
      iBF1.ShowBitSet();
      iBF1 = iBF2;
      printf("after iBF1 = iBF2\n");
      iBF1.ShowBitSet();
}

void CBloomFilter::__MallocBitSet(  size_t nBitNum ,size_t nArrNum ,const BIT_SET *pBitSet  )
{
      if( nBitNum <= 0 || nArrNum <=0 ) return;
      size_t nMallocByte = nArrNum * nSIZEOF_BIT_TYPE + nSIZEOF_BIT_SET;
      m_pBitSet = (BIT_SET*)malloc(nMallocByte);
      memset(m_pBitSet , 0 , nMallocByte);
      m_pBitSet->nArrNum = nArrNum;
      m_pBitSet->nBitNum = nBitNum;
      if( pBitSet )
            __CopyBitSet(pBitSet);
}

void CBloomFilter::__CopyBitSet( const BIT_SET *pBitSet)
{
      if( !pBitSet  || !m_pBitSet   )   return ;
      if( pBitSet->nArrNum > m_pBitSet->nArrNum ) return ;
      memcpy( m_pBitSet , pBitSet , SizeofBitSet( GetArrNum() ) );
}

void CBloomFilter::ShowBitSet()
{
      printf("bitNum = %d arrNum = %d\n",m_pBitSet->nBitNum,m_pBitSet->nArrNum);
      for(  size_t i = 0 ; i < GetArrNum() ; i++ )
            printf("%x\n", m_pBitSet->arrBit[i]);
}

}
