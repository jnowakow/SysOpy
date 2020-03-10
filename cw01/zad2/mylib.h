#ifndef mylib_h
#define mylib_h


struct MainTable
{
    struct OperationsBlock **blocks;
    int lastItemIndex;
};

struct OperationsBlock
{
    char **operation;
    int size;
};

struct filesToCompare
{
    char *file1;
    char *file2;
};

struct filesSequence
{
    struct filesToCompare *pairs;
    int size;
    
};



struct MainTable *createTable(int size);
struct filesSequence * defineFilesSequence(char **input, int n);
int createPair(char *input, struct filesToCompare *pair);
char *compareAndWrite(struct filesToCompare *pair);
int insertOperationsBlock(struct MainTable *table, char *tmpFileName);
int operationsCount(struct MainTable *table, int blockIndex);
int deleteBlock(struct MainTable *table, int index);
int deleteOperation(struct MainTable *table, int blockIndex, int operationIndex);




#endif