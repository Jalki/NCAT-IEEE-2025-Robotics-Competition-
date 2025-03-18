#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include <math.h>  // for fabs()

//As a reminder, the partial viewpoint print function visualizes already if a robot is
//able to rotate: there should be NO obstacles in view [other than the centre].
//otherwise, a rotation will not be possible

// ----- Configuration Constants (all in integer arithmetic) -----
// 2 cells per inch (resolution: ½‑inch per cell)
#define CELLS_PER_INCH 2

// Accessible (playable) field dimensions in inches
#define FIELD_WIDTH_IN 93       // accessible width in inches
#define PLAYABLE_HEIGHT_IN 45   // accessible height in inches

// Convert accessible area dimensions to cells
#define PLAYABLE_WIDTH_CELLS (FIELD_WIDTH_IN * CELLS_PER_INCH)      // 93 * 2 = 186
#define PLAYABLE_HEIGHT_CELLS (PLAYABLE_HEIGHT_IN * CELLS_PER_INCH)   // 45 * 2 = 90

// Border dimensions: 1.5" equals 3 cells (1.5 * 2 = 3)
#define BORDER_CELLS 3

// Effective (total) field dimensions in cells (with borders on all sides)
#define EFFECTIVE_WIDTH_CELLS (PLAYABLE_WIDTH_CELLS + 2 * BORDER_CELLS)   // 186 + 6 = 192
#define EFFECTIVE_HEIGHT_CELLS (PLAYABLE_HEIGHT_CELLS + 2 * BORDER_CELLS) // 90 + 6 = 96

// Playable area boundaries (cell indices) [for indexing purposes] as coordinates
#define PLAYABLE_LEFT  BORDER_CELLS
#define PLAYABLE_TOP   BORDER_CELLS
#define PLAYABLE_RIGHT (PLAYABLE_LEFT + PLAYABLE_WIDTH_CELLS - 1)    // e.g., 3 + 186 - 1 = 188
#define PLAYABLE_BOTTOM (PLAYABLE_TOP + PLAYABLE_HEIGHT_CELLS - 1)   // e.g., 3 + 90 - 1 = 92

// Robot "footprint" dimensions: 12×12 cells (half-size is 6 cells)
#define ROBOT_HALF_SIZE 12

// Rotation clearance: require a 15" square clearance around the robot.
// Since each cell is 0.5", 15" equals 30 cells, so half-size is 15 cells.
#define ROTATION_CLEARANCE_CELLS 18

// ----- Global Variables -----
// Grid stores the field (each cell is a character).
// 'B' marks borders, '0' marks accessible cells, and 'C' marks the robot's center.
char grid[EFFECTIVE_HEIGHT_CELLS][EFFECTIVE_WIDTH_CELLS];
int robot_row, robot_col;  // current position (grid indices) of the robot's center
char robot_dir = 'N';      // global robot facing direction (N, E, S, or W)

// ----- Function Prototypes -----
void initialize_grid(void);
void place_robot_random(void);
int can_move(int new_row, int new_col);
int can_rotate(void);
void print_surrounding_rows(void);
void move_robot_direction(char dir);  // moves robot in one of four directions
void rotate_robot(int angle);           // rotates robot by a multiple of 90 degrees
void set_robot_position(int new_row, int new_col); // relocates robot without clearance check
void print_entire_grid(void);
void debug_rotate_conditions(int start_row, int end_row, int start_col, int end_col);
void reconcile(double front_cm, double left_cm, double right_cm);

