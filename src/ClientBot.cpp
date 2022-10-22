#include <GUIChess.h>
#include <memory.h>
#include <arpa/inet.h>
#include <thread>
#include <sstream>
#include <math.h>
using namespace std;
using namespace chess;
extern char _binary_res_window_glade_start;
extern char _binary_res_window_glade_size;
const string cols = "ABCDEFGH";
ClientBot::ClientBot()
{
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0)
    {
        exit(1);
    }
    if (sockfd < 0)
    {
        cout << "Error opening socket" << endl;
        exit(2);
    }
    sockaddr_in hostAddress;
    memset(&hostAddress, 0, sizeof(sockaddr_in));
    hostAddress.sin_family = AF_INET;
    hostAddress.sin_addr.s_addr = INADDR_ANY;
    hostAddress.sin_port = DEFAULT_PLAYER_PORT;
    if (bind(sockfd, (sockaddr*)&hostAddress, sizeof(sockaddr_in)) < 0)
    {
        cout << "Error binding" << endl;
        exit(3);
    }
    cout << "Waiting for client" << endl;
    cout << "Host Name: " << gethostent()->h_name << endl;
    hostent* hostInfo = gethostent();
    while (*(hostInfo->h_aliases))
    {
        cout << "Alias Name: " << *(hostInfo->h_aliases++) << endl;
    }
    while (*(hostInfo->h_addr_list))
    {
        cout << "IP Address: " << inet_ntoa(*(in_addr*)(*(hostInfo->h_addr_list++))) << endl;
    }
    listen(sockfd, 5);
    sockaddr clientAddress;
    socklen_t clientLength = sizeof(clientAddress);
    newsockfd = accept(sockfd, &clientAddress, &clientLength);
    if (newsockfd < 0)
    {
        cout << "Error accepting" << endl;
        exit(4);
    }
    char buffer[16];
    memcpy(buffer, "ready5483", 10);
    write(newsockfd, buffer, 15);
    memset(buffer, 0, 16);
    read(newsockfd, buffer, 15);
    if (strcmp(buffer, "ready3845") != 0)
    {
        cout << "Client technology incompatable\n";
        exit(5);
    }
    cout << "Found client" << endl;
}
Move* ClientBot::findMove(Board* board)
{
    if (board->isDraw()) return Move::resign();
    char buffer[16];
get_move:
    memset(buffer, 0, 16);
    memcpy(buffer, "enter_move", 11);
    write(newsockfd, buffer, 15);
    memset(buffer, 0, 16);
    read(newsockfd, buffer, 15);
    string token = buffer;
    if (token == "resign")
    {
        return Move::resign();
    }
    int srcCol = cols.find(token[0]);
    int srcRow = token[1] - '1';
    int destCol = cols.find(token[3]);
    int destRow = token[4] - '1';
    Move* move = new Move(Point(srcRow, srcCol), Point(destRow, destCol));
    if (!board->canMove(move)) goto get_move;
    return move;
}
void ClientBot::handleWinner(int winner)
{
    char buffer[16];
    memcpy(buffer, "game_over", 10);
    write(newsockfd, buffer, 15);
    memcpy(buffer, (winner ? "black" : "white"), 6);
    write(newsockfd, buffer, 15);
    close(newsockfd);
    close(sockfd);
}
void ClientBot::handlePrint(Board* board)
{
    char buffer[16];
    memcpy(buffer, "print", 6);
    write(newsockfd, buffer, 15);
    write(newsockfd, gameReference->getCurrentBoard()->serialize(), 65);
}
void ClientBot::setPlayer(Player player)
{
    char buffer[16];
    memcpy(buffer, "set_player", 11);
    write(newsockfd, buffer, 15);
    write(newsockfd, &player, sizeof(Player));
}
void ClientBot::setGameReference(Game* gameReference)
{
    this->gameReference = gameReference;
}
void ClientBot::handleMove(Move* move, Board* board)
{
    char buffer[16];
    memcpy(buffer, "move", 5);
    write(newsockfd, buffer, 15);
    string moveStr = move->toSAN(board);
    memcpy(buffer, moveStr.c_str(), moveStr.length() + 1);
    write(newsockfd, buffer, 15);
}