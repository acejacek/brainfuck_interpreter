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
    int balance = 0;
    for (; *c; ++c) {
        if (*c == '[')
            balance++;
        if (*c == ']') {
            balance--;
            if (balance < 0) {
                fprintf(stderr, "ERROR: Unbalanced ']'\n");
                return false;
            }
        }
    }
   
    if (balance) {
        fprintf(stderr, "ERROR: Unbalanced '['\n");
        return false;
    }
   
    return true;
}

size_t* build_jump_table(unsigned char* prog, const size_t program_size)
{
    size_t* jump_table = malloc(program_size * sizeof(*jump_table));
    if (!jump_table) {
        fprintf(stderr, "ERROR: Buy more ram");
        exit(EXIT_FAILURE);
    }
   
    for (size_t addr = 0; prog[addr]; ++addr) {
        if (prog[addr] == '[') {
            int level = 0;
            for (size_t jumpto = addr + 1; ; ++jumpto) {
                if (prog[jumpto] == ']') {
                    if (level == 0) {
                        jump_table[addr] = jumpto;
                        jump_table[jumpto] = addr;
                        break;
                    }
                    level--;
                }
                else if (prog[jumpto] == '[')
                    level++;
            }
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

unsigned char* load_stream(size_t* program_size)
{
    const size_t chunk = 10000;
    *program_size = 0;
    size_t allocated = chunk;
    unsigned char* prog = malloc(allocated * sizeof(*prog));
   
    int c = 0;
    do {
        c = fgetc(stdin);

        if (c == EOF) c = 0;

        if (*program_size >= allocated) {
            allocated += chunk;
            prog = realloc(prog, allocated * sizeof(*prog));
            if (!prog) {
                fprintf(stderr, "ERROR: Buy more ram\n");
                exit(EXIT_FAILURE);   
            }
        }
        prog[*program_size] = (unsigned char)c;
        *program_size += 1;
               
    } while (c);
   
    return prog;
}

int main(int argc, char* argv[])
{
    size_t program_size = 0;
    unsigned char* prog = NULL;
   
    if (argc == 1)
        prog = load_stream(&program_size);
    else
        prog = load_program(argv[1], &program_size);

    if (!is_balanced(prog))
        exit(EXIT_FAILURE);

    size_t* jump_table = build_jump_table(prog, program_size);

    interprete(prog, jump_table);

    free(prog);
    free(jump_table);
    return EXIT_SUCCESS;
}

