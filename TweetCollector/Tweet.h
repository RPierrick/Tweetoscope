#ifndef PRODUCT_H
#define PRODUCT_H 

#include <string>
#include <map>
#include <vector>

using namespace std;

class Tweet {
	private:
		string type;
		string msg;
		int time;
		int magnitude;
		int source;
		int info;
	public:
		Tweet(string type, string msg, int time, int magnitude, int source, int info);
		string getType();
		string getMsg();
		int getTime();
		int getMagnitude();
		int getSource();
		int getInfo();
		string toJson();
		Tweet(string jsonString);
		
		int delta(std::map<int,std::vector<std::pair<int,int>>> Map,int key);
		string tuple_to_string(std::map<int,std::vector<std::pair<int,int>>> Map,int key);
		std::map<int,std::vector<std::pair<int,int>>> add_in_map(std::map<int,std::vector<std::pair<int,int>>> Map,int key,int time,int magnitude);
		std::map<int,std::vector<std::pair<int,int>>> erase_map(std::map<int,std::vector<std::pair<int,int>>> Map,int key);
		int last_time(std::map<int,std::vector<std::pair<int,int>>> Map,int key);
};
#endif
