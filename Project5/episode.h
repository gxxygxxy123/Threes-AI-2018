#pragma once
#include <list>
#include <vector>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <chrono>
#include <numeric>
#include "board.h"
#include "action.h"
#include "agent.h"

#include <cmath>

class statistic;

class episode {
friend class statistic;
public:
	episode() : ep_state(initial_state()), ep_score(0), ep_time(0) { ep_moves.reserve(10000);}

public:
	// new
	/*
	void update(weight_agent& table, const learning_agent& alpha_agent){
		for(unsigned i = ep_moves.size()-2;i >= 9; i-=2){
			//std::cout << ep_moves[i].reward<<'\n';
			const board &a = ep_moves[i].state;
			float error;
			if(i == ep_moves.size()-2){

				error = -value(ep_moves[i].state, table);
				error = error * alpha_agent.alpha  / (8*4*6) ;

				table[0][board::index(a,0,1,2,3,4,5, 15) * 5 + a.info()] += error;
				table[0][board::index(a,3,7,11,15,2,6, 15) * 5 + a.info()] += error;
				table[0][board::index(a,15,14,13,12,11,10, 15) * 5 + a.info()] += error;
				table[0][board::index(a,12,8,4,0,13,9, 15) * 5 + a.info()] += error;
				table[0][board::index(a,3,2,1,0,7,6, 15) * 5 + a.info()] += error;
				table[0][board::index(a,0,4,8,12,1,5, 15) * 5 + a.info()] += error;
				table[0][board::index(a,12,13,14,15,8,9, 15) * 5 + a.info()] += error;
				table[0][board::index(a,15,11,7,3,14,10, 15) * 5 + a.info()] += error;
				table[1][board::index(a,4,5,6,7,8,9, 15) * 5 + a.info()] += error;
				table[1][board::index(a,2,6,10,14,1,5, 15) * 5 + a.info()] += error;
				table[1][board::index(a,11,10,9,8,7,6, 15) * 5 + a.info()] += error;
				table[1][board::index(a,13,9,5,1,14,10, 15) * 5 + a.info()] += error;
				table[1][board::index(a,7,6,5,4,11,10, 15) * 5 + a.info()] += error;
				table[1][board::index(a,1,5,9,13,2,6, 15) * 5 + a.info()] += error;
				table[1][board::index(a,8,9,10,11,4,5, 15) * 5 + a.info()] += error;
				table[1][board::index(a,14,10,6,2,13,9, 15) * 5 + a.info()] += error;
				table[2][board::index(a,0,1,2,4,5,6, 15) * 5 + a.info()] += error;
				table[2][board::index(a,3,7,11,2,6,10, 15) * 5 + a.info()] += error;
				table[2][board::index(a,15,14,13,11,10,9, 15) * 5 + a.info()] += error;
				table[2][board::index(a,12,8,4,13,9,5, 15) * 5 + a.info()] += error;
				table[2][board::index(a,3,2,1,7,6,5, 15) * 5 + a.info()] += error;
				table[2][board::index(a,0,4,8,1,5,9, 15) * 5 + a.info()] += error;
				table[2][board::index(a,12,13,14,8,9,10, 15) * 5 + a.info()] += error;
				table[2][board::index(a,15,11,7,14,10,6, 15) * 5 + a.info()] += error;
				table[3][board::index(a,4,5,6,8,9,10, 15) * 5 + a.info()] += error;
				table[3][board::index(a,2,6,10,1,5,9, 15) * 5 + a.info()] += error;
				table[3][board::index(a,11,10,9,7,6,5, 15) * 5 + a.info()] += error;
				table[3][board::index(a,13,9,5,14,10,6, 15) * 5 + a.info()] += error;
				table[3][board::index(a,7,6,5,11,10,9, 15) * 5 + a.info()] += error;
				table[3][board::index(a,1,5,9,2,6,10, 15) * 5 + a.info()] += error;
				table[3][board::index(a,8,9,10,4,5,6, 15) * 5 + a.info()] += error;
				table[3][board::index(a,14,10,6,13,9,5, 15) * 5 + a.info()] += error;
				
			}
			else{
				error = ep_moves[i+2].reward + value(ep_moves[i+2].state,table) - value(ep_moves[i].state,table);
				error = error * alpha_agent.alpha  / (8*4*6) ;
				table[0][board::index(a,0,1,2,3,4,5, 15) * 5 + a.info()] += error;
				table[0][board::index(a,3,7,11,15,2,6, 15) * 5 + a.info()] += error;
				table[0][board::index(a,15,14,13,12,11,10, 15) * 5 + a.info()] += error;
				table[0][board::index(a,12,8,4,0,13,9, 15) * 5 + a.info()] += error;
				table[0][board::index(a,3,2,1,0,7,6, 15) * 5 + a.info()] += error;
				table[0][board::index(a,0,4,8,12,1,5, 15) * 5 + a.info()] += error;
				table[0][board::index(a,12,13,14,15,8,9, 15) * 5 + a.info()] += error;
				table[0][board::index(a,15,11,7,3,14,10, 15) * 5 + a.info()] += error;
				table[1][board::index(a,4,5,6,7,8,9, 15) * 5 + a.info()] += error;
				table[1][board::index(a,2,6,10,14,1,5, 15) * 5 + a.info()] += error;
				table[1][board::index(a,11,10,9,8,7,6, 15) * 5 + a.info()] += error;
				table[1][board::index(a,13,9,5,1,14,10, 15) * 5 + a.info()] += error;
				table[1][board::index(a,7,6,5,4,11,10, 15) * 5 + a.info()] += error;
				table[1][board::index(a,1,5,9,13,2,6, 15) * 5 + a.info()] += error;
				table[1][board::index(a,8,9,10,11,4,5, 15) * 5 + a.info()] += error;
				table[1][board::index(a,14,10,6,2,13,9, 15) * 5 + a.info()] += error;
				table[2][board::index(a,0,1,2,4,5,6, 15) * 5 + a.info()] += error;
				table[2][board::index(a,3,7,11,2,6,10, 15) * 5 + a.info()] += error;
				table[2][board::index(a,15,14,13,11,10,9, 15) * 5 + a.info()] += error;
				table[2][board::index(a,12,8,4,13,9,5, 15) * 5 + a.info()] += error;
				table[2][board::index(a,3,2,1,7,6,5, 15) * 5 + a.info()] += error;
				table[2][board::index(a,0,4,8,1,5,9, 15) * 5 + a.info()] += error;
				table[2][board::index(a,12,13,14,8,9,10, 15) * 5 + a.info()] += error;
				table[2][board::index(a,15,11,7,14,10,6, 15) * 5 + a.info()] += error;
				table[3][board::index(a,4,5,6,8,9,10, 15) * 5 + a.info()] += error;
				table[3][board::index(a,2,6,10,1,5,9, 15) * 5 + a.info()] += error;
				table[3][board::index(a,11,10,9,7,6,5, 15) * 5 + a.info()] += error;
				table[3][board::index(a,13,9,5,14,10,6, 15) * 5 + a.info()] += error;
				table[3][board::index(a,7,6,5,11,10,9, 15) * 5 + a.info()] += error;
				table[3][board::index(a,1,5,9,2,6,10, 15) * 5 + a.info()] += error;
				table[3][board::index(a,8,9,10,4,5,6, 15) * 5 + a.info()] += error;
				table[3][board::index(a,14,10,6,13,9,5, 15) * 5 + a.info()] += error;
			}
			//std::cout << error <<'\n';
		}
	}

	float value(const board& state, const weight_agent& table){
		float sum = 0;
			sum+=	table[0][board::index(state,0,1,2,3,4,5, 15) * 5 + state.info()] ;
			sum+=	table[0][board::index(state,3,7,11,15,2,6, 15) * 5 + state.info()] ;
			sum+=	table[0][board::index(state,15,14,13,12,11,10, 15) * 5 + state.info()] ;
			sum+=	table[0][board::index(state,12,8,4,0,13,9, 15) * 5 + state.info()] ;
			sum+=	table[0][board::index(state,3,2,1,0,7,6, 15) * 5 + state.info()] ;
			sum+=	table[0][board::index(state,0,4,8,12,1,5, 15) * 5 + state.info()] ;
			sum+=	table[0][board::index(state,12,13,14,15,8,9, 15) * 5 + state.info()] ;
			sum+=	table[0][board::index(state,15,11,7,3,14,10, 15) * 5 + state.info()] ;
			sum+=	table[1][board::index(state,4,5,6,7,8,9, 15) * 5 + state.info()] ;
			sum+=	table[1][board::index(state,2,6,10,14,1,5, 15) * 5 + state.info()] ;
			sum+=	table[1][board::index(state,11,10,9,8,7,6, 15) * 5 + state.info()] ;
			sum+=	table[1][board::index(state,13,9,5,1,14,10, 15) * 5 + state.info()] ;
			sum+=	table[1][board::index(state,7,6,5,4,11,10, 15) * 5 + state.info()] ;
			sum+=	table[1][board::index(state,1,5,9,13,2,6, 15) * 5 + state.info()] ;
			sum+=	table[1][board::index(state,8,9,10,11,4,5, 15) * 5 + state.info()] ;
			sum+=	table[1][board::index(state,14,10,6,2,13,9, 15) * 5 + state.info()] ;
			sum+=	table[2][board::index(state,0,1,2,4,5,6, 15) * 5 + state.info()] ;
			sum+=	table[2][board::index(state,3,7,11,2,6,10, 15) * 5 + state.info()] ;
			sum+=	table[2][board::index(state,15,14,13,11,10,9, 15) * 5 + state.info()] ;
			sum+=	table[2][board::index(state,12,8,4,13,9,5, 15) * 5 + state.info()] ;
			sum+=	table[2][board::index(state,3,2,1,7,6,5, 15) * 5 + state.info()] ;
			sum+=	table[2][board::index(state,0,4,8,1,5,9, 15) * 5 + state.info()] ;
			sum+=	table[2][board::index(state,12,13,14,8,9,10, 15) * 5 + state.info()] ;
			sum+=	table[2][board::index(state,15,11,7,14,10,6, 15) * 5 + state.info()] ;
			sum+=	table[3][board::index(state,4,5,6,8,9,10, 15) * 5 + state.info()] ;
			sum+=	table[3][board::index(state,2,6,10,1,5,9, 15) * 5 + state.info()] ;
			sum+=	table[3][board::index(state,11,10,9,7,6,5, 15) * 5 + state.info()] ;
			sum+=	table[3][board::index(state,13,9,5,14,10,6, 15) * 5 + state.info()] ;
			sum+=	table[3][board::index(state,7,6,5,11,10,9, 15) * 5 + state.info()] ;
			sum+=	table[3][board::index(state,1,5,9,2,6,10, 15) * 5 + state.info()] ;
			sum+=	table[3][board::index(state,8,9,10,4,5,6, 15) * 5 + state.info()] ;
			sum+=	table[3][board::index(state,14,10,6,13,9,5, 15) * 5 + state.info()] ;

		return sum;
	}
	*/
	// new end
	board& state() { return ep_state; }
	const board& state() const { return ep_state; }
	board::reward score() const{
		return ep_score;
	}
	void open_episode(const std::string& tag) {
		ep_open = { tag, millisec() };
	}
	void close_episode(const std::string& tag) {
		ep_close = { tag, millisec() };
	}
	bool apply_action(action move) {
		board::reward reward = move.apply(state());
		if (reward == -1) return false;
		//std::cout << reward << '\n';
		ep_moves.emplace_back(state(), move, reward, millisec() - ep_time);
		ep_score += reward;
		return true;
	}
	agent& take_turns(agent& play, agent& evil) {
		ep_time = millisec();
		return (std::max(step() + 1, size_t(9)) % 2) ? evil : play;
	}
	agent& last_turns(agent& play, agent& evil) {
		return take_turns(evil, play);
	}

public:
	size_t step(unsigned who = -1u) const {
		int size = ep_moves.size(); // 'int' is important for handling 0
		switch (who) {
		case action::slide::type: return (size - 1) / 2;
		case action::place::type: return (size - (size - 1) / 2);
		default:                  return size;
		}
	}

