//g++ main.cpp -o se_proc
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

class config{
public:
  config( const char * c ) : name( c ) {}
  std::string name;
  std::map< int, result * > res;

  void printName( std::ostream& os, int w=40 ) {
    if( name.length()>w ){
      os << std::setw(w) << name.substr( 0, w-2 );
    }else{
      os << std::setw(w) << name;
    }
  }
};

class result{
private:
  int rst;
  bool comp_rtm;
  double rtm;
public:
  result(){ rst = -1;comp_rtm = false; }
  config * c;
  std::vector< std::string > res;
  void print(){
    for( unsigned i=0; i<res.size(); i++ ){
      std::cout << res[i] << " ";
    }
  }
  int status() {
    if( rst==-1 ){
      if( strcmp( res[0].c_str(), "unsat" )==0 ){
        rst = 1;
      }else if( strcmp( res[0].c_str(), "sat" )==0 ){
        rst = 2;
      }else if( strcmp( res[0].c_str(), "t/o" )==0 ){
        rst = 3;
      }else{
        rst = 0;
      }
    }
    return rst;
  }
  double getTime() {
    if( !comp_rtm ){
      rtm = atof( res[1].c_str() );
      comp_rtm = true;
    }
    return rtm;
  }
  bool compare( result * r, double thresh ) {
    double q = rtm-r->rtm;
    return rst==r->rst && -thresh<q && q<thresh;
  }
};

class benchmark
{
public:
  benchmark( const char * c ) : name( c ) {}
  std::string name;
  std::vector< result * > res;
  bool compare( benchmark * b, double thresh ){
    if( res.size()==b->res.size() ){
      for( unsigned i=0; i<b->res.size(); i++ ){
        if( !b->res[i]->compare( res[i], thresh ) ){
          return false;
        }
      }
      return true;
    }else{
      return false;
    }
  }
};

