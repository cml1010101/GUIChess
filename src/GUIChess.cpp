#include <GUIChess.h>
#include <memory.h>
#include <arpa/inet.h>
#include <sstream>
#include <boost/program_options.hpp>
#include <thread>
using namespace boost::program_options;
using namespace std;
using namespace chess;
extern char _binary_res_window_glade_start;
extern char _binary_res_window_glade_size;
Game* gameReference;
Move* RandomBot::findMove(Board* board)
{
    vector<Move*> possibleMoves = {};
    auto pieces = board->getPieces(board->next);
    for (auto piece : pieces)
    {
        auto moves = piece->getPossibleMoves(board);
        for (auto move : moves)
        {
            possibleMoves.push_back(move);
        }
    }
    if (possibleMoves.size() == 0 || board->isDraw()) return Move::resign();
    return possibleMoves[rand() % possibleMoves.size()];
}
int hostChessGame(string whiteBotQuery, string blackBotQuery)
{
    Bot* whiteBot, *blackBot;
    if (whiteBotQuery == "host")
    {
        whiteBot = new HostBot();
    }
    else if (whiteBotQuery == "client")
    {
        whiteBot = new ClientBot();
    }
    else if (whiteBotQuery == "random")
    {
        whiteBot = new RandomBot();
    }
    else if (whiteBotQuery == "minimax")
    {
        whiteBot = new MinimaxBot(2);
    }
    if (blackBotQuery == "host")
    {
        blackBot = new HostBot();
    }
    else if (blackBotQuery == "client")
    {
        blackBot = new ClientBot();
    }
    else if (blackBotQuery == "random")
    {
        blackBot = new RandomBot();
    }
    else if (blackBotQuery == "minimax")
    {
        blackBot = new MinimaxBot(2);
    }
    Game game = Game(whiteBot, blackBot);
    gameReference = &game;
    while (game.getCurrentBoard()->winner == -1)
    {
        whiteBot->handlePrint(game.getCurrentBoard());
        blackBot->handlePrint(game.getCurrentBoard());
        game.step();
    }
    whiteBot->handleWinner(game.getCurrentBoard()->winner);
    blackBot->handleWinner(game.getCurrentBoard()->winner);
    return 0;
}
int main(int argc, char const *argv[])
{
    gtk_init(&argc, (char***)&argv);
    options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("host,h", "host a game of chess")
        ("connect,c", value<string>(), "connect to a game of chess")
        ("view,v", value<string>(), "view a game of chess")
        ("white,W", value<string>()->default_value("host"), "set up the white bot")
        ("black,B", value<string>()->default_value("client"), "set up the black bot")
    ;
    variables_map vm;
    store(parse_command_line(argc, argv, desc), vm);
    notify(vm);
    int ret = 0;
    if (vm.count("help"))
    {
        cout << desc << endl;
        return 0;
    }
    if (vm.count("host"))
    {
        ret = hostChessGame(vm["white"].as<string>(), vm["black"].as<string>());
    }
    else if (vm.count("connect"))
    {
        ret = connectChessGame(vm["connect"].as<string>().c_str());
    }
    else
    {
    }
    return ret;
}