	time_t time(unsigned who = -1u) const {
		time_t time = 0;
		size_t i = 2;
		switch (who) {
		case action::place::type:
			if (ep_moves.size()) time += ep_moves[0].time, i = 1;
			// no break;
		case action::slide::type:
			while (i < ep_moves.size()) time += ep_moves[i].time, i += 2;
			break;
		default:
			time = ep_close.when - ep_open.when;
			break;
		}
		return time;
	}

	std::vector<action> actions(unsigned who = -1u) const {
		std::vector<action> res;
		size_t i = 2;
		switch (who) {
		case action::place::type:
			if (ep_moves.size()) res.push_back(ep_moves[0]), i = 1;
			// no break;
		case action::slide::type:
			while (i < ep_moves.size()) res.push_back(ep_moves[i]), i += 2;
			break;
		default:
			res.assign(ep_moves.begin(), ep_moves.end());
			break;
		}
		return res;
	}

public:

	friend std::ostream& operator <<(std::ostream& out, const episode& ep) {
		out << ep.ep_open << '|';
		for (const move& mv : ep.ep_moves) out << mv;
		out << '|' << ep.ep_close;
		return out;
	}
	friend std::istream& operator >>(std::istream& in, episode& ep) {
		ep = {};
		std::string token;
		std::getline(in, token, '|');
		std::stringstream(token) >> ep.ep_open;
		std::getline(in, token, '|');
		for (std::stringstream moves(token); !moves.eof(); moves.peek()) {
			ep.ep_moves.emplace_back();
			moves >> ep.ep_moves.back();
			ep.ep_score += action(ep.ep_moves.back()).apply(ep.ep_state);
		}
		std::getline(in, token, '|');
		std::stringstream(token) >> ep.ep_close;
		return in;
	}

protected:

