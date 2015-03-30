#ifndef CBLOOMFILTER_H
#define CBLOOMFILTER_H

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
//index beginf from 0
namespace wellDB
{
//typedef  unsigned char  BIT_TYPE;
typedef  unsigned int BIT_TYPE;
//typedef long long unsigned int BIT_TYPE;

typedef
struct tag_BIT_SET
{
      size_t nBitNum;       //number of bit used
      size_t nArrNum;       //number of array index used
      BIT_TYPE arrBit[0];
}BIT_SET, *PBIT_SET;

const size_t nSIZEOF_BIT_SET = sizeof(BIT_SET);
const size_t nSIZEOF_BIT_TYPE = sizeof(BIT_TYPE);
const size_t nBIT_NUM_PER = nSIZEOF_BIT_TYPE * 8;

class CBloomFilter
{
      public:
            CBloomFilter( size_t nBitNum  );
            virtual ~CBloomFilter();
            CBloomFilter(const CBloomFilter& other);
            CBloomFilter& operator=(const CBloomFilter& other);
            size_t SizeofBitSet( size_t nArrNum )
            {      return  nSIZEOF_BIT_SET + nArrNum * nSIZEOF_BIT_TYPE;    }
            size_t GetArrNum()
            {     return m_pBitSet->nArrNum ;    }
            size_t GetBitNum()
            {     return m_pBitSet->nArrNum  ;   }
            void SetBit( size_t nPos)
            {
                  nPos %=  m_pBitSet->nBitNum;
                  m_pBitSet->arrBit[nPos/nBIT_NUM_PER] |= ( 1<< (nPos% nBIT_NUM_PER));
            }
            bool GetBit( size_t nPos )
            {
                  nPos %=  m_pBitSet->nBitNum;
                  return m_pBitSet->arrBit[nPos/nBIT_NUM_PER]  & ( 1<< (nPos% nBIT_NUM_PER));
            }
            void SetBitSeries( size_t gnPos[] ,size_t nNum );
            bool GetBitSeries( size_t gnPos[] ,size_t nNum );
            void ShowBitSet();
            static void STest();
      private:
            BIT_SET* m_pBitSet;
            void __MallocBitSet( size_t nBitNum ,size_t nArrNum ,const BIT_SET *pBitSet   );
            void __CopyBitSet( const BIT_SET *pBitSet );


};


}
#endif // CBLOOMFILTER_H
