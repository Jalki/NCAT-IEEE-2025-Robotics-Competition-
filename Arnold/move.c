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
#define CAVE_ENTRY_YAXIS 45//this is relative to the playable area, not the entire board with borders
// Rotation clearance: require a 15" square clearance around the robot.
// Since each cell is 0.5", 15" equals 30 cells, so half-size is 15 cells.
#define ROTATION_CLEARANCE_CELLS 18
#define PO(x) ((x) + BORDER_CELLS)

typedef struct {
    int row;
    int col;
    char dir;
} RobotState;



// ----- Global Variables -----
// Grid stores the field (each cell is a character).
// 'B' marks borders, '0' marks accessible cells, and 'C' marks the robot's center.
int lastGoodCoord[2];
char grid[EFFECTIVE_HEIGHT_CELLS][EFFECTIVE_WIDTH_CELLS];
int robot_row, robot_col, Nboxrow, Nboxcol, Gboxrow, Gboxcol;  // current position (grid indices) of the robot's center
char robot_dir = 'N';      // global robot facing direction (N, E, S, or W)
char lastevictedelement = '0';// this must be guaranteed or this wont work properly

// ----- Function Prototypes -----
// Note: underscores have been removed from function names in both declarations and comments.
//top level user functions

void rotaterobot(int angle);          // rotates robot by a multiple of 90 degrees=
void reconcile(double front_cm, double left_cm, double right_cm);
void initalizemovement(void);
int moverobotxy(double target_x, double target_y);//NEW: By popular demand, this was made
//Note: this is NOT a minmax algorithm since there are only 2 cases
void printboxconflicts(void);//Best for viewing wall/box conflicts [unreliable for general diagonosis]
int alignYcave(void);
void getrobotparams(void);//prints robot rotation and position

//helper functioins
int moverobotdirection(char rel_dir, double distance_in);  // moves robot in one of four directions
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
RobotState getRobotState(void) ;
void restoreRobotState(RobotState state);
void runEdgeCaseTests(void);


int main(void) {
    srand(time(NULL));  // Seed the random number generator

    // Initialize grid and regions.
    initalizemovement();
	runEdgeCaseTests();

    return 0;
}


// Example snippet demonstrating various navigation tests using moverobotxy()
// Each call attempts to move the robot’s center to an absolute (x,y) in inches
// relative to the playable area (0,0 in the top-left).
// Ensure that moverobotxy() and any needed global variables are already defined.

