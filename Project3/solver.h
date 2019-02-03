#pragma once
#include <iostream>
#include <algorithm>
#include <cmath>
#include "board.h"
#include <numeric>
#include <limits.h>
#include <float.h>
#include <set>
#include <vector>
#include <iomanip>
#include <unistd.h>
class state_type {
public:
	enum type : char {
		before  = 'b',
		after   = 'a',
		illegal = 'i'
	};
	type t;
public:
	state_type() : t(illegal) {}
	state_type(const state_type& st) = default;
	state_type(state_type::type code) : t(code) {}

	friend std::istream& operator >>(std::istream& in, state_type& type) {
		std::string s;
		if (in >> s) type.t = static_cast<state_type::type>((s + " ").front());
		return in;
	}

	friend std::ostream& operator <<(std::ostream& out, const state_type& type) {
		return out << char(type.t);
	}

	bool is_before()  const { return t == before; }
	bool is_after()   const { return t == after; }
	bool is_illegal() const { return t == illegal; }

private:
	//type t;
};

class state_hint {
public:
	state_hint(const board& state) : state(const_cast<board&>(state)) {}

	char type() const { return state.info() ? state.info() + '0' : 'x'; }
	operator board::cell() const { return state.info(); }

public:
	friend std::istream& operator >>(std::istream& in, state_hint& hint) {
		while (in.peek() != '+' && in.good()) in.ignore(1);
		char v; in.ignore(1) >> v;
		hint.state.info(v != 'x' ? v - '0' : 0);
		return in;
	}
	friend std::ostream& operator <<(std::ostream& out, const state_hint& hint) {
		return out << "+" << hint.type();
	}

private:
	board& state;
};

class state{
public:
	state_type t;
	board b;
	int last_action;// 0(up) 1(right) 2 (down) 3(left)
	bool bag[3];
	state():  t(static_cast<state_type::type>('a')), b(0,0,0,0,0,0,0), last_action(0){// empty state(root) after state
		//t.t = static_cast<state_type::type>('a');
		bag[0] = true;
		bag[1] = true;
		bag[2] = true;
	}

};

