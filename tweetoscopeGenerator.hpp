#pragma once

#include <iostream>
#include <iomanip>
#include <fstream>
#include <queue>
#include <thread>
#include <atomic>
#include <stdexcept>

#include <gaml.hpp>

namespace tweetoscope {

  using timestamp = std::size_t;

  ////////////////
  //            //
  // Parameters //
  //            //
  ////////////////

  enum class Param : int {
    tweets_cache_page_size, 
    tweets_cache_nb_pages, 
    cascades_cache_page_size,
    cascades_cache_nb_pages,
    cascade_random_start_time_range,
    seconds_per_time_unit,
    time_acceleration_factor,
    tweets_index_path,
    cascades_index_path,
    tweets_path,
    cascades_path,
    max_number_of_cascades,
    number_of_threads,
    kafka_topic,
    kafka_brokers
  };

  inline Param param_of_keyword(const std::string& keyword) {
    if(keyword == "tweets_cache_page_size")           return Param::tweets_cache_page_size; 
    if(keyword == "tweets_cache_nb_pages")            return Param::tweets_cache_nb_pages; 
    if(keyword == "cascades_cache_page_size")         return Param::cascades_cache_page_size;
    if(keyword == "cascades_cache_nb_pages")          return Param::cascades_cache_nb_pages;
    if(keyword == "cascade_random_start_time_range")  return Param::cascade_random_start_time_range;
    if(keyword == "seconds_per_time_unit")            return Param::seconds_per_time_unit;
    if(keyword == "time_acceleration_factor")         return Param::time_acceleration_factor;
    if(keyword == "tweets_index_path")                return Param::tweets_index_path;
    if(keyword == "cascades_index_path")              return Param::cascades_index_path;
    if(keyword == "tweets_path")                      return Param::tweets_path;
    if(keyword == "cascades_path")                    return Param::cascades_path;
    if(keyword == "max_number_of_cascades")           return Param::max_number_of_cascades;
    if(keyword == "number_of_threads")                return Param::number_of_threads;
    if(keyword == "kafka_topic")                      return Param::kafka_topic;
    if(keyword == "kafka_brokers")                    return Param::kafka_brokers;
    throw std::runtime_error(std::string("bad keyword found: ") + keyword);
  }

