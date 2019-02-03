#pragma once
#include <array>
#include <iostream>
#include <iomanip>
#include <cmath>
/**
 * array-based board for 2048
 *
 * index (1-d form):
 *  (0)  (1)  (2)
 *  (3)  (4)  (5)
 *
 */
/*
max:96(8)
0~8
*/
class board {
public:
	typedef uint32_t cell;
	typedef std::array<cell, 3> row;
	typedef std::array<row, 2> grid;
	typedef uint64_t data;
	typedef int reward;

	bool legal[6];

public:
	board() : tile(), attr(0) {
		for(int i = 0;i < 6;i++){
			legal[i] = true;
		}
	}
	board(const grid& b, data v = 0) : tile(b), attr(v) {}
	board(const board& b) = default;
	board& operator =(const board& b) = default;
	board(cell _0, cell _1, cell _2, cell _3, cell _4, cell _5, data hint){
		tile[0][0] = _0;
		tile[0][1] = _1;
		tile[0][2] = _2;
		tile[1][0] = _3;
		tile[1][1] = _4;
		tile[1][2] = _5;
		attr = hint;
	}
	operator grid&() { return tile; }
	operator const grid&() const { return tile; }
	row& operator [](unsigned i) { return tile[i]; }
	const row& operator [](unsigned i) const { return tile[i]; }
	cell& operator ()(unsigned i) { return tile[i / 3][i % 3]; }
	const cell& operator ()(unsigned i) const { return tile[i / 3][i % 3]; }

	data info() const { return attr; }
	data info(data dat) { data old = attr; attr = dat; return old; }

public:
	bool operator ==(const board& b) const { return tile == b.tile; }
	bool operator < (const board& b) const { return tile <  b.tile; }
	bool operator !=(const board& b) const { return !(*this == b); }
	bool operator > (const board& b) const { return b < *this; }
	bool operator <=(const board& b) const { return !(b < *this); }
	bool operator >=(const board& b) const { return !(*this < b); }

public:
	/**
	 * place a tile (index value) to the specific position (1-d form index)
	 * return 0 if the action is valid, or -1 if not
	 */
	bool is_empty(){
		if(	tile[0][0] == 0 && tile[0][1] == 0 && tile[0][2] == 0 && tile[1][0] == 0 && tile[1][1] == 0 && tile[1][2] == 0) return true;
		return false;
	}
	void print_board(){
		std::cout << tile[0][0] <<" "<<tile[0][1]<<" "<<tile[0][2]<<'\n'<<tile[1][0]<<" "<<tile[1][1]<<" "<<tile[1][2]<<'\n';
	}
	reward current_score(void){
		int sum = 0;
		for(int r = 0;r < 2;r++){
			auto& row = tile[r];
			for(int c = 0;c < 3;c++){
				int tile = row[c];
				if(tile >= 3 && tile <= 8){
					sum += pow(3, tile-2);
				}
			}
		}
		return sum;
	}
	reward place(unsigned pos, cell tile) {
		if (pos >= 6) return -1;
		if (tile != 1 && tile != 2 && tile != 3) return -1;
		operator()(pos) = tile;
		if(tile == 3)	return 3;
		return 0;
	}

	/**
	 * apply an action to the board
	 * return the reward of the action, or -1 if the action is illegal
	 */
	reward slide(unsigned opcode) {
		switch (opcode & 0b11) {
		case 0: return slide_up();
		case 1: return slide_right();
		case 2: return slide_down();
		case 3: return slide_left();
		default: return -1;
		}
	}

