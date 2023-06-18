// shared.h
#define SHARED_MEMORY_SIZE 10000
#define COLUMNS 100
#define SCORE 3
#define HELPERS 30
#define SPIES 30

struct shared_data {
    char value[COLUMNS][SHARED_MEMORY_SIZE];
};

struct shared_data1 {
    int num[SCORE];
};

struct shared_data2 {
    int swappers[HELPERS][2];
};

struct shared_data3 {
    char spies[SPIES][SHARED_MEMORY_SIZE];
};