int main( int argc, char* argv[] )
{
  std::vector< config * > all_configs;
  std::map< int, config * > configs;
  std::map< int, benchmark * > benchs;
  int nbench = 0;
  int nconfig = 0;
  
  //std::map< std::string, std::map< std::string, std::string > > table;

  std::fstream fs(argv[1], std::ios::in);
  //std::cout << "Open file : " << argv[1] << std::endl;
  
  std::string hd;
  getline( fs, hd );
  /*
  std::string l;
  std::vector< std::string > lines;
  while( !fs.eof() ){
    getline( fs, l );
    lines.push_back(l);
  }
  
  for( unsigned i=0; i<lines.size(); i++ )
  {
    l = lines[i];
    size_t pos = 0;
    size_t pos_prev = 0;
    pos = l.find(",", pos);
    std::string slv = l.substr( pos_prev, pos-pos_prev );
    pos_prev = pos;
    pos = l.find(",", pos);
    std::string bench = l.substr( pos_prev, pos-pos_prev );
    std::string rest = l.substr(pos,l.size()-pos);
    
    std::cout << "Process line : " << l << std::endl;
    std::cout << "  slv:" << slv << std::endl;
    std::cout << "  bench:" << bench << std::endl;
    std::cout << "  rest:" << rest << std::endl;
  }
  */
  
  size_t pos = 0;
  size_t pos_prev = 0;
  std::vector< std::string > hd_vec;
  while((pos = hd.find(",", pos)) != std::string::npos){
    hd_vec.push_back( hd.substr( pos_prev, pos-pos_prev ) );
    pos++;
    pos_prev = pos;
  }
  hd_vec.push_back( hd.substr( pos_prev, (hd.length()-1)-pos_prev ) );
  int bench_col = -1;
  int bench_id_col = -1;
  int solver_col = -1;
  int config_col = -1;
  int config_id_col = -1;
  int status_col = -1;
  int wtime_col = -1;
  int result_col = -1;


  std::cout << "Columns : ";
  for( unsigned i=0; i<hd_vec.size(); i++ ){
    std::cout << "(" << hd_vec[i] << ") & ";
    if( strcmp( hd_vec[i].c_str(), "benchmark" )==0 ){
      bench_col = i;
    }else if( strcmp( hd_vec[i].c_str(), "benchmark id" )==0 ){
      bench_id_col = i;
    }else if( strcmp( hd_vec[i].c_str(), "solver" )==0 ){
      solver_col = i;
    }else if( strcmp( hd_vec[i].c_str(), "configuration" )==0 ){
      config_col = i;
    }else if( strcmp( hd_vec[i].c_str(), "configuration id" )==0 ){
      config_id_col = i;
    }else if( strcmp( hd_vec[i].c_str(), "status" )==0 ){
      status_col = i;
    }else if( strcmp( hd_vec[i].c_str(), "wallclock time" )==0 ){
      wtime_col = i;
    }else if( strcmp( hd_vec[i].c_str(), "result" )==0 ){
      result_col = i;
    }
  }
  std::cout << std::endl;
  std::cout << "...finished read columns." << std::endl;

  if( bench_col==-1 ){
    std::cout << "Cannot find benchmark column." << std::endl;
  }else if( bench_id_col==-1 ){
    std::cout << "Cannot find benchmark id column." << std::endl;
  }else if( solver_col==-1 ){
    std::cout << "Cannot find solver column." << std::endl;
  }else if( config_col==-1 ){
    std::cout << "Cannot find configuration column." << std::endl;
  }else if( config_id_col==-1 ){
    std::cout << "Cannot find configuration id column." << std::endl;
  }else if( status_col==-1 ){
    std::cout << "Cannot find status column." << std::endl;
  }else if( wtime_col==-1 ){
    std::cout << "Cannot find wallclock time column." << std::endl;
  }else if( result_col==-1 ){
    std::cout << "Cannot find result column." << std::endl;
  }
  std::cout << "Reading file..." << std::endl;

  int count = 0;
  while( !fs.eof() ){
    std::vector< std::string > sline;
    for( unsigned i=0; i<hd_vec.size(); i++ ){
      std::string str;
      if( (i+1)==hd_vec.size() ){
        getline( fs, str );
        str = str.substr( 0, str.length()-1 );
      }else{
        getline( fs, str, ',' );
      }
      sline.push_back( str );
    }
    //std::cout << "[";
    //for( unsigned i=0; i<sline.size(); i++ ){
    //  std::cout << "[" << sline[i] << "] ";
    //}
    //std::cout << "]" << std::endl;

    //get the benchmark
    if( !fs.eof() ){
      int bid = atoi( sline[bench_id_col].c_str() );
      if( benchs.find( bid )==benchs.end() ){
        benchs[bid] = new benchmark(sline[bench_col].c_str());
        nbench++;
        //std::cout << "Benchmark " << bid << " :: " << benchs[bid]->name << std::endl;
      }
      //get the config
      int cid = atoi( sline[config_id_col].c_str() );
      if( configs.find( cid )==configs.end() ){
        std::stringstream ss;
        ss << sline[solver_col] << "_" << sline[config_col];
        configs[cid] = new config(ss.str().c_str());
        all_configs.push_back( configs[cid] );
        nconfig++;
        //std::cout << "Configuration " << cid << " :: " << ss.str() << std::endl;
      }
      result * res = new result;
      if( std::strcmp(sline[status_col].c_str(),"complete")==0 ){
        res->res.push_back( sline[result_col] );
      }else{
        res->res.push_back( sline[status_col] );
      }
      if( std::strcmp(res->res[0].c_str(),"timeout (cpu)")==0 ){
        res->res[0] = std::string("t/o");
      }else if( std::strcmp(res->res[0].c_str(),"timeout (wallclock)")==0 ){
        res->res[0] = std::string("t/o");
      }else if( std::strcmp(res->res[0].c_str(),"starexec-unknown")==0 ){
        res->res[0] = std::string("--");
      }else if( std::strcmp(res->res[0].c_str(),"Unsatisfiable")==0 || std::strcmp(res->res[0].c_str(),"Theorem")==0  || std::strcmp(res->res[0].c_str(),"correct")==0 ){
        res->res[0] = std::string("unsat");
      }else if( std::strcmp(res->res[0].c_str(),"Satisfiable")==0 || std::strcmp(res->res[0].c_str(),"CounterSatisfiable")==0 ){
        res->res[0] = std::string("sat");
      }
      res->res.push_back( sline[wtime_col] );
      res->c = configs[cid];
      configs[cid]->res[bid] = res;
    }
    count++;
  }
  fs.close();

  std::cout << "Print output header..." << std::endl;

  bool useSample = false;   // TODO
  std::stringstream sso;
  sso << argv[1] << ".out.csv";
  std::fstream fso(sso.str().c_str(), std::ios::out);
  std::stringstream ssos;
  ssos << argv[1] << ".out.sample.csv";
  std::fstream fsos(ssos.str().c_str(), std::ios::out);
  fso << "benchmark";
  fsos << "benchmark,num";
  for( std::map< int, config * >::iterator it = configs.begin(); it != configs.end(); ++it ){
    fso << "," << it->second->name << ",time";
    fsos << "," << it->second->name << ",time";
    for( std::map< int, result * >::iterator it2 = it->second->res.begin(); it2 != it->second->res.end(); ++it2 ){
      if( benchs.find( it2->first )==benchs.end() ){
        std::cout << "Cannot find benchmark " << it2->first << " ";
        it2->second->print();
        std::cout << std::endl;
        exit(1);
      }
      int status = it2->second->status();
      double t = it2->second->getTime();
      benchs[it2->first]->res.push_back( it2->second );
    }
  }
  fso << ",conflict" << std::endl;
  fsos << ",conflict" << std::endl;

  std::cout << "Compute sample..." << std::endl;
  std::stringstream ssco;
  ssco << "sample.txt";
  std::fstream fsp(ssco.str().c_str(), std::ios::out);
  //benchmark sampling
  std::map< int, bool > proc;
  std::map< benchmark *, int > bcounter;
  unsigned nunique = 0;
  unsigned ntotal = 0;
  if (useSample)
  {
    for( std::map< int, benchmark * >::iterator it = benchs.begin(); it != benchs.end(); ++it ){
      ntotal++;
      if( proc.find( it->first )==proc.end() ){
        int bbc = 1;
        nunique++;
        fsp << it->second->name << std::endl;
        std::map< int, benchmark * >::iterator it2 = it;
        it2++;
        for( ; it2 != benchs.end(); ++it2 ){
          if( proc.find( it2->first )==proc.end() ){
            if( it->second->compare( it2->second, .1 ) ){
              proc[it2->first] = true;
              bbc++;
            }
          }
        }
        bcounter[it->second] = bbc;
      }
    }
  }
  fsp.close();
  std::cout << nunique << " / " << ntotal << " unique benchmarks for sampling." << std::endl;

  std::cout << "Print output file..." << std::endl;
  //statistics
  std::map< config *, std::map< int, int > > stats;
  std::map< config *, std::map< int, double > > stats_d;

  std::map< int, std::map< config *, std::map< config *, std::vector< benchmark * > > > > solved_by_unique;

  for( std::map< int, benchmark * >::iterator it = benchs.begin(); it != benchs.end(); ++it ){
    int in_sample = 0;
    if (useSample)
    {
      in_sample = bcounter.find( it->second )!=bcounter.end() ? bcounter[it->second] : -1;
      if( in_sample>0 ){
        fsos << it->second->name << "," << in_sample;
      }
    }
    fso << it->second->name;
    if( it->second->res.size()!=nconfig ){
      std::cout << "Benchmark " << it->second->name << " has ";
      std::cout << it->second->res.size() << " results, nconfig=" << nconfig << std::endl;
      exit(1);
    }
    std::map< int, bool > bst;
    std::vector< config * > slv_c[2];
    std::map< config *, int > config_status;
    for( unsigned j=0; j<it->second->res.size(); j++ ){
      for( unsigned k=0; k<it->second->res[j]->res.size(); k++ ){
        fso << "," << it->second->res[j]->res[k];
        if( in_sample>0 ){
          fsos << "," << it->second->res[j]->res[k];
        }
      }
      config * cc = it->second->res[j]->c;
      int status = it->second->res[j]->status();
      double t = it->second->res[j]->getTime();
      config_status[cc] = status;
      //std::cout << "Status/time " << cc->name << " : " << status << " " << t << std::endl;
      if( stats[cc].find( status )==stats[cc].end() ){
        stats[cc][status] = 0;
        stats_d[cc][status] = 0.0;
      }
      stats[cc][status]++;
      stats_d[cc][status] += t;
      bst[status] = true;

      int solved = status==1 || status==2 ? 1 : 0;
      slv_c[solved].push_back( cc );
      for( unsigned k=0; k<slv_c[1-solved].size(); k++ ){
        if( solved==1 ){
          solved_by_unique[status][cc][slv_c[1-solved][k]].push_back( it->second );
        }else{
          int sstatus = config_status[slv_c[1-solved][k]];
          solved_by_unique[sstatus][slv_c[1-solved][k]][cc].push_back( it->second );
        }
      }
    }
    int conf = 0;
    if( bst.find( 1 )!=bst.end() && bst.find( 2 )!=bst.end() ){
      std::cout << "WARNING: Inconsistency found for benchmark " << it->second->name << std::endl;
      conf = 1;
    }
    fso << "," << conf << std::endl;
    if( in_sample>0 ){
      fsos << "," << conf << std::endl;
    }
  }
  fso.close();
  fsos.close();

  std::stringstream sss;
  sss << argv[1] << ".summary.csv";
  std::fstream fsss(sss.str().c_str(), std::ios::out);
  std::cout << std::setw(40) << "Configuration";
  std::cout << std::setw(8) << "#unsat";
  std::cout << std::setw(10) << "time";
  std::cout << std::setw(8) << "#sat";
  std::cout << std::setw(10) << "time";
  std::cout << std::setw(8) << "#solved";
  std::cout << std::setw(10) << "#total";
  std::cout << std::endl;
  for( std::map< int, config * >::iterator it = configs.begin(); it != configs.end(); ++it ){
    it->second->printName( std::cout );
    std::cout << std::setw(8) << stats[it->second][1];
    std::cout << std::setw(10) << stats_d[it->second][1];
    std::cout << std::setw(8) << stats[it->second][2];
    std::cout << std::setw(10) << stats_d[it->second][2];
    std::cout << std::setw(10) << (stats[it->second][1]+stats[it->second][2]);
    std::cout << std::setw(10) << nbench;
    std::cout << std::endl;
  }
  std::cout << std::endl;

  fsss << std::setw(40) << "Configuration";
  fsss << "," << "#unsat";
  fsss << "," << "time";
  fsss << "," << "#sat";
  fsss << "," << "time";
  fsss << "," << "#solved";
  fsss << "," << "#total";
  fsss << std::endl;
  for( std::map< int, config * >::iterator it = configs.begin(); it != configs.end(); ++it ){
    it->second->printName( fsss );
    fsss << "," << stats[it->second][1];
    fsss << "," << stats_d[it->second][1];
    fsss << "," << stats[it->second][2];
    fsss << "," << stats_d[it->second][2];
    fsss << "," << (stats[it->second][1]+stats[it->second][2]);
    fsss << "," << nbench;
    fsss << std::endl;
  }
  fsss << std::endl;


  std::stringstream chart[2];

  //output detailed comparisions
  for( unsigned i=0; i<all_configs.size(); i++ ){
    config * cc = all_configs[i];

    //std::stringstream ssco;
    //ssco << "/space/ajreynol/bin/se_proc_tmp/" << cc->name << ".txt";
    //std::fstream fsco(ssco.str().c_str(), std::ios::out);
    std::stringstream fsco;
    fsco << "Comparisons for configuration " << cc->name << " on " << argv[1] << std::endl << std::endl;

    for( unsigned s=0; s<2; s++ ){
      int status = s==0 ? 1 : 2;
      std::string sts( s==0 ? "unsatisfiable" : "satisfiable" );
      fsco << "For " << sts << " : " << std::endl;
      cc->printName( chart[s] );
      for( unsigned j=0; j<all_configs.size(); j++ ){
        chart[s] << std::setw(8);
        if( i==j ){
          chart[s] << "X";
        }else{
          for( unsigned r=0; r<2; r++ ){
            config * c1 = r==0 ? all_configs[j] : cc;
            config * c2 = r==0 ? cc : all_configs[j];
            if( !solved_by_unique[status][c1][c2].empty() ){
              if( r==0 ){
                fsco << "Unsolved " << sts << ", but by " << c1->name << " (" << solved_by_unique[status][c1][c2].size() << ") : " << std::endl;
              }else{
                fsco << "Solved " << sts << ", but not by " << c2->name << " (" << solved_by_unique[status][c1][c2].size() << ") : " << std::endl;
              }
              for( unsigned k=0; k<solved_by_unique[status][c1][c2].size(); k++ ){
                fsco << "  " << solved_by_unique[status][c1][c2][k]->name << std::endl;
              }
              fsco << std::endl;
            }
            if( r==1 ){
              chart[s] << solved_by_unique[status][c1][c2].size();
            }
          }
        }
        fsco << std::endl;
      }
      chart[s] << std::endl;
    }
    //fsco.close();
    std::cout << std::endl;
  }
  for( unsigned s=0; s<2; s++ ){
    std::string sts( s==0 ? "unsatisfiable" : "satisfiable" );
    std::cout << "For " << sts << " : " << std::endl;
    std::cout << chart[s].str() << std::endl;
  }

  for( unsigned s=0; s<2; s++ ){
    std::string sts( s==0 ? "unsatisfiable" : "satisfiable" );
    fsss << sts << std::endl;
    int status = s==0 ? 1 : 2;

    for( unsigned i=0; i<all_configs.size(); i++ ){
      config * ci = all_configs[i];
      ci->printName( fsss );
      for( unsigned j=0; j<all_configs.size(); j++ ){
        config * cj = all_configs[j];
        if( i==j ){
          fsss << ",X";
        }else{
          fsss << "," << solved_by_unique[status][ci][cj].size();
        }
      }
      fsss << std::endl;
    }
  }


}
