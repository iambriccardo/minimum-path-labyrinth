#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <zconf.h>

#define UNEXPLORED_CELL 'o'
#define EXPLORED_CELL 'x'
#define OBSTACLE_CELL 'b'
#define FATHER_CELL 'p'
#define LOOT_CELL 't'
#define FALSE 0
#define TRUE 1
#define POSSIBLE_POSITIONS 4
#define MAX_PATH_COST 2000000

#define random(x) rand() % x
#define randomize srand((unsigned)time(NULL))

struct position {
    int row;
    int col;
};

void init_labyrinth();
struct position* allocate_position_struct(int row, int col);
struct position* calculate_father_position();
struct position* calculate_loot_position(struct position* father_position);
void put_obstacles(int obstacles_number);
struct position** calculate_next_positions(int row, int col);
void print_labyrinth();
void* create_shared_memory(size_t size);

char **labyrinth = NULL;
int rows = 0;
int cols = 0;

struct position* current_position = NULL;
int next_positions_size = -1;
int father_pid;
int best_path_cost = MAX_PATH_COST;

// TODO start implementation of calculation of next positions

int main() {
    // Begin the program.
    printf("Quante righe vuoi avere: ");
    scanf("%d", &rows);
    printf("Quante colonne vuoi avere: ");
    scanf("%d", &cols);
    // Initialization of the labyrinth.
    init_labyrinth();
    // Printing the labyrinth.
    print_labyrinth();
    // Saving the fathers pid
    father_pid = getpid();
    // Variable that manages the loop.
    int continue_searching = TRUE;
    // Starting the search for the minimum path.
    while (continue_searching == TRUE) {
        // Checking if the current position isn't null.
        if (current_position != NULL) {
            // Checking if the current position isn't on a loot.
            if (labyrinth[current_position->row][current_position->col] != LOOT_CELL) {
                // Marking the node as explored.
                labyrinth[current_position->row][current_position->col] = EXPLORED_CELL;
                // Calculating all the possible next positions.
                struct position** next_positions = calculate_next_positions(current_position->row, current_position->col);
                // Checking if we found other positions.
                if (next_positions_size > 0) {
                    // Loop counter.
                    int i;
                    // Pid variables.
                    int pid = -1;
                    // Array containing all the childs.
                    int childs[next_positions_size];

                    // Looping through every free position.
                    for (i = 0; i < next_positions_size; i++) {
                        // Creating a tmp position to restore.
                        struct position* tmp_old_position = current_position;
                        // Updating the current position to the next one.
                        current_position = next_positions[i];
                        // Creation of a child and saving of it's pid inside the childs
                        // array.
                        pid = fork();
                        childs[i] = pid;
                        // If we are in the child we need to stop the loop.
                        // if we are in the father let's restore the old position.
                        if (pid == 0) {
                            break;
                        } else {
                            pid = -1;
                            current_position = tmp_old_position;
                        }
                    }

                    // If we are inside the father let's wait other sons to finish
                    // their exploration.
                    if (pid != 0) {
                        // Loop counter.
                        int j;
                        // Status of the other processes.
                        int cost;

                        // Loop through every process we forked and waiting for their result.
                        for (j = 0; j < next_positions_size; j++) {
                            // Waiting for the forked processes.
                            waitpid(childs[j], &cost, 0);
                            // Converting the cost.
                            cost = WEXITSTATUS(cost);

                            // Checking if we are in some father but not the main,
                            // so we can add + 1 to the cost.
                            // Else if we are in the main father save the result if is
                            // less than the previous
                            if (father_pid != getpid()) {
                                if (cost >= 1) {
                                    exit(1 + cost);
                                } else if (cost == 0) {
                                    exit(0);
                                }
                            } else {
                                if (cost >= 1) {
                                    if (cost < best_path_cost) {
                                        best_path_cost = cost;
                                    }
                                }
                            }

                        }

                        if (father_pid == getpid()) continue_searching = FALSE;
                    }
                } else {
                    exit(0);
                }
            } else {
                exit(1);
            }
        }
    }
    // Printing a separator.
    printf("\n----------\n");
    // Printing the labyrinth.
    print_labyrinth();
    // Printing the result.
    if (best_path_cost != MAX_PATH_COST) {
        printf("Il cammino minimo e' lungo %i!\n", best_path_cost);
    } else {
        printf("Cammino minimo non trovato!\n");
    }

}

