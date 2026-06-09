#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <ncurses.h>

#define MAX_ROWS 24
#define MAX_COLS 80
#define MAX_OBJECTS 100

// Shape Types
typedef enum {
    LINE,
    RECTANGLE,
    TRIANGLE,
    CIRCLE
} ShapeType;

// Object Structure to allow Modification and Deletion
typedef struct {
    int id;
    ShapeType type;
    int x1, y1; // Primary coordinates
    int x2, y2; // Secondary coordinates (Width/Height or Radix for Circle)
    int x3, y3; // Used specifically for Triangle third vertex
} GraphicObject;

// Global State
char canvas[MAX_ROWS][MAX_COLS];
GraphicObject object_list[MAX_OBJECTS];
int object_count = 0;
int next_id = 1;

// Function Prototypes
void init_canvas();
void render_canvas();
void draw_line(int x1, int y1, int x2, int y2);
void draw_rectangle(int x1, int y1, int x2, int y2);
void draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3);
void draw_circle(int xc, int yc, int r);
void put_pixel(int x, int y);
void add_object_menu();
void delete_object_menu();
void modify_object_menu();
void display_ui_and_canvas();

int main() {
    // Initialize ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    init_canvas();
    int choice;

    while (1) {
        display_ui_and_canvas();
        choice = toupper(getch());

        if (choice == 'Q') {
            break;
        }

        switch (choice) {
            case 'A':
                add_object_menu();
                break;
            case 'D':
                delete_object_menu();
                break;
            case 'M':
                modify_object_menu();
                break;
            default:
                // Flash the screen on invalid input
                flash(); 
                break;
        }
    }

    // End ncurses environment
    endwin();
    return 0;
}

// Resets canvas with underscores
void init_canvas() {
    for (int i = 0; i < MAX_ROWS; i++) {
        for (int j = 0; j < MAX_COLS; j++) {
            canvas[i][j] = '_';
        }
    }
}

// Safely plots a pixel within bounds
void put_pixel(int x, int y) {
    if (x >= 0 && x < MAX_COLS && y >= 0 && y < MAX_ROWS) {
        canvas[y][x] = '*';
    }
}

// Redraws all active objects onto a fresh canvas layer
void render_canvas() {
    init_canvas();
    for (int i = 0; i < object_count; i++) {
        GraphicObject obj = object_list[i];
        switch (obj.type) {
            case LINE:
                draw_line(obj.x1, obj.y1, obj.x2, obj.y2);
                break;
            case RECTANGLE:
                draw_rectangle(obj.x1, obj.y1, obj.x2, obj.y2);
                break;
            case TRIANGLE:
                draw_triangle(obj.x1, obj.y1, obj.x2, obj.y2, obj.x3, obj.y3);
                break;
            case CIRCLE:
                draw_circle(obj.x1, obj.y1, obj.x2); // x2 stores radius
                break;
        }
    }
}

// Bresenham's Line Generation Algorithm
void draw_line(int x1, int y1, int x2, int y2) {
    int dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
    int dy = -abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
    int err = dx + dy, e2;

    while (1) {
        put_pixel(x1, y1);
        if (x1 == x2 && y1 == y2) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x1 += sx; }
        if (e2 <= dx) { err += dx; y1 += sy; }
    }
}

void draw_rectangle(int x1, int y1, int x2, int y2) {
    draw_line(x1, y1, x2, y1);
    draw_line(x2, y1, x2, y2);
    draw_line(x2, y2, x1, y2);
    draw_line(x1, y2, x1, y1);
}

void draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3) {
    draw_line(x1, y1, x2, y2);
    draw_line(x2, y2, x3, y3);
    draw_line(x3, y3, x1, y1);
}

// Midpoint Circle Algorithm
void draw_circle(int xc, int yc, int r) {
    int x = 0, y = r;
    int d = 3 - 2 * r;
    while (y >= x) {
        put_pixel(xc + x, yc + y);
        put_pixel(xc - x, yc + y);
        put_pixel(xc + x, yc - y);
        put_pixel(xc - x, yc - y);
        put_pixel(xc + y, yc + x);
        put_pixel(xc - y, yc + x);
        put_pixel(xc + y, yc - x);
        put_pixel(xc - y, yc - x);
        x++;
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
        }
    }
}

// Main Window Refresher
void display_ui_and_canvas() {
    clear();
    render_canvas();

    // Print Canvas Frame
    for (int i = 0; i < MAX_ROWS; i++) {
        for (int j = 0; j < MAX_COLS; j++) {
            mvaddch(i, j, canvas[i][j]);
        }
    }

    // Print Status UI beneath the canvas
    mvprintw(MAX_ROWS + 1, 0, "=== 2D GRAPHICS EDITOR CONSOLE ===");
    mvprintw(MAX_ROWS + 2, 0, "[A] Add Object   [D] Delete Object   [M] Modify Object   [Q] Quit");
    mvprintw(MAX_ROWS + 4, 0, "Active Objects Count: %d", object_count);
    
    // Quick List window of current objects
    if (object_count > 0) {
        mvprintw(MAX_ROWS + 5, 0, "Objects: ");
        for (int i = 0; i < object_count && i < 5; i++) {
            char* types[] = {"Line", "Rectangle", "Triangle", "Circle"};
            printw("#%d (%s)  ", object_list[i].id, types[object_list[i].type]);
        }
    }
    refresh();
}

