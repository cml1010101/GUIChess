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
Board* board = new Board();
Player player;
GtkWidget* canvas;
Point selectedPoint;
volatile Move* moveSuggestion;
const string COLS = "ABCDEFGH";
void clientHandleButtonPress(GtkWidget* widget, GdkEventButton* event, void* data)
{
    if (board->next != player) return;
    Point newPoint = Point(
        (player == PLAYER_WHITE) ? (7 - floor(event->y / 80)) : floor(event->y / 80), event->x / 80);
    if (board->canMove(new Move(selectedPoint, newPoint)))
    {
        moveSuggestion = new Move(selectedPoint, newPoint);
    }
    if (board
        ->grid[newPoint.row][newPoint.col] == NULL ||
        board
        ->grid[newPoint.row][newPoint.col]->player !=
        board->next)
    {
        selectedPoint = {-1, -1};
    }
    else selectedPoint = newPoint;
    gtk_widget_queue_draw(canvas);
}
void clientHandleDraw(GtkWidget* widget, cairo_t* cairo, void* data)
{
    bool color = player == PLAYER_WHITE;
    for (size_t i = 0; i < 8; i++)
    {
        for (size_t j = 0; j < 8; j++)
        {
            cairo_rectangle(cairo, j * 80, i * 80, 80, 80);
            if (color)
            {
                cairo_set_source_rgb(cairo, 1, 1, 0);
            }
            else
            {
                cairo_set_source_rgb(cairo, 0.545, 0.271, 0.075);
            }
            color = !color;
            cairo_fill(cairo);
            if (board->
                grid[player == PLAYER_WHITE ? (7 - i) : i][j])
            {
                cairo_surface_t* surface;
                Player piecePlayer = board->
                    grid[player == PLAYER_WHITE ? (7 - i) : i][j]->player;
                PieceType type = board->
                    grid[player == PLAYER_WHITE ? (7 - i) : i][j]->type;
                if (piecePlayer == PLAYER_WHITE)
                {
                    switch (type)
                    {
                    case PIECE_PAWN:
                        surface = cairo_image_surface_create_from_png("res/wP.png");
                        break;
                    case PIECE_ROOK:
                        surface = cairo_image_surface_create_from_png("res/wR.png");
                        break;
                    case PIECE_KNIGHT:
                        surface = cairo_image_surface_create_from_png("res/wN.png");
                        break;
                    case PIECE_BISHOP:
                        surface = cairo_image_surface_create_from_png("res/wB.png");
                        break;
                    case PIECE_QUEEN:
                        surface = cairo_image_surface_create_from_png("res/wQ.png");
                        break;
                    case PIECE_KING:
                        surface = cairo_image_surface_create_from_png("res/wK.png");
                        break;
                    default:
                        break;
                    }
                }
                else
                {
                    switch (type)
                    {
                    case PIECE_PAWN:
                        surface = cairo_image_surface_create_from_png("res/bP.png");
                        break;
                    case PIECE_ROOK:
                        surface = cairo_image_surface_create_from_png("res/bR.png");
                        break;
                    case PIECE_KNIGHT:
                        surface = cairo_image_surface_create_from_png("res/bN.png");
                        break;
                    case PIECE_BISHOP:
                        surface = cairo_image_surface_create_from_png("res/bB.png");
                        break;
                    case PIECE_QUEEN:
                        surface = cairo_image_surface_create_from_png("res/bQ.png");
                        break;
                    case PIECE_KING:
                        surface = cairo_image_surface_create_from_png("res/bK.png");
                        break;
                    default:
                        break;
                    }
                }
                cairo_set_source_surface(cairo, surface, j * 80, i * 80);
                cairo_paint(cairo);
            }
        }
        color = !color;
    }
    if (selectedPoint)
    {
        cairo_set_source_rgba(cairo, 0, 0, 1, 0.5);
        cairo_rectangle(cairo, selectedPoint.col * 80, 
            ((player == PLAYER_WHITE) ? (7 - selectedPoint.row) : selectedPoint.row) * 80, 80, 80);
        cairo_fill(cairo);
        for (auto move :
            board->
                grid[selectedPoint.row][selectedPoint.col]->
                    getPossibleMoves(board))
        {
            cairo_set_source_rgba(cairo, 0, 0, 1, 0.25);
            cairo_rectangle(cairo, move->dest.col * 80, 
                ((player == PLAYER_WHITE) ? (7 - move->dest.row) : move->dest.row) * 80, 80, 80);
            cairo_fill(cairo);
        }
    }
}
int connectChessGame(const char* hostName)
{
    char buffer[16];
    sockaddr_in hostAddress;
    hostent* host;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0)
    {
        return 1;
    }
    host = gethostbyname(hostName);
    memset(&hostAddress, 0, sizeof(sockaddr_in));
    hostAddress.sin_family = AF_INET;
    memcpy(&hostAddress.sin_addr.s_addr, host->h_addr, host->h_length);
    hostAddress.sin_port = DEFAULT_PLAYER_PORT;
    if (connect(sockfd, (sockaddr*)&hostAddress, sizeof(hostAddress)) < 0)
    {
        cout << "Error connecting to " << hostName << endl;
        return 1;
    }
    memset(buffer, 0, 16);
    read(sockfd, buffer, 15);
    if (strcmp(buffer, "ready5483") != 0)
    {
        cout << "Host technology incompatable\n";
        return 2;
    }
    memcpy(buffer, "ready3845", 10);
    write(sockfd, buffer, 15);
    GtkBuilder* builder = gtk_builder_new();
    gtk_builder_add_from_string(builder, (char*)&_binary_res_window_glade_start, 
        (size_t)&_binary_res_window_glade_size, NULL);
    GtkWidget* window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
    canvas = GTK_WIDGET(gtk_builder_get_object(builder, "canvas"));
    gtk_window_set_title(GTK_WINDOW(window), "GChess");
    g_signal_connect(GTK_WINDOW(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(GTK_DRAWING_AREA(canvas), "button-press-event", 
        G_CALLBACK(clientHandleButtonPress), NULL);
    g_signal_connect(GTK_DRAWING_AREA(canvas), "draw", G_CALLBACK(clientHandleDraw), NULL);
    gtk_widget_set_events(canvas,
        gtk_widget_get_events(canvas) | GDK_BUTTON_PRESS_MASK);
    gtk_widget_show(window);
    thread async = thread(gtk_main);
    while (true)
    {
        read(sockfd, buffer, 15);
        string cmd = buffer;
        if (cmd == "print")
        {
            uint8_t serial[65];
            read(sockfd, serial, 65);
            board = Board::fromSerial(serial);
            gtk_widget_queue_draw(canvas);
        }
        else if (cmd == "enter_move")
        {
            moveSuggestion = NULL;
            while (moveSuggestion == NULL);
            char token[5];
            token[0] = COLS[moveSuggestion->src.col];
            token[1] = moveSuggestion->src.row + '1';
            token[2] = ' ';
            token[3] = COLS[moveSuggestion->dest.col];
            token[4] = moveSuggestion->dest.row + '1';
            memcpy(buffer, token, 5);
            write(sockfd, buffer, 15);
        }
        else if (cmd == "game_over")
        {
            read(sockfd, buffer, 15);
            cout << "Winner is " << buffer << endl;
            break;
        }
        else if (cmd == "set_player")
        {
            read(sockfd, &player, sizeof(Player));
        }
    }
    close(sockfd);
    return 0;
}