/*
int main(void) {
    srand(time(NULL));  // Seed the random number generator

    // Initialize the grid (borders and playable area).
    initialize_grid();

    // Place the robot at a random location within the accessible area.
    place_robot_random();

    // Show the initial 13x13 window.
    printf("Initial 13x13 window (centered on robot):\n");
    print_surrounding_rows();

    // Test moves:
    printf("\nAttempting to move left:\n");
    move_robot_direction('L');
    print_surrounding_rows();

    printf("\nAttempting to move up:\n");
    move_robot_direction('U');
    print_surrounding_rows();

    printf("\nAttempting to move right:\n");
    move_robot_direction('R');
    print_surrounding_rows();

    printf("\nAttempting to move down:\n");
    move_robot_direction('D');
    print_surrounding_rows();

    // --- Rotation Test Cases ---
    printf("\nTesting rotation at four corners:\n");

    // Top Left Corner: 7 inches (14 cells) from top and left edges:
    printf("\nRelocating robot to Top Left (7 inches from corner):\n");
    set_robot_position(PLAYABLE_TOP + 14, PLAYABLE_LEFT + 14); // (3+14, 3+14) = (17,17)
    rotate_robot(90);
	can_move(PLAYABLE_TOP + 12, PLAYABLE_LEFT + 12);
    print_surrounding_rows();

    // Top Right Corner: 7 inches from top and right edges:
    printf("\nRelocating robot to Top Right (7 inches from corner):\n");
    set_robot_position(PLAYABLE_TOP + 14, PLAYABLE_RIGHT - 14); // (3+14, 194-14) = (17,180)
	can_move(PLAYABLE_TOP + 12, PLAYABLE_RIGHT - 11);
    rotate_robot(90);
    print_surrounding_rows();

    // Bottom Left Corner: 7 inches from bottom and left edges:
    printf("\nRelocating robot to Bottom Left (7 inches from corner):\n");
    set_robot_position(PLAYABLE_BOTTOM - 14, PLAYABLE_LEFT + 14); // (92-14, 3+14) = (78,17)
	can_move(PLAYABLE_BOTTOM - 12, PLAYABLE_LEFT + 11);
    rotate_robot(90);
    print_surrounding_rows();

    // Bottom Right Corner: 7 inches from bottom and right edges:
    printf("\nRelocating robot to Bottom Right (7 inches from corner):\n");
    set_robot_position(PLAYABLE_BOTTOM - 14, PLAYABLE_RIGHT - 14); // (92-14, 194-14) = (78,180)
	can_move(PLAYABLE_BOTTOM - 12, PLAYABLE_RIGHT - 12);
    rotate_robot(90);
    print_surrounding_rows();
    print_entire_grid();
    return 0;
}
*/

// ----- New Rotation Function -----
// Rotates the robot by an angle (in degrees, multiple of 90, can be negative).
// Before updating the global facing direction, it checks that a 15" square
// (i.e., a (2*ROTATION_CLEARANCE_CELLS)x(2*ROTATION_CLEARANCE_CELLS) region)
// around the robot's center is free.
// If clearance is insufficient, the rotation is not allowed.
void rotate_robot(int angle) {
    if (angle % 90 != 0) {
        printf("Error: Rotation angle must be a multiple of 90.\n");
        return;
    }
    // Check if there is enough clearance for rotation.
    if (!can_rotate()) {
        printf("Rotation blocked: not enough clearance for a 15\" square around the robot.\n");
        return;
    }

    int delta = angle / 90;  // number of 90° steps

    // Map current direction to an index: 0 = N, 1 = E, 2 = S, 3 = W.
    int current_index;
    switch (robot_dir) {
        case 'N': current_index = 0; break;
        case 'E': current_index = 1; break;
        case 'S': current_index = 2; break;
        case 'W': current_index = 3; break;
        default: current_index = 0; break;
    }

    int new_index = (current_index - delta) % 4;
    if (new_index < 0)
        new_index += 4;

    char new_dirs[4] = {'N', 'E', 'S', 'W'};
    robot_dir = new_dirs[new_index];
    printf("Rotated robot by %d degrees. New facing direction: %c\n", angle, robot_dir);
}

// can_rotate() checks that a 15" square (30 cells by 30 cells) around the robot's center is free.
// It returns 1 if every cell in that region is either '0' or the robot's center 'C'.
// Otherwise, it returns 0.

