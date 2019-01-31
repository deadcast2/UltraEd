<img src="https://s3.amazonaws.com/kittypizza/ultraed.png" width="550">

## A WIP level editor/game engine for the Nintendo 64 

<img src="https://i.imgur.com/etDiBGp.gif" width="600">

### Setup

I've greatly reduced the setup requirements so all you need is Windows XP x86/x64 and Visual Studio 6. For convenience I put together a package with SP3 and other goodies like Git. http://www.mediafire.com/file/ozha9c7kmhg8ik3/UltraEdDeps.zip. Once your environment is setup just pull down the repo, <b>install the Microsoft Visual C++ 2008 and OpenAL redistributables</b> located in Editor/deps and then open the editor project file in Visual Studio. You should then be able to compile and run the editor!

### Notes

The reason why UltraEd only runs on Windows XP/NT is due to the ROM build process. I rely upon the N64 SDK to do the final ROM construction which utilizes old 16-bit executables. I realize I could still build an editor for a modern OS and then only build the ROM on XP/NT but I wanted the editor to feel *sort of like* Unity 3D. Everything within one environment! :)

### Donations

If you would like... you can donate to UltraEd's development! Give any amount. Even $0.00 lol! ^_^

[![paypal](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=R25G2EARP89AL)

### Currently Working On... (1/31/19)

1. Improving ROM build process. Will be VERY simple once completed.
2. Adding a simple, non-spatial partioned sphere-based collision detection system.
3. Fixing script output to have correct object indexes for cameras and gameobjects.
