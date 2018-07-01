# SimpleHttpServer

Very simple and crude HTTP server written in C 

## Build

On both platforms, compiled executable will be in `Debug` directory by default.

### Windows

1. Make sure you have Visual Studio 2017 installed (not tested with other versions of VS)
2. Download the repo, extract
3. Open `SimpleHttpServer.sln` and build

### Linux

```
git clone https://github.com/hx1997/SimpleHttpServer.git
cd SimpleHttpServer/SimpleHttpServer
make && make clean
```

## Usage

```
SimpleHttpServer [-p port] [-r www_root] [-i index_filename]
```

`port`: the port number to listen on for HTTP connections
`www_root`: path to the WWW root directory
`index_filename`: default file returned when requesting `/`

## License

MIT License