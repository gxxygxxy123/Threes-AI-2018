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


#include <float.h>
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
	virtual action take_action(board& b) {return action(); }
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

protected:

};

/**
 * base agent for agents with weight tables
 */
class weight_agent : public learning_agent {
public:
	weight_agent(const std::string& args = "") : learning_agent(args) {
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
		// 11390625*5= 56953125
		// hint = 0 ~ 4 actually 0 is impossible
		net.emplace_back(56953125);// 012345
		net.emplace_back(56953125);// 456789
		net.emplace_back(56953125);// 012456
		net.emplace_back(56953125);// 45689a 
		for(size_t i = 0; i < net.size();i++){
			for(size_t j = 0;j < net[i].size();j++){
				net[i][j] = 0.0f;
			}
		}

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

class rndenv : public weight_agent {
public:
	board::cell hint;
	int initializing;
	std::array<uint32_t, 9> start_pos;
	std::array<board::cell, 10> start_tile;
	rndenv(const std::string& args = "") : weight_agent("name=random role=environment " + args), start_pos({0,1,3,4,5,7,9,13,14}), start_tile({1,3,2,3,2,1,2,3,3,  2}){
		initializing = 0;
	}

	void clear_bag(void){
		this->bag.clear();
	}
	
	void fill_bag(void){
		bag.clear();
		bag.push_back(1);
		bag.push_back(2);
		bag.push_back(3);

		bag.push_back(1);
		bag.push_back(2);
		bag.push_back(3);

		bag.push_back(1);
		bag.push_back(2);
		bag.push_back(3);

		bag.push_back(1);
		bag.push_back(2);
		bag.push_back(3);
	}
	std::vector<board::cell>& this_bag(void){
		return this->bag;
	}
	void set_player_action(int a){
		this->player_action = a;
	}
	float evaluate(const board& tmp, board::reward reward){
		float sum = 0.0f;
			sum+=	net[0][board::index(tmp,0,1,2,3,4,5, 15) * 5 + tmp.info()] ;
			sum+=	net[0][board::index(tmp,3,7,11,15,2,6, 15) * 5 + tmp.info()] ;
			sum+=	net[0][board::index(tmp,15,14,13,12,11,10, 15) * 5 + tmp.info()] ;
			sum+=	net[0][board::index(tmp,12,8,4,0,13,9, 15) * 5 + tmp.info()] ;
			sum+=	net[0][board::index(tmp,3,2,1,0,7,6, 15) * 5 + tmp.info()] ;
			sum+=	net[0][board::index(tmp,0,4,8,12,1,5, 15) * 5 + tmp.info()] ;
			sum+=	net[0][board::index(tmp,12,13,14,15,8,9, 15) * 5 + tmp.info()] ;
			sum+=	net[0][board::index(tmp,15,11,7,3,14,10, 15) * 5 + tmp.info()] ;
			sum+=	net[1][board::index(tmp,4,5,6,7,8,9, 15) * 5 + tmp.info()] ;
			sum+=	net[1][board::index(tmp,2,6,10,14,1,5, 15) * 5 + tmp.info()] ;
			sum+=	net[1][board::index(tmp,11,10,9,8,7,6, 15) * 5 + tmp.info()] ;
			sum+=	net[1][board::index(tmp,13,9,5,1,14,10, 15) * 5 + tmp.info()] ;
			sum+=	net[1][board::index(tmp,7,6,5,4,11,10, 15) * 5 + tmp.info()] ;
			sum+=	net[1][board::index(tmp,1,5,9,13,2,6, 15) * 5 + tmp.info()] ;
			sum+=	net[1][board::index(tmp,8,9,10,11,4,5, 15) * 5 + tmp.info()] ;
			sum+=	net[1][board::index(tmp,14,10,6,2,13,9, 15) * 5 + tmp.info()] ;
			sum+=	net[2][board::index(tmp,0,1,2,4,5,6, 15) * 5 + tmp.info()] ;
			sum+=	net[2][board::index(tmp,3,7,11,2,6,10, 15) * 5 + tmp.info()] ;
			sum+=	net[2][board::index(tmp,15,14,13,11,10,9, 15) * 5 + tmp.info()] ;
			sum+=	net[2][board::index(tmp,12,8,4,13,9,5, 15) * 5 + tmp.info()] ;
			sum+=	net[2][board::index(tmp,3,2,1,7,6,5, 15) * 5 + tmp.info()] ;
			sum+=	net[2][board::index(tmp,0,4,8,1,5,9, 15) * 5 + tmp.info()] ;
			sum+=	net[2][board::index(tmp,12,13,14,8,9,10, 15) * 5 + tmp.info()] ;
			sum+=	net[2][board::index(tmp,15,11,7,14,10,6, 15) * 5 + tmp.info()] ;
			sum+=	net[3][board::index(tmp,4,5,6,8,9,10, 15) * 5 + tmp.info()] ;
			sum+=	net[3][board::index(tmp,2,6,10,1,5,9, 15) * 5 + tmp.info()] ;
			sum+=	net[3][board::index(tmp,11,10,9,7,6,5, 15) * 5 + tmp.info()] ;
			sum+=	net[3][board::index(tmp,13,9,5,14,10,6, 15) * 5 + tmp.info()] ;
			sum+=	net[3][board::index(tmp,7,6,5,11,10,9, 15) * 5 + tmp.info()] ;
			sum+=	net[3][board::index(tmp,1,5,9,2,6,10, 15) * 5 + tmp.info()] ;
			sum+=	net[3][board::index(tmp,8,9,10,4,5,6, 15) * 5 + tmp.info()] ;
			sum+=	net[3][board::index(tmp,14,10,6,13,9,5, 15) * 5 + tmp.info()] ;
		return (reward + sum);
	}
	virtual void open_episode(const std::string& flag = "") {
		initializing = 0;
		bag.clear();
		bag.push_back(1);
		bag.push_back(2);
		bag.push_back(3);

		bag.push_back(1);
		bag.push_back(2);
		bag.push_back(3);

		bag.push_back(1);
		bag.push_back(2);
		bag.push_back(3);

		bag.push_back(1);
		bag.push_back(2);
		bag.push_back(3);
		player_action = 4;
		num_bonus_tile = 0;
		num_gen_tile = 0;
		//
		hint = start_tile[0];
		this->fill_bag();
		this->set_player_action(4);
		for(size_t i = 0;i < bag.size();i++){
			if(bag[i] == start_tile[0]){
				bag.erase(bag.begin()+i); // remove first element
				break;
			}
		}

	}

	virtual action take_action(board& after){
		if(initializing<9){

			this->set_player_action(4);
			initializing++;
		}
		else{
			this->set_player_action(after.oppo_player_move);
		}
		uint32_t biggest = 0;
		for(int i = 0;i < 16;i++){
			if(after(i) > biggest)
					biggest = after(i);
		}

		if(bag.empty()){
			bag.push_back(1);
			bag.push_back(2);
			bag.push_back(3);

			bag.push_back(1);
			bag.push_back(2);
			bag.push_back(3);

			bag.push_back(1);
			bag.push_back(2);
			bag.push_back(3);

			bag.push_back(1);
			bag.push_back(2);
			bag.push_back(3);
		}
		switch(player_action){
			case 0:
				if(space.size() != 4)
					space.resize(4);
				space[0] = 12;
				space[1] = 13;
				space[2] = 14;
				space[3] = 15;
				break;
			case 1:
				if(space.size() != 4)
					space.resize(4);
				space[0] = 0;
				space[1] = 4;
				space[2] = 8;
				space[3] = 12;
				break;
			case 2:
				if(space.size() != 4)
					space.resize(4);
				space[0] = 0;
				space[1] = 1;
				space[2] = 2;
				space[3] = 3;
				break;
			case 3:
				if(space.size() != 4)
					space.resize(4);
				space[0] = 3;
				space[1] = 7;
				space[2] = 11;
				space[3] = 15;
				break;
			case 4:
				num_gen_tile++;
				after.info(start_tile[initializing]);
				hint = start_tile[initializing];
				if(hint == 1 || hint == 2 || hint == 3){
					for(size_t i = 0;i < bag.size();i++){
						if(bag[i] == hint){
							bag.erase(bag.begin()+i);
							break;
						}
					}
				}
				return action::place(start_pos[initializing-1], start_tile[initializing-1]);
				break;
			default:
				break;
		}
		int best_pos = -1;
		board::cell best_tile = hint, best_hint = bag.back(); // warning : maybe uninitialized, so initialize it with some number
		if(hint == 4){
			num_gen_tile++;
			num_bonus_tile++;
			bool hint_available[4] = {false, false, false, false};
			if(biggest >= 7 && (float)(num_bonus_tile+1)/(num_gen_tile+1) <= 1.0/21.0)	hint_available[3] = true;
			for(size_t i = 0;i < bag.size();i++){
				hint_available[ bag[i]-1 ] = true;
			}
			float _max = FLT_MAX;
			for(board::cell possible_tile = 4;possible_tile <= biggest-3;possible_tile++){
				for(int pos : space){
					for(int next_hint = 1;next_hint <= 4;next_hint++){
						if(hint_available[next_hint-1] && after(pos) == 0){
							float player_score = FLT_MIN;
							for(int op = 0;op < 4;op++){
								board tmp(after);
								tmp(pos) = possible_tile;
								int score = tmp.slide(op);
								if(score == -1) continue;
								tmp.info(next_hint);
								float val = evaluate(tmp, score);
								if(val > player_score){
									player_score = val;
								}
							}
							if(player_score < _max){
								best_pos = pos;
								best_hint = next_hint;
								best_tile = possible_tile;
								_max = player_score;
							}
						}
					}
				}
			}
		}
		else{
			best_tile = hint;
			num_gen_tile++;
			bool hint_available[4] = {false, false, false, false};
			if(biggest >= 7 && (float)(num_bonus_tile+1)/(num_gen_tile+1) <= 1.0/21.0)	hint_available[3] = true;
			for(size_t i = 0;i < bag.size();i++){
				hint_available[ bag[i]-1 ] = true;
			}
			float _max = FLT_MAX;
			for(int pos : space){
				for(int next_hint = 1;next_hint <= 4;next_hint++){
					if(hint_available[next_hint-1] && after(pos) == 0){
						float player_score = FLT_MIN;
						for(int op = 0;op < 4;op++){
							board tmp(after);
							tmp(pos) = best_tile;
							int score = tmp.slide(op);
							if(score == -1) continue;
							tmp.info(next_hint);
							float val = evaluate(tmp, score);
							if(val > player_score){
								player_score = val;
							}
						}
						if(player_score < _max){
							best_pos = pos;
							best_hint = next_hint;
							_max = player_score;
						}
					}
				}
			}
		}
		if(best_hint == 1 || best_hint == 2 || best_hint == 3){
			for(size_t i = 0;i < bag.size();i++){
				if(bag[i] == best_hint){
					bag.erase(bag.begin()+i);
					break;
				}
			}
		}
		hint = best_hint;
		after.info(hint);
		return action::place(best_pos, best_tile);
		return action();

	}

private:
	int player_action;
	std::vector<board::cell> bag;
	int num_bonus_tile;
	int num_gen_tile;
	std::vector<int> space;
	//std::random_device rd;
	//std::mt19937 gen;
	//std::uniform_int_distrubution<> bonus_tile;
	//std::uniform_int_distribution<> bonus_prob;
	std::default_random_engine engine;
};
class player : public weight_agent {
public:
	uint32_t get_player_choice(void){
		return this->choice;
	}
	player(const std::string& args = "") : weight_agent("name=dummy role=player " + args), /* My*/ choice(4),/* My end*/
		opcode({ 0, 1, 2, 3 }) {}
	virtual void open_episode(const std::string& flag = ""){
		choice = 4;
	}
	virtual action take_action(board& before) {
		////////////////////
		// below is original version
		
		int best_choice = -1;
		float _min = FLT_MIN; // origin -1000000000.0f
		for (int op : opcode) {
			board tmp(before);
			int score = tmp.slide(op);
			if(score == -1){
				continue;
			}

			float val = evaluate(tmp, score);

			if(val > _min){
				best_choice = op;
				_min = val;
			}
		}


		if(best_choice != -1){choice = best_choice; return action::slide(best_choice);}
		return action();
		
	}
	// new
	float evaluate(const board& tmp, board::reward reward){
		float sum = 0.0f;
			sum+=	net[0][board::index(tmp,0,1,2,3,4,5, 15) * 5 + tmp.info()] ;
			sum+=	net[0][board::index(tmp,3,7,11,15,2,6, 15) * 5 + tmp.info()] ;
			sum+=	net[0][board::index(tmp,15,14,13,12,11,10, 15) * 5 + tmp.info()] ;
			sum+=	net[0][board::index(tmp,12,8,4,0,13,9, 15) * 5 + tmp.info()] ;
			sum+=	net[0][board::index(tmp,3,2,1,0,7,6, 15) * 5 + tmp.info()] ;
			sum+=	net[0][board::index(tmp,0,4,8,12,1,5, 15) * 5 + tmp.info()] ;
			sum+=	net[0][board::index(tmp,12,13,14,15,8,9, 15) * 5 + tmp.info()] ;
			sum+=	net[0][board::index(tmp,15,11,7,3,14,10, 15) * 5 + tmp.info()] ;
			sum+=	net[1][board::index(tmp,4,5,6,7,8,9, 15) * 5 + tmp.info()] ;
			sum+=	net[1][board::index(tmp,2,6,10,14,1,5, 15) * 5 + tmp.info()] ;
			sum+=	net[1][board::index(tmp,11,10,9,8,7,6, 15) * 5 + tmp.info()] ;
			sum+=	net[1][board::index(tmp,13,9,5,1,14,10, 15) * 5 + tmp.info()] ;
			sum+=	net[1][board::index(tmp,7,6,5,4,11,10, 15) * 5 + tmp.info()] ;
			sum+=	net[1][board::index(tmp,1,5,9,13,2,6, 15) * 5 + tmp.info()] ;
			sum+=	net[1][board::index(tmp,8,9,10,11,4,5, 15) * 5 + tmp.info()] ;
			sum+=	net[1][board::index(tmp,14,10,6,2,13,9, 15) * 5 + tmp.info()] ;
			sum+=	net[2][board::index(tmp,0,1,2,4,5,6, 15) * 5 + tmp.info()] ;
			sum+=	net[2][board::index(tmp,3,7,11,2,6,10, 15) * 5 + tmp.info()] ;
			sum+=	net[2][board::index(tmp,15,14,13,11,10,9, 15) * 5 + tmp.info()] ;
			sum+=	net[2][board::index(tmp,12,8,4,13,9,5, 15) * 5 + tmp.info()] ;
			sum+=	net[2][board::index(tmp,3,2,1,7,6,5, 15) * 5 + tmp.info()] ;
			sum+=	net[2][board::index(tmp,0,4,8,1,5,9, 15) * 5 + tmp.info()] ;
			sum+=	net[2][board::index(tmp,12,13,14,8,9,10, 15) * 5 + tmp.info()] ;
			sum+=	net[2][board::index(tmp,15,11,7,14,10,6, 15) * 5 + tmp.info()] ;
			sum+=	net[3][board::index(tmp,4,5,6,8,9,10, 15) * 5 + tmp.info()] ;
			sum+=	net[3][board::index(tmp,2,6,10,1,5,9, 15) * 5 + tmp.info()] ;
			sum+=	net[3][board::index(tmp,11,10,9,7,6,5, 15) * 5 + tmp.info()] ;
			sum+=	net[3][board::index(tmp,13,9,5,14,10,6, 15) * 5 + tmp.info()] ;
			sum+=	net[3][board::index(tmp,7,6,5,11,10,9, 15) * 5 + tmp.info()] ;
			sum+=	net[3][board::index(tmp,1,5,9,2,6,10, 15) * 5 + tmp.info()] ;
			sum+=	net[3][board::index(tmp,8,9,10,4,5,6, 15) * 5 + tmp.info()] ;
			sum+=	net[3][board::index(tmp,14,10,6,13,9,5, 15) * 5 + tmp.info()] ;
		return (reward + sum);
	}
	// new end
private:
	uint32_t choice;
	std::array<int, 4> opcode;
};
