#pragma once
#include <vector>
#include <string>
#include <random>
#include <sstream>
#include <map>
#include <type_traits>
#include <algorithm>
#include "board.h"
#include "action.h"
#include "weight.h"
#include <fstream>
class weight_agent;
class agent {
public:
	agent(const std::string& args = "") {
		std::stringstream ss("name=unknown role=unknown " + args);
		for (std::string pair; ss >> pair; ) {
			std::string key = pair.substr(0, pair.find('='));
			std::string value = pair.substr(pair.find('=') + 1);
			meta[key] = { value };
		}
	}
	virtual ~agent() {}
	virtual void open_episode(const std::string& flag = "") {}
	virtual void close_episode(const std::string& flag = "") {}
	virtual action take_action(const board& b, weight_agent& table) { return action(); }
	virtual bool check_for_win(const board& b) { return false; }

public:
	virtual std::string property(const std::string& key) const { return meta.at(key); }
	virtual void notify(const std::string& msg) { meta[msg.substr(0, msg.find('='))] = { msg.substr(msg.find('=') + 1) }; }
	virtual std::string name() const { return property("name"); }
	virtual std::string role() const { return property("role"); }

protected:
	typedef std::string key;
	struct value {
		std::string value;
		operator std::string() const { return value; }
		template<typename numeric, typename = typename std::enable_if<std::is_arithmetic<numeric>::value, numeric>::type>
		operator numeric() const { return numeric(std::stod(value)); }
	};
	std::map<key, value> meta;
};

class random_agent : public agent {
public:
	random_agent(const std::string& args = "") : agent(args) {
		if (meta.find("seed") != meta.end())
			engine.seed(int(meta["seed"]));
	}
	virtual ~random_agent() {}

protected:
	std::default_random_engine engine;
};

/**
 * base agent for agents with weight tables
 */
class weight_agent : public agent {
public:
	weight_agent(const std::string& args = "") : agent(args) {
		if (meta.find("init") != meta.end()) // pass init=... to initialize the weight
			init_weights(meta["init"]);
		if (meta.find("load") != meta.end()) // pass load=... to load from a specific file
			load_weights(meta["load"]);
	}
	virtual ~weight_agent() {
		if (meta.find("save") != meta.end()) // pass save=... to save to a specific file
			save_weights(meta["save"]);
	}
	weight& operator [](unsigned i) { return net[i]; }
	const weight& operator [](unsigned i) const { return net[i]; }
protected:

	virtual void init_weights(const std::string& info) {
 		// 15^4 = 50625 
		// 15^6 = 11390625
		//net.emplace_back(50625);
		//net.emplace_back(50625);
		net.emplace_back(11390625);// 012345
		net.emplace_back(11390625);// 456789
		net.emplace_back(11390625);// 012456
		net.emplace_back(11390625);// 45689a
		for(size_t i = 0;i < net.size();i++){
			for(size_t j = 0;j < net[i].size();j++){
				net[i][j] = 0.0f;
			}
		}
		// now net.size() == 2; net[0].size() == 50625; net[1].size() == 50625
	}
	virtual void load_weights(const std::string& path) {
		std::ifstream in(path, std::ios::in | std::ios::binary);
		if (!in.is_open()) std::exit(-1);
		uint32_t size;
		in.read(reinterpret_cast<char*>(&size), sizeof(size));
		net.resize(size);
		for (weight& w : net) in >> w;
		in.close();
	}
	virtual void save_weights(const std::string& path) {
		std::ofstream out(path, std::ios::out | std::ios::binary | std::ios::trunc);
		if (!out.is_open()) std::exit(-1);
		uint32_t size = net.size();
		out.write(reinterpret_cast<char*>(&size), sizeof(size));
		for (weight& w : net) out << w;
		out.close();
	}

protected:
	std::vector<weight> net;
};

/**
 * base agent for agents with a learning rate
 */
class learning_agent : public agent {
public:
	float alpha;
	learning_agent(const std::string& args = "") : agent(args), alpha(0.1f) {
		if (meta.find("alpha") != meta.end())
			alpha = float(meta["alpha"]);
	}
	virtual ~learning_agent() {}
};
class rndenv : public random_agent {
public:

	rndenv(const std::string& args = "") : random_agent("name=random role=environment " + args){

		bag.clear();
		player_action = 4;
	}


