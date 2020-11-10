// Compile with:
// g++ -o TweetCollector Tweet_Collector.cpp Tweet.cpp `pkg-config --libs --cflags cppkafka`

#include <iostream>
#include <ostream>
#include <cppkafka/cppkafka.h>
#include "Tweet.h"
#include <map>
#include <vector>

int main() {
	std::map<int,std::vector<std::pair<int,int>>> myMap;
	
	std::map<int,int> memoire_envoi_series;
	std::map<int,int> memoire_envoi_properties;

	cppkafka::Configuration config = {
		{ "bootstrap.servers", "localhost:9092" },
		{ "auto.offset.reset", "earliest" },
		{ "group.id", "myOwnPrivateCppGroup" }
	};
	
	//Création du topic "tweet" en temps que consommateur :
	
	cppkafka::Consumer consumer(config);
	consumer.subscribe({"tweets"});
	
	//Création du topic "cascade_series" en temps que producteur
	
	cppkafka::Producer producer(config);
	cppkafka::MessageBuilder series("cascade_series");
	
	//Création du topic "cascade_properties" en temps que producteur
	
	cppkafka::MessageBuilder properties("cascade_properties");

	// Read messages
	while(true) {
		auto msg = consumer.poll();
		if(msg && ! msg.get_error()) {
			Tweet T(msg.get_payload());
			
			int source = T.getSource(),time = T.getTime(),mag = T.getMagnitude();
			int info = T.getInfo();
			if (memoire_envoi_properties[info] == 0)  //On n'écrit plus dans la myMap[info] quand celle-ci a été envoyé dans "cascade_properties"
			{
				myMap = T.add_in_map(myMap,info,time,mag);
			}
			
			//Partie envoi des séries partielles :
			
			if (T.delta(myMap,info) > 600)
			{
				if (memoire_envoi_series[info] == 0)
				{
					std::string cascade = T.tuple_to_string(myMap,info);
					std::ostringstream ostr;
					std::string identifiant_tweet = "tw" + std::to_string(info);
					ostr << "{ 'type : 'serie', 'cid : "<< identifiant_tweet <<" , 'T_obs' : " << 600 << " , " <<cascade <<"}";
					std::string message {ostr.str()};
					series.payload(message);
					producer.produce(series);
					memoire_envoi_series[info] = 1;
					}
			}
			for(auto it = myMap.cbegin(); it != myMap.cend(); ++it)
			{
				int key = it->first;
				if((time - T.last_time(myMap,key)) > 1000)
				{
					if (memoire_envoi_properties[key] == 0)
					{
						std::ostringstream ostr2;
						std::string identifiant_tweet = "tw" + std::to_string(key);
						int n_tot = myMap[key].size();
						ostr2 << "{ 'type' : 'size', 'cid' : " << identifiant_tweet << " , 'n_tot' : " << n_tot << " , 't_end' : " << T.last_time(myMap,key) << "}";
						std::string message2 {ostr2.str()};
						properties.payload(message2);
						producer.produce(properties);
						memoire_envoi_properties[key] = 1;
						//myMap.erase(key);
					}
				}
			}
			/*std::cout << T.getTime() << " " << T.getMagnitude() << " " << T.getInfo() <<  std::endl;
			auto message = std::string(msg.get_payload());
			std::cout << message << std::endl;*/
		}
	}
}

