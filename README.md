# Brainfuck interpreter

Complie with `make`.

Interpreter accepts programs as parameter:
```sh
./bf HelloWorld.bf
```

Alternativelly, program can be passed via pipe:

```sh
echo "++++++++[>++++[>++>+++>+++>+<<<<-]>+>+>->>+[<]<-]>>.>---.+++++++..+++.>>.<-.<.+++.------.--------.>>+.>++." | ./bf
cat sierpinski.b | ./bf
```

## Resources
- _Hello World!_ example is from  [Wikipedia](https://en.wikipedia.org/wiki/Brainfuck)
- Other examples are from [brainfuck.org](https://brainfuck.org)

