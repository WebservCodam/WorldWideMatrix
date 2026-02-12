#!/bin/bash

HOST="localhost"
PORT="8080"
TEST_DIR="./requests"
ADD_TEST="./requests/maybe"

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

echo "------------------------------------------------"
echo "Running Tests against $HOST:$PORT"
echo "------------------------------------------------"

run_tests_in_dir() {
    directory=$1
    echo "Checking directory: $directory"
    
    for file in "$directory"/*; do
        [ -e "$file" ] || continue
        
        filename=$(basename "$file")
        expected_code=$(echo "$filename" | cut -d'_' -f1)
        
        if [[ "$file" == *.txt ]]; then
            # .txt files: Sanitize and enforce \r\n line endings
            response=$({ cat "$file"; printf "\n\n"; } | tr -d '\r' | awk '{print $0"\r"}' | nc -w 1 $HOST $PORT)
        elif [[ "$file" == *.raw ]]; then
            # .raw files: Send raw bytes exactly as they are
            response=$(cat "$file" | nc -w 1 $HOST $PORT)
        else
            continue
        fi
        
        actual_code=$(echo "$response" | grep -oE "HTTP/1.1 [0-9]+" | awk '{print $2}')

        if [ "$actual_code" == "$expected_code" ]; then
            echo -e "${GREEN}[PASS] $filename${NC}"
        else
            echo -e "${RED}[FAIL] $filename${NC}"
            echo -e "       Expected: $expected_code"
            echo -e "       Got:      $actual_code"
        fi
    done
}

run_tests_in_dir "$TEST_DIR"
echo -e "${NC}\n--- Additional tests ---"
run_tests_in_dir "$ADD_TEST"