void init_labyrinth() {
    // Creation of the matrix, only rows for now.
    labyrinth = (char**) create_shared_memory(sizeof(char) * rows);

    // Creation of cols of the matrix.
    int i;
    for (i = 0; i < rows; i++) labyrinth[i] = (char*) create_shared_memory(sizeof(char) * cols);

    // Setting all the cells as UNEXPLORED.
    int j, z;
    for (j = 0; j < rows; j++) {
        for (z = 0; z < cols; z++) {
            labyrinth[j][z] = UNEXPLORED_CELL;
        }
    }

    // Calculating and adding father start position to the matrix.
    struct position* father_position = calculate_father_position();
    current_position = father_position;
    labyrinth[father_position->row][father_position->col] = FATHER_CELL;

    // Calculating and adding loot position to the matrix.
    struct position* loot_position = calculate_loot_position(father_position);
    printf("%i %i\n", loot_position->row, loot_position->col);
    labyrinth[loot_position->row][loot_position->col] = LOOT_CELL;

    // Putting down obstacles.
    put_obstacles(10);
}

struct position* allocate_position_struct(int row, int col) {
    // Allocates some space for a position struct.
    struct position* position = (struct position*) malloc(sizeof(struct position));
    // Adds data to the position.
    position->row = row;
    position->col = col;

    // Returns the position struct filled with the right data.
    return position;
}

struct position* calculate_father_position() {
    // Returns the new position structure.
    return allocate_position_struct(0, random(cols));
}

struct position* calculate_loot_position(struct position* father_position) {
    // Loop and generate a new number until we found the correct one
    while (TRUE) {
        int row = random(rows);
        int col = random(cols);

        // Checking if the position is the different than the start position.
        if (row != father_position->row &&
                col != father_position->col) return allocate_position_struct(row, col);
    }
}

void put_obstacles(int obstacles_number) {
    // Calculates the maximum number of obstacles that we can put.
    int max_obstacles_number = (int) (rows * cols * 0.20);

    // Checking if the obstacles that the users wants to add are greater than
    // the 20% of the number of cells.
    if (obstacles_number > max_obstacles_number) obstacles_number = max_obstacles_number;

    int i = 0;
    // Loop n times that corresponds of the obstacles amount.
    while (i < obstacles_number) {
        // Allocation of the position struct.
        struct position* obstacle_position = (struct position*) malloc(sizeof(struct position));
        // Adds data to the position.
        obstacle_position->row = random(rows);
        obstacle_position->col = random(cols);

        // Checks if we have space for an obstacle.
        if (labyrinth[obstacle_position->row][obstacle_position->col] == UNEXPLORED_CELL) {
            // Adds the obstacle inside the labyrinth.
            labyrinth[obstacle_position->row][obstacle_position->col] = OBSTACLE_CELL;
            i++;
        }
    }
}

struct position** calculate_next_positions(int row, int col) {
    // Allocate the positions array.
    struct position** next_positions = malloc(sizeof(struct position) * POSSIBLE_POSITIONS);
    // Creating an index for the position.
    int position_index = 0;

    // Top verification.
    if (row - 1 >= 0) {
        if (labyrinth[row - 1][col] == UNEXPLORED_CELL || labyrinth[row - 1][col] == LOOT_CELL) {
            next_positions[position_index] = allocate_position_struct(row - 1, col);
            position_index++;
        }
    }

    // Bottom verification.
    if (row + 1 < rows) {
        if (labyrinth[row + 1][col] == UNEXPLORED_CELL || labyrinth[row + 1][col] == LOOT_CELL) {
            next_positions[position_index] = allocate_position_struct(row + 1, col);
            position_index++;
        }
    }

    // Left verification.
    if (col - 1 >= 0) {
        if (labyrinth[row][col - 1] == UNEXPLORED_CELL || labyrinth[row][col - 1] == LOOT_CELL) {
            next_positions[position_index] = allocate_position_struct(row, col - 1);
            position_index++;
        }
    }

    // Right verification.
    if (col + 1 < cols) {
        if (labyrinth[row][col + 1] == UNEXPLORED_CELL || labyrinth[row][col + 1] == LOOT_CELL) {
            next_positions[position_index] = allocate_position_struct(row, col + 1);
            position_index++;
        }
    }

    next_positions_size = position_index;

    // Returns the new position structure.
    return next_positions;
}

void print_labyrinth() {
    // Loop through the matrix and print the values.
    int i, j;
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            printf("%c", labyrinth[i][j]);
        }
        printf("\n");
    }
}

void* create_shared_memory(size_t size) {
    // Our memory buffer will be readable and writable:
    int protection = PROT_READ | PROT_WRITE;

    // The buffer will be shared (meaning other processes can access it), but
    // anonymous (meaning third-party processes cannot obtain an address for it),
    // so only this process and its children will be able to use it:
    int visibility = MAP_ANONYMOUS | MAP_SHARED;

    // The remaining parameters to `mmap()` are not important for this use case,
    // but the manpage for `mmap` explains their purpose.
    return mmap(NULL, size, protection, visibility, 0, 0);
}