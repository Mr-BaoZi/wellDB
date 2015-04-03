#include "CBloomFilter.h"
#include "CBtree.h"
#include "CFile.h"

int main()
{
      char cPath[] = "index.br";
      wellDB::CBtree iBtree;
      if ( iBtree.Init(cPath, new wellDB::CFdFile(), 328))
      {
            iBtree.Show();
            //iBtree.Traversal();
            printf("ok!\n");
      }


}
