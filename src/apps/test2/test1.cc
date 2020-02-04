#include <rows/csv.h>
#include <rows/data_utils.h>
#include <rows/tuple_utils.h>
#include <iostream>

using namespace std;
using namespace rows;

// main test program
int main(){
  // create database. statment and resultset
  auto db=make_shared<csv::stream_db>(cin,',');
  auto stmt=db->createInputStatement(":0:1",1);
  auto rs=stmt->execute();

  // create a data fetcher to retrieve rows with
  data_fetcher<csv::result_set>df(rs);
  tuple<optional<string>,optional<int>>row;
  while(df.next()){
    df.fetch(row);
    cout<<row<<endl;
  }
}