	void clear_bag(void){
		this->bag.clear();
	}
	void set_player_action(int a){
		this->player_action = a;
	}
	virtual action take_action(const board& after, weight_agent& table){
		if(bag.empty()){
			bag.push_back(1);
			bag.push_back(2);
			bag.push_back(3);
			std::shuffle(bag.begin(), bag.end(), engine);
		}
		switch(player_action){
			case 0:
				space = {12, 13, 14, 15};
				break;
			case 1:
				space = {0, 4, 8, 12};
				break;
			case 2:
				space = {0, 1, 2, 3};
				break;
			case 3:
				space = {3, 7, 11, 15};
				break;
			case 4:
				ini_space = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
				std::shuffle(ini_space.begin(), ini_space.end(), engine);
				for(int pos : ini_space){
					if(after(pos) != 0)	continue;
					board::cell tile = bag.back();
					bag.pop_back();
					return action::place(pos,tile);
				}
				return action();//-1
				break;
			default:
				break;
		}
		std::shuffle(space.begin(), space.end(), engine);
		for(int pos : space){
			if(after(pos) != 0) continue;
			board::cell tile = bag.back();
			bag.pop_back();
			return action::place(pos, tile);
		}
		return action();

	}

private:

	int player_action;
	std::array<int, 16> ini_space;
	std::vector<int> bag;

	std::array<int , 4> space;
};
class player : public random_agent {
public:
	uint32_t get_player_choice(void){
		return this->choice;
	}

	player(const std::string& args = "") : random_agent("name=dummy role=player " + args), choice(4),
		opcode({ 0, 1, 2, 3 }) {}

	virtual action take_action(const board& before, weight_agent& table) {
		
		int best_choice = -1;
		float _min = -10000000.0f;
		for (int op : opcode) {
			board tmp(before);
			int score = tmp.slide(op);/* if score equals 0 represents it moves but doesn't merge*/
			if(score == -1){
				continue;
			}
			float val = evaluate(tmp, op, table, score);
			if(val > _min){
				best_choice = op;
				_min = val;
			}
		}
		/* 
		best_choice wont work well, because if it can merge, it will merge in the front.
		for here, all the choice will not merge.
		*/
		
		if(best_choice != -1){choice = best_choice; return action::slide(best_choice);}
		return action();
	}
	float evaluate(const board& tmp, int opcode, const weight_agent& table, board::reward reward){
		float sum = 0.0f;
		
			sum+=	table[0][board::index(tmp,0,1,2,3,4,5, 15)] ;
			sum+=	table[0][board::index(tmp,3,7,11,15,2,6, 15)] ;
			sum+=	table[0][board::index(tmp,15,14,13,12,11,10, 15)] ;
			sum+=	table[0][board::index(tmp,12,8,4,0,13,9, 15)] ;
			sum+=	table[0][board::index(tmp,3,2,1,0,7,6, 15)] ;
			sum+=	table[0][board::index(tmp,0,4,8,12,1,5, 15)] ;
			sum+=	table[0][board::index(tmp,12,13,14,15,8,9, 15)] ;
			sum+=	table[0][board::index(tmp,15,11,7,3,14,10, 15)] ;
			sum+=	table[1][board::index(tmp,4,5,6,7,8,9, 15)] ;
			sum+=	table[1][board::index(tmp,2,6,10,14,1,5, 15)] ;
			sum+=	table[1][board::index(tmp,11,10,9,8,7,6, 15)] ;
			sum+=	table[1][board::index(tmp,13,9,5,1,14,10, 15)] ;
			sum+=	table[1][board::index(tmp,7,6,5,4,11,10, 15)] ;
			sum+=	table[1][board::index(tmp,1,5,9,13,2,6, 15)] ;
			sum+=	table[1][board::index(tmp,8,9,10,11,4,5, 15)] ;
			sum+=	table[1][board::index(tmp,14,10,6,2,13,9, 15)] ;
			sum+=	table[2][board::index(tmp,0,1,2,4,5,6, 15)] ;
			sum+=	table[2][board::index(tmp,3,7,11,2,6,10, 15)] ;
			sum+=	table[2][board::index(tmp,15,14,13,11,10,9, 15)] ;
			sum+=	table[2][board::index(tmp,12,8,4,13,9,5, 15)] ;
			sum+=	table[2][board::index(tmp,3,2,1,7,6,5, 15)] ;
			sum+=	table[2][board::index(tmp,0,4,8,1,5,9, 15)] ;
			sum+=	table[2][board::index(tmp,12,13,14,8,9,10, 15)] ;
			sum+=	table[2][board::index(tmp,15,11,7,14,10,6, 15)] ;
			sum+=	table[3][board::index(tmp,4,5,6,8,9,10, 15)] ;
			sum+=	table[3][board::index(tmp,2,6,10,1,5,9, 15)] ;
			sum+=	table[3][board::index(tmp,11,10,9,7,6,5, 15)] ;
			sum+=	table[3][board::index(tmp,13,9,5,14,10,6, 15)] ;
			sum+=	table[3][board::index(tmp,7,6,5,11,10,9, 15)] ;
			sum+=	table[3][board::index(tmp,1,5,9,2,6,10, 15)] ;
			sum+=	table[3][board::index(tmp,8,9,10,4,5,6, 15)] ;
			sum+=	table[3][board::index(tmp,14,10,6,13,9,5, 15)] ;
		return (reward + sum);
	}
private:
	uint32_t choice;
	std::array<int, 4> opcode;
};