	struct move {
		// new
		board state;
		// new end
		action code;
		board::reward reward;
		time_t time;
		move(board state = {}, action code = {}, board::reward reward = 0, time_t time = 0) : state(state), code(code), reward(reward), time(time) {}

		operator action() const { return code; }
		friend std::ostream& operator <<(std::ostream& out, const move& m) {
			out << m.code;
			if (m.reward) out << '[' << std::dec << m.reward << ']';
			if (m.time) out << '(' << std::dec << m.time << ')';
			return out;
		}
		friend std::istream& operator >>(std::istream& in, move& m) {
			in >> m.code;
			m.reward = 0;
			m.time = 0;
			if (in.peek() == '[') {
				in.ignore(1);
				in >> std::dec >> m.reward;
				in.ignore(1);
			}
			if (in.peek() == '(') {
				in.ignore(1);
				in >> std::dec >> m.time;
				in.ignore(1);
			}
			return in;
		}
	};

	struct meta {
		std::string tag;
		time_t when;
		meta(const std::string& tag = "N/A", time_t when = 0) : tag(tag), when(when) {}

		friend std::ostream& operator <<(std::ostream& out, const meta& m) {
			return out << m.tag << "@" << std::dec << m.when;
		}
		friend std::istream& operator >>(std::istream& in, meta& m) {
			return std::getline(in, m.tag, '@') >> std::dec >> m.when;
		}
	};

	static board initial_state() {
		return {};
	}
	static time_t millisec() {
		auto now = std::chrono::system_clock::now().time_since_epoch();
		return std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
	}

private:
	board ep_state;
	board::reward ep_score;
	std::vector<move> ep_moves;
	time_t ep_time;

	meta ep_open;
	meta ep_close;
};
