#ifndef CFILE_H
#define CFILE_H
#include <fstream>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>

namespace wellDB
{

class CFileBase
{
      public:
            CFileBase();
            virtual ~CFileBase() ;
            virtual bool Open(const char * cFilePath) ;
            virtual size_t Write( const void *pvBuf , size_t nCount ) ;
            virtual size_t Read(void *pvBuf, size_t nCount) ;
            virtual off_t Seek(off_t nOffset , int nFromWhere) ;
};

class CFdFile: public CFileBase
{
      public :
            CFdFile();
            ~CFdFile();

            virtual bool Open(const char * cFilePath);
            virtual size_t Write( const void *pvBuf , size_t nCount );
            virtual size_t Read(void *pvBuf, size_t nCount);
            virtual off_t Seek(off_t nOffset , int nFromWhere);
      private:
            int m_f;
};

class CStdFile: public CFileBase
{
      public:
            CStdFile();
            ~CStdFile();
            virtual bool Open(const char * cFilePath);
            virtual size_t Write( const void *pvBuf , size_t nCount );
            virtual size_t Read(void *pvBuf, size_t nCount);
            virtual off_t Seek(off_t nOffset , int nFromWhere);
      private:
            FILE* m_f;
};

/*
class CCppFile: public CFileBase
{
      public:
            CCppFile();
            ~CCppFile();
            virtual bool Open(const char * cFilePath);
            virtual size_t Write( const void *pvBuf , size_t nCount );
            virtual size_t Read(void *pvBuf, size_t nCount);
            virtual off_t Seek(off_t nOffset , int nFromWhere);
      private:
            std::fstream m_f;
};
*/

}
#endif // CFILE_H
