#ifndef GINPUTFILE_H
#define GINPUTFILE_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <string>
#include <fstream>

using namespace std;

#define MAX_LINE_LENGTH 32768



//------------------------------------------------------------
/*!
 *  \class GInputFile
 *
 *  \brief
 */

class GInputFile {
  private:
	std::string fileName;
    FILE* file;

    char line[MAX_LINE_LENGTH];
    char token[MAX_LINE_LENGTH];
    int indiceToken;
    int strlenline;

  public:
	GInputFile():file(NULL){}

    GInputFile(const char* nom):file(NULL){ fileName = std::string(nom) ;  }
	GInputFile(const std::string& nom):file(NULL){ fileName = nom ;  }

	inline bool isOpen(){return file!=NULL;}

    ~GInputFile(){}

    const char* getFileName(){return fileName.c_str();}
    void setFileName(char* st){fileName=st;}

    char* readLine() ;
    char* readUncommentedLine() ;


    char* getNextToken() ;
    char* getNextToken(char) ;
    string getEndOfLine();
    int   getNextIntToken() ;
    float getNextFloatToken() ;
    char* seek(char *) ;

    void open() ;
    void open(char *) ;
    void close() ;
};

//------------------------------------------------------------
/*!
 *  \class GOutputFile
 *
 * file that is used to write data. It has to be opened
 * before the data are written and closed after that.
 *
 */
class GOutputFile {
  protected :
    std::string fileName ;
    std::ofstream file ;
	int opened ;

  public:
	GOutputFile():opened(0){}
    GOutputFile(const std::string& nom):fileName(nom), opened(0){}
	GOutputFile(char* nom):opened(0){fileName = std::string (nom) ;}
	//GOutputFile(const char* nom):opened(0){fileName = std::string (nom) ;}
    virtual ~GOutputFile(){}

	virtual inline void setFileName (char* nom) { fileName = std::string (nom) ; }
	virtual inline void setFileName (const string& nom) { fileName = std::string (nom) ; }

	void open() {
		file.open (fileName.c_str(), std::ios_base::app|std::ios_base::out) ;
		opened = true ;
	}

	void openFromStart() {
		file.open (fileName.c_str(), std::ios_base::out) ;
		opened = true ;
	}

	void close() {
		if (opened) {
			file.close() ;
			opened = false ;
		}
	}

	virtual GOutputFile& operator<< (std::string& x) {file << x ; return *this ; }
	virtual GOutputFile& operator<< (char*  x) {file << x ; return *this ; }
	virtual GOutputFile& operator<< (int  x) {file << x ; return *this ; }
	virtual GOutputFile& operator<< (float x) {file << x ; return *this ; }
	virtual GOutputFile& operator<< (const char*  x) {file << x ; return *this ; }

} ;

#endif
