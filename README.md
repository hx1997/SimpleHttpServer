# SimpleHttpServer

Very simple and crude HTTP server written in C 

## Features

- [x] support GET requests for static pages
- [ ] support GET/POST requests for PHP pages by using FastCGI
- [ ] multiprocessing mode on Linux

## Build

On both platforms, compiled executable will be in `Debug` directory by default.

### Windows

1. Make sure you have Visual Studio 2017 installed (not tested with other versions of VS)
2. Download the repo, extract
3. Open `SimpleHttpServer.sln` and build

### Linux

```
$ git clone https://github.com/hx1997/SimpleHttpServer.git
$ cd SimpleHttpServer/SimpleHttpServer
$ mkdir ../Debug
$ make && make clean
```

## Usage

```
SimpleHttpServer [-p port] [-r www_root] [-i index_filename]
```

`port`: number of the port to listen on for HTTP connections, defaults to 8080

`www_root`: path to the WWW root directory, defaults to `./www`

`index_filename`: default file returned when requesting `/`, defaults to `index.html`

## License

MIT License