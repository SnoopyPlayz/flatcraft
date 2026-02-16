# flatcraft
top down minecraft made in c++ with [raylib](https://github.com/raysan5/raylib). <br>

## Joining game
flatcraft --join localhost 1236

## example Image:

## building
install premake5, clang then:
```
git clone https://github.com/SnoopyPlayz/flatcraft.git
cd flatcraft
premake5 gmake
make -j$(nproc)
```
## cross compiling to windows
add ```config=release_windows64``` to the end of the make command

## controls
0 - 9 - select block <br>
right click - place block<br>
left click - remove block<br>
wasd - move<br>
