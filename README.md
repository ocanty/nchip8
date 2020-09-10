nchip8
==
A CHIP-8 emulator/interpreter that operates entirely in the terminal.

Mainly for use over ssh by bored sysadmins

<img src="https://gfycat.com/gratefulcookedhalicore.gif" width="410" height="300">

Building
----
_Requires ncurses and **latest** clang/gcc due to C++17 \<optional\>_

```
git clone https://github.com/ocanty/nchip8.git
cd nchip8
cmake CMakeLists.txt
make
```

Running
----
```
cd bin
./nchip8 <rom path> <cpu cycles per second>
```

You can find ROM packs freely available around the internet.

**Keys**

```
1 2 3 4 -> 1 2 3 C
Q W E R -> 4 5 6 D
A S D F -> 7 8 9 E
Z X C V -> A 0 B F
```

**Compatibility**

Nearly all tested ROMs work perfectly.

Sound is not implemented as to implement the CHIP-8 buzzer over a TTY would use the bell (PC speaker), which is quite annoying