void debug_rotate_conditions(int start_row, int end_row, int start_col, int end_col) {
    printf("DEBUG: start_row = %d, PLAYABLE_TOP = %d, (start_row < PLAYABLE_TOP) = %s\n",
           start_row, PLAYABLE_TOP, (start_row < PLAYABLE_TOP) ? "true" : "false");
    printf("DEBUG: end_row   = %d, PLAYABLE_BOTTOM = %d, (end_row > PLAYABLE_BOTTOM) = %s\n",
           end_row, PLAYABLE_BOTTOM, (end_row > PLAYABLE_BOTTOM) ? "true" : "false");
    printf("DEBUG: start_col = %d, PLAYABLE_LEFT = %d, (start_col < PLAYABLE_LEFT) = %s\n",
           start_col, PLAYABLE_LEFT, (start_col < PLAYABLE_LEFT) ? "true" : "false");
    printf("DEBUG: end_col   = %d, PLAYABLE_RIGHT = %d, (end_col > PLAYABLE_RIGHT) = %s\n",
           end_col, PLAYABLE_RIGHT, (end_col > PLAYABLE_RIGHT) ? "true" : "false");
}

int can_rotate(void) {
    int start_row = robot_row - ROTATION_CLEARANCE_CELLS;
    int end_row = robot_row + ROTATION_CLEARANCE_CELLS;
    int start_col = robot_col - ROTATION_CLEARANCE_CELLS;
    int end_col = robot_col + ROTATION_CLEARANCE_CELLS;

    debug_rotate_conditions(start_row, end_row, start_col, end_col);
    // Ensure the region is within the playable boundaries.
    if (start_row < PLAYABLE_TOP || end_row > PLAYABLE_BOTTOM ||
        start_col < PLAYABLE_LEFT || end_col > PLAYABLE_RIGHT)
        return 0;

    for (int i = start_row; i <= end_row; i++) {
        for (int j = start_col; j <= end_col; j++) {
            if (grid[i][j] != '0' && grid[i][j] != 'C')
                return 0;
        }
    }
    return 1;
}

// ----- New Movement Function -----
// Moves the robot one cell in the given direction:
// 'L' -> West, 'R' -> East, 'U' -> North, 'D' -> South.
// It checks that a 12x12 region around the candidate new center is free.
// If allowed, the robot's current position is cleared and updated.
// Note: The global facing direction (robot_dir) is NOT updated here.
void move_robot_direction(char rel_dir) {
    int new_row = robot_row;
    int new_col = robot_col;

    // Determine movement based on the robot's current facing direction and relative move.
    switch(rel_dir) {
        case 'F':  // Forward relative to robot's current facing.
            if (robot_dir == 'N') {
                new_row = robot_row - 1;
            } else if (robot_dir == 'S') {
                new_row = robot_row + 1;
            } else if (robot_dir == 'E') {
                new_col = robot_col + 1;
            } else if (robot_dir == 'W') {
                new_col = robot_col - 1;
            }
            break;
        case 'B':  // Backward relative to robot's facing.
            if (robot_dir == 'N') {
                new_row = robot_row + 1;
            } else if (robot_dir == 'S') {
                new_row = robot_row - 1;
            } else if (robot_dir == 'E') {
                new_col = robot_col - 1;
            } else if (robot_dir == 'W') {
                new_col = robot_col + 1;
            }
            break;
        case 'L':  // Left relative to robot's facing.
            if (robot_dir == 'N') {
                new_col = robot_col - 1;
            } else if (robot_dir == 'S') {
                new_col = robot_col + 1;
            } else if (robot_dir == 'E') {
                new_row = robot_row - 1;
            } else if (robot_dir == 'W') {
                new_row = robot_row + 1;
            }
            break;
        case 'R':  // Right relative to robot's facing.
            if (robot_dir == 'N') {
                new_col = robot_col + 1;
            } else if (robot_dir == 'S') {
                new_col = robot_col - 1;
            } else if (robot_dir == 'E') {
                new_row = robot_row + 1;
            } else if (robot_dir == 'W') {
                new_row = robot_row - 1;
            }
            break;
        default:
            printf("Invalid relative direction: %c\n", rel_dir);
            return;
    }

    if (can_move(new_row, new_col)) {
        grid[robot_row][robot_col] = '0';  // clear current position
        robot_row = new_row;
        robot_col = new_col;
        grid[robot_row][robot_col] = 'C';  // mark new position
        printf("Moved robot relative '%c' to (%d, %d). Facing remains: %c\n", rel_dir, robot_row, robot_col, robot_dir);
    } else {
        printf("Cannot move robot relative '%c' from (%d, %d) - blocked or out of bounds.\n", rel_dir, robot_row, robot_col);
    }
}


