---
name: verify
description: Build, launch, and drive the webserv HTTP server to verify changes at runtime.
---

# Verifying webserv

## Build & launch
```bash
make                                        # builds ./webserv (C++17, incremental)
./webserv config_files/valid/example.conf   # run from repo root — paths are cwd-relative
```
Listens on 127.0.0.1:8085 and 127.0.0.2:8085. Logs debug output to stdout.
Run it in the background and kill by PID when done.

## Flows worth driving (example.conf)
- Static (non-CGI path): `curl -H "Host: server2" http://127.0.0.1:8085/` → 200 "meowwwww"
  (server2 has no cgi_handler, so it exercises the non-CGI routing branch).
- CGI: `curl http://127.0.0.1:8085/cgi-bin` → runs www/cgi-bin/script.py → 200 "Hello World!".
- Redirect: `location /old` has `return 301 /new`.
- Uploads: POST to `/uploads` (upload_path www/uploads).

## Gotchas
- CGI scripts are exec'd directly (`execve(scriptpath, ...)`, relies on shebang) —
  the script needs its exec bit set (`chmod +x www/cgi-bin/script.py`) or every CGI
  request 502s immediately.
- A server-level `cgi_handler` is copied into EVERY location of that server
  (LocationDirective.cpp), so isCgiRequest is true for all URIs on server 1 in
  example.conf — even `/` and the `/old` redirect go down the CGI path. Use the
  Host: server2 vhost to test non-CGI behavior without editing the config.
- keepalive_timeout is 2s in example.conf; curl with `--max-time` to avoid hangs.
