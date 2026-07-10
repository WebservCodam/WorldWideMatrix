# webserv

A non-blocking HTTP/1.1 server written in C++17, configured with an
nginx-inspired configuration file. It serves static sites, handles file
uploads, runs CGI scripts, and hosts multiple virtual servers on multiple
interfaces and ports — all driven by a single `epoll` event loop.

## Build & run

```sh
make                                        # produces ./webserv
./webserv config_files/valid/example.conf   # run with a config file
```

`webserv` takes exactly one argument: the path to a configuration file. It
exits with an error if the argument is missing or the file cannot be opened or
parsed. Send `SIGINT` (Ctrl-C) to shut it down cleanly.

Requirements: a C++17 compiler (`c++`) and `make`. No external libraries.

## Features

- **HTTP/1.1** with persistent connections (keep-alive) and request pipelining.
- **Methods**: `GET`, `POST` (file upload), `DELETE`. Unsupported methods
  return `405` with an `Allow` header; malformed requests return `4xx` without
  crashing.
- **Static files** with configurable roots, a default index file per directory,
  and optional directory listing (autoindex).
- **CGI** (e.g. Python) for `GET` and `POST`, executed in the script's own
  directory so relative paths inside the script resolve correctly.
- **Virtual hosts**: multiple `server` blocks can share an `interface:port`
  and are selected by the `Host` header.
- **Multiple listeners**: one server can listen on several `interface:port`
  pairs; several servers can listen on different ones.
- **Configurable error pages**, **client body size limits**, **HTTP
  redirects** (`return`), and a **keep-alive timeout**.

## Architecture

The program is built around one `epoll` instance in a single event loop
(`Webserv::startServers`). Every socket — listening, client, and CGI pipe — is
non-blocking and registered with `epoll`. I/O happens only when `epoll_wait`
reports a file descriptor ready, and each ready client is serviced for exactly
one read or write per loop pass so no direction can starve another. Nothing
reads or writes an event-driven descriptor without first being told it is ready.

The main components:

| Area | Files | Responsibility |
|------|-------|----------------|
| Event loop / connections | `Webserv.*` | `epoll` registration, accept, buffered reads/writes, keep-alive, CGI orchestration |
| Request routing | `Server.*` | Match a request to a `location`, enforce methods and body-size limits, serve static files, uploads, deletes, redirects, error pages |
| HTTP parsing | `httpparser/` | Parse the request line, headers, and body (Content-Length and chunked); report `INCOMPLETE` / `COMPLETE` / `ERROR` |
| CGI | `Cgi.*`, `Webserv::handleCGI` | Fork the interpreter, wire up `stdin`/`stdout` pipes, build the CGI environment |
| Configuration | `configparser/` | Lex → parse → validate the config file into `ServerConfig` objects |

A request flows: `epoll` reports the client readable → bytes are buffered and
parsed → the matching `Server`/`location` is selected → the response is either
built directly (static/upload/delete/error) or produced by a CGI child whose
output is read back through a pipe → the response is flushed when `epoll`
reports the client writable.

## Configuration

A config file contains one or more `server` blocks. Each may contain multiple
`location` blocks. Directives end with `;`; blocks are wrapped in `{ }`.

### Server-level directives

| Directive | Example | Meaning |
|-----------|---------|---------|
| `listen` | `listen 127.0.0.1:8085;` | `interface:port` to bind. May appear multiple times. |
| `server_name` | `server_name "Server 1";` | Host name used to pick a virtual server on a shared `interface:port`. |
| `root` | `root /www/;` | Base directory for serving files. |
| `index` | `index index.html;` | Default file served for a directory request. |
| `autoindex` | `autoindex on;` | Enable directory listing when no index is present. |
| `client_max_body_size` | `client_max_body_size 2m;` | Maximum request body size (`k`/`m` suffixes or raw bytes). |
| `keepalive_timeout` | `keepalive_timeout 2;` | Idle seconds before a kept-alive connection is closed. |
| `error_page` | `error_page 404 404.html;` | Custom page for a status code. |
| `cgi_handler` | `cgi_handler .py /usr/bin/python3;` | Map a file extension to an interpreter. |

### Location-level directives

A `location /path { ... }` block matches request URIs by longest prefix. It
accepts `root`, `index`, `autoindex`, `client_max_body_size`, and `cgi_handler`
(same meaning as above), plus:

| Directive | Example | Meaning |
|-----------|---------|---------|
| `methods` | `methods GET POST;` | Allowed HTTP methods for this route. |
| `return` | `return 301 /new;` | Reply with a status; for `3xx`, redirect to the given URI. |
| `upload_path` | `upload_path www/uploads;` | Directory where uploaded (`POST`) bodies are written. Required when `POST` is allowed. |

### Example

```nginx
server {
    listen 127.0.0.1:8085;
    server_name "Server 1";
    client_max_body_size 2m;
    keepalive_timeout 2;

    error_page 404 404.html;
    root /www/;
    autoindex on;
    cgi_handler .py /usr/bin/python3;

    location / {
        index index.html;
        methods GET;
    }

    location /uploads {
        methods GET POST DELETE;
        upload_path www/uploads;
        autoindex on;
    }

    location /cgi-bin {
        index script.py;
        methods GET;
    }
}
```

More examples live in `config_files/valid/`; `config_files/invalid/` holds
files that must be rejected by the parser.

## Testing

```sh
# Functional checks with curl / telnet
curl -i http://127.0.0.1:8085/                       # static GET
curl -i -X POST --data "hi" http://127.0.0.1:8085/uploads/f.txt   # upload
curl -i -X DELETE http://127.0.0.1:8085/uploads/f.txt             # delete
curl -i http://127.0.0.1:8085/cgi-bin                # CGI

# Stress test (availability should stay well above 99.5%)
siege -b -t30s -c50 http://127.0.0.1:8085/

# Memory
valgrind --leak-check=full ./webserv config_files/valid/example.conf
```

The `www/` directory holds sample site content, including `www/cgi-bin/` for
CGI scripts.