// ----- Provided Functions (modified) -----
// can_move() checks if a 12x12 footprint around the candidate new center is free ('0').
// (Note: It considers 'C' as the robot's current position, so that won't block movement.)
int can_move(int new_row, int new_col) {
    int start_row = new_row - ROBOT_HALF_SIZE;
    int end_row = new_row + ROBOT_HALF_SIZE;
    int start_col = new_col - ROBOT_HALF_SIZE;
    int end_col = new_col + ROBOT_HALF_SIZE;

    // Debug prints: show the candidate position and the computed footprint boundaries.
    printf("DEBUG: can_move() candidate: new_row = %d, new_col = %d\n", new_row, new_col);
    printf("DEBUG: Footprint boundaries: start_row = %d, end_row = %d, start_col = %d, end_col = %d\n",
           start_row, end_row, start_col, end_col);

    // Ensure the footprint stays within the playable boundaries.
    if (start_row < PLAYABLE_TOP || end_row > PLAYABLE_BOTTOM ||
        start_col < PLAYABLE_LEFT || end_col > PLAYABLE_RIGHT) {
        printf("DEBUG: Footprint out of playable boundaries.\n");
        return 0;
    }

    // Check that every cell in the footprint is free ('0').
    for (int i = start_row; i <= end_row; i++) {
        for (int j = start_col; j <= end_col; j++) {
            if (grid[i][j] != '0' && grid[i][j] != 'C') {
                printf("DEBUG: Obstacle found at (%d, %d): %c\n", i, j, grid[i][j]);
                return 0;
            }
        }
    }

    printf("DEBUG: Footprint is clear.\n");
    return 1;
}


// initialize_grid() marks border cells with 'B' and inner cells with '0'.
void initialize_grid(void) {
    for (int i = 0; i < EFFECTIVE_HEIGHT_CELLS; i++) {
        for (int j = 0; j < EFFECTIVE_WIDTH_CELLS; j++) {
            if (i < BORDER_CELLS || i >= EFFECTIVE_HEIGHT_CELLS - BORDER_CELLS ||
                j < BORDER_CELLS || j >= EFFECTIVE_WIDTH_CELLS - BORDER_CELLS)
                grid[i][j] = 'B';  // Border (inaccessible)
            else
                grid[i][j] = '0';  // Accessible (playable) area
        }
        grid[i][EFFECTIVE_WIDTH_CELLS] = '\0'; // Null-terminate each row for printing
    }
}

// place_robot_random() places the robot's center at a random location within the accessible area.
// The robot's center is marked with 'C'. (No direction indicator cell is set.)
void place_robot_random(void) {
    int min_row = PLAYABLE_TOP + ROBOT_HALF_SIZE;
    int max_row = PLAYABLE_BOTTOM - (ROBOT_HALF_SIZE - 1);
    int min_col = PLAYABLE_LEFT + ROBOT_HALF_SIZE;
    int max_col = PLAYABLE_RIGHT - (ROBOT_HALF_SIZE - 1);

    robot_row = min_row + rand() % (max_row - min_row + 1);
    robot_col = min_col + rand() % (max_col - min_col + 1);

    // Mark the robot center on the grid.
    grid[robot_row][robot_col] = 'C';
    // The global robot_dir remains as initially set (default 'N').
}

// set_robot_position() relocates the robot to a specified (new_row, new_col) coordinate.
// It clears the old robot position and marks the new one with 'C'.
void set_robot_position(int new_row, int new_col) {
    // Clear previous robot center.
    grid[robot_row][robot_col] = '0';
    robot_row = new_row;
    robot_col = new_col;
    grid[robot_row][robot_col] = 'C';
    printf("Robot relocated to (%d, %d).\n", robot_row, robot_col);
}

