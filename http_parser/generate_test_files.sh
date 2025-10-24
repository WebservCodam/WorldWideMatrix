#!/bin/bash

mkdir -p http_files

echo "Generating normal POST request..."
printf "POST /submit HTTP/1.1\r\nHost: example.com\r\nContent-Length: 13\r\nConnection: keep-alive\r\n\r\nHello=World!" > http_files/normal_post.txt

echo "Generating normal GET request..."
printf "GET /index.html HTTP/1.1\r\nHost: example.com\r\nConnection: close\r\n\r\n" > http_files/normal_get.txt

echo "Generating chunked POST request..."
printf "POST /upload HTTP/1.1\r\nHost: example.com\r\nTransfer-Encoding: chunked\r\nConnection: keep-alive\r\n\r\n4\r\nWiki\r\n5\r\npedia\r\n0\r\n\r\n" > http_files/chunked_post.txt

echo "Generating edge case: empty body with Content-Length 0..."
printf "POST /empty HTTP/1.1\r\nHost: example.com\r\nContent-Length: 0\r\nConnection: keep-alive\r\n\r\n" > http_files/empty_body.txt

echo "All test files generated in http_files/."
