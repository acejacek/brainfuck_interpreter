#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <error.h>
#include <stdbool.h>

#define MEMORY_SIZE 30
#define DEBUG

typedef enum optimizations {
    OPTI_ZERO = 1,
    OPTI_MOVL = 2,
    OPTI_MOVR = 4,
    OPTI_VOID = 8,
} Optimization;

typedef enum {
    INST_INC = '+',
    INST_DEC = '-',
    INST_LFT = '<',
    INST_RHT = '>',
    INST_OUT = '.',
    INST_INP = ',',
    INST_JZE = '[',
    INST_JNZ = ']',
    INST_invalid = '?'
} Instruction;

typedef struct {
    uint8_t* memory;
    size_t capacity;
} Memory;

typedef struct {
    uint8_t operand;
    Instruction inst;
    size_t param;
} Code;

typedef struct {
    Code* code;
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
        if (prog.code[addr].operand == '[')
            balance++;
        if (prog.code[addr].operand == ']') {
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

void precalculate_jumps(Program prog)
{
    for (size_t addr = 0; addr < prog.size; ++addr) {
        if (prog.code[addr].operand == '[') {
            int level = 0;
            for (size_t jumpto = addr + 1; ; ++jumpto) {
                if (prog.code[jumpto].operand == ']') {
                    if (level == 0) {
                        prog.code[addr].param = jumpto;
                        prog.code[jumpto].param = addr;
                        break;
                    }
                    level--;
                }
                else if (prog.code[jumpto].operand == '[')
                    level++;
            }
        }
    }
}

Program optimize(Program prog, Optimization opti)
{
    // [-] = set memory to zero Z

    size_t opt[10] = {0};
    for (size_t addr = 0; (opti & OPTI_ZERO) && addr < prog.size - 2; ++addr) {
        if (prog.code[addr    ].operand == '[' &&
            prog.code[addr + 1].operand == '-' &&
            prog.code[addr + 2].operand == ']') {

            prog.code[addr].operand = 'Z';

            for (size_t to = addr + 1, from = addr + 3; from < prog.size; ++to, ++from) {
                prog.code[to].operand = prog.code[from].operand;
                prog.code[to].param = prog.code[from].param;
            }

            prog.size -= 2;
            opt[0]++;
        }
    }
    // [-<<<+>>>} = move momory 3 positions left
    for (size_t addr = 0; (opti & OPTI_MOVL) && addr < prog.size - 5; ++addr) {
        if (prog.code[addr    ].operand == '[' &&
            prog.code[addr + 1].operand == '-' &&
            prog.code[addr + 2].operand == '<' &&
            prog.code[addr + 3].operand == '+' &&
            prog.code[addr + 4].operand == '>' &&
            prog.code[addr + 5].operand == ']' &&
            prog.code[addr + 1]. param  ==  1  &&
            prog.code[addr + 3]. param  ==  1) {

            prog.code[addr].operand = 'M';
            prog.code[addr].param = - prog.code[addr + 2].param;

            for (size_t to = addr + 1, from = addr + 6; from < prog.size; ++to, ++from) {
                prog.code[to].operand = prog.code[from].operand;
                prog.code[to].param = prog.code[from].param;
            }

            prog.size -= 5;
            opt[1]++;
        }
    }
    // [->>>+<<<} = move momory 3 positions right
    for (size_t addr = 0; (opti & OPTI_MOVR) && addr < prog.size - 5; ++addr) {
        if (prog.code[addr    ].operand == '[' &&
            prog.code[addr + 1].operand == '-' &&
            prog.code[addr + 2].operand == '>' &&
            prog.code[addr + 3].operand == '+' &&
            prog.code[addr + 4].operand == '<' &&
            prog.code[addr + 5].operand == ']' &&
            prog.code[addr + 1]. param  ==  1  &&
            prog.code[addr + 3]. param  ==  1) {

            prog.code[addr].operand = 'M';
            prog.code[addr].param = prog.code[addr + 2].param;

            for (size_t to = addr + 1, from = addr + 6; from < prog.size; ++to, ++from) {
                prog.code[to].operand = prog.code[from].operand;
                prog.code[to].param = prog.code[from].param;
            }

            prog.size -= 5;
            opt[2]++;
        }
    }

    // revmoe pairs: <> +- >< -+
    for (size_t addr = 0; (opti & OPTI_VOID) && addr < prog.size - 1; ++addr) {
        if ((prog.code[addr    ].operand == '<' &&
             prog.code[addr + 1].operand == '>') || 
            (prog.code[addr    ].operand == '+' &&
             prog.code[addr + 1].operand == '-') || 
            (prog.code[addr    ].operand == '>' &&
             prog.code[addr + 1].operand == '<') || 
            (prog.code[addr    ].operand == '-' &&
             prog.code[addr + 1].operand == '+')) { 

            for (size_t to = addr, from = addr + 2; from < prog.size; ++to, ++from) {
                prog.code[to].operand = prog.code[from].operand;
                prog.code[to].param = prog.code[from].param;
            }

            prog.size -= 2;
            opt[3]++;
        }
    }

#ifdef DEBUG
    for (size_t i = 0; i < 4; ++i) {
        printf("Optimization %zu: %zu\n", i, opt[i]);
    }
#endif

    return prog;
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

void interprete(Program prog)
{
    Memory m = { 0 };
    check_memory(&m, MEMORY_SIZE);
#ifdef DEBUG
    size_t instruction_counter = 0;
#endif

    size_t pointer = 0;
   
    for (size_t addr = 0; addr < prog.size; ++addr) {
#ifdef DEBUG
        ++instruction_counter;
#endif
        switch(prog.code[addr].operand) {
            case '>':
                pointer += prog.code[addr].param;
                check_memory(&m, pointer);
                break;

            case '<':
                if ((long int)pointer - (long int)prog.code[addr].param < 0) {
                    fprintf(stderr, "ERROR: move pointer before memory start'\n");
                    exit(EXIT_FAILURE);
                }
                pointer -= prog.code[addr].param;
                break;

            case '+':
                m.memory[pointer] += (uint8_t)prog.code[addr].param;
                break;

            case '-':
                m.memory[pointer] -= (uint8_t)prog.code[addr].param;
                break;

            case '.':
                for (size_t i = 0; i < prog.code[addr].param; ++i)
                    putchar(m.memory[pointer]);
                break;
               
            case ',': 
                for (size_t i = 0; i < prog.code[addr].param; ++i) {
                    int x = getchar();
                    if (x != EOF)
                        m.memory[pointer] = (uint8_t)x;
                }
                break;

            case '[':
                if (m.memory[pointer] == 0)
                    addr = prog.code[addr].param;
                break;
               
            case ']':
                if (m.memory[pointer] != 0)
                    addr = prog.code[addr].param;
                break;

            case 'Z':
                m.memory[pointer] = 0;
                break;

            case 'M':
                {
                    const size_t dest = pointer + prog.code[addr].param;
                    m.memory[dest] += m.memory[pointer];
                    m.memory[pointer] = 0;
                }
                break;
               
            default:
                // unreachable (comments removed from program)
        }
    }
    free(m.memory);
#ifdef DEBUG
    printf("Executed %zu instructions\n", instruction_counter);
#endif
}

Program load_program(FILE* f)
{
    const size_t chunk = 512;
    Program prog = { 0 };
    size_t allocated = 0;

    uint8_t prev = 0;
   
    while (1) {
        int c = fgetc(f);
       
        if (c == EOF)
            break;
       
        if (!is_valid_code(c))
            continue;

        if (c == prev && (c != '[' && c != ']')) {
            prog.code[prog.size - 1].param += 1;
            continue;
        }

        if (prog.size >= allocated) {
            allocated += chunk;
            prog.code = realloc(prog.code, allocated * sizeof(*prog.code));
            if (!prog.code) {
                fprintf(stderr, "ERROR: Buy more ram\n");
                exit(EXIT_FAILURE);   
            }
        }
        prog.code[prog.size].operand = (uint8_t)c;
        prog.code[prog.size].param = 1;

        prog.size += 1;
        prev = c;
    }
   
    return prog;
}

void execute(Program prog)
{
    if (!is_balanced(prog))
        exit(EXIT_FAILURE);

   /* executed instructions in mandelbrot.bg:
    * 3 018 468 909 no optimizations
    * 2 994 391 927 ZERO
    * 2 702 050 882 MOVR MOVL
    * 2 702 050 882 ZERO MORV MOVL
    */
    prog = optimize(prog, OPTI_ZERO | OPTI_MOVL | OPTI_MOVR | OPTI_VOID );
    precalculate_jumps(prog);
    interprete(prog);

    free(prog.code);
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

