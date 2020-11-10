#include "Tweet.h"
#include <map>
#include <vector>
#include <tuple>


Tweet::Tweet(std::string type, std::string msg, int time, int magnitude, int source, int info){
	this->type = type;
	this->msg = msg;
	this->time = time;
	this->magnitude = magnitude;
	this->source = source;
	this->info = info;
}

std::string Tweet::getType(){
	return type;
}

std::string Tweet::getMsg(){
	return msg;
}

int Tweet::getTime(){
	return time;
}

int Tweet::getMagnitude(){
	return magnitude;
}

int Tweet::getSource(){
	return source;
}

int Tweet::getInfo(){
	return info;
}

Tweet::Tweet(string jsonString){
	 /* jsonString: {"type": "retweet", "msg": "None", "t": 1527, "m": 592, "source": 2, "info": "cascade=19707"} */
   string s1 = jsonString.substr(jsonString.find_first_of(":")+2, jsonString.length());
   /*s1: retweet", "msg": "None", "t": 1527, "m": 592, "source": 2, "info": "cascade=19707"} */
   string type = s1.substr(0, s1.find_first_of("\""));
   
   string s2 = s1.substr(s1.find_first_of(":")+2, s1.length());
   string msg = s2.substr(0, s2.find_first_of("\""));
   
   string s3 = s2.substr(s2.find_first_of(":")+2, s2.length());
   string time = s3.substr(0, s3.find_first_of("\""));
   
   string s4 = s3.substr(s3.find_first_of(":")+2, s3.length());
   string magnitude = s4.substr(0, s4.find_first_of("\""));
   
   string s5 = s4.substr(s4.find_first_of(":")+2, s4.length());
   string source = s5.substr(0, s5.find_first_of("\""));
   
   string s6 = s5.substr(s5.find_first_of(":")+2, s5.length()); //"cascade=19028"}
   string s7 = s6.substr(s6.find_first_of("=")+1,s6.length());
   string info = s7.substr(0,s7.find_first_of("\""));
   /*string s6 = s5.substr(s5.find_first_of(":")+2, s5.length());
   string info = s6.substr(0, s6.find_first_of("\""));*/
   
   
   this->type = type;
   this->msg = msg;
   this->time = stoi(time);
   this->magnitude = stoi(magnitude);
   this->source = stoi(source);
   this->info = stoi(info);
}

int Tweet::delta(std::map<int,std::vector<std::pair<int,int>>> Map,int key){
	auto vec = Map[key];
	int fin = vec.size()-1;
	int dernier = vec[fin].first;
	int premier = vec[0].first;
	int d = dernier-premier;
	return (d);
}

string Tweet::tuple_to_string(std::map<int,std::vector<std::pair<int,int>>> Map,int key){
	std::string str = "[";
	auto vec = Map[key];
	for(int i=0; i != vec.size(); ++i)
	{
		str = str + "(" + std::to_string(vec[i].first) + "," + std::to_string(vec[i].second) + ")" + ";";
	}
	str.erase(str.length()-1,1);
	str = str + "]";
	return str;
}

std::map<int,std::vector<std::pair<int,int>>> Tweet::add_in_map(std::map<int,std::vector<std::pair<int,int>>> Map,int key,int time,int magnitude){
	auto vecteur = Map[key];
	vecteur.push_back(std::make_pair(time,magnitude));
	Map[key] = vecteur;
	return Map; 
}

std::map<int,std::vector<std::pair<int,int>>> Tweet::erase_map(std::map<int,std::vector<std::pair<int,int>>> Map,int key){
	int d = delta(Map,key);
	if (d > 1000) {
		Map.erase(key);
	}
	return Map;	
}

int Tweet::last_time(std::map<int,std::vector<std::pair<int,int>>> Map,int key){
	auto vecteur = Map[key];
	int dernier_temps = vecteur[vecteur.size()-1].first;
	return(dernier_temps);
}
