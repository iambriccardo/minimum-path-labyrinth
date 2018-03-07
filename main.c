#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#define UNEXPLORED_CELL 'o'
#define EXPLORED_CELL 'x'
#define OBSTACLE_CELL 'b'
#define FATHER_CELL 'p'
#define LOOT_CELL 't'
#define FALSE 0
#define TRUE 1
#define POSSIBLE_POSITIONS 4

#define random(x) rand() % x
#define randomize srand((unsigned)time(NULL))

struct position {
    int row;
    int col;
};

void init_labyrinth();
struct position* calculate_father_position();
struct position* calculate_loot_position();
void put_obstacles(int obstacles_number);
struct position** calculate_next_positions();
void print_labyrinth();
void* create_shared_memory(size_t size);

char **labyrinth = NULL;
int rows = 0;
int cols = 0;

// TODO comment everything
// TODO start implementation of calculation of next positions

int main() {
    // Begin the program
    printf("Quante righe vuoi avere: ");
    scanf("%d", &rows);
    printf("Quante colonne vuoi avere: ");
    scanf("%d", &cols);
    // Initialization of the labyrinth
    init_labyrinth();
    // Printing the labyrinth;
}

void init_labyrinth() {
    // Creation of the matrix, only rows for now.
    labyrinth = (char**) create_shared_memory(sizeof(char) * rows);

    // Creation of cols of the matrix.
    for (int i = 0; i < rows; i++) labyrinth[i] = (char*) create_shared_memory(sizeof(char) * cols);

    // Setting all the cells as UNEXPLORED.
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            labyrinth[i][j] = UNEXPLORED_CELL;
        }
    }

    // Calculating and adding father start position to the matrix.
    struct position* father_position = calculate_father_position();
    labyrinth[father_position->row][father_position->col] = FATHER_CELL;

    // Calculating and adding loot position to the matrix.
    struct position* loot_position = calculate_loot_position();
    printf("%i %i\n", loot_position->row, loot_position->col);
    labyrinth[loot_position->row][loot_position->col] = LOOT_CELL;

    // Putting down obstacles.
    put_obstacles(10);
}

struct position* calculate_father_position() {
    struct position* father_position = (struct position*) malloc(sizeof(struct position));
    father_position->row = 0;
    father_position->col = random(cols);

    return father_position;
}

struct position* calculate_loot_position() {
    struct position* loot_position = (struct position*) malloc(sizeof(struct position));
    loot_position->row = random(rows);
    loot_position->col = random(cols);

    return loot_position;
}

void put_obstacles(int obstacles_number) {
    int max_obstacles_number = (int) (rows * cols * 0.20);

    if (obstacles_number > max_obstacles_number) obstacles_number = max_obstacles_number;

    int i = 0;

    while (i < obstacles_number) {
        struct position* obstacle_position = (struct position*) malloc(sizeof(struct position));
        obstacle_position->row = random(rows);
        obstacle_position->col = random(cols);

        if (labyrinth[obstacle_position->row][obstacle_position->col] == UNEXPLORED_CELL) {
            labyrinth[obstacle_position->row][obstacle_position->col] = OBSTACLE_CELL;
            i++;
        }
    }
}

struct position** calculate_next_positions() {
    struct position** next_positions = malloc(sizeof(struct position) * POSSIBLE_POSITIONS);
}

void print_labyrinth() {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
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