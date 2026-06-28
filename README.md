# Brainfuck interpreter

First, compile it with `make`:
```sh
make
./bf --help
```

Interpreter accepts program, or programs, as parameter:
```sh
./bf HelloWorld.bf
```

Alternatively, program can be passed via pipe:

```sh
echo "++++++++[>++++[>++>+++>+++>+<<<<-]>+>+>->>+[<]<-]>>.>---.+++++++..+++.>>.<-.<.+++.------.--------.>>+.>++." | ./bf
cat sierpinski.b | ./bf
```

## Resources
- _Hello World!_ example is from  [Wikipedia](https://en.wikipedia.org/wiki/Brainfuck)
- Other examples are from [brainfuck.org](https://brainfuck.org)

