#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define breakpoint()  asm ("int3; nop")
#define MEMORY_SIZE 30000

unsigned char memory[MEMORY_SIZE] = {0};
size_t pointer = 0;

bool is_balanced(unsigned char* c) {
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
int main(int argc, char* argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Missing input file\n");
        exit(EXIT_FAILURE);
    }

    if (!is_balanced(argv[1])) {
        fprintf(stderr, "ERROR: unbalanced '[' ']'\n");
        exit(EXIT_FAILURE);
    }
            
    for (unsigned char* c = argv[1]; *c; ++c) {
        switch(*c) {
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
                //breakpoint();
                if (memory[pointer] == 0) {
                    int level = 0;
                    for (++c;; ++c) {
                        if (*c == ']') {
                            if (level == 0)
                                break;
                            level--;
                        }
                        else if (*c == '[')
                            level++;
                    }
                }
                break;
               
            case ']':
                if (memory[pointer] != 0) {
                    int level = 0;
                    for (--c;;--c) {
                        if (*c == '[') {
                            if (level == 0) break;
                            level--;
                        }
                        else if (*c == ']')
                            level++;
                    }
                }
                break;
               
            default:
                // comment
        }
    }
  
    //putchar('\n'); 
   
    return EXIT_SUCCESS;
}

