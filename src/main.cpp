#include "CBloomFilter.h"
#include "CBtree.h"
#include "CFile.h"
int main()
{
      //wellDB::CBloomFilter::STest();
      char cPath[] = "index.br";
      wellDB::CBtree iBtree;
      if ( iBtree.Init(cPath, new wellDB::CStdFile(), 168 ) )
      {
            printf("ok!\n");
      }


}
