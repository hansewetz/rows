#pragma once
#include "tuple_utils.h"
#include <memory>
#include <tuple>
#include <type_traits>
#include <stdexcept>
#include <iostream>

namespace rows{
// ---- data fetcher
// (fetch data from a result set)


template<typename T>struct data_fetcher_aux;

// size_t
template<>struct data_fetcher_aux<std::optional<int>>{
  template<typename ResultSet>
  static void fetch(std::size_t ind,std::shared_ptr<ResultSet>rs,std::optional<int>&i){i=rs->getInt(ind);}
};
// string
template<>struct data_fetcher_aux<std::optional<std::string>>{
  template<typename ResultSet>
  static void fetch(std::size_t ind,std::shared_ptr<ResultSet>rs,std::optional<std::string>&i){i=rs->getString(ind);}
};
// NOTE! more specializations
// ...

// data fetcher class
// (retrieve data from result set)
// (based on types in a tuple the 'fetch' method will retrieve items of the corresponding tuple  element types)
// NOTE! add constrain in ResultSet
template<typename ResultSet>
class data_fetcher{
public:
  // ctors, dtor
  data_fetcher(std::shared_ptr<ResultSet>rs):rs_(rs){}

  // next row
  bool next(){return rs_->next();}

  // get data for current row
  template<typename row_t>
  void fetch(row_t&row){apply_with_index(*this,row);}

  // get data at a specific index
  // (this is a function object)(
  template<typename T>
  void operator()(std::size_t ind,T&t)const{data_fetcher_aux<T>::fetch(ind,rs_,t);}
private:
  // result set dealing with underlying data
  std::shared_ptr<ResultSet>rs_;
};
}
