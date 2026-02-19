# Webserv Validation Test Files

This directory contains comprehensive test files for validating the NGINX-style configuration parser and validator.

## Directory Structure

```
test_files/
├── valid/                  # Valid configurations (should pass validation)
├── invalid/
│   ├── directive_names/    # Invalid directive names
│   ├── wrong_context/      # Directives in wrong contexts
│   ├── wrong_param_count/  # Wrong number of parameters
│   └── invalid_values/     # Invalid parameter values
├── edge_cases/             # Boundary conditions and edge cases
└── README.md
```

## Usage

Run the tester with any configuration file:

```bash
./tester test_files/valid/basic.conf
./tester test_files/invalid/directive_names/unknown_directive.conf
```

## Test Categories

### Valid Configurations (`valid/`)

These files demonstrate correct syntax and should pass all validation checks:

| File | Description |
|------|-------------|
| `basic.conf` | Minimal valid configuration with one server |
| `multi_server.conf` | Multiple server blocks with different listen addresses |
| `with_locations.conf` | Server with location blocks |
| `with_error_pages.conf` | Configuration with error_page directives |
| `with_limit_except.conf` | HTTP method restrictions using limit_except |
| `with_allow_deny.conf` | Access control with allow/deny directives |

### Invalid Directive Names (`invalid/directive_names/`)

Tests for handling unknown or misspelled directives:

| File | Error Type |
|------|------------|
| `unknown_directive.conf` | Completely unknown directive name |
| `typo_in_directive.conf` | Common typo (server_nam instead of server_name) |
| `invalid_block.conf` | Unknown block directive |

### Wrong Context (`invalid/wrong_context/`)

Tests for directives used in invalid contexts:

| File | Error Description |
|------|-------------------|
| `listen_in_http.conf` | `listen` directive in `http` block (should be in `server`) |
| `server_in_location.conf` | `server` block inside `location` (should be in `http`) |
| `http_in_server.conf` | `http` block inside `server` (should be in `main`) |
| `worker_processes_in_http.conf` | `worker_processes` in `http` (should be in `main`) |

### Wrong Parameter Count (`invalid/wrong_param_count/`)

Tests for incorrect number of parameters:

| File | Error Description |
|------|-------------------|
| `worker_processes_no_params.conf` | `worker_processes` with no parameters (needs 1) |
| `listen_no_params.conf` | `listen` with no parameters (needs 1) |
| `root_no_params.conf` | `root` with no parameters (needs 1) |
| `listen_too_many_params.conf` | `listen` with too many parameters (max 1) |
| `error_page_one_param.conf` | `error_page` with only 1 parameter (needs at least 2) |

### Invalid Parameter Values (`invalid/invalid_values/`)

Tests for syntactically valid but semantically invalid values:

| File | Error Description |
|------|-------------------|
| `invalid_port.conf` | Port number > 65535 |
| `invalid_ip_address.conf` | Malformed IP address (999.999.999.999) |
| `invalid_worker_processes.conf` | worker_processes value > 16 |
| `invalid_autoindex.conf` | autoindex with "yes" instead of "on"/"off" |
| `invalid_root_path.conf` | root with relative path (must start with /) |
| `invalid_return_code.conf` | return with invalid HTTP status code |
| `invalid_error_page_code.conf` | error_page with non-error code (200) |
| `invalid_http_method.conf` | limit_except with invalid HTTP method |
| `invalid_index.conf` | index with value other than "index.html" |
| `invalid_allow_address.conf` | allow/deny with invalid IP address |

### Edge Cases (`edge_cases/`)

Tests for boundary conditions and special scenarios:

| File | Description |
|------|-------------|
| `empty_file.conf` | Completely empty configuration file |
| `minimal_valid.conf` | Absolute minimum valid configuration |
| `nested_locations.conf` | Location blocks nested within location blocks |
| `max_server_names.conf` | Maximum number of server_name parameters (20) |
| `http_without_server.conf` | http block with no server (missing required child) |
| `server_without_listen.conf` | server block with no listen (missing required child) |
| `location_empty_block.conf` | location with empty body |
| `multiple_error_pages.conf` | Many error codes in error_page directives |
| `limit_except_empty.conf` | limit_except block with no allow/deny rules |

## Validation Rules Tested

### Directive-Level Validation

1. **Directive Name**: Must be recognized in NGINX_DIRECTIVE_SPECS
2. **Context**: Must appear in allowed context (main, http, server, location, limit_except)
3. **Parameter Count**: Must be within minArgs and maxArgs range
4. **Parameter Values**: Must pass directive-specific validation function

### Block-Level Validation

1. **Required Children**: Certain blocks require specific child directives:
   - `http` requires at least one `server`
   - `server` requires at least one `listen`
2. **Non-Empty Blocks**: Block directives should contain children
3. **Context Propagation**: Child directives must be valid in their parent's context

### Specific Directive Rules

- **worker_processes**: 1-16 or "auto"
- **listen**: Valid port (1-65535) and/or IP address
- **root**: Must start with `/` (absolute path)
- **index**: Currently hardcoded to "index.html"
- **autoindex**: Must be "on" or "off"
- **error_page**: First N params are error codes (400-599), last param is URI
- **return**: First param is HTTP status code (100-599), optional second param is URL/text
- **limit_except**: Methods must be GET, POST, DELETE, or HEAD; must contain allow/deny
- **allow/deny**: Must be valid IP, CIDR notation, or "all"

## Expected Behavior

### Valid Files
- Lexer: Tokenize successfully
- Parser: Build complete AST
- Validator: Return true (pass all checks)

### Invalid Files
- Lexer: May tokenize successfully
- Parser: May parse successfully
- Validator: Return false (fail validation)

### Edge Cases
- Some should pass, some should fail depending on implementation
- Test boundary conditions and error handling
