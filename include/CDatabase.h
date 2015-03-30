#ifndef CDATABASE_H
#define CDATABASE_H

namespace wellDB
{

class CDatabase
{
      public:
            CDatabase();
            virtual ~CDatabase();

      protected:
      private:

      CDatabase(const CDatabase& other);
      CDatabase& operator=(const CDatabase& other);
};


}

#endif // CDATABASE_H
