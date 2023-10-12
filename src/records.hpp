#ifndef records_hpp
#define records_hpp

#include <string>
#include <optional>
#include <vector>
#include <fstream>
#include <iostream>

inline std::string trim(const std::string &s, const char *t = " \t\n\r\f\v")
{
  if (s.find_first_not_of(t) == std::string::npos)
    return "";
  return s.substr(s.find_first_not_of(t), s.find_last_not_of(t) - s.find_first_not_of(t) + 1);
}

struct record_t
{
  std::string name;
  enum action_t
  {
    WIPE,
    KEEP,
    REPLACE,
    MERGE
  } action;
  std::optional<std::vector<record_t>> children;
};

record_t from_yaml(std::ifstream &yaml_file)
{
  record_t placeholder = {.name = "placeholder", .action = record_t::MERGE, .children = std::vector<record_t>{}};

  std::vector<std::reference_wrapper<record_t>> records_stack;
  std::vector<int> idents_stack;
  records_stack.push_back(placeholder);
  idents_stack.push_back(-1);

  std::string line;
  int line_number = 0;
  while (getline(yaml_file, line))
  {
    line_number++;

    // handling comments
    if (line.find('#') != std::string::npos)
      line = line.substr(0, line.find('#'));

    int ident = line.find_first_not_of(' ');

    // ignoring empty line
    if (ident == std::string::npos)
      continue;

    int colon_pos = line.find(':');
    std::string lhs = trim(line.substr(0, colon_pos));
    std::string rhs = trim(line.substr(colon_pos + 1));

    record_t current_record = {.name = lhs, .children = std::nullopt};
    if (rhs == "")
    {
      current_record.action = record_t::MERGE;
      current_record.children = std::vector<record_t>{};
    }
    else if (rhs == "wipe")
      current_record.action = record_t::WIPE;
    else if (rhs == "keep")
      current_record.action = record_t::KEEP;
    else if (rhs == "replace")
      current_record.action = record_t::REPLACE;
    else
      throw std::runtime_error("Error: Invalid action " + rhs + " at line " + std::to_string(line_number) + ".");

    if (ident < idents_stack.back())
    {
      while (ident < idents_stack.back())
      {
        records_stack.pop_back();
        idents_stack.pop_back();
      }
      if (ident != idents_stack.back())
        throw std::runtime_error("Error: Indentation mismatch. Expected " + std::to_string(idents_stack.back()) + " spaces, got " + std::to_string(ident) + " spaces at line " + std::to_string(line_number) + ".");

      records_stack.pop_back();
      records_stack.back().get().children->push_back(current_record);
      records_stack.push_back(records_stack.back().get().children->back());
    }
    else if (ident == idents_stack.back())
    {
      records_stack.pop_back();
      records_stack.back().get().children->push_back(current_record);
      records_stack.push_back(records_stack.back().get().children->back());
    }
    else if (ident > idents_stack.back())
    {
      if (records_stack.back().get().children)
      {
        records_stack.back().get().children->push_back(current_record);
        records_stack.push_back(records_stack.back().get().children->back());
        idents_stack.push_back(ident);
      }
      else
        throw std::runtime_error("Error: Child found for non-merge record {" + records_stack.back().get().name + ", " + std::to_string(records_stack.back().get().action) + " } at line " + std::to_string(line_number) + ".");
    }
  }
  return placeholder.children->front();
}

void to_yaml(std::ostream &yaml_file, const record_t &record, int ident = 0) {
  yaml_file << std::string(ident, ' ') << record.name << ": ";
  switch (record.action)
  {
  case record_t::WIPE:
    yaml_file << "wipe" << std::endl;
    break;
  case record_t::KEEP:
    yaml_file << "source" << std::endl;
    break;
  case record_t::REPLACE:
    yaml_file << "destination" << std::endl;
    break;
  case record_t::MERGE:
    yaml_file << std::endl;
    for (const auto &child : record.children.value())
      to_yaml(yaml_file, child, ident + 2);
    break;
  }
}

#endif