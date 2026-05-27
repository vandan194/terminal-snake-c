#include <ncurses.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_SCORE 256
#define FRAME_TIME 150000

// 2D vector for positions
typedef struct {
    int x;
    int y;
} vec2;

// Function prototypes
void init(void);
void process_input(void);
void restart_game(void);
void update(void);
void game_over(void);
void draw(void);
void draw_border(int y, int x, int width, int height);
void quit_game(void);
void print_score(void);

bool collide(vec2 a, vec2 b);
bool collide_snake_body(vec2 point);

vec2 spawn_berry(void);

int score = 0;
char score_message[32];

bool is_running = true;

int screen_width = 25;
int screen_height = 20;

// Snake data
vec2 head = {0,0};
vec2 dir = {1,0};
vec2 segments[MAX_SCORE+1];

vec2 berry;

WINDOW *win;

int main(void)
{
    init();

    // Main game loop
    while(true)
    {
        process_input();
        update();
        draw();
    }

    quit_game();
    return 0;
}

// Update score text
void print_score()
{
    sprintf(score_message , "[Score : %d]",score);
}

void init()
{
    // Seed random number generator
    srand(time(NULL));

    win = initscr();

    // Enable keyboard input settings
    keypad(win, true);
    noecho();
    nodelay(win, true);
    curs_set(0);

    // Initialize colors
    if(has_colors() == FALSE)
    {
        endwin();
        fprintf(stderr,"Your terminal does not support colours\n");
        exit(0);
    }

    start_color();
    use_default_colors();

    init_pair(1,COLOR_RED,-1);
    init_pair(2,COLOR_GREEN,-1);
    init_pair(3, COLOR_YELLOW, -1);

    // Spawn first berry
    berry.x = rand() % screen_width;
    berry.y = rand() % screen_height;

    print_score();
}

void process_input()
{
    int pressed = wgetch(win);

    // Prevent snake from reversing into itself
    if(pressed == KEY_LEFT)
    {
        if(dir.x == 1)
        {
            return;
        }

        dir.x = -1;
        dir.y = 0;
    }

    if(pressed == KEY_RIGHT)
    {
        if(dir.x == -1)
        {
            return;
        }

        dir.x = 1;
        dir.y = 0;
    }

    if(pressed == KEY_DOWN)
    {
        if(dir.y == -1)
        {
            return;
        }

        dir.x = 0;
        dir.y = 1;
    }

    if(pressed == KEY_UP)
    {
        if(dir.y == 1)
        {
            return;
        }

        dir.x = 0;
        dir.y = -1;
    }

    // Restart game
    if(pressed == ' ' && !is_running)
    {
        restart_game();
    }

    // ESC key quits game
    if(pressed == 27)
    {
        is_running = false;
        quit_game();
    }
}

void restart_game()
{
    // Reset snake state
    head.x = 0;
    head.y = 0;

    dir.x = 1;
    dir.y = 0;

    is_running = true;
    score = 0;

    print_score();
}

// Checks if two positions collide
bool collide(vec2 a, vec2 b)
{
    if(a.x == b.x && a.y == b.y)
    {
        return true;
    }

    return false;
}

// Checks collision with snake body
bool collide_snake_body(vec2 point)
{
    for(int i = 0; i < score; i++)
    {
        if(collide(point,segments[i]))
        {
            return true;
        }
    }

    return false;
}

void update()
{
    // Move body segments forward
    for(int i = score; i > 0; i--)
    {
        segments[i] = segments[i-1];
    }

    segments[0] = head;

    // Move snake head
    head.x += dir.x;
    head.y += dir.y;

    // Check wall and self collision
    if(collide_snake_body(head) ||
       head.x < 0 ||
       head.y < 0 ||
       head.x >= screen_width ||
       head.y >= screen_height)
    {
        is_running = false;
        game_over();
    }

    // Berry eaten
    if(collide(head, berry))
    {
        if(score < MAX_SCORE)
        {
            score++;
            print_score();
        }
        else
        {
            printf("You Win!");
        }

        berry = spawn_berry();
    }

    usleep(FRAME_TIME);
}

void game_over()
{
    while(!is_running)
    {
        process_input();

        mvaddstr(screen_height/2 , screen_width - 16,
            "             Game Over                ");

        mvaddstr(screen_height/2 +1, screen_width - 16,
            "[SPACE] to restart,  [ESC] to quit ");

        attron(COLOR_PAIR(3));

        draw_border(screen_height / 2 - 1,
            screen_width - 17,17,2);

        attroff(COLOR_PAIR(3));
    }
}

// Generates berry at random valid position
vec2 spawn_berry()
{
    vec2 berry = {
        1 + rand()%(screen_width -2),
        rand()%(screen_height -2)
    };

    // Keep generating until valid spot found
    while(collide(head,berry) || collide_snake_body(berry))
    {
        berry.x = 1 + rand()%(screen_width -2);
        berry.y = rand()%(screen_height -2);
    }

    return berry;
}

void draw()
{
    erase();

    // Draw berry
    attron(COLOR_PAIR(1));
    mvaddch(berry.y + 1, berry.x * 2 + 1, '@');
    attroff(COLOR_PAIR(1));

    // Draw snake
    attron(COLOR_PAIR(2));

    for(int i = 0; i < score; i++)
    {
        mvaddch(segments[i].y + 1,
                segments[i].x *2 +1,
                ACS_DIAMOND);
    }

    mvaddch(head.y + 1, head.x*2+1, 'O');

    attroff(COLOR_PAIR(2));

    // Draw border
    attron(COLOR_PAIR(3));
    draw_border(0,0,screen_width,screen_height);
    attroff(COLOR_PAIR(3));

    mvaddstr(0, screen_width - 5, score_message);

    refresh();
}

// Draws game border using ncurses box characters
void draw_border(int y, int x, int width, int height)
{
    // Top border
    mvaddch(y,x, ACS_ULCORNER);
    mvaddch(y, x+width*2+1, ACS_URCORNER);

    for(int i = 1; i< width*2+1; i++)
    {
        mvaddch(y, x+i, ACS_HLINE);
    }

    // Side borders
    for(int i = 1; i<=height; i++)
    {
        mvaddch(y+i, x , ACS_VLINE);
        mvaddch(y+i, x+width*2+1 , ACS_VLINE);
    }

    // Bottom border
    mvaddch(y + height + 1, x, ACS_LLCORNER);
    mvaddch(y + height + 1, x+width*2+1, ACS_LRCORNER);

    for(int i = 1; i < width *2 +1; i++)
    {
        mvaddch(y+height+1, x + i, ACS_HLINE);
    }
}

void quit_game()
{
    endwin();
    exit(0);
}
