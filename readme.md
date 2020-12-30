# Shadowply

## Setup

- use cmake-gui to create Visual Studio files
- download ffmpeg shared libraries https://github.com/BtbN/FFmpeg-Builds
    - make sure to get the gpl shared files
- move files into <project>/libs/ffmpeg
- copy dll's from bin to debug executable output folder
- download pthread-win32 https://github.com/GerHobbelt/pthread-win32
    - build project with visual studio
    - extract output files to <project>/libs/pthread-win32
