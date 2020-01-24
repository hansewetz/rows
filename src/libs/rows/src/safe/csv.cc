#include "rows/csv.h"
#include <iostream>
#include "boost/algorithm/string.hpp"
#include "boost/lexical_cast.hpp"
using namespace std;
namespace rows::csv{

// ---- stream_db
// ctor
stream_db::stream_db(istream&is,char sep):is_(is),sep_(sep){
}
// getters
istream&stream_db::is()const noexcept{return is_;}
char stream_db::sep()const noexcept{return sep_;}

// create input statement
shared_ptr<input_statement>stream_db::createInputStatement(string const&query,size_t prefetchCount){
  auto ptr=shared_from_this();
  return make_shared<input_statement>(shared_from_this(),query,prefetchCount);
}

// ---- input_statement
// ctor
input_statement::input_statement(shared_ptr<stream_db>db,string query,size_t prefetchCount):
    db_(db),query_(query),cols_(parseQuery(query)),prefetchCount_(prefetchCount==0?1:prefetchCount){
}
// helper method parsing a query
vector<size_t>input_statement::parseQuery(string const&query){
  // NOTE! not yet done - must pick columns to keep
  return vector<size_t>{0,2};
}
// execute query and return result set
shared_ptr<result_set>input_statement::execute(){
  return make_shared<result_set>(shared_from_this());
}
// getters
shared_ptr<stream_db>input_statement::db()const{return db_.lock();}
string const&input_statement::query()const noexcept{return query_;}
vector<size_t>input_statement::cols()const noexcept{return cols_;}
size_t input_statement::prefetchCount()const noexcept{return prefetchCount_;}
istream&input_statement::is()const{return db_.lock()->is();}
char input_statement::sep()const{return db_.lock()->sep();}

// --- result_set
// ctor
// (after construction, 'next()' must be called to get to first entry in result set)
result_set::result_set(shared_ptr<input_statement>stmt):stmt_(stmt),currind_(-1),end_(false){
}
// move to next row in result set
bool result_set::next(){
  if(!prefetch())return false;    // fetch data (if needed)
  ++currind_;                     // bump to next index in rows_ array
  return !end_;                   // return true if we have data
}
// prefetch rows if needed
// ('currind_' is left at '-1' when new data is loaded and must be incremented)
bool result_set::prefetch(){
  // do not preload if we have unprocessed rows or we have reached end
  size_t nrowsavailable=currind_==-1ul?rows_.size():rows_.size()-currind_-1;
  if(nrowsavailable>0)return true;          // if we have data then no need to pre-load
  if(end_)return false;                     // if we reached end then no point trying to pre-load

  // clear out existing data
  rows_.clear();

  // preload rows
  size_t prefetchCount=stmt_.lock()->prefetchCount();
  istream&is=stmt_.lock()->is();
  char sep=stmt_.lock()->sep();
  for(size_t ind=0;ind<prefetchCount;++ind){
    // read line
    string line;
    if(!getline(is,line)){
      end_=true;
      break;
    }
    // break up line into fields
// NOTE! this should probably go into a utility file
    vector<string>cols;
    boost::split(cols,line,boost::is_any_of(string(1,sep)));
    row_t row;
    for(auto&&col:cols){
      if(col.size()==0)row.emplace_back(optional<string>{});
      else row.emplace_back(optional<string>(col));
    }
    rows_.emplace_back(std::move(row));
  }
  nrowsavailable=rows_.size();
  currind_=-1;
  if(nrowsavailable==0)end_=true;
  return nrowsavailable>0;
}
// column getters
optional<int>result_set::getInt(std::size_t ind){
  // NOTE! check that we actually have a row and that index is within bounds
  auto const&v=rows_[currind_][ind];
  if(!v)return optional<int>{};
  return optional<int>(boost::lexical_cast<int>(v.value()));
}
optional<string>result_set::getString(std::size_t ind){
  // NOTE! check that we actually have a row and that index is within bounds
  return rows_[currind_][ind];
}
}
