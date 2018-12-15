nchip8
==
A CHIP-8 emulator/interpreter that operates entirely in the terminal.

Mainly for use over ssh for bored sysadmins

![](https://giant.gfycat.com/GratefulCookedHalicore.gif)

Building
----
_Requires ncurses_

```
git clone git@github.com:ocanty/nchip8.git
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