	reward slide_left() {
		//if(is_empty())	return 0;
		board prev = *this;
		int original_score = this->current_score();
		for(int r = 0;r < 2; r++){
			auto& row = tile[r];
			int left = 0;
			for(int c = 0;c < 2/*only to 2*/;c++){
				uint32_t tile = row[c];
				if(tile == 0){
					if(row[c+1] != 0){//swap
						row[c] = row[c+1];
						row[c+1] = 0;
					}
				}
				else if(tile == 1 || tile == 2){
					if(row[c+1] == (3-tile) && c == left){// merge
						row[c] = 3;
						row[c+1] = 0;
					}
					else{// can't merge
						left++;
					}
				}
				else if(tile == row[c+1] && c == left && tile <= 8u){// merge
					row[c] ++;
					row[c+1] = 0;
				}
				else{
					left++;
				}
			}
		}
		return (*this != prev) ? (this->current_score() - original_score) : -1;
	}
	reward slide_right() {
		//if(is_empty())	return 0;
		reflect_horizontal();
		reward score = slide_left();
		reflect_horizontal();
		return score;
	}
	reward slide_up() {
		//if(is_empty())	return 0;
		board prev = *this;
		int original_score = this->current_score();
		for(int c = 0;c < 3; c++){
			if(tile[1][c] == 0){
				continue;
			}
			else if(tile[0][c] == 0){
				tile[0][c] = tile[1][c];
				tile[1][c] = 0;
			}
			else if((tile[0][c] == 1 && tile[1][c] == 2) || (tile[0][c] == 2 && tile[1][c]==1)){
				tile[0][c] = 3;
				tile[1][c] = 0;
			}
			else if((tile[0][c] == tile[1][c]) && (tile[0][c] != 0) && (tile[0][c] != 1) && (tile[0][c]) != 2){
				tile[0][c]++;
				tile[1][c] = 0;
			}
		}
		return (*this != prev) ? (this->current_score() - original_score) : -1;
	}
	reward slide_down() {
		//if(is_empty())	return 0;
		board prev = *this;
		int original_score = this->current_score();
		for(int c = 0;c < 3; c++){
			if(tile[0][c] == 0){
				continue;
			}
			else if(tile[1][c] == 0){
				tile[1][c] = tile[0][c];
				tile[0][c] = 0;
			}
			else if((tile[1][c] == 1 && tile[0][c] == 2) || (tile[1][c] == 2 && tile[0][c]==1)){
				tile[1][c] = 3;
				tile[0][c] = 0;
			}
			else if((tile[0][c] == tile[1][c]) && (tile[0][c] != 0) && (tile[0][c] != 1) && (tile[0][c]) != 2){
				tile[1][c]++;
				tile[0][c] = 0;
			}
		}
		return (*this != prev) ? (this->current_score() - original_score) : -1;
	}

	void reflect_horizontal() {
		for (int r = 0; r < 2; r++) {
			std::swap(tile[r][0], tile[r][2]);
		}
	}

	void reflect_vertical() {
		for (int c = 0; c < 3; c++) {
			std::swap(tile[0][c], tile[1][c]);
		}
	}
	void reverse() { reflect_horizontal(); reflect_vertical(); }

public:
	friend std::ostream& operator <<(std::ostream& out, const board& b) {
		int out_num;
		for (int i = 0; i < 6; i++) {
			if(b.legal[i] == false){
				out << std::setw(std::min(i, 1)) << "" << b(i);
				continue;
			}
			if(b(i) == 0)	out_num = 0;
			else if(b(i) == 1)	out_num = 1;
			else if(b(i) == 2)	out_num = 2;
			else if(b(i) >= 3)	out_num = ((1<<(b(i)-3))*3);
			out << std::setw(std::min(i, 1)) << "" << out_num;
		}
		return out;
	}
	friend std::istream& operator >>(std::istream& in, board& b) {
		for(int i = 0;i < 6;i++){
			b.legal[i] = true;
		}
		for (int i = 0; i < 6; i++) {
			while (!std::isdigit(in.peek()) && in.good()) in.ignore(1);
			int a;
			in >> a;
			if(a == 0)	b(i) = 0;
			else if(a == 1)	b(i) = 1;
			else if(a == 2)	b(i) = 2;
			else if(a == 3)	b(i) = 3;
			else if(a == 6)	b(i) = 4;
			else if(a == 12)	b(i) = 5;
			else if(a == 24)	b(i) = 6;
			else if(a == 48)	b(i) = 7;
			else if(a == 96)	b(i) = 8;
			else{// invalid input
				std::cout << "Invalid cell Input:"<< a << '\n';
				b(i) = a;
				b.legal[i] = false; // invalid
			}

		}
		return in;
	}

private:
	grid tile;
	data attr;//attr = 1(+1), 2(+2), 3(+3)
};