// Context Menu for adding objects with Letter triggers
void add_object_menu() {
    if (object_count >= MAX_OBJECTS) {
        mvprintw(MAX_ROWS + 6, 0, "Canvas History Full! Delete something first.");
        getch();
        return;
    }

    mvprintw(MAX_ROWS + 6, 0, "Select Shape Type: [L]ine  [R]ectangle  [T]riangle  [C]ircle  (Any other key cancels)");
    refresh();
    int type_choice = toupper(getch());

    GraphicObject new_obj;
    new_obj.id = next_id;

    // Echo input back temporarily for field coordination entries
    echo();
    curs_set(1);

    if (type_choice == 'L') {
        new_obj.type = LINE;
        mvprintw(MAX_ROWS + 7, 0, "Enter X1 Y1 X2 Y2 (e.g. 10 5 40 15): ");
        scanw("%d %d %d %d", &new_obj.x1, &new_obj.y1, &new_obj.x2, &new_obj.y2);
    } else if (type_choice == 'R') {
        new_obj.type = RECTANGLE;
        mvprintw(MAX_ROWS + 7, 0, "Enter Top-Left X Y and Bottom-Right X Y: ");
        scanw("%d %d %d %d", &new_obj.x1, &new_obj.y1, &new_obj.x2, &new_obj.y2);
    } else if (type_choice == 'T') {
        new_obj.type = TRIANGLE;
        mvprintw(MAX_ROWS + 7, 0, "Enter X1 Y1 X2 Y2 X3 Y3: ");
        scanw("%d %d %d %d %d %d", &new_obj.x1, &new_obj.y1, &new_obj.x2, &new_obj.y2, &new_obj.x3, &new_obj.y3);
    } else if (type_choice == 'C') {
        new_obj.type = CIRCLE;
        mvprintw(MAX_ROWS + 7, 0, "Enter Center X Y and Radius R: ");
        scanw("%d %d %d", &new_obj.x1, &new_obj.y1, &new_obj.x2); // using x2 to store radius
    } else {
        noecho();
        curs_set(0);
        return;
    }

    noecho();
    curs_set(0);
    
    object_list[object_count++] = new_obj;
    next_id++;
}

void delete_object_menu() {
    if (object_count == 0) return;
    
    int target_id;
    echo(); curs_set(1);
    mvprintw(MAX_ROWS + 6, 0, "Enter Object ID to Delete: ");
    scanw("%d", &target_id);
    noecho(); curs_set(0);

    int index = -1;
    for (int i = 0; i < object_count; i++) {
        if (object_list[i].id == target_id) {
            index = i;
            break;
        }
    }

    if (index != -1) {
        // Shift remaining items over
        for (int i = index; i < object_count - 1; i++) {
            object_list[i] = object_list[i + 1];
        }
        object_count--;
    } else {
        mvprintw(MAX_ROWS + 7, 0, "ID not found! Press any key.");
        getch();
    }
}

void modify_object_menu() {
    if (object_count == 0) return;

    int target_id;
    echo(); curs_set(1);
    mvprintw(MAX_ROWS + 6, 0, "Enter Object ID to Modify: ");
    scanw("%d", &target_id);

    int index = -1;
    for (int i = 0; i < object_count; i++) {
        if (object_list[i].id == target_id) {
            index = i;
            break;
        }
    }

    if (index != -1) {
        GraphicObject *obj = &object_list[index];
        mvprintw(MAX_ROWS + 7, 0, "Enter new measurements for this shape: ");
        
        if (obj->type == LINE || obj->type == RECTANGLE) {
            mvprintw(MAX_ROWS + 8, 0, "Enter X1 Y1 X2 Y2: ");
            scanw("%d %d %d %d", &obj->x1, &obj->y1, &obj->x2, &obj->y2);
        } else if (obj->type == TRIANGLE) {
            mvprintw(MAX_ROWS + 8, 0, "Enter X1 Y1 X2 Y2 X3 Y3: ");
            scanw("%d %d %d %d %d %d", &obj->x1, &obj->y1, &obj->x2, &obj->y2, &obj->x3, &obj->y3);
        } else if (obj->type == CIRCLE) {
            mvprintw(MAX_ROWS + 8, 0, "Enter Center X Y and Radius R: ");
            scanw("%d %d %d", &obj->x1, &obj->y1, &obj->x2);
        }
    } else {
        mvprintw(MAX_ROWS + 7, 0, "ID not found! Press any key.");
        noecho(); curs_set(0);
        getch();
    }
    
    noecho(); curs_set(0);
}