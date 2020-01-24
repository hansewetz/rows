#include <rows/csv.h>
#include <rows/tuple_utils.h>
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
/*
  // test1
  using row_t=tuple<int,string,size_t>;
  row_t row{1,"Hello"s,17ul};
  cout<<make_tuple(1,"Hello")<<endl;

  apply_with_index(Foo{},std::forward<row_t>(row));
  cout<<endl;
  apply_with_index(Foo{},std::make_tuple(1,"Hello"s,1.2));
  cout<<endl;

  using bind_t=tuple<size_t,size_t,size_t>;
  bind_t bind{7,8,9};
  apply_with_index_template(Bar<row_t>{},bind);
  cout<<endl;

  using utup_t=uniform_tuple_builder<3,int>::type;
  cout<<utup_t{5,6,7}<<endl;
*/
  // test2
  auto db=make_shared<csv::stream_db>(cin,',');
  auto stmt=db->createInputStatement(":1:1:1:1",1);
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