void runEdgeCaseTests(void) {
    // 1. Start: place the robot at some known location, e.g. near the top-left corner
    //    or the center of the field. For example:
    moverobotdirection('B', 0.5);
    moverobotxy(6,12);  // ~12 inches in from top-left

    getrobotparams();

    printf("\n[TEST] Robot placed near top-left corner, facing north.\n");

    if (!alignYcave()) {
        printf("[ERROR] Could not align to cave.\n");
    }

    // 2. Move to the “cave” area on the right side. Suppose the cave entrance is near x=80, y=20.
    //    (Adjust these coordinates to match your actual field.)
    printf("\n[TEST] Moving into the cave on the right side.\n");
    if (!moverobotxy(68,6)) {
        printf("[ERROR] Could not navigate to (80,20) in the cave entrance.\n");
    }

    // 3. Move deeper into the cave, say near x=85, y=35. (Hypothetical corridor.)
    //    The code tries “x-then-y” and “y-then-x” paths automatically.
    printf("\n[TEST] Going deeper into the cave.\n");
    if (!moverobotxy(68,38.5)) {
        printf("[ERROR] Could not navigate deeper inside the cave at (85,35).\n");
    }

    printf("\n[FTEST] LOWER STUD TEST\n");
    if (!moverobotxy(86.5,38.5)) {
        printf("[ERROR] Could not pass lower studs.\n");//PASS
    }

    printf("\n[FTEST] UPPER STUD TEST\n");
    if (!moverobotxy(86.5,6)) {
        printf("[ERROR] Could not pass upper studs.\n");//PASS
    }
    // 6. Next, top-right corner is (93,0).
    printf("\n[TEST] Navigating to top-right playable corner (93,0).\n");
    if (!moverobotxy(93.0, 0.0)) {
        printf("[ERROR] Could not reach top-right corner.\n");//PASS
    }

    // 7. Bottom-right corner is (93,45).
    printf("\n[TEST] Navigating to bottom-right playable corner (93,45).\n");
    if (!moverobotxy(93.0, 45.0)) {
        printf("[ERROR] Could not reach bottom-right corner.\n");
    }

    // 8. Bottom-left corner is (0,45).
    printf("\n[TEST] Y-Aligning and finishing test.\n");
    alignYcave();
    printf("\n[TEST] RIGHTMOST TEST\n");
    if (!moverobotxy(86.5,22.5)) {//maxed right side
        printf("[ERROR] Could rightmost cave..\n");//PASS
    }

    printf("\n[TEST] LOWER STUD TEST\n");
    if (!moverobotxy(86.5,37)) {
        printf("[ERROR] Could not pass lower studs.\n");//PASS
    }

    printf("\n[TEST] UPPER STUD TEST\n");
    if (!moverobotxy(86.5,7.5)) {
        printf("[ERROR] Could not pass upper studs.\n");//PASS
    }

    printf("y aligning...");
    alignYcave();

    printf("\n[TEST] UPPER RIGHT CORNER OUTSIDE CAVE\n");
    if (!moverobotxy(48.5,6)) {
        printf("[ERROR] Could not to outside cave corner\n");//PASS
    }


       printf("\n[TEST] UUNOCCUPIED G BOX\n");
    if (!moverobotxy(48.5,38.5)) {
        printf("[ERROR] Could not to outside cave corner\n");//PASS
    }

    printf("\n[TEST] HOME TEST\n");
    if (!moverobotxy(38.5,31)) {
        printf("[ERROR] Could not go home\n");//PASS
    }



    printf("\n[TEST] Finished edge-case tests.\n");
}

void initalizemovement(void){
    initializegrid();
    initregions();
	printf("The movement system is initialized, the board is:\n");
	printentiregrid();
	printf("The rotation extrema viewpoint is:\n");
	printsurroundingrows();
    
    
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
        printboxconflicts();
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
        start_col < PLAYABLE_LEFT || end_col > PLAYABLE_RIGHT) {
        printf("DEBUG: Region for rotation is out of playable boundaries.\n");
        return 0;
    }

    // Check that every cell in the region is allowed.
    for (int i = start_row; i <= end_row; i++) {
        for (int j = start_col; j <= end_col; j++) {
            if (grid[i][j] != '0' && grid[i][j] != 'C' &&
                grid[i][j] != '1' && grid[i][j] != '2' &&
                grid[i][j] != '3' && grid[i][j] != '4' &&
                grid[i][j] != 'T' && grid[i][j] != 'G' && grid[i][j] != 'N') {//theoretically boxes shouldnt be detected first
                printf("DEBUG: canrotate() found invalid cell at (%d, %d): '%c'\n", i, j, grid[i][j]);
                return 0;
            } else {
               // printf("DEBUG: canrotate() valid cell at (%d, %d): '%c'\n", i, j, grid[i][j]);
            }
        }
    }

    //CLAMPING: For reasons discovered from moving along the edge of the map
    int min_row_box = start_row - 6;
    int max_row_box = end_row + 6;
    int min_col_box = start_col - 6;
    int max_col_box = end_col + 6;

    // Clamp the extended region to the entire field dimensions (i.e. the full grid)
    if (min_row_box < 0)                    min_row_box = 0;
    if (max_row_box > EFFECTIVE_HEIGHT_CELLS - 1)  max_row_box = EFFECTIVE_HEIGHT_CELLS - 1;
    if (min_col_box < 0)                    min_col_box = 0;
    if (max_col_box > EFFECTIVE_WIDTH_CELLS - 1)   max_col_box = EFFECTIVE_WIDTH_CELLS - 1;

    // For box check: if any cell in an extended region is marked 'G' or 'N', return 0.
    for (int i = min_row_box; i <= max_row_box; i++) {
        for (int j = min_col_box; j <= max_col_box; j++) {\
            if (grid[i][j] == 'G' || grid[i][j] == 'N') {
                printf("DEBUG: canrotate() found box at (%d, %d): '%c'\n", i, j, grid[i][j]);
                return 0;
            } else {
               // printf("DEBUG: canrotate() box-check valid cell at (%d, %d): '%c'\n", i, j, grid[i][j]);
            }
        }
    }
    
   // printf("DEBUG: canrotate() - Region is clear for rotation.\n");
    return 1;
}


