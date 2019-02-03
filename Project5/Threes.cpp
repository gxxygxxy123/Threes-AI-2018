#include <iostream>
#include <fstream>
#include <iterator>
#include <string>
#include <regex>
#include <memory>
#include "board.h"
#include "action.h"
#include "agent.h"
#include "episode.h"
#include "statistic.h"
#include "arena.h"
#include "io.h"
#include <vector>
int shell(int argc, const char* argv[]) {
	arena host("anonymous");
	std::string play_args, evil_args;
	for (int i = 1; i < argc; i++) {
		std::string para(argv[i]);
		if (para.find("--name=") == 0 || para.find("--account=") == 0) {
			host.set_account(para.substr(para.find("=") + 1));
		} else if (para.find("--login=") == 0) {
			host.set_login(para.substr(para.find("=") + 1));
		} else if (para.find("--save=") == 0 || para.find("--dump=") == 0) {
			host.set_dump_file(para.substr(para.find("=") + 1));
		} else if (para.find("--play") == 0) {
			std::shared_ptr<agent> play(new player(para.substr(para.find("=") + 1)));
			host.register_agent(play);
			play_args = para.substr(para.find("=") + 1);
		} else if (para.find("--evil") == 0) {
			std::shared_ptr<agent> evil(new rndenv(para.substr(para.find("=") + 1)));
			host.register_agent(evil);
			evil_args = para.substr(para.find("=") + 1);
		}
	}

	std::regex match_move("^#\\S+ \\S+$"); // e.g. "#M0001 ?", "#M0001 #U"
	std::regex match_ctrl("^#\\S+ \\S+ \\S+$"); // e.g. "#M0001 open Slider:Placer", "#M0001 close score=15424"
	std::regex arena_ctrl("^[@$].+$"); // e.g. "@ login", "@ error the account "Name" has already been taken"
	std::regex arena_info("^[?%].+$"); // e.g. "? message from anonymous: 2048!!!"
	for (std::string command; input() >> command; ) {
		try {
			if (std::regex_match(command, match_move)) {
				std::string id, move;
				std::stringstream(command) >> id >> move;

				if (move == "?") {
					// your agent need to take an action
					action a = host.at(id).take_action();
					host.at(id).apply_action(a);
					if(a.type() == action::place::type){ // env
						int hint = host.at(id).state().info(); // your hint tile here
						output() << id << ' ' << a << '+' << hint << std::endl; 
					}
					else{
						output() << id << ' ' << a << std::endl;
					}
				} else {
					// perform your opponent's action
					action a;
					std::stringstream(move) >> a;
					int hint = 0;
					if(a.type() == action::place::type){ // env
						hint = move[3] - '0'; // move should be "PT+H" where H is '1','2','3','4'
					}
					else{
						if(move[1] == 'U')	host.at(id).state().oppo_player_move = 0;
						if(move[1] == 'R')	host.at(id).state().oppo_player_move = 1;
						if(move[1] == 'D')	host.at(id).state().oppo_player_move = 2;
						if(move[1] == 'L')	host.at(id).state().oppo_player_move = 3;
					}
					host.at(id).apply_action(a); // you should pass hint to your player
					host.at(id).state().info(hint);
				}

			} else if (std::regex_match(command, match_ctrl)) {
				std::string id, ctrl, tag;
				std::stringstream(command) >> id >> ctrl >> tag;

				if (ctrl == "open") {
					// a new match is pending
					if (host.open(id, tag)) {
						output() << id << " open accept" << std::endl;
					} else {
						output() << id << " open reject" << std::endl;
					}
				} else if (ctrl == "close") {
					// a match is finished
					host.close(id, tag);
				}

			} else if (std::regex_match(command, arena_ctrl)) {
				std::string ctrl;
				std::stringstream(command).ignore(1) >> ctrl;

				if (ctrl == "login") {
					// register yourself and your agents
					std::stringstream agents;
					for (auto who : host.list_agents()) {
						agents << " " << who->name() << "(" << who->role() << ")";
					}
					output("@ ") << "login " << host.login() << agents.str() << std::endl;

				} else if (ctrl == "status") {
					// display current local status
					info() << "+++++ status +++++" << std::endl;
					info() << "login: " << host.account();
					for (auto who : host.list_agents()) {
						info() << " " << who->name() << "(" << who->role() << ")";
					}
					info() << std::endl;
					info() << "match: " << host.list_matches().size() << std::endl;
					for (auto ep : host.list_matches()) {
						info() << ep->name() << " " << (*ep) << std::endl;
					}
					info() << "----- status -----" << std::endl;

				} else if (ctrl == "error" || ctrl == "exit") {
					// some error messages or exit command
					std::string message = command.substr(command.find_first_not_of("@$ "));
					info() << message << std::endl;
					break;
				}

			} else if (std::regex_match(command, arena_info)) {
				// message from arena server
			}
		} catch (std::exception& ex) {
			std::string message = std::string(typeid(ex).name()) + ": " + ex.what();
			message = message.substr(0, message.find_first_of("\r\n"));
			output("? ") << "exception " << message << " at \"" << command << "\"" << std::endl;
		}
	}

	return 0;
}

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
		} else if (para.find("--shell") == 0) {
			return shell(argc, argv);
		}
	}
 /*
	statistic stat(total, block, limit);

	if (load.size()) {
		std::ifstream in(load, std::ios::in);
		in >> stat;
		in.close();
		summary |= stat.is_finished();
	}

	player play(play_args);
	// new
	table = weight_agent(play_args);
	learning_agent alpha_agent(play_args);

	//table.init_net();
	//std::cerr << "table[0]size:"<<table[0].size()<<'\n';
	// new end
	rndenv evil(evil_args);
	while (!stat.is_finished()) {
		//learning_agent alpha_agent;
		play.open_episode("~:" + evil.name());
		evil.open_episode(play.name() + ":~");

		stat.open_episode(play.name() + ":" + evil.name());
		episode& game = stat.back();
		// new
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

		// new end
		while (true) {
			agent& who = game.take_turns(play, evil);
			action move = who.take_action(game.state(), table);
			if (game.apply_action(move) != true) break;//step() will ++ !
			// new
			if(game.step() > 8 && game.step() % 2 == 0){//player's turn:10 12 14..
				evil.set_player_action(play.get_player_choice());
			}
			// new end
			if (who.check_for_win(game.state())) break;
			//printf("debug 123\n");
		}
		// new
		game.update(table, alpha_agent);
		// new end
		agent& win = game.last_turns(play, evil);
		stat.close_episode(win.name());

		play.close_episode(win.name());
		evil.close_episode(win.name());
	}
	//for(int i = 0;i <1000;i++)
		//std::cout << "table[0]["<<i<<"]:"<<table[0][i]<<'\n';
	if (summary) {
		stat.summary();
	}

	if (save.size()) {
		std::ofstream out(save, std::ios::out | std::ios::trunc);
		out << stat;
		out.close();
	}
 */
	return 0;
}
