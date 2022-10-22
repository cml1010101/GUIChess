#include <GUIChess.h>
#include <memory.h>
#include <arpa/inet.h>
#include <thread>
#include <math.h>
using namespace std;
using namespace chess;
extern char _binary_res_window_glade_start;
extern char _binary_res_window_glade_size;
void hostHandleDraw(GtkWidget* widget, cairo_t* cairo, void* host)
{
    HostBot* bot = (HostBot*)host;
    bot->draw(cairo);
}
void hostHandleButtonPress(GtkWidget* widget, GdkEventButton* event, void* host)
{
    HostBot* bot = (HostBot*)host;
    bot->handleButtonPress(event);
}
void quit()
{
    gtk_main_quit();
    exit(0);
}
HostBot::HostBot()
{
    GtkBuilder* builder = gtk_builder_new();
    gtk_builder_add_from_string(builder, (char*)&_binary_res_window_glade_start, 
        (size_t)&_binary_res_window_glade_size, NULL);
    window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
    canvas = GTK_WIDGET(gtk_builder_get_object(builder, "canvas"));
    rightBox = GTK_WIDGET(gtk_builder_get_object(builder, "rightBox"));
    leftBox = GTK_WIDGET(gtk_builder_get_object(builder, "leftBox"));
    gtk_window_set_title(GTK_WINDOW(window), "GChess");
    g_signal_connect(GTK_WINDOW(window), "destroy", G_CALLBACK(quit), NULL);
    g_signal_connect(GTK_DRAWING_AREA(canvas), "button-press-event", 
        G_CALLBACK(hostHandleButtonPress), this);
    g_signal_connect(GTK_DRAWING_AREA(canvas), "draw", G_CALLBACK(hostHandleDraw), this);
    gtk_widget_set_events(canvas,
        gtk_widget_get_events(canvas) | GDK_BUTTON_PRESS_MASK);
    gtk_widget_show(window);
    async = thread(gtk_main);
}
Move* HostBot::findMove(Board* board)
{
    if (board->isDraw()) return Move::resign();
    moveSuggestion = NULL;
    while (moveSuggestion == NULL);
    if (!board->canMove((Move*)moveSuggestion))
    {
        return findMove(board);
    }
    return (Move*)moveSuggestion;
}
void HostBot::handlePrint(Board* board)
{
    gtk_widget_queue_draw(canvas);
}
void HostBot::handleButtonPress(GdkEventButton* event)
{
    if (gameReference->getCurrentBoard()->next != player) return;
    Point newPoint = Point(
        (player == PLAYER_WHITE) ? (7 - floor(event->y / 80)) : floor(event->y / 80), event->x / 80);
    if (gameReference->getCurrentBoard()->canMove(new Move(selectedPoint, newPoint)))
    {
        moveSuggestion = new Move(selectedPoint, newPoint);
    }
    if (gameReference->getCurrentBoard()
        ->grid[newPoint.row][newPoint.col] == NULL ||
        gameReference->getCurrentBoard()
        ->grid[newPoint.row][newPoint.col]->player !=
        gameReference->getCurrentBoard()->next)
    {
        selectedPoint = {-1, -1};
    }
    else selectedPoint = newPoint;
    gtk_widget_queue_draw(canvas);
}
void HostBot::setPlayer(Player player)
{
    this->player = player;
}
void HostBot::setGameReference(Game* gameReference)
{
    this->gameReference = gameReference;
}
void HostBot::draw(cairo_t* cairo)
{
    if (gameReference == NULL) return;
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
            if (gameReference->getCurrentBoard()->
                grid[player == PLAYER_WHITE ? (7 - i) : i][j])
            {
                cairo_surface_t* surface;
                Player piecePlayer = gameReference->getCurrentBoard()->
                    grid[player == PLAYER_WHITE ? (7 - i) : i][j]->player;
                PieceType type = gameReference->getCurrentBoard()->
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
            gameReference->getCurrentBoard()->
                grid[selectedPoint.row][selectedPoint.col]->
                    getPossibleMoves(gameReference->getCurrentBoard()))
        {
            cairo_set_source_rgba(cairo, 0, 0, 1, 0.25);
            cairo_rectangle(cairo, move->dest.col * 80, 
                ((player == PLAYER_WHITE) ? (7 - move->dest.row) : move->dest.row) * 80, 80, 80);
            cairo_fill(cairo);
        }
    }
}
void HostBot::handleMove(Move* move, Board* board)
{
    cout << move->toSAN(board) << endl;
    auto item = gtk_label_new(move->toSAN(board).c_str());
    auto row = gtk_list_box_row_new();
    auto box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    g_object_set(box, "margin-start", 10, "margin-end", 10, NULL);
    gtk_container_add_with_properties(GTK_CONTAINER(box), item, "expand", TRUE, NULL);
    gtk_container_add(GTK_CONTAINER(row), box);
    gtk_list_box_insert(GTK_LIST_BOX(((player == board->next) ? leftBox : rightBox)), row, -1);
    gtk_widget_show_all(((player == board->next) ? leftBox : rightBox));
}
void HostBot::handleWinner(Winner winner)
{
    GtkWidget* message = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, 
        GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "Game Over");
    string winnerStr = "None";
    switch (winner)
    {
    case WINNER_BLACK:
        winnerStr = "Winner: Black";
        break;
    case WINNER_WHITE:
        winnerStr = "Winner: White";
        break;
    case WINNER_DRAW:
        winnerStr = "Game resulted in a draw";
    }
    gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(message), winnerStr.c_str());
    gtk_window_close(GTK_WINDOW(window));
    gtk_dialog_run(GTK_DIALOG(message));
}