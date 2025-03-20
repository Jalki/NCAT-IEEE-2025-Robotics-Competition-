#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
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
#define PO(x) ((x) + BORDER_CELLS)
int lastGoodCoord[2];

// ----- Global Variables -----
// Grid stores the field (each cell is a character).
// 'B' marks borders, '0' marks accessible cells, and 'C' marks the robot's center.
char grid[EFFECTIVE_HEIGHT_CELLS][EFFECTIVE_WIDTH_CELLS];
int robot_row, robot_col, Nboxrow, Nboxcol, Gboxrow, Gboxcol;  // current position (grid indices) of the robot's center
char robot_dir = 'N';      // global robot facing direction (N, E, S, or W)
char lastevictedelement = '0';// this must be guaranteed or this wont work properly

// ----- Function Prototypes -----
// Note: underscores have been removed from function names in both declarations and comments.
//top level user functions
void moverobotdirection(char rel_dir, double distance_in);  // moves robot in one of four directions
void rotaterobot(int angle);          // rotates robot by a multiple of 90 degrees=
void reconcile(double front_cm, double left_cm, double right_cm);
void initalizemovement(void);

//helper functioins
void initializegrid(void);
void placerobotrandom(void);
int canmove(int new_row, int new_col, int *lastrow, int *lastcol);
int canrotate(void);
void printsurroundingrows(void);
void printentiregrid(void);
void debugrotateconditions(int start_row, int end_row, int start_col, int end_col);
void setrobotposition(int new_row, int new_col); // relocates robot without clearance check
void initregions(void);
int canmoveto(double delta_row_in, double delta_col_in);
int canmovetorecursive(int cur_row, int cur_col, int dest_row, int dest_col);


void initalizemovement(void){
    initializegrid();
    initregions();
	printf("The movement system is initialized, the board is:\n");
	printentiregrid();
	printf("The rotation extrema viewpoint is:\n");
	printsurroundingrows();
    
    
}
int main(void) {
    srand(time(NULL));  // Seed the random number generator

    // Initialize grid and regions.
    initalizemovement();


    // --- Test moverobotdirection and then perform a rotation test after each move ---
    
    // Test 1: Move up extent.
    // For a robot facing North, a forward ('F') movement of 32 inches should move it upward.
    printf("\nTest 1: Move up extent: calling moverobotdirection('F', 32.0)...\n");
    moverobotdirection('F',3+ 3);
    printsurroundingrows();
    printf("\nRotation Test 1: Rotating robot 90 degrees...\n");
    rotaterobot(90);
    printsurroundingrows();

    // Test 2: Move down extent.
    // For a robot facing (after rotation) the appropriate direction, use 'B' for backward.
    printf("\nTest 2: Move down extent: calling moverobotdirection('B', 2.0)...\n");
    moverobotdirection('B', 2.0);
    printsurroundingrows();
    printf("\nRotation Test 2: Rotating robot -90 degrees...\n");
    rotaterobot(-90);
    printsurroundingrows();

    // Test 3: Move right extent.
    printf("\nTest 3: Move right extent: calling moverobotdirection('R', 10.5)...\n");
    moverobotdirection('R', 10.5);
    printsurroundingrows();
    printf("\nRotation Test 3: Rotating robot 180 degrees...\n");
    rotaterobot(180);
    printsurroundingrows();

    // Test 4: Move left extent.
    printf("\nTest 4: Move left extent: calling moverobotdirection('L', 25.0)...\n");
    moverobotdirection('L', 25.0);
    printsurroundingrows();
    printf("\nRotation Test 4: Rotating robot 90 degrees...\n");
    rotaterobot(90);
    printsurroundingrows();

    // Test 5: Error test - invalid relative direction.
    printf("\nTest 5: Error test: calling moverobotdirection('X', 1.0)...\n");
    moverobotdirection('X', 1.0);

    return 0;
}




