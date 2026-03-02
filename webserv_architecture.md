```mermaid
graph TB
    %% ==============================================
    %% PROGRAM INITIALIZATION
    %% ==============================================
    Start([🚀 Program Start<br/>main.cpp:59]) --> ArgValidation{Validate Arguments<br/>argc == 2?}
    ArgValidation -->|❌ Invalid| ErrorArgs[❌ Error: Expecting input file<br/>exit(EXIT_FAILURE)]
    ArgValidation -->|✅ Valid| FileOperations[📁 File Operations]

    FileOperations --> OpenFile[📖 Open config file<br/>std::ifstream file(argv[1])]
    OpenFile --> FileCheck{File opened successfully?}
    FileCheck -->|❌ Failed| ErrorFile[❌ Error: Could not open file<br/>exit(EXIT_FAILURE)]
    FileCheck -->|✅ Success| ReadFileContent[📋 Read file to stringstream<br/>buffer << file.rdbuf()]

    %% ==============================================
    %% CONFIGURATION PARSING PIPELINE
    %% ==============================================
    ReadFileContent --> ParserCreation[🔧 Create Parser<br/>Parser(input)]
    ParserCreation --> ParseMethod[🎯 Parser::parse()]

    subgraph ConfigParser["⚙️ Configuration Parsing Pipeline"]
        %% LEXICAL ANALYSIS
        ParseMethod --> LexerCreation[📝 Create Lexer<br/>Lexer(_input)]
        LexerCreation --> TokenizeProcess[🔍 Lexer::tokenize()]

        subgraph LexicalAnalysis["📝 Lexical Analysis (Lexer.cpp)"]
            TokenizeProcess --> CharLoop{Process each character<br/>pos < _input.length()}

            CharLoop --> WhitespaceCheck{Whitespace?<br/>isspace(char)}
            WhitespaceCheck -->|✅ Yes| HandleWhitespace[Skip whitespace<br/>Handle newlines \\n<br/>Update line/col numbers]
            HandleWhitespace --> CharLoop

            WhitespaceCheck -->|❌ No| CommentCheck{Comment?<br/>char == '#'}
            CommentCheck -->|✅ Yes| SkipComment[Skip to newline<br/>Ignore comment content]
            SkipComment --> CharLoop

            CommentCheck -->|❌ No| SingleCharCheck{Single char token?<br/>'{', '}', ';', ','}
            SingleCharCheck -->|✅ Yes| CreateSingleToken[Create Token:<br/>LBRACE, RBRACE<br/>SEMICOLON, COMMA]
            CreateSingleToken --> AddToTokenList[Add to _tokens vector]
            AddToTokenList --> CharLoop

            SingleCharCheck -->|❌ No| StringCheck{String literal?<br/>char == '"' or '\\''}
            StringCheck -->|✅ Yes| ConsumeString[consumeString()<br/>Handle escape chars<br/>Check for unterminated]
            ConsumeString --> CreateStringToken[Create STRING token]
            CreateStringToken --> AddToTokenList

            StringCheck -->|❌ No| WordCheck{Valid word char?<br/>!isspace && !special}
            WordCheck -->|✅ Yes| ConsumeWord[consumeWord()<br/>Read until delimiter]
            ConsumeWord --> CreateWordToken[Create WORD token]
            CreateWordToken --> AddToTokenList

            WordCheck -->|❌ No| LexError[❌ Lexical Error<br/>Unexpected character]

            CharLoop -->|🏁 End of input| AddEOFToken[Add END_OF_FILE token]
        end

        %% SYNTAX ANALYSIS
        AddEOFToken --> DirectiveParsing[🌲 Parse Directives]

        subgraph SyntaxAnalysis["🌲 Syntax Analysis (Parser.cpp)"]
            DirectiveParsing --> DirectiveLoop{More tokens to parse?<br/>!isAtEnd()}

            DirectiveLoop -->|✅ Yes| ParseSingleDirective[parseDirective()]
            ParseSingleDirective --> LookAhead[Look-ahead analysis<br/>Find ';' or '{' terminator]

            LookAhead --> DirectiveType{Directive type?}
            DirectiveType -->|Simple| ParseSimple[parseSimpleDirective()<br/>name params;]
            DirectiveType -->|Block| ParseBlock[parseBlockDirective()<br/>name params { children }]

            ParseSimple --> InitDirective[initializeDirective()<br/>Set name, context, params]
            ParseBlock --> InitDirective

            InitDirective --> ParseParameters[parseParameters()<br/>Collect WORD/STRING tokens]
            ParseParameters --> BlockCheck{Is block directive?}

            BlockCheck -->|❌ No| ExpectSemicolon[Expect ';' token<br/>Simple directive end]
            BlockCheck -->|✅ Yes| ExpectLBrace[Expect '{' token<br/>Block directive start]

            ExpectLBrace --> ParseChildren[Parse child directives<br/>Recursive parseDirective()]
            ParseChildren --> SetContext[Set parent context<br/>child->setContext(parent->getName())]
            SetContext --> SetParent[Set parent pointer<br/>child->setParent(parent)]
            SetParent --> ExpectRBrace[Expect '}' token<br/>Block directive end]

            ExpectSemicolon --> AddDirectiveToAST[Add directive to AST]
            ExpectRBrace --> AddDirectiveToAST
            AddDirectiveToAST --> DirectiveLoop

            DirectiveLoop -->|❌ No more| CreateConfigFile[Create ConfigFile AST<br/>std::unique_ptr<ConfigFile>]
        end

        %% SEMANTIC VALIDATION
        CreateConfigFile --> SemanticValidation[🔍 Semantic Validation]

        subgraph ValidationPhase["🔍 Validation Phase (DirectiveSpecs.cpp)"]
            SemanticValidation --> ValidateLoop{For each directive<br/>validateSemantics()}
            ValidateLoop --> ValidateDirective[validateDirective(node)]

            ValidateDirective --> CheckSpecs[Check NGINX_DIRECTIVE_SPECS<br/>Lookup directive definition]
            CheckSpecs --> ValidateContext[validateContext()<br/>Check validContexts set]
            ValidateContext --> ValidateArgCount[Check argument count<br/>minArgs <= count <= maxArgs]
            ValidateArgCount --> ValidateChildren[validateRequiredChildren()<br/>Check required child directives]
            ValidateChildren --> CallCustomValidator[Call custom validator<br/>validateArgs function pointer]

            CallCustomValidator --> SpecificValidators{Directive-specific validators}
            SpecificValidators --> ServerValidator[validateServerDirective()]
            SpecificValidators --> LocationValidator[validateLocationDirective()]
            SpecificValidators --> ListenValidator[validateListenDirective()<br/>Check address:port format]
            SpecificValidators --> RootValidator[validateRootDirective()<br/>Check path exists]
            SpecificValidators --> IndexValidator[validateIndexDirective()]
            SpecificValidators --> ErrorPageValidator[validateErrorPageDirective()]
            SpecificValidators --> ReturnValidator[validateReturnDirective()]
            SpecificValidators --> MethodsValidator[validateMethodsDirective()<br/>GET, POST, DELETE]
            SpecificValidators --> ClientMaxBodyValidator[validateClientMaxBodySizeDirective()]
            SpecificValidators --> KeepaliveValidator[validateKeepaliveTimeoutDirective()]

            ServerValidator --> ValidationComplete[✅ Validation Complete]
            LocationValidator --> ValidationComplete
            ListenValidator --> ValidationComplete
            RootValidator --> ValidationComplete
            IndexValidator --> ValidationComplete
            ErrorPageValidator --> ValidationComplete
            ReturnValidator --> ValidationComplete
            MethodsValidator --> ValidationComplete
            ClientMaxBodyValidator --> ValidationComplete
            KeepaliveValidator --> ValidationComplete

            ValidationComplete --> ValidateLoop
            ValidateLoop -->|🏁 All validated| ReturnAST[Return validated AST]
        end
    end

    %% ==============================================
    %% SERVER CONFIGURATION CREATION
    %% ==============================================
    ReturnAST --> CreateServersMethod[🏗️ ConfigFile::createServers()]

    subgraph ServerCreation["🏗️ Server Configuration Creation (Configuration.cpp)"]
        CreateServersMethod --> FindServerDirectives[findAllDirectives("server")<br/>Get all server blocks]
        FindServerDirectives --> ServerIterationLoop{For each server directive}

        ServerIterationLoop -->|📋 Process server| ProcessServerConfig[Process Server Configuration]
        ProcessServerConfig --> InitServerVars[Initialize server variables:<br/>serverName, listenDirectives<br/>maxBodySize, errorPages<br/>locations, keepalive_timeout]

        InitServerVars --> ProcessServerChildren[Process server children directives]
        ProcessServerChildren --> ServerNameProcess[server_name → processServerName()]
        ProcessServerChildren --> ListenProcess[listen → processListen()<br/>Parse address:port]
        ProcessServerChildren --> MaxBodyProcess[client_max_body_size → processClientMaxBodySize()]
        ProcessServerChildren --> ErrorPageProcess[error_page → processErrorPages()<br/>Map error codes to pages]
        ProcessServerChildren --> LocationProcess[location → processLocation()<br/>Create Location objects]
        ProcessServerChildren --> KeepaliveProcess[keepalive_timeout → processKeepaliveTimeout()]

        ServerNameProcess --> BuildServerConfig[Build ServerConfig object]
        ListenProcess --> BuildServerConfig
        MaxBodyProcess --> BuildServerConfig
        ErrorPageProcess --> BuildServerConfig
        LocationProcess --> ProcessLocationChildren[Process location children:<br/>root, index, autoindex<br/>methods, return]
        KeepaliveProcess --> BuildServerConfig

        ProcessLocationChildren --> CreateLocationObject[Create Location object<br/>with processed settings]
        CreateLocationObject --> BuildServerConfig

        BuildServerConfig --> AddToServersList[Add ServerConfig to _servers vector]
        AddToServersList --> ServerIterationLoop

        ServerIterationLoop -->|🏁 All servers processed| ServersCreated[✅ Servers Created]
    end

    %% ==============================================
    %% SERVER INITIALIZATION & NETWORKING
    %% ==============================================
    ServersCreated --> ServerInitialization[🌐 Server::init_server()]

    subgraph NetworkSetup["🌐 Network Setup (Server.cpp)"]
        ServerInitialization --> CreateSocketsLoop{For each server config}
        CreateSocketsLoop --> GetListenDirectives[Get listen directives<br/>address:port pairs]
        GetListenDirectives --> CreateSocket[createSocket(address, port)<br/>System socket creation]
        CreateSocket --> ConfigureSocket[Configure socket options<br/>SO_REUSEADDR, non-blocking]
        ConfigureSocket --> BindSocket[bind() to address:port]
        BindSocket --> ListenSocket[listen() for connections]
        ListenSocket --> AddServerFD[Add server_fd to server_fds vector]
        AddServerFD --> CreateSocketsLoop

        CreateSocketsLoop -->|🏁 All sockets created| AddToEpoll[Add servers to epoll]
        AddToEpoll --> EpollLoop{For each server_fd}
        EpollLoop --> CreateEpollEvent[Create epoll_event<br/>EPOLLIN | EPOLLET | EPOLLPRI | EPOLLHUP]
        CreateEpollEvent --> EpollCtlAdd[epoll_ctl(EPOLL_CTL_ADD)<br/>Add server fd to epoll]
        EpollCtlAdd --> EpollLoop

        EpollLoop -->|🏁 All added| StartEventLoop[🔄 Start Event Loop]
    end

    %% ==============================================
    %% RUNTIME EVENT LOOP
    %% ==============================================
    StartEventLoop --> EventLoop[🔄 Main Event Loop<br/>Server::start_server()]

    subgraph RuntimeLoop["🔄 Runtime Event Loop (Server.cpp)"]
        EventLoop --> EpollWait[epoll_wait(events, 1000, 5000)<br/>Wait for events with 5s timeout]

        EpollWait --> ProcessEvents{Process each event<br/>for (events[i])}
        ProcessEvents --> CheckEventType{Event type?}

        CheckEventType -->|🔗 Server FD| NewConnection[connect_new(server_fd)<br/>Accept new connections]
        CheckEventType -->|📥 EPOLLIN| ReadFromClient[connect_in(client_fd)<br/>Read HTTP request data]
        CheckEventType -->|📤 EPOLLOUT| WriteToClient[connect_out(client_fd)<br/>Send HTTP response]

        %% NEW CONNECTION HANDLING
        NewConnection --> AcceptLoop[Accept loop<br/>Handle multiple pending connections]
        AcceptLoop --> AcceptConnection[accept() system call<br/>Get client socket]
        AcceptConnection --> SetNonBlocking[Set client socket non-blocking<br/>fcntl(F_SETFL, O_NONBLOCK)]
        SetNonBlocking --> CreateClientEntry[Add to client map<br/>list.emplace(client_fd, client_fd)]
        CreateClientEntry --> AddClientToEpoll[Add client to epoll<br/>EPOLLIN | EPOLLET events]
        AddClientToEpoll --> AcceptLoop

        %% READ DATA HANDLING
        ReadFromClient --> ReadBuffer[recv(client_fd, buffer, 8192)<br/>Read HTTP request data]
        ReadBuffer --> AppendToBuffer[Append to client buffer<br/>list.at(client_fd)._buf += buffer]
        AppendToBuffer --> ParseHTTP[parse(client_fd)<br/>Parse HTTP request]

        subgraph HTTPParsing["🌐 HTTP Request Parsing (HttpParser)"]
            ParseHTTP --> CreateParser[Create HttpParser instance]
            CreateParser --> CreateContext[Create ConnectionContext<br/>ctx.buffer = client_buffer]
            CreateContext --> ParseRequest[parser.parseRequest(ctx)]
            ParseRequest --> ParseStatus{Parse status?}

            ParseStatus -->|✅ COMPLETE| ExtractRequestData[Extract request data:<br/>method, uri, version<br/>headers, body]
            ParseStatus -->|⏳ INCOMPLETE| WaitMoreData[Wait for more data<br/>Keep connection open]
            ParseStatus -->|❌ ERROR| ParseError[Parse error<br/>Invalid HTTP format]

            ExtractRequestData --> LogRequestInfo[Log request information<br/>method, uri, headers]
        end

        LogRequestInfo --> SwitchToOutput[Switch to EPOLLOUT<br/>epoll_ctl(EPOLL_CTL_MOD)]
        SwitchToOutput --> ProcessEvents

        %% WRITE RESPONSE HANDLING
        WriteToClient --> ReadHTMLFile[Read HTML file<br/>std::ifstream("index.html")]
        ReadHTMLFile --> BuildHTTPResponse[Build HTTP response:<br/>Status line, headers<br/>Content-Length, Content-Type]
        BuildHTTPResponse --> SendResponse[write(client_fd, response)<br/>Send complete HTTP response]
        SendResponse --> CheckKeepAlive{Keep connection alive?}

        CheckKeepAlive -->|❌ Close| CloseConnection[close_client(client_fd)<br/>Remove from epoll and map]
        CheckKeepAlive -->|✅ Keep alive| SwitchToInput[Switch back to EPOLLIN<br/>epoll_ctl(EPOLL_CTL_MOD)]

        SwitchToInput --> ProcessEvents
        CloseConnection --> ProcessEvents

        ProcessEvents -->|🏁 All events processed| HealthCheck[check_health()<br/>Check client timeouts]

        %% HEALTH CHECK
        HealthCheck --> TimeoutLoop{For each client}
        TimeoutLoop --> CheckClientTime[Check client timestamp<br/>CheckTime() == -1?]
        CheckClientTime -->|⏰ Timeout| TimeoutClose[Close timed out client<br/>15 seconds inactivity]
        CheckClientTime -->|✅ Active| TimeoutLoop
        TimeoutClose --> TimeoutLoop

        TimeoutLoop -->|🏁 All checked| EventLoop
    end

    %% ==============================================
    %% ERROR HANDLING
    %% ==============================================
    ErrorArgs --> ProgramExit[💥 Program Exit]
    ErrorFile --> ProgramExit
    LexError --> ConfigErrorExit[💥 Config Error Exit<br/>Print error with line/column]
    ParseMethod -.->|ConfigError thrown| ConfigErrorExit
    SemanticValidation -.->|Validation error| ConfigErrorExit
    ConfigErrorExit --> ProgramExit

    %% ==============================================
    %% KEY DATA STRUCTURES
    %% ==============================================
    subgraph DataStructures["📊 Key Data Structures"]
        direction TB

        TokenStruct[📝 Token<br/>- type: TokenType<br/>- value: string<br/>- line, column: size_t]

        DirectiveStruct[🌲 Directive<br/>- name, context: string<br/>- parameters: vector&lt;string&gt;<br/>- parent: Directive*<br/>- children: vector&lt;unique_ptr&lt;Directive&gt;&gt;<br/>- line, column: size_t]

        ConfigFileStruct[📋 ConfigFile<br/>- directives: vector&lt;unique_ptr&lt;Directive&gt;&gt;<br/>- servers: vector&lt;ServerConfig&gt;<br/>+ findDirective(), createServers()]

        ServerConfigStruct[🏠 ServerConfig<br/>- serverName: string<br/>- listenDirectives: vector&lt;ListenDirective&gt;<br/>- maxBodySize: unsigned long long<br/>- errorPages: unordered_map&lt;int, ErrorPage&gt;<br/>- locations: vector&lt;Location&gt;<br/>- keepalive_timeout: int]

        LocationStruct[📍 Location<br/>- path, root, index: string<br/>- autoindex: bool<br/>- methods: bool (GET, POST, DELETE)<br/>- returnPage: ReturnPage]

        ListenDirectiveStruct[🔌 ListenDirective<br/>- address: string (default: "127.0.0.1")<br/>- port: string (default: "8080")]

        ErrorPageStruct[❌ ErrorPage<br/>- errorCode: int<br/>- isRedirect: bool<br/>- redirectCode: int<br/>- URI: string]

        ReturnPageStruct[↩️ ReturnPage<br/>- code: int<br/>- isURI: bool<br/>- page: string]
    end

    %% ==============================================
    %% STYLING
    %% ==============================================
    classDef startEnd fill:#e1f5fe,stroke:#01579b,stroke-width:3px
    classDef error fill:#ffebee,stroke:#c62828,stroke-width:2px
    classDef success fill:#e8f5e8,stroke:#2e7d32,stroke-width:2px
    classDef process fill:#fff3e0,stroke:#ef6c00,stroke-width:2px
    classDef decision fill:#f3e5f5,stroke:#7b1fa2,stroke-width:2px
    classDef dataStruct fill:#e0f2f1,stroke:#00695c,stroke-width:2px

    class Start,ProgramExit startEnd
    class ErrorArgs,ErrorFile,LexError,ConfigErrorExit,ParseError error
    class ServersCreated,ValidationComplete,ReturnAST,AddEOFToken success
    class EventLoop,ProcessEvents,ParseHTTP,BuildHTTPResponse process
    class ArgValidation,FileCheck,CharLoop,DirectiveLoop,CheckEventType decision
    class TokenStruct,DirectiveStruct,ConfigFileStruct,ServerConfigStruct,LocationStruct,ListenDirectiveStruct,ErrorPageStruct,ReturnPageStruct dataStruct
```

