#include "CDatabase.h"
namespace wellDB
{

CDatabase::CDatabase()
{
      //ctor
}

CDatabase::~CDatabase()
{
      //dtor
}

CDatabase::CDatabase(const CDatabase& other)
{
      //copy ctor
}

CDatabase& CDatabase::operator=(const CDatabase& rhs)
{
      if (this == &rhs) return *this; // handle self assignment
      //assignment operator
      return *this;
}

}
