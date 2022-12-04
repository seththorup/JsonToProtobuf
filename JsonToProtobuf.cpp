#include "jsontoprotobuf.h"

#include <boost/json.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <iostream>

using google::protobuf::util::JsonParseOptions;
using std::cout;
using std::endl;
using std::exception;
using std::string;

using namespace google::protobuf;
using namespace boost::property_tree;
using namespace boost::property_tree::json_parser;

namespace thorup {

jsontoprotobuf::jsontoprotobuf() {
  m_options.add_whitespace = true;
  m_options.always_print_primitive_fields = true;
  m_options.preserve_proto_field_names = true;
}

bool jsontoprotobuf::convert_json_to_protobuf(const string &json_file_name,
                                              const string &object_name,
                                              Message &proto_msg) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  if (!std::filesystem::exists(json_file_name)) {
    string error_report{"Error: file does not exsist " + json_file_name};
    throw(error_report);
    return false;
  }

  ptree json_file_ptree;
  if (read_json(json_file_name, json_file_ptree); json_file_ptree.empty()) {
    throw("File failed to load properly");
    return false;
  }

  // purpose built function to parse linkbudget config file data_links_list
  auto ptree = json_file_ptree.get_child_optional(object_name);
  if (ptree.get_ptr() == nullptr) {
    throw("Child not found");
    return false;
  }

  try {
    std::ostringstream msg_oss;
    write_json(msg_oss, ptree.value());
    auto json_string = msg_oss.str();

    google::protobuf::util::JsonParseOptions options;
    JsonStringToMessage(json_string, &proto_msg, options);
  } catch (exception &ex) {
    cout << __FUNCTION__ << " error in processing ptree! " << ex.what() << endl;
    return false;
  }
  return true;
}

bool jsontoprotobuf::convert_protobuf_to_json(const Message &proto_msg,
                                              string &json_string,
                                              string output_file_name = "") {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  bool status{true};
  std::ofstream json_file_handler;

  try {
    // Create a json_string from sr.
    MessageToJsonString(proto_msg, &json_string, m_options);

    json_file_handler.open(output_file_name,
                           std::ofstream::out | std::ofstream::trunc);
    json_file_handler << json_string;

    cout << "writing file: " << output_file_name << endl;
  } catch (exception &ex) {
    cout << __FUNCTION__ << " failed! Exception = " << ex.what() << endl;
    status = false;
  }

  json_file_handler.close();
  return status;
}

} // namespace thorup