RobotState getRobotState(void) {
    RobotState state;
    state.row = robot_row;
    state.col = robot_col;
    state.dir = robot_dir;
    return state;
}

void restoreRobotState(RobotState state) {
    setrobotposition(state.row, state.col);
    robot_dir = state.dir;
}

// moverobotxy: Moves the robot to an absolute target (x,y) given in inches relative to the playable area
// (with (0,0) as the upper-left corner). It attempts two orders of movement (x then y, or y then x)
// using moverobotdirection, and returns 1 on success, 0 on failure.
int moverobotxy(double target_x, double target_y) {//todo: track successful deltas [nothing if none] to send to the arduino
    // Convert target coordinates (inches relative to playable area) to grid coordinates.
    int target_col = (int) round(target_x * CELLS_PER_INCH) + BORDER_CELLS;
    int target_row = (int) round(target_y * CELLS_PER_INCH) + BORDER_CELLS;

    // Compute current position in inches relative to playable area.
    double current_x = (robot_col - BORDER_CELLS) / (double)CELLS_PER_INCH;
    double current_y = (robot_row - BORDER_CELLS) / (double)CELLS_PER_INCH;

    double dx = target_x - current_x; // horizontal difference in inches
    double dy = target_y - current_y; // vertical difference in inches
    char primarydeltaaxis = '0';//default: 0 means no movement is allowed
    printf("moverobotxy: Target (inches): (%.2f, %.2f), Current (inches): (%.2f, %.2f), dx = %.2f, dy = %.2f\n",
           target_x, target_y, current_x, current_y, dx, dy);

    // Save original robot state.
    RobotState orig = getRobotState();
    int success = 0;

    // Order 1: Move horizontally (x) then vertically (y).
    printf("moverobotxy: Trying order 1 (x then y)...\n");
    primarydeltaaxis = 'x';
    if (dx != 0.0) {
        char rel_x;
        if (robot_dir == 'N')
            rel_x = (dx > 0) ? 'R' : 'L';
        else if (robot_dir == 'S')
            rel_x = (dx > 0) ? 'L' : 'R';
        else if (robot_dir == 'E')
            rel_x = (dx > 0) ? 'F' : 'B';
        else if (robot_dir == 'W')
            rel_x = (dx > 0) ? 'B' : 'F';
        else
            rel_x = 'F';
        if (!moverobotdirection(rel_x, (dx > 0) ? dx : -dx)) {
            //printf("moverobotxy: Order 1 failed during horizontal move.\n");
            restoreRobotState(orig);//possibly redundant
            goto try_order2;
        }
    }
    if (dy != 0.0) {
        char rel_y;
        if (robot_dir == 'N')
            rel_y = (dy < 0) ? 'F' : 'B';
        else if (robot_dir == 'S')
            rel_y = (dy < 0) ? 'B' : 'F';
        else if (robot_dir == 'E')
            rel_y = (dy < 0) ? 'L' : 'R';
        else if (robot_dir == 'W')
            rel_y = (dy < 0) ? 'R' : 'L';
        else
            rel_y = 'F';
        if (!moverobotdirection(rel_y, (dy > 0) ? dy : -dy)) {
            //printf("moverobotxy: Order 1 failed during vertical move.\n");
           restoreRobotState(orig); //possibly redundant
            goto try_order2;
        }
    }
    success = 1;
    goto finish;

try_order2:
    // Order 2: Move vertically then horizontally.
    printf("moverobotxy: Trying order 2 (y then x)...\n");
    primarydeltaaxis = 'y';
    restoreRobotState(orig);
    if (dy != 0.0) {
        char rel_y;
        if (robot_dir == 'N')
            rel_y = (dy < 0) ? 'F' : 'B';
        else if (robot_dir == 'S')
            rel_y = (dy < 0) ? 'B' : 'F';
        else if (robot_dir == 'E')
            rel_y = (dy < 0) ? 'L' : 'R';
        else if (robot_dir == 'W')
            rel_y = (dy < 0) ? 'R' : 'L';
        else
            rel_y = 'F';
        if (!moverobotdirection(rel_y, (dy > 0) ? dy : -dy)) {
            //printf("moverobotxy: Order 2 failed during vertical move.\n");
            success = 0;
            goto finish;
        }
    }
    if (dx != 0.0) {
        char rel_x;
        if (robot_dir == 'N')
            rel_x = (dx > 0) ? 'R' : 'L';
        else if (robot_dir == 'S')
            rel_x = (dx > 0) ? 'L' : 'R';
        else if (robot_dir == 'E')
            rel_x = (dx > 0) ? 'F' : 'B';
        else if (robot_dir == 'W')
            rel_x = (dx > 0) ? 'B' : 'F';
        else
            rel_x = 'F';
        if (!moverobotdirection(rel_x, (dx > 0) ? dx : -dx)) {
            //printf("moverobotxy: Order 2 failed during horizontal move.\n");
            success = 0;
            goto finish;
        }
    }
    success = 1;

finish:
    if (success) {
        //Jaleen notes:
        //"dx", "dy" are the deltas to send
        //the c code responsible for sending to the ardunio starts here. here, you can call a function existing in a different script, making sure the script
        //that is a level closer to the arduino/pi boundary included in this code
        //note: the primary axis indicates what movement along which axis must occur first. The motions are done one axis at a time, do not try diagonals
        //because this grid doesnt support diagonal checks and is not designed around such movements

        //pass primary axis and deltas in a function here

        printf("moverobotxy: Successfully moved to target playable inches (%.2f, %.2f) corresponding to grid (%d, %d).\n",
               target_x, target_y, target_row-BORDER_CELLS, target_col-BORDER_CELLS);
        return 1;
    } else {
        primarydeltaaxis = '0';//by default, this disallows any movement regardless of delta values
        //you could still call the function for feedback on the code that interfaces the pi/arduino

        printf("moverobotxy: Could not find a valid path to the target.\n");
        restoreRobotState(orig);
        return 0;
    }
}


