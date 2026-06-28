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

typedef struct {
    uint8_t* code;
    size_t size;
} Program;

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

bool is_balanced(const Program prog) {
    int balance = 0;
    for (size_t addr = 0;  addr < prog.size; ++addr) {
        if (prog.code[addr] == '[')
            balance++;
        if (prog.code[addr] == ']') {
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

size_t* build_jump_table(const Program prog)
{
    size_t* jump_table = malloc(prog.size * sizeof(*jump_table));
    if (!jump_table) {
        fprintf(stderr, "ERROR: Buy more ram");
        exit(EXIT_FAILURE);
    }
   
    for (size_t addr = 0; addr < prog.size; ++addr) {
        if (prog.code[addr] == '[') {
            int level = 0;
            for (size_t jumpto = addr + 1; ; ++jumpto) {
                if (prog.code[jumpto] == ']') {
                    if (level == 0) {
                        jump_table[addr] = jumpto;
                        jump_table[jumpto] = addr;
                        break;
                    }
                    level--;
                }
                else if (prog.code[jumpto] == '[')
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

void interprete(Program prog, size_t* jump_table)
{
    Memory m = { 0 };
    check_memory(&m, MEMORY_SIZE);
   
    size_t pointer = 0;
   
    for (size_t addr = 0; addr < prog.size; ++addr) {
        switch(prog.code[addr]) {
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

Program load_program(FILE* f)
{
    const size_t chunk = 512;
    Program prog = { 0 };
    size_t allocated = 0;
   
    while (1) {
        int c = fgetc(f);

        if (c == EOF)
            break;

        if (!is_valid_code(c))
            continue;
       
        if (prog.size >= allocated) {
            allocated += chunk;
            prog.code = realloc(prog.code, allocated * sizeof(*prog.code));
            if (!prog.code) {
                fprintf(stderr, "ERROR: Buy more ram\n");
                exit(EXIT_FAILURE);   
            }
        }
        prog.code[prog.size] = (uint8_t)c;
        prog.size += 1;
    }
   
    return prog;
}

void execute(Program prog)
{
    if (!is_balanced(prog))
        exit(EXIT_FAILURE);

    size_t* jump_table = build_jump_table(prog);

    interprete(prog, jump_table);

    free(prog.code);
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
    Program prog;
   
    if (argc == 1) {
        prog = load_program(stdin);
        execute(prog);
    }
    else {
        if (strcmp(argv[1], "--help") == 0)
            help(argv[0]);
       
        if (strcmp(argv[1], "-") == 0) {
            prog = load_program(stdin);
            execute(prog);
            exit(EXIT_SUCCESS);
        }  
       
        for (int p = 1; p < argc; ++p) {
            FILE* f = fopen(argv[p], "r");
            if (!f) {
                perror("ERROR: Can't open file");
                exit(EXIT_FAILURE);   
            }
            prog = load_program(f);
            fclose(f);
           
            execute(prog);
        }
    }
   
    return EXIT_SUCCESS;
}