## Architecture Overview

This webserv implementation follows a classic multi-phase approach:

### 1. **Configuration Parsing Pipeline**
- **Lexical Analysis**: Converts config file text into tokens
- **Syntax Analysis**: Builds Abstract Syntax Tree (AST) from tokens
- **Semantic Validation**: Validates directives against NGINX specifications
- **Server Creation**: Converts AST into runtime ServerConfig objects

### 2. **Network Layer**
- **Socket Creation**: Creates listening sockets for each server configuration
- **Epoll Setup**: Adds all server sockets to epoll for efficient I/O multiplexing
- **Event Loop**: Handles incoming connections and HTTP requests asynchronously

### 3. **HTTP Processing**
- **Connection Management**: Accepts and tracks client connections
- **Request Parsing**: Parses HTTP requests using dedicated HttpParser
- **Response Generation**: Builds and sends HTTP responses
- **Keep-Alive Support**: Manages persistent connections with timeout handling

### 4. **Key Features**
- **Multi-server support**: Can handle multiple server configurations
- **Location blocks**: Supports complex routing with location-specific settings
- **Error pages**: Custom error page handling with redirect support
- **Method restrictions**: Configurable HTTP method support per location
- **Timeout management**: Automatic cleanup of inactive connections
- **Non-blocking I/O**: Efficient handling of multiple simultaneous connections

### File Structure
- `src/main.cpp`: Program entry point and initialization
- `src/configparser/`: Configuration parsing components (Lexer, Parser, Validation)
- `src/Server.cpp`: Main server implementation and event loop
- `src/Client.cpp`: Client connection handling
- `parser/`: HTTP request parsing utilities