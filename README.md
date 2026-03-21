# flatcraft
top down minecraft made in **c++** with [raylib](https://github.com/raysan5/raylib). <br>



## example Image:
<img width="650" height="400" alt="image" src="https://github.com/user-attachments/assets/ff81c8c8-835b-4319-8ec0-0f2254d7f236" />

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
