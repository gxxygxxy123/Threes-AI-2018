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
	virtual action take_action(const board& b) { return action(); }
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

protected:
	virtual void init_weights(const std::string& info) {

		net.emplace_back(50625); // 15^4 = 50625
		net.emplace_back(50625);

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
	learning_agent(const std::string& args = "") : agent(args), alpha(0.1f) {
		if (meta.find("alpha") != meta.end())
			alpha = float(meta["alpha"]);
	}
	virtual ~learning_agent() {}

protected:
	float alpha;
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
	virtual action take_action(const board& after){
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

	player(const std::string& args = "") : random_agent("name=dummy role=player " + args),/* My*/ choice(4),/* My end*/
		opcode({ 0, 1, 3, 2 }) {}// Origin is 0 1 2 3 but for greedy 0 1 3 2 is better

	virtual action take_action(const board& before) {
		int best_choice = -1, _max = -1;
		//std::shuffle(opcode.begin(), opcode.end(), engine);
		for (int op : opcode) {
			int score = board(before).slide(op);/* if score equals 0 represents it moves but doesn't merge*/
			if(score > 0){choice = op; return action::slide(op);}//if it can merge then do
			if(score != -1){// it can move
				if(score > _max){
					best_choice = op;
					_max = score;
				}
			}
		}
		/* 
		best_choice wont work well, because if it can merge, it will merge in the front.
		for here, all the choice will not merge.
		*/
		if(best_choice != -1){choice = best_choice; return action::slide(best_choice);}
		return action();
	}
private:
	uint32_t choice;
	std::array<int, 4> opcode;
};
