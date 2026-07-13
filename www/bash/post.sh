#!/bin/bash

body=$(cat)

printf "Content-Type: text/html\r\n\r\n"
echo "<html>"
echo "<body>"
echo "<h1>Hello from Bash!</h1>"
echo "<p>Method: ${REQUEST_METHOD}</p>"
echo "<p>Query string: ${QUERY_STRING}</p>"
echo "<p>Content-Length: ${CONTENT_LENGTH}</p>"
echo "<p>Body: ${body}</p>"
echo "</body>"
echo "</html>"
