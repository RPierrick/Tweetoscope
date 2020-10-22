/*

  This is how you can compile this file in order to get the 'tweet-generator' executable.

  g++ -o tweet-generator tweet-generator.cpp -O3 $(pkg-config --cflags --libs gaml) -lpthread -lcppkafka

*/

#include <iostream>
#include <iterator>
#include <cppkafka/cppkafka.h>
#include "tweetoscopeGenerator.hpp"

// This is the actual definition of the static attribute in tweetoscope::EventBase class.
// This is a thread-safe global variable.
std::atomic<unsigned int> tweetoscope::EventBase::next_free_idf = 0;

int main(int argc, char* argv[]) {
  if(argc != 2) {
    std::cout << "Usage : " << argv[0] << " params_file" << std::endl;
    return 0;
  }

  // random seed initialization
  std::random_device rd;
  std::mt19937 gen(rd());

  tweetoscope::Params params(argv[1]);
  std::cout << params << std::endl
	    << std::endl
	    << std::endl;

  // First, we create the indexed dataset in order to trigger the
  // index creation of necessary. Do not forget to remove the .idx
  // files if you change your data. Indeed, if the files exist, the
  // program will rely on them to access the data.
  auto parse_tweets   = tweetoscope::tweet_parser(params);
  auto tweets         = gaml::make_indexed_dataset(parse_tweets, params._tweets_path, params._tweets_index_path);
  auto parse_cascades = tweetoscope::cascade_parser(tweets.begin());
  auto cascades       = gaml::make_indexed_dataset(parse_cascades, params._cascades_path, params._cascades_index_path);
  std::cout << "Dataset files read" << std::endl;

  // Then, we compute a shuffle of the saccades, each thread will use the same permutation.
  auto cascades_shuffled = gaml::packed_shuffle(cascades.begin(), cascades.end(), params._cascades_cache_page_size, gen);

  // At this point, we have created data sets, and a final permutation
  // of the cascades. If needed, the indices files have been computed.
  // In next code, each thread will restart this process, relying the
  // the permutation that we have computed. The other sets will be
  // unused and ignored. This is not a waste of time, since the set
  // creation are lazy, i.e. data is actually retrieved on demand. As
  // we hav not asked for any data, nothing has really happen so far
  // (except an eventual and time consuming index file creation once).

  // Now, we start threads. Each thread is a lambda-function, that well build its own gaml objects for getting data.
  std::vector<std::thread> threads;
  for(unsigned int thread_id = 0; thread_id < params._number_of_threads; ++thread_id)
    threads.emplace_back([thread_id, &gen, &params, &shuf_idx = cascades_shuffled.index_table()]() {
	// Create the kafka producer
	cppkafka::Configuration config {
	  {"metadata.broker.list", params._kafka_brokers },
	  {"log.connection.close", false }
	};
	cppkafka::MessageBuilder builder {params._kafka_topic};
	cppkafka::Producer       producer(config);

	// We access the tweets (a cache is used).
	auto parse_tweets    = tweetoscope::tweet_parser(params);
	auto tweet_data      = gaml::make_indexed_dataset(parse_tweets, params._tweets_path, params._tweets_index_path);
	auto tweet_dataset   = gaml::cache(tweet_data.begin(), tweet_data.end(), params._tweets_cache_page_size, params._tweets_cache_nb_pages);


	// We access the cascades (a cache is used).... and we shuffle
	// using the "custom" permutation, providing it with the
	// suffle we have built at start. All threads thus suffle the same
	// way.
	auto parse_cascades  = tweetoscope::cascade_parser(tweet_dataset.begin());
	auto cascade_data    = gaml::make_indexed_dataset(parse_cascades, params._cascades_path, params._cascades_index_path);
	auto cascade_cached  = gaml::cache(cascade_data.begin(), cascade_data.end(), params._cascades_cache_page_size, params._cascades_cache_nb_pages);
	auto cascade_dataset = gaml::custom(cascade_cached.begin(),
					    shuf_idx.begin(), shuf_idx.end()); // All threads share the same pre-computed permutation.

	// k-fold is the tool used for cross-validation. It splits a dataset into params._number_of_threads chuncks.
	auto kfold           = gaml::partition::KFold(cascade_dataset.begin(), cascade_dataset.end(), params._number_of_threads);

	// Here, each thread consider its chunk, i.e. the thread_id-th chunk of the k-fold partition.
	auto sched           = tweetoscope::scheduler<decltype(tweet_dataset.begin())>(params, kfold.begin(thread_id), kfold.end(thread_id), gen);

	// At the point, each thread owns a scheduler for one piece of all the cascades, gathered into pieces after having being shuffled.

	// Now the simulation starts, simulating tweet cascades. Each
	// thread is a source of tweets and retweets, from many
	// cascades, so that the generator simulates parallel sources
	// of tweets. We are inside one of these sources here.
	try {
	  while(true) {
	    auto evt = ++sched;
	    // Tag the event as coming from this thread source
	    // This tag is used when serializing the event
	    evt.source_id = thread_id;

	    auto key = std::to_string(evt.id());
	    builder.key(key);

	    std::ostringstream ostr;
	    ostr << evt;
	    auto msg = ostr.str();
	    builder.payload(msg);

	    // It is critical to have the msg, and key variable
	    // still alive in memory when producing the message
	    // because builder takes references on them 
	    producer.produce(builder);
	  }
	}
	catch (tweetoscope::end_of_simulation&) {}
      });

  // We wait for the termination of all threads.
  for(auto& t : threads) t.join();

  return 0;

}
