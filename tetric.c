#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define BOARD_WIDTH 15
#define BOARD_HEIGHT 30

int board[BOARD_HEIGHT][BOARD_WIDTH];
int score = 0;

void init_board();
int clear_lines();
int is_line_full(int y);
void remove_line(int y);
int is_game_over();

typedef struct {
    int x;
    int y;
    int shape[4][4];
    int color;
} Tetromino;

void new_tetromino();
void rotate_tetromino();
void move_tetromino(int dx, int dy);
int check_collision(int new_x, int new_y);
void lock_tetromino();

void init_renderer();
void cleanup_renderer();
void render();

int handle_input();

void game_loop();
void update_score(int lines_cleared);

Tetromino current_tetromino;
int start_y, start_x;
int speed_level = 6;
int score_for_next_level = 500;

const int TETROMINOES[7][4][4] = {
    {{0, 1, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
    {{0, 1, 1, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
    {{1, 1, 0, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
    {{1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
    {{1, 1, 0, 0}, {1, 0, 0, 0}, {1, 0, 0, 0}, {0, 0, 0, 0}},
    {{0, 1, 1, 0}, {0, 0, 1, 0}, {0, 0, 1, 0}, {0, 0, 0, 0}},
    {{1, 1, 0, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}
};

void init_board() {
    memset(board, 0, sizeof(board));
    score = 0;
    speed_level = 6;
    score_for_next_level = 500;
}

int clear_lines() {
    int lines_cleared = 0;
    for (int y = BOARD_HEIGHT - 1; y >= 0; y--) {
        if (is_line_full(y)) {
            remove_line(y);
            lines_cleared++;
            y++;
        }
    }
    return lines_cleared;
}

int is_line_full(int y) {
    for (int x = 0; x < BOARD_WIDTH; x++) {
        if (board[y][x] == 0) {
            return 0;
        }
    }
    return 1;
}

void remove_line(int y) {
    for (int i = y; i > 0; i--) {
        for (int j = 0; j < BOARD_WIDTH; j++) {
            board[i][j] = board[i - 1][j];
        }
    }
    for (int j = 0; j < BOARD_WIDTH; j++) {
        board[0][j] = 0;
    }
}

int is_game_over() {
    for (int x = 0; x < BOARD_WIDTH; x++) {
        if (board[0][x] != 0) {
            return 1;
        }
    }
    return 0;
}

void new_tetromino() {
    int type = rand() % 7;
    memcpy(current_tetromino.shape, TETROMINOES[type], sizeof(current_tetromino.shape));
    current_tetromino.x = BOARD_WIDTH / 2 - 2;
    current_tetromino.y = 0;
    current_tetromino.color = type + 1;

    if (check_collision(current_tetromino.x, current_tetromino.y)) {
        init_board();
    }
}

void rotate_tetromino() {
    int temp[4][4];
    memcpy(temp, current_tetromino.shape, sizeof(temp));

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            current_tetromino.shape[i][j] = temp[3 - j][i];
        }
    }

    if (check_collision(current_tetromino.x, current_tetromino.y)) {
        memcpy(current_tetromino.shape, temp, sizeof(temp));
    }
}

void move_tetromino(int dx, int dy) {
    if (!check_collision(current_tetromino.x + dx, current_tetromino.y + dy)) {
        current_tetromino.x += dx;
        current_tetromino.y += dy;
    } else if (dy > 0) {
        lock_tetromino();
        int lines_cleared = clear_lines();
        if (lines_cleared > 0) {
            update_score(lines_cleared);
        }
        new_tetromino();
    }
}

int check_collision(int new_x, int new_y) {
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (current_tetromino.shape[y][x]) {
                int board_x = new_x + x;
                int board_y = new_y + y;

                if (board_x < 0 || board_x >= BOARD_WIDTH || board_y >= BOARD_HEIGHT) {
                    return 1;
                }
                if (board_y >= 0 && board[board_y][board_x]) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

void lock_tetromino() {
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (current_tetromino.shape[y][x]) {
                int board_x = current_tetromino.x + x;
                int board_y = current_tetromino.y + y;
                if (board_y >= 0) {
                    board[board_y][board_x] = current_tetromino.color;
                }
            }
        }
    }
}

void init_renderer() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    timeout(100);
    curs_set(0);
    start_color();
    init_pair(1, COLOR_CYAN, COLOR_BLACK);
    init_pair(2, COLOR_BLUE, COLOR_BLACK);
    init_pair(3, COLOR_WHITE, COLOR_BLACK);
    init_pair(4, COLOR_YELLOW, COLOR_BLACK);
    init_pair(5, COLOR_GREEN, COLOR_BLACK);
    init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(7, COLOR_RED, COLOR_BLACK);

    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    start_y = (rows - BOARD_HEIGHT) / 2;
    start_x = (cols - BOARD_WIDTH * 2) / 2;
}

void cleanup_renderer() {
    endwin();
}

void render() {
    clear();

    mvprintw(start_y - 1, start_x - 2, "+");
    mvprintw(start_y - 1, start_x + BOARD_WIDTH * 2, "+");
    mvprintw(start_y + BOARD_HEIGHT, start_x - 2, "+");
    mvprintw(start_y + BOARD_HEIGHT, start_x + BOARD_WIDTH * 2, "+");
    for (int x = 0; x < BOARD_WIDTH * 2; x++) {
        mvprintw(start_y - 1, start_x + x, "-");
        mvprintw(start_y + BOARD_HEIGHT, start_x + x, "-");
    }
    for (int y = 0; y < BOARD_HEIGHT; y++) {
        mvprintw(start_y + y, start_x - 2, "|");
        mvprintw(start_y + y, start_x + BOARD_WIDTH * 2, "|");
    }


    for (int y = 0; y < BOARD_HEIGHT; y++) {
        for (int x = 0; x < BOARD_WIDTH; x++) {
            if (board[y][x]) {
                attron(COLOR_PAIR(board[y][x]));
                mvprintw(start_y + y, start_x + x * 2, "[]");
                attroff(COLOR_PAIR(board[y][x]));
            } else {
                mvprintw(start_y + y, start_x + x * 2, " .");
            }
        }
    }

    attron(COLOR_PAIR(current_tetromino.color));
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (current_tetromino.shape[y][x]) {
                mvprintw(start_y + current_tetromino.y + y, start_x + (current_tetromino.x + x) * 2, "[]");
            }
        }
    }
    attroff(COLOR_PAIR(current_tetromino.color));

    char game_info_str[50];
    int level = 7 - speed_level;
    sprintf(game_info_str, "Score: %d | Level: %d", score, level);
    int info_len = strlen(game_info_str);
    int info_x = start_x + (BOARD_WIDTH * 2 - info_len) / 2;
    mvprintw(start_y - 2, info_x, "%s", game_info_str);

    refresh();
}

int handle_input() {
    int ch = getch();
    switch (ch) {
        case KEY_LEFT:
            move_tetromino(-1, 0);
            break;
        case KEY_RIGHT:
            move_tetromino(1, 0);
            break;
        case KEY_DOWN:
            move_tetromino(0, 1);
            break;
        case KEY_UP:
            rotate_tetromino();
            break;
        case 'q':
            return 1;
    }
    return 0;
}

void game_loop() {
    srand(time(NULL));
    init_board();
    new_tetromino();
    init_renderer();

    int timer = 0;
    while (!is_game_over()) {
        if (handle_input()) {
            break;
        }

        timer++;
        if (timer > speed_level) {
            move_tetromino(0, 1);
            timer = 0;
        }

        render();
        usleep(100000);
    }

    cleanup_renderer();
}

void update_score(int lines_cleared) {
    switch (lines_cleared) {
        case 1:
            score += 100;
            break;
        case 2:
            score += 300;
            break;
        case 3:
            score += 500;
            break;
        case 4:
            score += 800;
            break;
    }

    if (score >= score_for_next_level) {
        if (speed_level > 1) {
            speed_level--;
        }
        score_for_next_level += 500;
    }
}

int main() {
    game_loop();
    return 0;
}

