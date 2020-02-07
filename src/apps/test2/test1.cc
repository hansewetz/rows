#include <rows/csv.h>
#include <rows/data_utils.h>
#include <rows/tuple_utils.h>
#include <iostream>

using namespace std;
using namespace rows;

// main test program
int main(){
  // row type for this test
  using row_t=tuple<optional<string>,optional<int>,optional<string>>;

  // create database. statment and resultset
  auto db=make_shared<csv::stream_db>(cin,',');
  auto stmt=db->createInputStatement(":1:0:1",1);
  auto rs=stmt->execute();

  // create a data fetcher to retrieve rows with
  data_fetcher<csv::result_set>df(rs);
  row_t row;
  while(df.next()){
    cout<<df.fetch(row)<<endl;
  }
}
