#include <rows/csv.h>
#include <rows/tuple_utils.h>
#include <rows/tuple_utils.h>
#include <boost/iterator/iterator_facade.hpp>
#include <memory>
#include <stdexcept>
#include <iostream>

using namespace std;
using namespace rows;

struct Foo{
  template<typename T>
  void operator()(size_t ind,T&&t){
    cout<<"["<<ind<<", "<<std::forward<T>(t)<<"]";
  }
};
template<typename T>
struct Bar{
  template<size_t Ind>
  void apply(size_t size){
    cout<<"["<<Ind<<", "<<size<<"]";
  }
};
// main test program
int main(){
  // test2
  auto db=make_shared<csv::stream_db>(cin,',');
  auto stmt=db->createInputStatement(":3:2:1",1);
  auto rs=stmt->execute();
  int cnt=0;
  size_t ncols=rs->ncols();
  while(rs->next()){
    cout<<"row: "<<cnt<<": [";
    for(size_t i=0;i<ncols;++i){
      auto const&v=rs->get<int>(i);
      if(v)cout<<rs->get<int>(i).value();
      else cout<<"null";
      if(i<ncols-1)cout<<", ";
    }
    cout<<"]"<<endl;
  }
}