// ----- New Rotation Function -----
// Rotates the robot by an angle (in degrees, multiple of 90, can be negative).
// Before updating the global facing direction, it checks that a 15" square
// (i.e., a (2*ROTATION_CLEARANCE_CELLS)x(2*ROTATION_CLEARANCE_CELLS) region)
// around the robot's center is free.
// If clearance is insufficient, the rotation is not allowed.
void rotaterobot(int angle) {
    if (angle % 90 != 0) {
        printf("Error: Rotation angle must be a multiple of 90.\n");
        return;
    }
    // Check if there is enough clearance for rotation.
    if (!canrotate()) {
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

// canrotate() checks that a 15" square (30 cells by 30 cells) around the robot's center is free.
// It returns 1 if every cell in that region is either '0' or the robot's center 'C'.
// Otherwise, it returns 0.

void debugrotateconditions(int start_row, int end_row, int start_col, int end_col) {
    printf("DEBUG: start_row = %d, PLAYABLE_TOP = %d, (start_row < PLAYABLE_TOP) = %s\n",
           start_row, PLAYABLE_TOP, (start_row < PLAYABLE_TOP) ? "true" : "false");
    printf("DEBUG: end_row   = %d, PLAYABLE_BOTTOM = %d, (end_row > PLAYABLE_BOTTOM) = %s\n",
           end_row, PLAYABLE_BOTTOM, (end_row > PLAYABLE_BOTTOM) ? "true" : "false");
    printf("DEBUG: start_col = %d, PLAYABLE_LEFT = %d, (start_col < PLAYABLE_LEFT) = %s\n",
           start_col, PLAYABLE_LEFT, (start_col < PLAYABLE_LEFT) ? "true" : "false");
    printf("DEBUG: end_col   = %d, PLAYABLE_RIGHT = %d, (end_col > PLAYABLE_RIGHT) = %s\n",
           end_col, PLAYABLE_RIGHT, (end_col > PLAYABLE_RIGHT) ? "true" : "false");
}

int canrotate(void) {
    int start_row = robot_row - ROTATION_CLEARANCE_CELLS;
    int end_row = robot_row + ROTATION_CLEARANCE_CELLS;
    int start_col = robot_col - ROTATION_CLEARANCE_CELLS;
    int end_col = robot_col + ROTATION_CLEARANCE_CELLS;

    debugrotateconditions(start_row, end_row, start_col, end_col);
    // Ensure the region is within the playable boundaries.
    if (start_row < PLAYABLE_TOP || end_row > PLAYABLE_BOTTOM ||
        start_col < PLAYABLE_LEFT || end_col > PLAYABLE_RIGHT)
        return 0;

    for (int i = start_row; i <= end_row; i++) {
        for (int j = start_col; j <= end_col; j++) {
            if (grid[i][j] != '0' && grid[i][j] != 'C' && grid[i][j] != '1' && grid[i][j] != '2' && grid[i][j] != '3' && grid[i][j] != '4' && grid[i][j] != 'T')
                return 0;
        }
    }
	//for boxes
	    for (int i = start_row-6; i <= end_row+6; i++) {
        for (int j = start_col-6; j <= end_col+6; j++) {
            if (grid[i][j] == 'G' || grid[i][j] == 'N')
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
void moverobotdirection(char rel_dir, double distance_in) {
    double delta_row_in = 0.0;
    double delta_col_in = 0.0;

    // Determine movement delta (in inches) based on robot_dir and relative direction.
    switch (rel_dir) {
        case 'F':  // Forward relative to current facing.
            if (robot_dir == 'N')
                delta_row_in = -distance_in;
            else if (robot_dir == 'S')
                delta_row_in = distance_in;
            else if (robot_dir == 'E')
                delta_col_in = distance_in;
            else if (robot_dir == 'W')
                delta_col_in = -distance_in;
            break;
        case 'B':  // Backward relative to current facing.
            if (robot_dir == 'N')
                delta_row_in = distance_in;
            else if (robot_dir == 'S')
                delta_row_in = -distance_in;
            else if (robot_dir == 'E')
                delta_col_in = -distance_in;
            else if (robot_dir == 'W')
                delta_col_in = distance_in;
            break;
        case 'L':  // Left relative to current facing.
            if (robot_dir == 'N')
                delta_col_in = -distance_in;
            else if (robot_dir == 'S')
                delta_col_in = distance_in;
            else if (robot_dir == 'E')
                delta_row_in = -distance_in;
            else if (robot_dir == 'W')
                delta_row_in = distance_in;
            break;
        case 'R':  // Right relative to current facing.
            if (robot_dir == 'N')
                delta_col_in = distance_in;
            else if (robot_dir == 'S')
                delta_col_in = -distance_in;
            else if (robot_dir == 'E')
                delta_row_in = distance_in;
            else if (robot_dir == 'W')
                delta_row_in = -distance_in;
            break;
        default:
            printf("Invalid relative direction: %c\n", rel_dir);
            return;
    }

    // Use canmoveto (which expects delta values in inches) to check the path.
    if (canmoveto(delta_row_in, delta_col_in)) {
        // Convert the delta from inches to cells.
        int d_row = (int) round(delta_row_in * CELLS_PER_INCH);
        int d_col = (int) round(delta_col_in * CELLS_PER_INCH);
        int new_row = robot_row + d_row;
        int new_col = robot_col + d_col;
        setrobotposition(new_row, new_col);
        printf("Moved robot relative '%c' by %.2f inches to (%d, %d). Facing remains: %c\n",
               rel_dir, distance_in, robot_row, robot_col, robot_dir);
    } else {
        printf("Cannot move robot relative '%c' by %.2f inches from (%d, %d) - blocked or out of bounds.\n",
               rel_dir, distance_in, robot_row, robot_col);
    }
}



// ----- Provided Functions (modified) -----
// canmove() checks if a 12x12 footprint around the candidate new center is free ('0').
// (Note: It considers 'C' as the robot's current position, so that won't block movement.)
int canmove(int new_row, int new_col, int *lastrow, int *lastcol) {
    int start_row = new_row - ROBOT_HALF_SIZE;
    int end_row = new_row + ROBOT_HALF_SIZE;
    int start_col = new_col - ROBOT_HALF_SIZE;
    int end_col = new_col + ROBOT_HALF_SIZE;

    // Debug prints: show the candidate position and the computed footprint boundaries.
    printf("DEBUG: canmove() candidate: new_row = %d, new_col = %d\n", new_row, new_col);
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
            if (grid[i][j] != '0' && grid[i][j] != 'C' && grid[i][j] != '1' && grid[i][j] != '2' && grid[i][j] != '3' && grid[i][j] != '4' && grid[i][j] != 'T') {
				//difference: i-lastrow, j-lastcol
				int drC = i - *lastrow;
				int dcC = j - *lastcol;
				double drow = (double)drC / CELLS_PER_INCH;
				double dcol = (double)dcC / CELLS_PER_INCH;
                printf("DEBUG: Obstacle found at (%d, %d): %c. The conflict delta is <%.2f,%.2f>\n", i, j, grid[i][j], drow,dcol);
                return 0;
            } else {
			
			*lastrow = new_row;//if this run was good, that means our current position is valid
			*lastcol = new_col;
				
			}
        }
    }
//for box check
    for (int i = start_row-6; i <= end_row+6; i++) {
        for (int j = start_col-6; j <= end_col+6; j++) {
            if (grid[i][j] == 'N' || grid[i][j] == 'G') {
                printf("DEBUG: Box found at (%d, %d): %c\n", i, j, grid[i][j]);
				int drC = i - *lastrow;
				int dcC = j - *lastcol;
				double drow = (double)drC / CELLS_PER_INCH;
				double dcol = (double)dcC / CELLS_PER_INCH;
                printf("DEBUG: Box found at (%d, %d): %c. The conflict delta is <%.2f,%.2f>\n", i, j, grid[i][j], drow,dcol);
                return 0;
            } else {
			
			*lastrow = new_row;//if this run was good, that means our current position is valid
			*lastcol = new_col;
				
			}
        }
    }
    printf("DEBUG: Footprint is clear.\n");
    return 1;
}


// Recursive helper: Checks if the path from (cur_row, cur_col) to (dest_row, dest_col) is clear.
// Assumes that the movement is along one axis only.
int canmovetorecursive(int cur_row, int cur_col, int dest_row, int dest_col) {
    // Base case: reached destination.
    if (cur_row == dest_row && cur_col == dest_col)
        return 1;
    
    int next_row = cur_row;
    int next_col = cur_col;
    if (cur_row != dest_row) {
        next_row = (cur_row < dest_row) ? cur_row + 1 : cur_row - 1;
    } else if (cur_col != dest_col) {
        next_col = (cur_col < dest_col) ? cur_col + 1 : cur_col - 1;
    }
    
    // Check if next cell is free.
    if (!canmove(next_row, next_col, lastGoodCoord, lastGoodCoord + 1)) {
		int drowC = cur_row - robot_row;
		int dcolC = cur_col - robot_col;
		double drow = (double)drowC / CELLS_PER_INCH;
		double dcol = (double)dcolC / CELLS_PER_INCH;
        printf("Error: Path blocked in way of at (%d, %d), but CAN be moved to <%d, %d>. Correction delta to use <%.2f,%.2f>: \n", next_row, next_col, lastGoodCoord[0], lastGoodCoord[1], drow, dcol);
        return 0;
    }
    
    return canmovetorecursive(next_row, next_col, dest_row, dest_col);
}

// Main function: canmoveto
// Accepts a delta in inches along one axis only (the other must be 0). 
// It converts the delta to cells and uses the current robot position as the start.
// Returns 1 if the entire path is clear, 0 otherwise.
int canmoveto(double delta_row_in, double delta_col_in) {
    // Check that movement is along one axis only.

    printf("DEBUG: fabs(delta_row_in) = %.4f, fabs(delta_col_in) = %.6f\n",
       (delta_row_in), (delta_col_in));


    if ((delta_row_in != 0.0 && delta_col_in != 0.0)) {
        printf("Error: Movement delta must be along one axis only.\n");
        return 0;
    }
    
    // Convert the delta from inches to cells.
    int d_row = (int) (delta_row_in * CELLS_PER_INCH);
    int d_col = (int) (delta_col_in * CELLS_PER_INCH);
    
    int dest_row = robot_row + d_row;
    int dest_col = robot_col + d_col;
    
    // Print debug info:
    printf("DEBUG: canmoveto: Current position: (%d, %d). Delta: (%d, %d) <Y,x>. Destination <R,C>: (%d, %d).\n",
           robot_row, robot_col, d_row, d_col, dest_row, dest_col);
    
    return canmovetorecursive(robot_row, robot_col, dest_row, dest_col);
}


// initializegrid() marks border cells with 'B' and inner cells with '0'.
void initializegrid(void) {
    for (int i = 0; i < EFFECTIVE_HEIGHT_CELLS; i++) {
        for (int j = 0; j < EFFECTIVE_WIDTH_CELLS; j++) {
            if (i < BORDER_CELLS || i >= EFFECTIVE_HEIGHT_CELLS - BORDER_CELLS ||
                j < BORDER_CELLS || j >= EFFECTIVE_WIDTH_CELLS - BORDER_CELLS)
                grid[i][j] = 'B';  // Border (inaccessible)
            else
                grid[i][j] = '0';  // Accessible (playable) area
        }
        //grid[i][EFFECTIVE_WIDTH_CELLS] = '\0'; // Null-terminate each row for printing
    }
}

// placerobotrandom() places the robot's center at a random location within the accessible area.
// The robot's center is marked with 'C'. (No direction indicator cell is set.)
void placerobotrandom(void) {
    int min_row = PLAYABLE_TOP + ROBOT_HALF_SIZE;
    int max_row = PLAYABLE_BOTTOM - (ROBOT_HALF_SIZE - 1);
    int min_col = PLAYABLE_LEFT + ROBOT_HALF_SIZE;
    int max_col = PLAYABLE_RIGHT - (ROBOT_HALF_SIZE - 1);

    //robot_row = min_row + rand() % (max_row - min_row + 1);
   //robot_col = min_col + rand() % (max_col - min_col + 1);

    // Mark the robot center on the grid.
    setrobotposition(min_row + rand() % (max_row - min_row + 1),min_col + rand() % (max_col - min_col + 1));
    // The global robot_dir remains as initially set (default 'N').
}

// setrobotposition() relocates the robot to a specified (new_row, new_col) coordinate.
// It clears the old robot position and marks the new one with 'C'.
void setrobotposition(int new_row, int new_col) {//row = y, col= x
    // Check that the target cell contains a valid element.
    char valid_chars[] = {'0', 'T', '1', '2', '3', '4'};
    int valid = 0;
    for (int i = 0; i < sizeof(valid_chars)/sizeof(valid_chars[0]); i++) {
        if (grid[(new_row)][(new_col)] == valid_chars[i]) {
            valid = 1;
            break;
        }
    }
    if (!valid) {
        printf("Error: Cannot relocate robot to invalid cell value '%c' at (%d, %d).\n",
               grid[(new_row)][(new_col)], (new_row), (new_col));
        return;
    }

    // Save the current value at the new location.
    
    printf("Robot current location is (%d, %d). Last evicted element: '%c'\n", 
           robot_row, robot_col, lastevictedelement);
    // Clear previous robot center.
    grid[robot_row][(robot_col)] =  lastevictedelement;
    robot_row = new_row;
    robot_col = new_col;
	lastevictedelement = grid[(new_row)][(new_col)];
    grid[(robot_row)][(robot_col)] = 'C';
	lastGoodCoord[0] = robot_row;
	lastGoodCoord[1] = robot_col;
    printf("ROBOT RELOCATED TO (%d, %d)!!!! Last evicted element: '%c'\n", 
           robot_row, robot_col, lastevictedelement);
	//printentiregrid();
}


// printsurroundingrows() prints a 13x13 window centered on the robot's position.
// It prints 15 rows above and 15 rows below and 15 columns to the left and right.
// The top and bottom headers print an asterisk at the robot's column.
// The left header prints an asterisk for the robot's row.
void printsurroundingrows(void) {
    int start_row = robot_row - ROTATION_CLEARANCE_CELLS;
    int end_row = robot_row + ROTATION_CLEARANCE_CELLS;
    if (start_row < 0) start_row = 0;
    if (end_row >= EFFECTIVE_HEIGHT_CELLS) end_row = EFFECTIVE_HEIGHT_CELLS - 1;

    int start_col = robot_col - ROTATION_CLEARANCE_CELLS;
    int end_col = robot_col + ROTATION_CLEARANCE_CELLS;
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



void printentiregrid(void) {
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
        if (canmove(new_row, new_col, lastGoodCoord, lastGoodCoord + 1)) {
			//TODO: [warning] two function explicity set robot centre
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




void initregions(void) {
    int i, j;
	Nboxrow = 6;
	Nboxcol = 47;
	Gboxrow = 83;
	Gboxcol = 102;
	//robot origin = x=~ 62, y=81
    // Region 1: from <0,0> to <24,17> = '4'
	setrobotposition(80, 65);
    for (j = 0; j < 24; j++) {     // x coordinate
        for (i = 0; i <= 17; i++) { // y coordinate
            grid[PO(i)][PO(j)] = '4';//ok
        }
    }

    for (j = 0; j < 24; j++) {     // x coordinate
        for (i = 17; i <= 19; i++) { // y coordinate
            grid[PO(i)][PO(j)] = 'T';//ok
        }
    }
    
    
    // Region 2: from <0,17> to <24,33> = '3'
    for (j = 0; j < 24; j++) {
        for (i = 19; i <= 35; i++) {
            grid[PO(i)][PO(j)] = '3';//ok
        }
    }
    
    for (j = 0; j < 24; j++) {     // x coordinate
        for (i = 35; i <= 37; i++) { // y coordinate
            grid[PO(i)][PO(j)] = 'T';//ok
        }
    }
    
    // Region 3: from <0,33> to <24,49> = '2'
    for (j = 0; j < 24; j++) {
        for (i = 37; i <= 53; i++) {
            grid[PO(i)][PO(j)] = '2';//
        }
    }
    
    for (j = 0; j < 24; j++) {     // x coordinate
        for (i = 53; i <= 55; i++) { // y coordinate
            grid[PO(i)][PO(j)] = 'T';//
        }
    }
    
    // Region 4: from <0,49> to <24,81> = '1'
    for (j = 0; j < 24; j++) {
        for (i = 55; i <= 71; i++) {
            grid[PO(i)][PO(j)] = '1';//
        }
    }
    for (j = 0; j < 24; j++) {     // x coordinate
        for (i = 71; i <= 73; i++) { // y coordinate
            grid[PO(i)][PO(j)] = 'T';//ok
        }
    }
    
    // Region 5: from <0,81> to <24,98> = '0'
    for (j = 0; j < 24; j++) {
        for (i = 73; i <= 89; i++) {
            grid[PO(i)][PO(j)] = '0';
        }
    }
	//boxes initalized <R,C>
	
    grid[PO(Nboxrow)][PO(Nboxcol)] = 'N';
    grid[PO(Gboxrow)][PO(Gboxcol)] = 'G';
    
    // Region 7: from <98,90> to <110,78> = 'G'
    // With y axis pointing downward, we interpret this as the region spanning
    // x from 98 to 110 and y from 78 to 90.

    // Region 8: from <110,90> to <123,61> = 'W'
    // Interpreted as x from 110 to 123 and y from 61 to 90.
    for (j = 110; j <= 123; j++) {
        for (i = 61; i <= 89; i++) {
            grid[PO(i)][PO(j)] = 'W';
        }
    }
    // Region 9: mirrored region: from <110,0> to <123,29> = 'W'
    for (j = 110; j <= 123; j++) {
        for (i = 0; i <= 28; i++) {
            grid[PO(i)][PO(j)] = 'W';
        }
    }
    // Region 10: from <186,0> to <181,3> = 'S'
    // Interpreted as x from 181 to 186 and y from 0 to 3.
    for (j = 179; j <= 185; j++) {
        for (i = 0; i <= 2; i++) {
            grid[PO(i)][PO(j)] = 'S';
        }
    }
    // Region 11: from <186,90> to <181,87> = 'S'
    // Interpreted as x from 181 to 186 and y from 87 to 90.
    for (j = 179; j <= 185; j++) {
        for (i = 87; i <= 89; i++) {
            grid[PO(i)][PO(j)] = 'S';
        }
    }
}


