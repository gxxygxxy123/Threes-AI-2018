#include <iostream>
#include <fstream>
#include <iterator>
#include <string>
#include "board.h"
#include "action.h"
#include "agent.h"
#include "episode.h"
#include "statistic.h"
#include <vector>

int main(int argc, const char* argv[]) {
	std::cout << "Threes-Demo: ";
	std::copy(argv, argv + argc, std::ostream_iterator<const char*>(std::cout, " "));
	std::cout << std::endl << std::endl;

	size_t total = 1000, block = 0, limit = 0;
	std::string play_args, evil_args;
	std::string load, save;
	bool summary = false;
	for (int i = 1; i < argc; i++) {
		std::string para(argv[i]);
		if (para.find("--total=") == 0) {
			total = std::stoull(para.substr(para.find("=") + 1));
		} else if (para.find("--block=") == 0) {
			block = std::stoull(para.substr(para.find("=") + 1));
		} else if (para.find("--limit=") == 0) {
			limit = std::stoull(para.substr(para.find("=") + 1));
		} else if (para.find("--play=") == 0) {
			play_args = para.substr(para.find("=") + 1);
		} else if (para.find("--evil=") == 0) {
			evil_args = para.substr(para.find("=") + 1);
		} else if (para.find("--load=") == 0) {
			load = para.substr(para.find("=") + 1);
		} else if (para.find("--save=") == 0) {
			save = para.substr(para.find("=") + 1);
		} else if (para.find("--summary") == 0) {
			summary = true;
		}
	}

	statistic stat(total, block, limit);

	if (load.size()) {
		std::ifstream in(load, std::ios::in);
		in >> stat;
		in.close();
		summary |= stat.is_finished();
	}

	player play(play_args);

	weight_agent table(play_args);
	learning_agent alpha_agent(play_args);

	rndenv evil(evil_args);
	while (!stat.is_finished()) {

		play.open_episode("~:" + evil.name());
		evil.open_episode(play.name() + ":~");

		stat.open_episode(play.name() + ":" + evil.name());
		episode& game = stat.back();

		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> dis(1,3);
		int first_num = dis(gen);
		game.state().info(first_num);
		evil.fill_bag();
		evil.set_player_action(4);
		for(size_t i = 0;i < evil.this_bag().size();i++){
			if(evil.this_bag()[i] == first_num){
				evil.this_bag().erase(evil.this_bag().begin()+i); // remove first element
				break;
			}
		}


		while (true) {
			agent& who = game.take_turns(play, evil);
			action move = who.take_action(game.state(), table);
			if (game.apply_action(move) != true) break;//step() will ++ !

			if(game.step() > 8 && game.step() % 2 == 0){//player's turn:10 12 14..
				evil.set_player_action(play.get_player_choice());
			}

			if (who.check_for_win(game.state())) break;
		}

		game.update(table, alpha_agent);

		agent& win = game.last_turns(play, evil);
		stat.close_episode(win.name());

		play.close_episode(win.name());
		evil.close_episode(win.name());
	}
	if (summary) {
		stat.summary();
	}

	if (save.size()) {
		std::ofstream out(save, std::ios::out | std::ios::trunc);
		out << stat;
		out.close();
	}

	return 0;
}
