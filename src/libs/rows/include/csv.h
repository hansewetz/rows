#pragma once
#include <iosfwd>
#include <string>
#include <memory>
#include <vector>
#include <optional>
namespace rows::csv{

// forward decl
class input_statement;
class result_set;

// --- stream based csv database
// (the database is stream based which means that an entry cannot be read more than once)
class stream_db:public std::enable_shared_from_this<stream_db>{
public:
  // ctors, dtor
  stream_db(std::istream&is,char sep=',');

  // getters
  std::istream&is()const noexcept;
  char sep()const noexcept;

  // create statement
  std::shared_ptr<input_statement>createInputStatement(std::string const&query,std::size_t prefetchCount);
private:
  std::istream&is_;
  char sep_;
};

// --- input statement class
class input_statement:public std::enable_shared_from_this<input_statement>{
public:
  // ctors,dtor
  input_statement(std::shared_ptr<stream_db>db,std::string query,std::size_t prefetchCount);

  // execute query and return result set
  std::shared_ptr<result_set>execute();

  // getters
  std::shared_ptr<stream_db>db()const;
  std::string const&query()const noexcept;
  std::vector<std::size_t>cols()const noexcept;
  std::size_t prefetchCount()const noexcept;
  std::istream&is()const;
  char sep()const;
private:
  // helpers
  static std::vector<std::size_t>parseQuery(std::string const&query);
  static std::size_t convert2ind(std::vector<char>const&digits);

  // state
  std::weak_ptr<stream_db>db_;
  std::string query_;
  std::vector<std::size_t>cols_;
  std::size_t prefetchCount_;
};
// --- result_set

// column getters from result set
template<typename T>std::optional<T>getAux(std::size_t ind,result_set&rs);

// class
class result_set{
public:
  // ctor,dtor
  // (after construction, 'next()' must be called to get to first entry in result set)
  result_set(std::shared_ptr<input_statement>stmt);

  // move to next row
  bool next();

  // get fields within current row
  template<typename T>std::optional<T>get(std::size_t ind){return getAux<T>(ind,*this);}

  // get #of columns with current row
  std::size_t ncols()const;

  // column getters
  std::optional<int>getInt(std::size_t ind);
  std::optional<std::string>getString(std::size_t ind);
private:
  using row_t=std::vector<std::optional<std::string>>;  // row of strings (columns converted to a type on request)

  // helpers
  bool prefetch();                                      // fetch rows - returns true if we have data, else false
  void checkGet(std::size_t ind)const;                  // check indexing into a column in rows_ array

  // state
  std::weak_ptr<input_statement>stmt_;                  // pointer to statement
  std::vector<row_t>rows_;                              // rows of strings
  std::size_t currind_;                                 // current index (-1 if no row available)
  bool end_;                                            // did we reached end of result set
  std::size_t maxcolind_;                               // max col index we will pick from a row
  std::size_t rownum_;                                  // current row number
};
// column getters implementation
template<>std::optional<int>getAux<int>(std::size_t ind,result_set&rs){return rs.getInt(ind);}
template<>std::optional<std::string>getAux<std::string>(std::size_t ind,result_set&rs){return rs.getString(ind);}
}
