#pragma once
#include <ostream>
#include <tuple>
#include <utility>
#include <cxxabi.h>
#include <type_traits>
namespace rows{
//-----------------------------------------
// meta function to check if a type is a tuple
template<class>struct is_tuple_aux:std::false_type{}; 
template<class...T>struct is_tuple_aux<std::tuple<T...>>:std::true_type{};
template<typename T>struct is_tuple:is_tuple_aux<std::decay_t<T>>{};

// tuple as a concept
template<typename T>
concept Tuple=is_tuple<T>::value;

//-----------------------------------------
// print a variable together with its type 
template<typename T>
void print_type_value(std::ostream&os,T&&t){
  int status;
  os<<"("<<abi::__cxa_demangle(typeid(T).name(),0,0,&status)<<": "<<std::forward<T>(t)<<")";
}
template<Tuple Tu,std::size_t...Inds>
void print_tuple(std::ostream&os,Tu&&tu,std::index_sequence<Inds...>){
  (print_type_value(os,get<Inds>(std::forward<Tu>(tu))),...);
}
template<Tuple T>
std::ostream&operator<<(std::ostream&os,T&&t){
  os<<"[";
  print_tuple(os,std::forward<T>(t),std::make_index_sequence<std::tuple_size_v<std::decay_t<T>>>{});
  return os<<"]";
}
//-----------------------------------------
// apply a function object to each element in a tuple, passing index and tuple element as parameters
template<typename Func,typename...Args> 
void apply_with_index(Func f,std::tuple<Args...>const&tu){
  apply_with_index_aux(tu,f,std::make_index_sequence<sizeof...(Args)>{});
}
template<Tuple T,typename F,std::size_t...Inds>
void apply_with_index_aux(T&&tu,F f,std::index_sequence<Inds...>){
  (f(Inds,get<Inds>(std::forward<T>(tu))),...);
}
template<typename Func,Tuple T>
void apply_with_index(Func f,T&&tu){
  apply_with_index_aux(std::forward<T>(tu),f,std::make_index_sequence<std::tuple_size_v<std::decay_t<T>>>{});
}
//-----------------------------------------
// apply function with signature void apply<int>(T t) to each tuple element
template<Tuple T,typename F,std::size_t...Inds>
void apply_with_index_template_aux(T&&tu,F f,std::index_sequence<Inds...>){
    (f.template apply<Inds>(std::get<Inds>(std::forward<T>(tu))),...);
}
template<typename Func,Tuple T>
void apply_with_index_template(Func f,T&&tu){
  apply_with_index_template_aux(std::forward<T>(tu),f,std::make_index_sequence<std::tuple_size_v<std::decay_t<T>>>{});
}
//-----------------------------------------
// make a tuple type having all N elements of same type
template<std::size_t N,typename T>
struct uniform_tuple_builder{
  using type=decltype(std::tuple_cat(std::declval<std::tuple<T>>(),std::declval<typename uniform_tuple_builder<N-1,T>::type>()));
};
template<typename T>
struct uniform_tuple_builder<1,T>{
  using type=std::tuple<T>;
};
template<typename T>
struct uniform_tuple_builder<0,T>{
  using type=std::tuple<>;
};
}
