## Controls:
- WASD + Space for movement
- E to dash
- S for settings (in menu)
- Enter to start game

## Notice:

This game is fairly resource intensive, and GPU heavy.

First of all, be aware that asset loading takes a few moments, so don't panic if the game doesn't start immediately or if the window is shown as "unresponsive". If you want to see the progress, launch it from the terminal. 

This game was developed with an AMD Ryzen 7 7840U (16 threads) on integrated graphics with 512MB of VRAM, where it comfortably runs with a 16ms frametime (60 FPS) at 1080p, so lower end devices might struggle. In order to get the most out of it, make sure you're running on AC power (not battery) and on performance mode (if your OS supports it). 

Additionally, most of the rendering cost is dependent on the window framebuffer size, so it is a good idea to adjust the render scale in the settings before starting the game if it doesn't run smoothly enough on your machine. On Windows and Mac you can expect a significant chunk of memory to be used up by the program (about 1-2GB so just keep an eye on your RAM usage before running).

Finally, this game was developed in just a few weeks, being intended primarily as a tech demo for my OpenGL renderer, so it may be a bit unstable. Please feel free to reach out to report any bugs or issues, but be aware that I most likely will not be maintaining this project (though I do have plans to rewrite the renderer sometime).

Thanks for downloading, and enjoy!
Jens.