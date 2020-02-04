#include "rows/csv.h"
#include "rows/error.h"
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
  // parse query - query must have the form: '[:[d]+]*'
  // example: '1:4:0' will pick columns: 1,4,0
  vector<size_t>ret;
  enum state_t{start=0,sep=1,digit=2,stop=3};
  state_t state=start;
  size_t qind=0,qlen=query.size();
  vector<char>currdigits;
  while(true){
    char c='\0';
    if(qind<qlen)c=query[qind++];
    switch(state){
      case start:
        if(c==':'){
          state=sep;
        }else
        if(c=='\0'){
          state=stop;
        }else{
          THROW_ROWS_RUNTIME("query string starting with invalid character: '"<<c<<"'");
        }
        break;
      case sep:
        if(c>='0'&&c<='9'){
          currdigits.push_back(c);
          state=digit;
        }else{
          THROW_ROWS_RUNTIME("in query string, found '"<<c<<"' when expected a digit 0-9");
        }
        break;
      case digit:
        if(c>='0'&&c<='9'){
          currdigits.push_back(c);
          state=digit;
        }else
        if(c==':'){
          ret.push_back(convert2ind(currdigits));
          currdigits.clear();
          state=sep;
        }else
        if(qind==qlen){
          ret.push_back(convert2ind(currdigits));
          currdigits.clear();
          state=stop;
        }else{
          THROW_ROWS_RUNTIME("in query string, found '"<<c<<"' when expected a digit 0-9 or a separator ':'");
        }
        break;
      case stop:
        break;
    }
    if(state==stop)break;
  }
  return ret;
}
// convert a sequence of chars to a number 
// (used by 'parseQuery')
size_t input_statement::convert2ind(vector<char>const&digits){
  size_t ret=0;
  for(size_t i=0;i<digits.size();++i){
    ret+=10*ret+digits[i]-'0';
  }
  return ret;
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
result_set::result_set(shared_ptr<input_statement>stmt):stmt_(stmt),currind_(-1),end_(false),maxcolind_(0),rownum_(-1){
  // get info about columns to load
  auto const&colinds=stmt_.lock()->cols();
  for(size_t colind:colinds)maxcolind_=max(maxcolind_,colind);
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
  auto const&colinds=stmt_.lock()->cols();
  for(size_t ind=0;ind<prefetchCount;++ind){
    // read line
    string line;
    if(!getline(is,line)){
      end_=true;
      break;
    }
    // track row number
    ++rownum_;

    // break up line into fields
    vector<string>cols;
    boost::split(cols,line,boost::is_any_of(string(1,sep)));
    if(maxcolind_>=cols.size())THROW_ROWS_RUNTIME("row number: "<<rownum_<<" (counting from 0) does not have eneough columns - #cols in row: "<<cols.size()<<", required #columns: "<<maxcolind_+1);
    row_t row;
    for(auto&&colind:colinds){
      string&col=cols[colind];
      if(col.size()==0)row.push_back(nullopt);
      else row.emplace_back(optional<string>(col));
    }
    rows_.emplace_back(std::move(row));
  }
  nrowsavailable=rows_.size();
  currind_=-1;
  if(nrowsavailable==0)end_=true;
  return nrowsavailable>0;
}
// check indexing into a column in rows_ array
void result_set::checkGet(std::size_t ind)const{
  if(currind_>rows_.size())THROW_ROWS_RUNTIME("<internal error>: invalid row index ("<<currind_<<") into result set when retrieving column at row "<<rownum_<<" and column index: "<<ind);
  if(ind>rows_[currind_].size())THROW_ROWS_RUNTIME("invalid column index into result row when retrieving column at row: "<<rownum_<<" and column index: "<<ind);
}
// get #of columns with current row
size_t result_set::ncols()const{
  return stmt_.lock()->cols().size();
}
// int - column getter
optional<int>result_set::getInt(std::size_t ind){
  checkGet(ind);
  auto const&v=rows_[currind_][ind];
  if(!v){
    return optional<int>{};
  }
  return optional<int>(boost::lexical_cast<int>(v.value()));
}
// string - column getter
optional<string>result_set::getString(std::size_t ind){
  checkGet(ind);
  return rows_[currind_][ind];
}
}
