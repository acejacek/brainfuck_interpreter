#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <error.h>

#define MEMORY_SIZE 30

typedef struct {
    uint8_t* memory;
    size_t capacity;
} Memory;

bool is_valid_code(const uint8_t c)
{
    switch (c) {
        case '+': 
        case '-': 
        case '>': 
        case '<': 
        case '.': 
        case ',': 
        case '[': 
        case ']': 
            return true;
    }
    return false;
}

bool is_balanced(uint8_t* c) {
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
        error(0, 0, "ERROR: Unbalanced '['\n");
        return false;
    }
   
    return true;
}

size_t* build_jump_table(uint8_t* prog, const size_t program_size)
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

void check_memory(Memory* m, const size_t expected)
{
    if (m->capacity >= expected) return;

    size_t new_capacity = expected;
    if (m->capacity) 
        new_capacity = m->capacity * 2;
    
    // no realloc, as memory must be zeroed when initialized 
    uint8_t* new_memory = calloc(new_capacity, sizeof(*new_memory));
    if (!new_memory) {
        fprintf(stderr, "ERROR: Buy more ram");
        exit(EXIT_FAILURE);
    }
   
    if (m->memory != NULL) { 
        memcpy(new_memory, m->memory, m->capacity);
        free(m->memory);
    }

    m->memory = new_memory;
    m->capacity = new_capacity;
}

void interprete(uint8_t* prog, size_t* jump_table)
{
    Memory m = { 0 };
    check_memory(&m, MEMORY_SIZE);
   
    size_t pointer = 0;
   
    for (size_t addr = 0; prog[addr]; ++addr) {
        switch(prog[addr]) {
            case '>':
                pointer += 1;
                check_memory(&m, pointer);
                break;

            case '<':
                if (pointer == 0) {
                    fprintf(stderr, "ERROR: move pointer before memory start'\n");
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
                // unreachable (comments removed from program)
        }
    }
    free(m.memory);
}

uint8_t* load_program(FILE* f, size_t* program_size)
{
    const size_t chunk = 512;
    uint8_t* prog = NULL;
    *program_size = 0;
    size_t allocated = 0;
   
    int c = 0;
    do {
        c = fgetc(f);

        if (c == EOF)
            c = 0;
        else if (!is_valid_code(c))
            continue;
       
        if (*program_size >= allocated) {
            allocated += chunk;
            prog = realloc(prog, allocated * sizeof(*prog));
            if (!prog) {
                fprintf(stderr, "ERROR: Buy more ram\n");
                exit(EXIT_FAILURE);   
            }
        }
        prog[*program_size] = (uint8_t)c;
        *program_size += 1;
               
    } while (c);
   
    return prog;
}

void execute(uint8_t* prog, size_t program_size)
{
    if (!is_balanced(prog))
        exit(EXIT_FAILURE);

    size_t* jump_table = build_jump_table(prog, program_size);

    interprete(prog, jump_table);

    free(prog);
    free(jump_table);
}

void help(const char* binary)
{
    printf("Usage: %s [FILE]...\n", binary);
    puts("Brainfuck interpreter. Execute FILE(s), with result printed to standard output.\n");
    puts("With no FILE, or when FILE is -, read standard input.");
    puts("  --help\tdisplay this help and exit");
    puts("\nExamples:");
    printf("  %s HelloWorld.bf\n", binary);
    printf("  cat HelloWorld.bf | %s\n", binary);

    exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[])
{
    uint8_t* prog = NULL;
    size_t program_size = 0;
   
    if (argc == 1) {
        prog = load_program(stdin, &program_size);
        execute(prog, program_size);
    }
    else {
        if (strcmp(argv[1], "--help") == 0)
            help(argv[0]);
       
        if (strcmp(argv[1], "-") == 0) {
            prog = load_program(stdin, &program_size);
            execute(prog, program_size);
            exit(EXIT_SUCCESS);
        }  
       
        for (int p = 1; p < argc; ++p) {
            FILE* f = fopen(argv[1], "r");
            if (!f) {
                perror("ERROR: Can't open file");
                exit(EXIT_FAILURE);   
            }
            prog = load_program(f, &program_size);
            fclose(f);
           
            execute(prog, program_size);
        }
    }
   
    return EXIT_SUCCESS;
}

