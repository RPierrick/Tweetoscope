/*

  The boost library has clever tools for handling program
  parameters. Here, for the sake of code simplification, we use a
  custom class.

*/


#pragma once

#include <tuple>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstddef>
#include <stdexcept>

namespace tweetoscope {
  namespace params {
    namespace section {
      struct Kafka {
	std::string brokers;
      };
      
      struct Topic {
	std::string in, out_series, out_properties;
      };
      
      struct Times {
	std::vector<std::size_t> observation;
	std::size_t              terminated;
      };

      struct Cascade {
	std::size_t min_cascade_size;
      };
    }
    
      
    struct collector {
    private:
      std::string current_section;

      std::pair<std::string, std::string> parse_value(std::istream& is) {
	char c;
	std::string buf;
	is >> std::ws >> c;
	while(c == '#' || c == '[') {
	  if(c == '[') std::getline(is, current_section, ']');
	  std::getline(is, buf, '\n');
	  is >> std::ws >> c;
	}
	is.putback(c);
	std::string key, val;
	is >> std::ws;
	std::getline(is, key, '=');
	is >> val;
	std::getline(is, buf, '\n');
	return {key, val};
      }
      
    public:

      section::Kafka   kafka;
      section::Topic   topic;
      section::Times   times;
      section::Cascade cascade;

      collector(const std::string& config_filename) {
	std::ifstream ifs(config_filename.c_str());
	if(!ifs)
	  throw std::runtime_error(std::string("Cannot open \"") + config_filename + "\" for reading parameters.");
	ifs.exceptions(std::ios::failbit | std::ios::badbit | std::ios::eofbit);
	try {
	  while(true) {
	    auto [key, val] = parse_value(ifs);
	    if(current_section == "kafka") {
	      if(key == "brokers") kafka.brokers = val;
	    }
	    else if(current_section == "topic") {
	      if     (key == "in")             topic.in             = val;
	      else if(key == "out_series")     topic.out_series     = val;
	      else if(key == "out_properties") topic.out_properties = val;
	    }
	    else if(current_section == "times") {
	      if     (key == "observation")    times.observation.push_back(std::stoul(val));
	      else if(key == "terminated")     times.terminated = std::stoul(val);
	    }
	    else if(current_section == "cascade") {
	      if (key == "min_cascade_size")   cascade.min_cascade_size = std::stoul(val);
	    }
	  }
	}
	catch(const std::exception& e) {/* nope, end of file occurred. */}
      }
    };

    inline std::ostream& operator<<(std::ostream& os, const collector& c) {
      os << "[kafka]" << std::endl
	 << "  brokers=" << c.kafka.brokers << std::endl
	 << std::endl
	 << "[topic]" << std::endl
	 << "  in=" << c.topic.in << std::endl
	 << "  out_series=" << c.topic.out_series << std::endl
	 << "  out_properties=" << c.topic.out_properties << std::endl
	 << std::endl
	 << "[times]" << std::endl;
      for(auto& o : c.times.observation)
	os << "  observation=" << o << std::endl;
      os << "  terminated=" << c.times.terminated << std::endl
	 << std::endl
	 << "[cascade]" << std::endl
	 << "  min_cascade_size=" << c.cascade.min_cascade_size << std::endl;
	  return os;
    }
  }
}

