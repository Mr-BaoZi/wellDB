/*
wellDB
wellDB is a simple storage engine in the linux OS.
Well dosen't means good or great.
Instead, well is a hole in the ground ,there is water at the bottom.
Its pronunciation is "jing" in chinese.
The chinese character of well is 井，looks like #.
*/
#include "CBloomFilter.h"
#include "CBtree.h"
#include "CFile.h"
int main()
{
      //wellDB::CBloomFilter::STest();
      /*char cPath[] = "index.br";
      wellDB::CBtree iBtree(cPath);
      iBtree.Show();*/
      char cPath[] = "fuck";
      wellDB::CFileBase *f = new wellDB::CCppFile();
      f->Open(cPath);

     /* wellDB::PTR_AND_KEY PtrAndKey1;
      wellDB::PTR_AND_KEY PtrAndKey2;
      PtrAndKey1.nKey = 230;
      PtrAndKey1.nPtr = 33;
      PtrAndKey2 = 231;

      printf("%d\n",  PtrAndKey1 <  22 );
      printf("%d\n", wellDB::nSIZEOF_PTR_AND_KEY   );*/
}
