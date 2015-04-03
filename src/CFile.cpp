#include "CFile.h"
namespace wellDB
{

CFileBase::CFileBase()
{     }
CFileBase::~CFileBase()
{     }
bool CFileBase::Open(const char* cFilePath)
{    return true;    }
off_t CFileBase::Seek( off_t nOffset , int nFromWhere )
{    return 0;  }
size_t CFileBase::Write( const void *pvBuf , size_t nCount )
{   return 0;   }
size_t CFileBase::Read(void *pvBuf, size_t nCount)
{   return 0;   }



CFdFile::CFdFile()
{
      //
}

CFdFile::~CFdFile()
{
      close(m_f);
}

bool CFdFile::Open(const char* cFilePath)
{
      m_f = open(cFilePath, O_RDWR |O_CREAT, 0666  );
      if ( m_f == -1 )
            return false;
      return true;
}

size_t CFdFile::Write( const void *pvBuf , size_t nCount )
{
      return write(m_f, pvBuf , nCount);
}

size_t CFdFile::Read( void *pvBuf, size_t nCount )
{
      return  read(m_f, pvBuf, nCount);
}

off_t CFdFile::Seek( off_t nOffset , int nFromWhere )
{
      return lseek(m_f, nOffset, nFromWhere);
}


CStdFile::CStdFile()
{

}

CStdFile::~CStdFile()
{
      fclose(m_f);
}

bool CStdFile::Open(const char* cFilePath)
{
      m_f = fopen (cFilePath ,"rb+" );
      if(m_f == NULL)
            m_f = fopen (cFilePath , "wb+" );
      if( m_f == NULL  ) return false;
      return true;
}

size_t CStdFile::Read(void *pvBuf, size_t nCount)
{
      return nCount*fread( pvBuf, nCount ,1, m_f);
}

size_t CStdFile::Write( const void *pvBuf , size_t nCount )
{
      return  nCount*fwrite(pvBuf, nCount , 1 ,m_f);
}

off_t CStdFile::Seek(off_t nOffset , int nFromWhere)
{
      fseek(m_f , nOffset, nFromWhere);
      return  ftell(m_f);
}

/*
CCppFile::CCppFile()
{

}

CCppFile::~CCppFile()
{
      m_f.close();
}

bool CCppFile::Open(const char* cFilePath)
{
      m_f.open(cFilePath,std::ios::out|std::ios::binary);
      m_f.close();
      m_f.open(cFilePath,std::ios::in|std::ios::out|std::ios::binary);
      if(!m_f) return false;
      return true;
}

size_t CCppFile::Read(void *pvBuf, size_t nCount)
{
     m_f.read((char *)pvBuf , nCount);
     size_t n = m_f.gcount();
     printf("read = %d\n" , n);
     return n;
}

size_t CCppFile::Write( const void *pvBuf , size_t nCount )
{
      m_f.write((char *)pvBuf,nCount);
      return   m_f.gcount();
}

off_t CCppFile::Seek(off_t nOffset , int nFromWhere)
{
      std::ios::seekdir nWherece = std::ios::beg;
      switch(nFromWhere)
      {
            case SEEK_SET:
                  nWherece = std::ios::beg;
                  break;
            case SEEK_CUR:
                  nWherece = std::ios::cur;
                  break;
            case SEEK_END:
                  nWherece = std::ios::end;
                  break;
      }
      m_f.seekp(nOffset ,nWherece);
      m_f.seekg(nOffset ,nWherece);
      return m_f.tellg();
}
*/


}