class solver {
public:
	typedef float value_t;
public:
	class answer {
	public:
		answer(int min = INT_MAX, value_t avg = 0.0/0.0, int max = INT_MIN) : min(min), avg(avg), max(max) {}
	    friend std::ostream& operator <<(std::ostream& out, const answer& ans) {
	    	return !std::isnan(ans.avg) ? (out << std::setprecision(7)<< ans.min << " " << ans.avg << " " << ans.max) : (out << "-1");
		}
	public:
		int min;
		value_t avg;
		int max;
	};

public:
	solver(const std::string& args) {
		before_table.resize(531441, std::vector<answer>(4));
		after_table.resize(531441, std::vector<answer>(16));
		std::vector< std::vector<answer> >::iterator it;
		std::vector<answer>::iterator p;
		for(it = before_table.begin();it != before_table.end();it++){
			for(p = (*it).begin();p !=(*it).end();p++){
				(*p).min = INT_MAX;
				(*p).avg = 0.0/0.0;// nan
				(*p).max = INT_MIN;
			}
		}
		for(it = after_table.begin();it != after_table.end();it++){
			for(p = (*it).begin();p !=(*it).end();p++){
				(*p).min = INT_MAX;
				(*p).avg = 0.0/0.0;
				(*p).max = INT_MIN;
			}
		}
		state now; // empty state
		for(int i = 1;i <= 3;i++){ // put i
			for(int j = 1;j <= 3;j++){// next_hint
				if(i == j) continue;
				for(int k = 0;k < 6;k++){ // put at k
					for(int last = 0;last < 4;last++){// last_action = 0~3 actually unneccassary
						state tmp(now);
						tmp.t = static_cast<state_type::type>('b');

						tmp.bag[0] = true; tmp.bag[1] = true; tmp.bag[2] = true;

						tmp.b.info(j);
						tmp.last_action = last;
						tmp.bag[i-1] = false; // take out
						tmp.b.place(k, i); // put in
						//?? tmp.last_action = ?
						DFS(tmp);
					}

				}
			}
		}
	}
	answer DFS(state now){// keep track of type board & hint last_action
		if(now.t.is_after()){// computer puts
			if(after_table[now.b(0)+now.b(1)*9+now.b(2)*9*9+now.b(3)*9*9*9+now.b(4)*9*9*9*9+now.b(5)*9*9*9*9*9][now.b.info() + now.last_action*4].min != INT_MAX){
				return after_table[now.b(0)+now.b(1)*9+now.b(2)*9*9+now.b(3)*9*9*9+now.b(4)*9*9*9*9+now.b(5)*9*9*9*9*9][now.b.info() + now.last_action*4];
			}
			int min = INT_MAX, max = INT_MIN;
			value_t sum = 0.0;
			int cnt = 0;
			int i = now.b.info(); // put i absolutely
			std::set<int> pos;
			pos.clear();
			switch(now.last_action){
				case 0:
					pos = {3,4,5};
					break;
				case 1:
					pos = {0,3};
					break;
				case 2:
					pos = {0,1,2};
					break;
				case 3:
					pos = {2,5};
					break;
				default:
					std::cerr << "Error" << '\n'; 
					break;
			}
			std::set<int>::iterator it;
			for(it = pos.begin(); it != pos.end(); it++){ // put at *it
				if(now.b(*it) != 0) continue;// something inside the tile
				state tmp(now);
				tmp.t = static_cast<state_type::type>('b');
				tmp.bag[i-1] = false; // take out of the bag
				tmp.b.place(*it, i); // put
				if((!tmp.bag[0]) && (!tmp.bag[1]) && (!tmp.bag[2])){
					tmp.bag[0] = true;
					tmp.bag[1] = true;
					tmp.bag[2] = true;
				}
				for(int j = 1;j <= 3;j++){
					if(tmp.bag[j-1] == false) continue;
					tmp.b.info(j);
					answer rec = DFS(tmp);
					min = (rec.min < min) ? rec.min : min;
					max = (rec.max > max) ? rec.max : max;
					cnt++;
					sum += rec.avg;					
				}
			}
			answer ret;
			ret.min = min;
			ret.avg = sum/(float)cnt;
			ret.max = max;
			after_table[now.b(0)+now.b(1)*9+now.b(2)*9*9+now.b(3)*9*9*9+now.b(4)*9*9*9*9+now.b(5)*9*9*9*9*9][now.b.info() + now.last_action*4] = ret;
			return ret;
		}
		else if(now.t.is_before()){//player moves
			if(before_table[now.b(0)+now.b(1)*9+now.b(2)*9*9+now.b(3)*9*9*9+now.b(4)*9*9*9*9+now.b(5)*9*9*9*9*9][now.b.info()].min != INT_MAX){
				return before_table[now.b(0)+now.b(1)*9+now.b(2)*9*9+now.b(3)*9*9*9+now.b(4)*9*9*9*9+now.b(5)*9*9*9*9*9][now.b.info()];
			}
			bool is_leaf = true;
			int min, max;
			value_t avg, flag_avg = FLT_MIN;

			for(int op = 0;op < 4;op++){// up right down left
				state tmp(now);
				tmp.t = static_cast<state_type::type>('a');
				if(tmp.b.slide(op) == -1) {
					continue;
				} // cant move
				tmp.last_action = op;
				answer rec = DFS(tmp);
				if(rec.avg > flag_avg){
					min = rec.min;
					avg = rec.avg;
					max = rec.max;
					flag_avg = rec.avg;
				}
				is_leaf = false;
			}
			if(is_leaf){
				//sleep(5);
				board::reward a = now.b.current_score();
				before_table[now.b(0)+now.b(1)*9+now.b(2)*9*9+now.b(3)*9*9*9+now.b(4)*9*9*9*9+now.b(5)*9*9*9*9*9][now.b.info()].min = a;
				before_table[now.b(0)+now.b(1)*9+now.b(2)*9*9+now.b(3)*9*9*9+now.b(4)*9*9*9*9+now.b(5)*9*9*9*9*9][now.b.info()].avg = a;
				before_table[now.b(0)+now.b(1)*9+now.b(2)*9*9+now.b(3)*9*9*9+now.b(4)*9*9*9*9+now.b(5)*9*9*9*9*9][now.b.info()].max = a;
				answer ret;
				ret.min = a;
				ret.avg = a;
				ret.max = a;
				return ret;
			}
			else{
				answer ret;
				ret.min = min;
				ret.avg = avg;
				ret.max = max;
				before_table[now.b(0)+now.b(1)*9+now.b(2)*9*9+now.b(3)*9*9*9+now.b(4)*9*9*9*9+now.b(5)*9*9*9*9*9][now.b.info()] = ret;
				return ret;
			}
		}
		//error!
		std::cerr << "error"<<'\n';
		answer ret;
		ret.min = INT_MAX;
		ret.avg = 0.0/0.0;
		ret.max = INT_MIN;
		return ret;
	}
	answer solve(const board& state, state_type type) {
		//       do NOT recalculate the tree at here
		for(int i = 0;i < 6;i++){
			if(state.legal[i] == false){
				return {};// invalid input
			}
		}
		if(type.is_before()){
			answer ret = before_table[state(0)+state(1)*9+state(2)*9*9+state(3)*9*9*9+state(4)*9*9*9*9+state(5)*9*9*9*9*9][(int)(state.info())];
			if(ret.min == INT_MAX)
				return {};
			else
				return ret;
		}
		else if(type.is_after()){
			for(int i = 0;i < 4;i++){
				if(after_table[(state(0)+state(1)*9+state(2)*9*9+state(3)*9*9*9+state(4)*9*9*9*9+state(5)*9*9*9*9*9)][(int)(state.info()) + i*4].min != INT_MAX){
					return after_table[(int)(state(0)+state(1)*9+state(2)*9*9+state(3)*9*9*9+state(4)*9*9*9*9+state(5)*9*9*9*9*9)][(int)(state.info()) + i*4];
				}
			}
			return {};
		}
		// to fetch the hint (if type == state_type::after, hint will be 0)
//		board::cell hint = state_hint(state);

		// for a legal state, return its three values.
//		return { min, avg, max };
		// for an illegal state, simply return {}
		return {};
	}
	void print_table(){
		std::vector< std::vector<answer> >::iterator it;
		std::vector<answer>::iterator p;
		for(it = before_table.begin();it != before_table.end();it++){
			for(p = (*it).begin();p !=(*it).end();p++){
				std::cout <<"before:"<<" ";
				std::cout <<(*p).min <<" ";
				std::cout <<(*p).avg <<" ";
				std::cout <<(*p).max <<'\n';
			}
		}
		for(it = after_table.begin();it != after_table.end();it++){
			for(p = (*it).begin();p !=(*it).end();p++){
				std::cout <<"after:" <<" ";
				std::cout <<(*p).min <<" ";
				std::cout <<(*p).avg <<" ";
				std::cout <<(*p).max <<'\n';
			}
		}
	}
private:
	// 531441 = 9^6
	std::vector<std::vector<answer> > before_table;
	//answer before_table[1][1][1][1][1][1][1];// 1 3 5
	//answer before_table[9][9][9][9][9][9][4];// 1 3 5
	// after[before,after,illegal][...][+0(error),+1,+2,+3][up,right,down,left(last_action)]
	std::vector<std::vector<answer> > after_table;
	//answer after_table[1][1][1][1][1][1][1][1];// 0 2 4
	//answer after_table[9][9][9][9][9][9][4][4];// 0 2 4
};
