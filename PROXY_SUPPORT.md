# Proxy Support

## Overview

The WebSocket implementation in this project supports HTTP proxies for connections. This is particularly useful when running servers behind corporate firewalls or in environments where direct internet access is restricted.

## Automatic Proxy Detection (Linux)

On Linux platforms, the WebSocket system automatically detects proxy settings from standard environment variables. This provides seamless integration with system-wide proxy configurations without requiring additional setup.

### Supported Environment Variables

The system checks the following environment variables in order of precedence:

1. `HTTPS_PROXY` - Used for secure HTTPS connections (uppercase)
2. `https_proxy` - Used for secure HTTPS connections (lowercase)
3. `HTTP_PROXY` - Used for HTTP connections (uppercase)
4. `http_proxy` - Used for HTTP connections (lowercase)
5. `ALL_PROXY` - Used as a catch-all for all protocols (uppercase)
6. `all_proxy` - Used as a catch-all for all protocols (lowercase)

Both uppercase and lowercase variants are supported to ensure compatibility with different system configurations.

### Usage

To enable proxy support on Linux, simply set the appropriate environment variable before starting the server:

```bash
# Set proxy for all connections
export HTTPS_PROXY="http://proxy.example.com:8080"

# Or use HTTP_PROXY
export HTTP_PROXY="http://proxy.example.com:8080"

# Or use ALL_PROXY for all protocols
export ALL_PROXY="http://proxy.example.com:8080"

# Start the server (the proxy will be automatically detected)
./YourServerExecutable
```

### Proxy URL Format

The proxy URL should be in the following format:

```
http://[username:password@]hostname:port
```

Examples:
- `http://proxy.example.com:8080` - Simple proxy without authentication
- `http://user:pass@proxy.example.com:8080` - Proxy with authentication

## Manual Proxy Configuration

If you prefer to configure the proxy manually or if automatic detection doesn't work for your setup, you can still configure the proxy through the HTTP module's proxy settings. The automatic detection only applies when no proxy is already configured through the HTTP module.

## Platform Support

- **Linux**: ✅ Automatic proxy detection from environment variables (HTTPS_PROXY, HTTP_PROXY, ALL_PROXY)
- **Windows**: Uses HTTP module proxy configuration (automatic environment variable detection not implemented)

## Implementation Details

The automatic proxy detection is implemented in `Plugins/WebSockets/Source/WebSockets/Private/Lws/LwsWebSocketsManager.cpp` in the `InitWebSockets` function. The detection only occurs on Linux platforms (controlled by the `PLATFORM_LINUX` preprocessor directive) and only when no proxy has been configured through the HTTP module.

When a proxy is detected from environment variables, it is logged to the WebSockets log with the message:
```
Using proxy from environment variable <variable_name>: <proxy_url>
```

For example:
```
Using proxy from environment variable HTTPS_PROXY: http://proxy.example.com:8080
```

This helps with debugging and confirming that the proxy configuration is being applied correctly.
