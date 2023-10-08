#ifndef records_hpp
#define records_hpp

#include <string>
#include <optional>
#include <vector>
#include <fstream>
using namespace std;

inline std::string trim(const std::string &s, const char *t = " \t\n\r\f\v")
{
  if (s.find_first_not_of(t) == std::string::npos)
    return "";
  return s.substr(s.find_first_not_of(t), s.find_last_not_of(t) - 1);
}

struct record_t
{
  string name;
  enum action_t
  {
    WIPE,
    SOURCE,
    DESTINATION,
    MERGE
  } action;
  optional<vector<record_t>> children;
};

record_t from_yaml(ifstream &yaml_file)
{
  record_t root_record;

  vector<reference_wrapper<record_t>> records_stack;
  vector<int> idents_stack;
  records_stack.push_back(root_record);
  idents_stack.push_back(0);

  string line;
  while (getline(yaml_file, line))
  {
    // handling comments
    if (line.find('#') != string::npos)
      line = line.substr(0, line.find('#'));

    int ident = line.find_first_not_of(' ');

    // ignoring empty line
    if (ident == string::npos)
      continue;

    int colon_pos = line.find(':');
    string lhs = trim(line.substr(0, colon_pos - 1));
    string rhs = trim(line.substr(colon_pos + 1));

    record_t current_record = {.name = lhs};
    if (rhs == "")
    {
      current_record.action = record_t::MERGE;
      current_record.children = vector<record_t>{};
    }
    else if (rhs == "wipe")
    {
      current_record.action = record_t::WIPE;
    }
    else if (rhs == "source")
    {
      current_record.action = record_t::SOURCE;
    }
    else if (rhs == "destination")
    {
      current_record.action = record_t::DESTINATION;
    }
    else
    {
      throw runtime_error("Error: Invalid action.");
    }

    if (ident < idents_stack.back())
    {
      while (ident < idents_stack.back())
      {
        records_stack.pop_back();
        idents_stack.pop_back();
      }
      if (ident != idents_stack.back())
        throw "Error: Indentation mismatch.";

      records_stack.pop_back();
      records_stack.back().get().children->push_back(current_record);
      records_stack.push_back(current_record);
    }
    else if (ident == idents_stack.back())
    {
      records_stack.pop_back();
      records_stack.back().get().children->push_back(current_record);
      records_stack.push_back(current_record);
    }
    else if (ident > idents_stack.back())
    {
      if (records_stack.back().get().children)
      {
        records_stack.back().get().children->push_back(current_record);
        records_stack.push_back(current_record);
        idents_stack.push_back(ident);
      }
      else
        throw "Error: Child found for non-merge record.";
    }
  }

  return root_record;
}

#endif