#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define breakpoint()  asm ("int3; nop")
#define MEMORY_SIZE 30000

static bool is_balanced(unsigned char* c) {
    int ballance = 0;
    for (; *c; ++c) {
        if (*c == '[')
            ballance++;
        if (*c == ']') {
            ballance--;
            if (ballance < 0)
                return false;
        }
    }
   
    return ballance == 0;
}

void build_jump_table(unsigned char* prog, size_t* jump_table)
{ 
    for (size_t addr = 0; prog[addr]; ++addr) {
        switch (prog[addr]) {
            case '[': {
                          int level = 0;
                          for (size_t jumpto = addr + 1; ; ++jumpto) {
                              if (prog[jumpto] == ']') {
                                  if (level == 0) {
                                      jump_table[addr] = jumpto;
                                      break;
                                  }
                                  level--;
                              }
                              else if (prog[jumpto] == '[')
                                  level++;
                          }
                      }
                      break;
            case ']': {
                          int level = 0;
                          for (size_t jumpto = addr - 1; ; --jumpto) {
                              if (prog[jumpto] == '[') {
                                  if (level == 0) {
                                      jump_table[addr] = jumpto;
                                      break;
                                  }
                                  level--;
                              }
                              else if (prog[jumpto] == ']')
                                  level++;
                          }
                      }
                      break;
            default:
                      // nothing
        }
    }
}

void interprete(unsigned char* prog, size_t* jump_table)
{
    unsigned char memory[MEMORY_SIZE] = {0};
    size_t pointer = 0;
   
    for (size_t addr = 0; prog[addr]; ++addr) {
        switch(prog[addr]) {
            case '>':
                pointer += 1;
                if (pointer >= MEMORY_SIZE) {
                    fprintf(stderr, "ERROR: out of memory'\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case '<':
                if (pointer == 0) {
                    fprintf(stderr, "ERROR: move pointer before memory begin'\n");
                    exit(EXIT_FAILURE);
                }
                pointer -= 1;
                break;
            case '+':
                memory[pointer] += 1;
                break;
            case '-':
                memory[pointer] -= 1;
                break;
            case '.':
                putchar(memory[pointer]);
                break;
               
            case ',': {
                          char x = getchar();
                          if (x != EOF)
                              memory[pointer] = x;
                      }
                break;

            case '[':
                if (memory[pointer] == 0) {
                    addr = jump_table[addr];
                }
                break;
               
            case ']':
                if (memory[pointer] != 0) {
                    addr = jump_table[addr];
                }
                break;
               
            default:
                // comment
        }
    }
}

unsigned char* load_program(const char* fname)
{
    FILE* fp = fopen(fname, "rb");
    if (!fp) {
        perror("Can't open file");
        exit(EXIT_FAILURE);   
    }
   
    fseek(fp, 0L, SEEK_END);
    long sz = ftell(fp);
    rewind(fp);
   
    unsigned char* prog = calloc(sz + 2, sizeof(*prog));
    if (!prog) {
        fprintf(stderr, "ERROR: missing input file\n");
        exit(EXIT_FAILURE);   
    }

    fread(prog, sz, 1, fp);
    fclose(fp);
   
    return prog;
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        fprintf(stderr, "ERROR: missing input file\n");
        exit(EXIT_FAILURE);
    }

    if (!is_balanced(argv[1])) {
        fprintf(stderr, "ERROR: unbalanced '[' ']'\n");
        exit(EXIT_FAILURE);
    }

    unsigned char* prog = load_program(argv[1]);
   
    size_t jump_table[strlen(prog) + 1];
    build_jump_table(prog, jump_table);

    interprete(prog, jump_table);

    free(prog);
    return EXIT_SUCCESS;
}