// ----- New Movement Function -----
// Moves the robot one cell in the given direction:
// 'L' -> West, 'R' -> East, 'U' -> North, 'D' -> South.
// It checks that a 12x12 region around the candidate new center is free.
// If allowed, the robot's current position is cleared and updated.
// Note: The global facing direction (robot_dir) is NOT updated here.

int moverobotdirection(char rel_dir, double distance_in) {
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
            return 0;
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
		return 1;
    } else {
        printf("Cannot move robot relative '%c' by %.2f inches from (%d, %d) - blocked or out of bounds.\n",
               rel_dir, distance_in, robot_row, robot_col);
		return 0;
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
  //  printf("DEBUG: canmove() candidate: new_row = %d, new_col = %d\n", new_row, new_col);
   // printf("DEBUG: Footprint boundaries: start_row = %d, end_row = %d, start_col = %d, end_col = %d\n",
        //   start_row, end_row, start_col, end_col);

    // Ensure the footprint stays within the playable boundaries.
    if (start_row < PLAYABLE_TOP || end_row > PLAYABLE_BOTTOM ||
        start_col < PLAYABLE_LEFT || end_col > PLAYABLE_RIGHT) {
        printf("DEBUG: Footprint out of playable boundaries.\n");
        return 0;
    }

    // Check that every cell in the footprint is free ('0').
    for (int i = start_row; i <= end_row; i++) {
        for (int j = start_col; j <= end_col; j++) {
            if (grid[i][j] != '0' && grid[i][j] != 'C' && grid[i][j] != '1' && grid[i][j] != '2' && grid[i][j] != '3' && grid[i][j] != '4' && grid[i][j] != 'T' && grid[i][j] != 'G' && grid[i][j] != 'N') {
				//difference: i-lastrow, j-lastcol
				int drC = i - *lastrow;
				int dcC = j - *lastcol;
				double drow = (double)drC / CELLS_PER_INCH;
				double dcol = (double)dcC / CELLS_PER_INCH;
                printf("[Absolute playable field] DEBUG: Wall bounds (%d, %d): %c, while traversing at <%d,%d> The conflict distance of wall-centre is <%.2f,%.2f> [note that indicated here, the actual distance is 1/2\" less] \n",  i-3, j-3, grid[i][j],*lastrow-3, *lastcol-3,  drow,dcol);
               
                return 0;
            } else {
           // printf("if this is the first run, these coords should be equal <%d,%d> [current], <%d,%d> [goodcoords]\n", robot_row, robot_col, *lastrow, *lastcol);
			*lastrow = new_row;//if this run was good, that means our current position is valid. we would not record something
			*lastcol = new_col;//that failed
				
			}
        }
    }
//for box check
    // 2) Clamp box-check region so it doesn't wrap around the grid
    int min_row_box = start_row - 6;
    int max_row_box = end_row + 6;
    int min_col_box = start_col - 6;
    int max_col_box = end_col + 6;

    // Clamp the extended region to the entire field dimensions (i.e. the full grid)
    if (min_row_box < 0)                    min_row_box = 0;
    if (max_row_box > EFFECTIVE_HEIGHT_CELLS - 1)  max_row_box = EFFECTIVE_HEIGHT_CELLS - 1;
    if (min_col_box < 0)                    min_col_box = 0;
    if (max_col_box > EFFECTIVE_WIDTH_CELLS - 1)   max_col_box = EFFECTIVE_WIDTH_CELLS - 1;

    for (int i = min_row_box; i <= max_row_box; i++) {
        for (int j = min_col_box; j <= max_col_box; j++) {
            if (grid[i][j] == 'N' || grid[i][j] == 'G') {
                printf("DEBUG: Box found at (%d, %d): %c\n", i-3, j-3,grid[i][j]);
				int drC = i - *lastrow;
				int dcC = j - *lastcol;
				double drow = (double)drC / CELLS_PER_INCH;
				double dcol = (double)dcC / CELLS_PER_INCH;
                printf("[Absolute playable field] DEBUG: Box found at (%d, %d): %c, while traversing at <%d,%d> The conflict distance of box-centre is <%.2f,%.2f> [note that indicated here, the actual distance is 1/2\" less] \n", i-3, j-3, grid[i][j],*lastrow-3, *lastcol-3,  drow,dcol);
                printboxconflicts();
                return 0;
            } else {
			
			*lastrow = new_row;//if this run was good, that means our current position is valid
			*lastcol = new_col;
				
			}
        }
    }
    //printf("DEBUG: Footprint is clear.\n");
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

    printf("DEBUG: CANMOVETO (delta_row_in) = %.4f, (delta_col_in) = %.6f\n",
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
    printf("DEBUG: canmoveto: Current position: (%.2f, %.2f). Delta: (%.2f, %.2f) <Y,x>. [Playable field absolute] Destination: (%.2f, %.2f).\n",
        (robot_row - BORDER_CELLS) / (double)CELLS_PER_INCH,
        (robot_col - BORDER_CELLS) / (double)CELLS_PER_INCH,
        d_row / (double)CELLS_PER_INCH,
        d_col / (double)CELLS_PER_INCH,
        (dest_row - BORDER_CELLS) / (double)CELLS_PER_INCH,
        (dest_col - BORDER_CELLS) / (double)CELLS_PER_INCH);
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


int alignYcave(void) {
    // Compute the current x coordinate (in inches relative to the playable area)
    double current_x = (robot_col - BORDER_CELLS) / (double) CELLS_PER_INCH;
    // The cave entry y-axis is given relative to the playable area (in inches)
    double target_y = (double) CAVE_ENTRY_YAXIS / (double) CELLS_PER_INCH;

    printf("alignYcave: Current X position (inches) = %.2f\n", current_x);
    printf("alignYcave: Aligning Y to cave entry at %.2f inches.\n", target_y);

    // Use the global robot_col (converted to inches) and the target_y.
    int result = moverobotxy(current_x, target_y);
    if (result) {
        printf("alignYcave: Successfully aligned Y to cave entry.\n");
    } else {
        printf("alignYcave: Failed to align Y to cave entry.\n");
    }
    return result;
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
    char valid_chars[] = {'0', 'T', '1', '2', '3', '4', 'C'};//'C' exists in the case the state needs to revert 
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
	lastGoodCoord[0] = robot_row;//the state recall intrinsically resets lastgoodcoords after a failed attempt in finding a path
	lastGoodCoord[1] = robot_col;
    printf("ROBOT RELOCATED TO (%d, %d)!!!! Last evicted element: '%c'\n", 
           robot_row, robot_col, lastevictedelement);

    printsurroundingrows();     
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
            printf("%-5s ", ">");
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


void printboxconflicts(void) {
    // Extend the window by an extra 6 cells on each side.
    int extra = 6;
    int start_row = robot_row - ROTATION_CLEARANCE_CELLS - extra;
    int end_row = robot_row + ROTATION_CLEARANCE_CELLS + extra;
    if (start_row < 0) start_row = 0;
    if (end_row >= EFFECTIVE_HEIGHT_CELLS) end_row = EFFECTIVE_HEIGHT_CELLS - 1;

    int start_col = robot_col - ROTATION_CLEARANCE_CELLS - extra;
    int end_col = robot_col + ROTATION_CLEARANCE_CELLS + extra;
    if (start_col < 0) start_col = 0;
    if (end_col >= EFFECTIVE_WIDTH_CELLS) end_col = EFFECTIVE_WIDTH_CELLS - 1;

    printf("Box conflicts view:\n");

    // Top header: mark the robot's column with an asterisk, and print "|" at multiples of 5.
    printf("      ");
    for (int j = start_col; j <= end_col; j++) {
        if (j == robot_col)
            printf("%-5s", "*");
        else if (j % 5 == 0)
            printf("%-5s", "|");
        else
            printf("%-5s", " ");
    }
    printf("\n");

    // Print rows: left header prints ">" for the robot's row, otherwise the row number.
    for (int i = start_row; i <= end_row; i++) {
        if (i == robot_row)
            printf("%-5s ", ">");
        else
            printf("%-5d ", i);
        for (int j = start_col; j <= end_col; j++) {
            printf("%c", grid[i][j]);
        }
        printf("\n");
    }

    // Bottom header, same as top.
    printf("      ");
    for (int j = start_col; j <= end_col; j++) {
        if (j == robot_col)
            printf("%-5s", "*");
        else if (j % 5 == 0)
            printf("%-5s", "|");
        else
            printf("%-5s", " ");
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
            printf("%-5s ", ">");
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
   // grid[PO(Gboxrow)][PO(Gboxcol)] = 'G';
    
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


void getrobotparams(void) {
    double pos_x = (robot_col - BORDER_CELLS) / (double)CELLS_PER_INCH;
    double pos_y = (robot_row - BORDER_CELLS) / (double)CELLS_PER_INCH;
    char *facing;
    
    // Convert the robot's facing direction character to a full word.
    switch(robot_dir) {
        case 'N': facing = "North"; break;
        case 'S': facing = "South"; break;
        case 'E': facing = "East";  break;
        case 'W': facing = "West";  break;
        default:  facing = "Unknown"; break;
    }
    
    printf("Robot Parameters: Facing %s, Position: (%.2f in, %.2f in) relative to playable field\n", 
           facing, pos_x, pos_y);
}
