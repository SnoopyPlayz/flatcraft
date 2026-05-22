# flatcraft
top down minecraft made in **c++** with [raylib](https://github.com/raysan5/raylib). <br>

## example Image:
<img width="1277" height="719" alt="flatcraft" src="https://github.com/user-attachments/assets/658b51b7-82d6-4234-bca3-23ae017f9b8a" />

## building
install premake5, clang then:
```
git clone https://github.com/SnoopyPlayz/flatcraft.git
cd flatcraft
premake5 gmake
make -j$(nproc)
```
## Starting game and server
```flatcraft```

## Joining game
```flatcraft --join localhost```

## cross compiling to windows
add ```config=release_windows64``` to the end of the make command

## controls
0 - 9 - select block <br>
right click - place block<br>
left click - remove block<br>
wasd - move<br>

## code note
do not add new/random namespaces (including anonymous namespaces) unless explicitly needed for the task.
