//g++ main.cpp -o se_split_csv
#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include <vector>
#include <stdlib.h>
#include <sstream>
#include <cstring>
#include <iomanip>

class result;

int main( int argc, char* argv[] )
{

  std::fstream fs(argv[1], std::ios::in);
  //std::cout << "Open file : " << argv[1] << std::endl;

  std::cout << "Read header..." << std::endl;
  // header 
  std::string hd;
  getline( fs, hd );
  
  std::cout << "Read input..." << std::endl;
  // partition the input
  std::map< std::string, std::vector< std::string > > splits;
  std::string str;
  std::string sstr;
  size_t pos;
  size_t cpos;
  size_t cpos_prev;
  while( !fs.eof() ){
    getline( fs, str );
    //std::cout << "read: " << str << std::endl;
    pos = str.find("/",0);
    //std::cout << "pos: " << pos << std::endl;
    cpos = 0;
    cpos_prev = 0;
    while(cpos<pos && cpos != std::string::npos){
      cpos_prev = cpos+1;
      cpos = str.find(",", cpos_prev);
    }
    //std::cout << "cpos: " << cpos << std::endl;
    //std::cout << "cpos_prev: " << cpos_prev << std::endl;
    if( cpos != std::string::npos && pos != std::string::npos )
    {
      sstr = str.substr(cpos_prev,pos-cpos_prev);
      splits[sstr].push_back(str);
      //std::cout << "Add to " << sstr << ", size = " << splits[sstr].size() << std::endl;
    }
  }
  
  // write out to files
  std::string inputfile(argv[1]);
  for( std::map< std::string, std::vector< std::string > >::iterator its = splits.begin(); its != splits.end(); ++its )
  {
    std::stringstream ss;
    pos = inputfile.find(".",0);
    if( pos==std::string::npos )
    {
      ss << inputfile;
    }
    else
    {
      ss << inputfile.substr(0,pos);
    }
    ss << "_" << its->first << ".csv";
    std::cout << "Write " << ss.str() << "..." << std::endl;
    std::fstream fso(ss.str(), std::ios::out);
    fso << hd << std::endl;
    for( unsigned i=0; i<its->second.size(); i++ )
    {
      fso << its->second[i] << std::endl;
    }
    fso.close();
  }
}
