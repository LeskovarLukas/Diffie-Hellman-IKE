# leskovar_project
[![wakatime](https://wakatime.com/badge/github/LeskovarLukas/leskovar_project.svg)](https://wakatime.com/badge/github/LeskovarLukas/leskovar_project)

Simple project simulating TLS (I guess its Version 1.0)

Libraries in use:
- [Asio](https://think-async.com/Asio/)
- [BigInt](https://github.com/faheel/BigInt)
- [CLI11](https://github.com/CLIUtils/CLI11)
- [spdlog](https://github.com/gabime/spdlog)
- [Nlohoman JSON](https://github.com/nlohmann/json)
- [PicoSHA2](https://github.com/okdshin/PicoSHA2)

For version history see [Changelog](https://github.com/LeskovarLukas/leskovar_project/blob/main/CAHNGELOG.org)

## Installation 
```
git clone git@github.com:LeskovarLukas/leskovar_project.git
```

## Build
```
cd build && meson ..
ninja -j 4              // not recommended for systems with less than 8GB of RAM
```
or 

```
cd build && meson ..
ninja                   // may take longer
```

## Run
First start the server:
```
./tls_server -p 4433 -t 10000 -l info
```

Then you may have multiple clients connect to the server:
```
./tls_client -n localhost -p 4433 -d 0 -l info

