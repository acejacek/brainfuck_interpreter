#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define breakpoint()  asm ("int3; nop")
#define MEMORY_SIZE 100

typedef struct {
    unsigned char* memory;
    size_t size;
    size_t capacity;
} Memory;

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

size_t* build_jump_table(unsigned char* prog, const size_t program_size)
{
    size_t* jump_table = malloc(program_size * sizeof(*jump_table));
    if (!jump_table) {
        fprintf(stderr, "ERROR: Buy more ram");
        exit(EXIT_FAILURE);
    }
   
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
    return jump_table;
}

void check_memory(Memory* m, const size_t growth)
{
    if (m->capacity < m->size + growth) {
        if (m->memory == NULL) {
            m->memory = calloc(growth, sizeof(*(m->memory)));
            if (!m->memory) {
                fprintf(stderr, "ERROR: Buy more ram");
                exit(EXIT_FAILURE);
            }
            m->capacity = growth;
        }
        else {
            m->capacity *= 2;
            m->memory = realloc(m->memory, m->capacity * (sizeof(*m->memory)));
            if (!m->memory) {
                fprintf(stderr, "ERROR: Buy more ram");
                exit(EXIT_FAILURE);
            }
        }
    }
}

void interprete(unsigned char* prog, size_t* jump_table)
{
    Memory m = { 0 };
    check_memory(&m, MEMORY_SIZE);
   
    size_t pointer = 0;
   
    for (size_t addr = 0; prog[addr]; ++addr) {
        switch(prog[addr]) {
            case '>':
                check_memory(&m, 1);
                pointer += 1;
                break;
            case '<':
                if (pointer == 0) {
                    fprintf(stderr, "ERROR: move pointer before memory begin'\n");
                    exit(EXIT_FAILURE);
                }
                pointer -= 1;
                break;
            case '+':
                m.memory[pointer] += 1;
                break;
            case '-':
                m.memory[pointer] -= 1;
                break;
            case '.':
                putchar(m.memory[pointer]);
                break;
               
            case ',': {
                          char x = getchar();
                          if (x != EOF)
                              m.memory[pointer] = x;
                      }
                break;

            case '[':
                if (m.memory[pointer] == 0) {
                    addr = jump_table[addr];
                }
                break;
               
            case ']':
                if (m.memory[pointer] != 0) {
                    addr = jump_table[addr];
                }
                break;
               
            default:
                // comment
        }
    }
    free(m.memory);
}

unsigned char* load_program(const char* fname, size_t* program_size)
{
    FILE* fp = fopen(fname, "rb");
    if (!fp) {
        perror("Can't open file");
        exit(EXIT_FAILURE);   
    }
   
    fseek(fp, 0L, SEEK_END);
    *program_size = ftell(fp) + 2;
    rewind(fp);
   
    unsigned char* prog = calloc(*program_size, sizeof(*prog));
    if (!prog) {
        fprintf(stderr, "ERROR: Buy more ram\n");
        fclose(fp);
        exit(EXIT_FAILURE);   
    }

    fread(prog, *program_size - 2, 1, fp);
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

    size_t program_size;
    unsigned char* prog = load_program(argv[1], &program_size);
    size_t* jump_table = build_jump_table(prog, program_size);

    interprete(prog, jump_table);

    free(prog);
    free(jump_table);
    return EXIT_SUCCESS;
}