  // This illustrates the use of istreams to parse input streams of bytes.
  inline Param parse_param_keyword(std::istream& is) {
    char c = '#';
    while(c == '#') {
      is >> c;
      if(c == '#') is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    is.putback(c);
    std::string keyword;
    is >> keyword;
    return param_of_keyword(keyword);
  }

  struct Params {
  public:

    unsigned int _tweets_cache_page_size          = 1000;             // number.
    unsigned int _tweets_cache_nb_pages           =   10;             // number.
    unsigned int _cascades_cache_page_size        = 1000;             // number.
    unsigned int _cascades_cache_nb_pages         =   10;             // number.
    double       _cascade_random_start_time_range =   10;             // minutes.
    double       _seconds_per_time_unit           =    1;             // number.
    double       _time_acceleration_factor        =  500;             // number
    std::string  _tweets_index_path               = "tweets.idx";     // file name.
    std::string  _cascades_index_path             = "cascades.idx";   // file name.
    std::string  _tweets_path                     = "";               // input file name.
    std::string  _cascades_path                   = "";               // input file name.
    unsigned int _max_number_of_cascades          = 10;               // number.
    unsigned int _number_of_threads               =  1;               // number.
    std::string  _kafka_topic                     = "tweets";         // topic name
    std::string  _kafka_brokers                   = "localhost:9092"; // string with a comma separated

  private:

    void parse(std::istream& is) {
      is.exceptions(std::ios::failbit | std::ios::badbit | std::ios::eofbit);
      try {
	while(true) {
	  switch(parse_param_keyword(is)) {
	  case Param::tweets_cache_page_size:          is >> _tweets_cache_page_size;          break;
	  case Param::tweets_cache_nb_pages:           is >> _tweets_cache_nb_pages;           break;
	  case Param::cascades_cache_page_size:        is >> _cascades_cache_page_size;        break;
	  case Param::cascades_cache_nb_pages:         is >> _cascades_cache_nb_pages;         break;
	  case Param::cascade_random_start_time_range: is >> _cascade_random_start_time_range; break;
	  case Param::seconds_per_time_unit:           is >> _seconds_per_time_unit;           break;
	  case Param::time_acceleration_factor:        is >> _time_acceleration_factor;        break;
	  case Param::tweets_index_path:               is >> _tweets_index_path;               break;
	  case Param::cascades_index_path:             is >> _cascades_index_path;             break;
	  case Param::tweets_path:                     is >> _tweets_path;                     break;
	  case Param::cascades_path:                   is >> _cascades_path;                   break;
	  case Param::max_number_of_cascades:          is >> _max_number_of_cascades;          break;
	  case Param::number_of_threads:               is >> _number_of_threads;               break;
	  case Param::kafka_topic:                     is >> _kafka_topic;                     break;
	  case Param::kafka_brokers:                   is >> _kafka_brokers;                   break;
	  }
	  is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}
      }
      catch(std::ios_base::failure&) {}
    }

  public:
    Params()                         = default;
    Params(const Params&)            = default;
    Params& operator=(const Params&) = default;
    Params(std::istream& is) {parse(is);}
    Params(const std::string& filename) {
      std::ifstream is(filename.c_str());
      if(is) parse(is);
      else   throw std::runtime_error(std::string("Cannot open param file ") + filename);
    }

    timestamp range_start() const {return _cascade_random_start_time_range * 60;}
  }; 

  std::ostream& operator<<(std::ostream& os, const Params& params) {
    os << "Parmeters : " << std::endl       
       << "  tweets_path                     = " << params._tweets_path                     << std::endl                    
       << "  cascades_path                   = " << params._cascades_path                   << std::endl   
       << "  tweets_index_path               = " << params._tweets_index_path               << std::endl              
       << "  cascades_index_path             = " << params._cascades_index_path             << std::endl            
       << "  tweets_cache_page_size          = " << params._tweets_cache_page_size          << std::endl               
       << "  tweets_cache_nb_pages           = " << params._tweets_cache_nb_pages           << std::endl           
       << "  cascades_cache_page_size        = " << params._cascades_cache_page_size        << std::endl             
       << "  cascades_cache_nb_pages         = " << params._cascades_cache_nb_pages         << std::endl         
       << "  cascade_random_start_time_range = " << params._cascade_random_start_time_range << std::endl
       << "  seconds_per_time_unit           = " << params._seconds_per_time_unit           << std::endl         
       << "  time_acceleration_factor        = " << params._time_acceleration_factor        << std::endl         
       << "  max_number_of_cascades          = " << params._max_number_of_cascades          << std::endl     
       << "  number_of_threads               = " << params._number_of_threads               << std::endl
       << "  kafka_topic                     = " << params._kafka_topic                     << std::endl
       << "  kafka_brokers                   = " << params._kafka_brokers                   << std::endl;
    return os;
  }

  ////////////
  //        //
  // Tweets //
  //        //
  ////////////

  struct Tweet {
    timestamp time     = 0;          //!< The tweet timestamp. The time origin is specific to each tweet source.
    double parsed_time = 0;          //!< This is the time value extracted from the datafile, stored here at parsing time for further conversion into an actual integral timestamp.
    double magnitude   = 0;          //!< The magnitude is the importance of the tweet.
    std::string msg    = "None";     //!< This is a message, for debugging in further kafka stages.

    /**
     * The expression (tweet + start) gives a tweet whose timestamp is shifted with the actual cascade start time.
     * @param start_time The cascade start time, the reference clock is related to the tweet source.
     */
    Tweet operator+(timestamp start_time) const {
      return {start_time + time, parsed_time, magnitude, msg};
    }
  };

  /**
   * Serialization operator for producing the tweet in kafka
   */
  inline std::ostream& operator<<(std::ostream& os, const Tweet& tweet) {
    os << "{\"time\": "       << tweet.time 
       << ", \"magnitude\": " << tweet.magnitude
       << ", \"msg\": \""     << tweet.msg 
       << "\"}";
    return os;
  }

  /**
   * Deserialization operator when reading a tweet from the
   * database. The file may host a supplementary message attribute. It
   * is not read here, but the tweet parser will handle that reading,
   * if that attribute is in the data file.
   */
  inline std::istream& operator>>(std::istream& is, Tweet& tweet) {
    char sep;
    is >> tweet.parsed_time >> sep >> tweet.magnitude;
    return is;
  }

  /**
   * This parser fits the concept of gaml parsers. There are examples
   * of this in the gaml documentation.
   */
  struct TweetParser : public gaml::BasicParser {
    using value_type = Tweet;  
    mutable bool has_msg_field = false; //!< A third 'msg' column in the datafile is optional, so wee need to determine wether it is there or not.
    const Params& params;

    TweetParser(const Params& params) : gaml::BasicParser(), params(params) {}

    void writeSeparator(std::ostream& os) const {os << std::endl;}

    /**
     * This parses the file header and determine wether we have a 3rd 'msg' column.
     */
    void readBegin(std::istream& is) const {
      std::string header_line;
      std::getline(is, header_line, '\n');
      unsigned int number_of_commas = 0;
      for(auto c: header_line) if(c == ',') ++number_of_commas;
      has_msg_field = (number_of_commas == 2);
      if(number_of_commas != 1 && number_of_commas != 2) 
	throw std::runtime_error("Expecting 1 or 2 ',' in data file");
    }
    
    bool readSeparator(std::istream& is) const {is >> std::ws; return !is.eof();}

    void read(std::istream& is, value_type& data) const {
        is >> data;
        if(has_msg_field) {
            char c;
            is >> c;
            if(c != ',') 
                throw std::runtime_error(std::string("Expected ,  but got ") + c);
            std::getline(is, data.msg, '\n');
        }
        // Convert the stamp (double) in the datafiles into integral seconds (i.e. a timestamp).
        data.time = timestamp(data.parsed_time * params._seconds_per_time_unit + .5);
    }

    void write(std::ostream& os, const value_type& data) const {os << data;}
  };

  /**
   * This is a smart way to build a tweet parser.
   */
  inline auto tweet_parser(const Params& params) {return TweetParser(params);}

  

  /////////////
  //         //
  // Cascade //
  //         //
  /////////////

  /**
   * This represents a tweet cascade stored in the cascade datafile.
   */
  template<typename DataIt>
  struct Cascade {
  private:
    /**
     * When a cascade is parsed from a file, we get first tweet and
     * last tweet in the tweet files as tweet integral identifiers,
     * which are offsets from the first tweet in the tweet file. This
     * function updates the tweet iterators (begin and end) so that
     * they correspond to the base iterator incremented with these
     * offsets.
     */
    void update_from_offsets() {
      if(base != DataIt()) {
	begin = base + offset_begin;
	end   = base + offset_end;
      }
    }
    
  public:
    
    DataIt      base;           //!< The first data in the dataset.
    std::size_t offset_begin;   //!< Offset of the first data in the cascade.
    std::size_t offset_end;     //!< Offset of the last data (not included) in the cascade.
    DataIt      begin;          //!< Actual iterator to the first tweet (i.e. base+offset_begin).
    DataIt      end;            //!< Actual iterator to the first tweet (i.e. base+offset_end).

    Cascade()                          = default;
    Cascade(const Cascade&)            = default;
    Cascade& operator=(const Cascade&) = default;
    void operator=(const std::pair<std::size_t, std::size_t>& offsets) {std::tie(offset_begin, offset_end) = offsets; update_from_offsets();}
    void operator=(DataIt base)                                        {this->base                         = base   ; update_from_offsets();}
  };


  /**
   * We parse the cascade from the file.
   */
  template<typename DataIt>
  inline std::istream& operator>>(std::istream& is, Cascade<DataIt>& cascade) {
    char sep;
    std::size_t offset_begin, offset_end;
    is >> offset_begin >> sep >> offset_end;
    cascade = {offset_begin-1, offset_end};
    return is;
  }

  /**
   * This serializes a cascade (for debugging).
   */
  template<typename DataIt>
  inline std::ostream& operator<<(std::ostream& os, const Cascade<DataIt>& cascade) {
    if(cascade.base == DataIt()) // The base is not initialised
      os << "[Unanchored (" << cascade.offset_begin << ", " << cascade.offset_end << ")]";
    else {
      std::cout << '[';
      auto it = cascade.begin;
      if(cascade.begin != cascade.end) os << *(it++);
      while(it != cascade.end)         os << ", " << *(it++);
      os << ']';
    }
    return os;
  }


  /**
   * This parser fits the gaml parser concept as well.
   */
  template <typename DataIt>
  struct CascadeParser : public gaml::BasicParser {

    using value_type = Cascade<DataIt>;

    DataIt base;

    CascadeParser(DataIt base) : gaml::BasicParser(), base(base) {}
    void writeSeparator(std::ostream& os)                  const {os << std::endl;} 
    void readBegin(std::istream& is)                       const {is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');}
    bool readSeparator(std::istream& is)                   const {is >> std::ws; return !is.eof();}
    void read(std::istream& is,        value_type& data)   const {is >> data; data = base;}
    void write(std::ostream& os, const value_type& data)   const {os << data;}
  };

  template<typename DataIt>
  auto cascade_parser(DataIt base) {return CascadeParser<DataIt>(base);}
  

  ///////////////
  //           //
  // Scheduler //
  //           //
  ///////////////

  /* The scheduler handles tweet emissions. So an event is basically a
     tweet. During the simulation, the scheduler determines the date
     of emission of the simulated tweets. It stores them into a
     priority queue, so that they can be actually emitted in their
     date order. Indeed, the scheduler simulates a tweet source, where
     several tweet cascades are executed. The events keep trace of the
     cascade they come from, they handle a boolean for knowing if thy
     are a initial tweet or a retweet, etc... This is why events are a
     bit more than juste a tweet. */

  /**
   * This is the base class for Events. Indeed, the event type is a
   * template (depending on the data iterator type), but we need a
   * static variable to tag each event with a specific
   * identifier. This static part is handled in the event base, and
   * then we inherit the template from it. Some other non type
   * dependant information are also handled in this base class.
   */
  struct EventBase {
  private:

    static std::atomic<unsigned int> next_free_idf;
    unsigned int idf;

  public:

    timestamp   start_time;   //!< This is the start of the cascade.
    timestamp   date;         //!< This is the date of the tweet emission.
    std::size_t cascade_id;   //!< This identifies the cascade that concerns this tweet.
    bool        first_tweet;  //!< This tells wether the tweet ois an initial tweet or a retweet.
    std::size_t source_id;    //!< This identifies the tweet source (i.e the scheduler in this simulation context).
    
    unsigned int id() const {return idf;}
    /**
     * This comparison will be used to order the events in the
     * scheduler queue. Here, the most prioritary event is the one
     * with the smallest date.
     */
    bool operator<(const EventBase& other) const {return date > other.date;}

  protected:

    EventBase()                            = delete;
    EventBase(const EventBase&)            = default;
    EventBase& operator=(const EventBase&) = default;
    EventBase(timestamp start_time, std::size_t cascade_id)
      : idf(next_free_idf++),
	start_time(start_time),
	date(start_time),
	cascade_id(cascade_id),
	first_tweet(true){}
  };

  // This declares the serialization operator, defined afterwards.
  template<typename DataIt> struct Event;
  template<typename DataIt>
  std::ostream& operator<<(std::ostream& os, const Event<DataIt>& evt);

  /**
   * This is the data iterator type dependent completion of our event class.
   */
  template<typename DataIt>
  struct Event : public EventBase {
  private:

    DataIt begin, end, current;

    friend std::ostream& operator<<<DataIt>(std::ostream& os, const Event<DataIt>& evt);

  public:


    Event()                        = delete;
    Event(const Event&)            = default;
    Event& operator=(const Event&) = default;
    
    template<typename CascadeIt>
    Event(CascadeIt cascade, timestamp start_time)
      : EventBase(start_time, cascade.index()), // cascade.index() is the index of the cascade in the datafile, it can be used as a unique identifier.
	begin((*cascade).begin),
	end((*cascade).end),
	current((*cascade).begin)
    {}

    /**
     * The expression *event returns a tweet, according to the current
     * data iterator, but shifted with the cascade starting time.
     */
    Tweet operator*() const {return (*current) + start_time;}

    /**
     * ++evt makes the event correspond to the next event in the cascade.
     */
    Event& operator++() {
      date        = start_time + (*(++current)).time;
      first_tweet = false;
      return *this;
    }
    
    /**
     * evt++ makes the event correspond to the next event in the
     * cascade, but returns the event before the skip.
     */
    Event operator++(int) {
      auto tmp = *this;
      operator++();
      return tmp;
    }
    
    /**
     * if(evt) allows to check that the event can be incremented to
     * skip to next one.
     */
    operator bool() const {return current != end;}
  };

  /**
   * This serializes an event into a stream with a JSON format, as kafka requires.
   */
  template<typename DataIt>
  std::ostream& operator<<(std::ostream& os, const Event<DataIt>& evt) {
    auto tweet = *evt;
    os << "{";
    if(evt.first_tweet)
      os << "\"type\": \"tweet\"";
    else
      os << "\"type\": \"retweet\"";
    os << ", \"msg\": \"" << tweet.msg << '"'
       << ", \"t\": " << tweet.time 
       << ", \"m\": " << tweet.magnitude 
       << ", \"source\": " << evt.source_id
       << ", \"info\": \"cascade=" << evt.cascade_id << '"' 
       << '}';
    return os;
  }

  // This is the end of simulation exception.
  struct end_of_simulation {
    end_of_simulation()                                    = default;
    end_of_simulation(const end_of_simulation&)            = default;
    end_of_simulation& operator=(const end_of_simulation&) = default;
  };

  /**
   * The scheduler represents one tweet source, where several cascades
   * are generated.x
   */
  template<typename DataIt, typename CascadeIt, typename RANDOM_GEN>
  class Scheduler {
  private:

    const Params& params;
    CascadeIt     begin, end, current; //!< The scheduler handles cascades in [begin, end[, current is the currently considered one.
    RANDOM_GEN&   gen;

    std::optional<unsigned int>              nb_iter;       //!< If the optional is set, simulation is stopped after nb_iter events.
    std::uniform_int_distribution<timestamp> cascade_date;  //!< This enables to choose the date of a cascade start randomly.
    std::priority_queue<Event<DataIt>>       event_queue;   //!< This is the event queue, ordered by increasing timestamps.
    unsigned int                             step = 0;      //!< This is the simuation step number.
    timestamp                                now = 0;       //!< This is the current date simulated.

  public:

    Scheduler(const Params& params, 
	      CascadeIt begin, CascadeIt end,
	      RANDOM_GEN& gen)
      : params(params), begin(begin), end(end), current(begin), gen(gen),
	nb_iter(),
	cascade_date(0, params.range_start()),
	event_queue(),
	step(0), now(0) {
      // We put the first cascades in the queue.
      current = begin;
      for(unsigned int i = 0; i < params._max_number_of_cascades && current != end; ++i) event_queue.push({current++, now + cascade_date(gen)});
    }

    void set_walltime(unsigned int nb_iter) {this->nb_iter = nb_iter;}

    /**
     * This produces the next event.
     */
    Event<DataIt> operator++() {
      // We flush the events from the loop.
      if((begin != end) // fails if there have been no cascades at all.
	 && (!event_queue.empty())
	 && (!nb_iter || (*nb_iter > step))) {
	++step;

	auto evt = event_queue.top();
	event_queue.pop();

	int duration_ms = int( 1000 * (evt.date - now) / params._time_acceleration_factor); 	
	std::this_thread::sleep_for(std::chrono::milliseconds(duration_ms));
	now = evt.date;

	// Post and skip
	auto res = evt++;

	// If the cascade is not over, we push the next event, otherwise  we start a new one
	if(evt)                 event_queue.push(evt);
	else if(current != end) event_queue.push({current++, now + cascade_date(gen)});
	return res;
      }
      else
	throw end_of_simulation {};

    }
  };

  template<typename DataIt, typename CascadeIt, typename RANDOM_GEN>
  auto scheduler(const Params& params, CascadeIt begin, CascadeIt end, RANDOM_GEN& gen) {return Scheduler<DataIt, CascadeIt, RANDOM_GEN>(params, begin, end, gen);}


}