// print_surrounding_rows() prints a 13x13 window centered on the robot's position.
// It prints 15 rows above and 15 rows below and 15 columns to the left and right.
// The top and bottom headers print an asterisk at the robot's column.
// The left header prints an asterisk for the robot's row.
void print_surrounding_rows(void) {
    int start_row = robot_row - 15;
    int end_row = robot_row + 15;
    if (start_row < 0) start_row = 0;
    if (end_row >= EFFECTIVE_HEIGHT_CELLS) end_row = EFFECTIVE_HEIGHT_CELLS - 1;

    int start_col = robot_col - 15;
    int end_col = robot_col + 15;
    if (start_col < 0) start_col = 0;
    if (end_col >= EFFECTIVE_WIDTH_CELLS) end_col = EFFECTIVE_WIDTH_CELLS - 1;

    // Top header: for each column in the window, print an asterisk if it equals robot_col; otherwise, print a blank (or "|" at multiples of 5).
    printf("      ");
    for (int j = start_col; j <= end_col; j++) {
        if(j == robot_col){
            printf("%-5s", "*");}
        if (j % 5 == 0 && j != robot_col) {
            printf("%-5s", "|");
        }
    }
    printf("\n");

    // Print each row in the window.
    for (int i = start_row; i <= end_row; i++) {
        // Left header: print an asterisk if this row equals robot_row; otherwise, print the row number.
        if (i == robot_row)
            printf("%-5s ", "*");
        else
            printf("%-5d ", i);
        // Print the row's data for columns in the window.
        for (int j = start_col; j <= end_col; j++) {
            printf("%c", grid[i][j]);
        }
        printf("\n");
    }
    printf("      ");
    for (int j = start_col; j <= end_col; j++) {
        if(j == robot_col){
            printf("%-5s", "*");}
        if (j % 5 == 0 && j != robot_col) {
            printf("%-5s", "|");
        }
    }
    printf("\n");
}



void print_entire_grid(void) {
    // Print top header: loop over all columns in the grid.
    printf("      ");

    for (int j = 0; j < EFFECTIVE_WIDTH_CELLS; j++) {
        if(j == robot_col){
            printf("%-5s", "*");}
        if (j % 5 == 0 && j != robot_col) {
            printf("%-5s", "|");
        }
    }
    printf("\n");
    // Print every row.
    for (int i = 0; i < EFFECTIVE_HEIGHT_CELLS; i++) {
        if (i == robot_row)
            printf("%-5s ", "*");
        else
            printf("%-5d ", i);
        for (int j = 0; j < EFFECTIVE_WIDTH_CELLS; j++) {
            printf("%c", grid[i][j]);
        }
        printf("\n");
    }

    // Print bottom header (same as top header)
    printf("      ");
    for (int j = 0; j < EFFECTIVE_WIDTH_CELLS; j++) {
        if(j == robot_col){
            printf("%-5s", "*");}
        if (j % 5 == 0 && j != robot_col) {
            printf("%-5s", "|");
        }
    }
    printf("\n");
}


void reconcile(double front_cm, double left_cm, double right_cm) {
    // Convert sensor readings from cm to inches.
    double front_in = front_cm / 2.54;
    double left_in = left_cm / 2.54;
    double right_in = right_cm / 2.54;

    // Print the converted values for debugging.
    printf("Reconcile: front = %.2f in, left = %.2f in, right = %.2f in\n",
           front_in, left_in, right_in);

    // Check if any two readings differ by more than 1 inch.
    if (fabs(front_in - left_in) > 1.0 ||
        fabs(front_in - right_in) > 1.0 ||
        fabs(left_in - right_in) > 1.0) {



        int new_row = robot_row;
        int new_col = robot_col;

        // Check that the new position maintains at least ROBOT_HALF_SIZE cells of clearance.
        if (can_move(new_row, new_col)) {

            // Update the robot's position.
            grid[robot_row][robot_col] = '0';  // clear current position
            robot_row = new_row;
            robot_col = new_col;
            grid[robot_row][robot_col] = 'C';  // mark new position

            printf("Reconcile: Inconsistent sensor readings detected. ");
            printf("Adjusted position 0.5 inch forward to (%d, %d).\n", robot_row, robot_col);
        } else {
            printf("Reconcile: Adjustment not possible due to clearance restrictions.\n");
        }
    } else {
        printf("Reconcile: Sensor readings consistent within 1 inch.\n");
    }
}
