# HLMMOServer
> I wish this was in JavaScript

### Prerequisites
1. Don't use Mac - why would you do that?
2. Install [xmake](https://xmake.io/) for compiling
3. Install [Visual Studio Community - Desktop C++](https://visualstudio.microsoft.com/downloads/) for the compiling (you can use VSCode if you want)
4. Clone repository

### Getting Started
Inside the root, run

```sh
xmake
```

Type `Y` to allow `xmake` to install any dependency updates and let it run. 
Once the build is completed, copy all the `.dll` files into the folder that `xmake` created at `./build/windows/x64/release` alongside `HLMMOServer.exe`.

Run `HLMMOServer.exe` and confirm that the server is running by the `Listening on 27015` message.
