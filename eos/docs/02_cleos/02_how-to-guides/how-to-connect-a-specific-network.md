## Goal

Connect to a specific `nodsig` or `ksigd` host to send COMMAND

`clsig` and `ksigd` can connect to a specific node by using the `--url` or `--wallet-url` optional arguments, respectively, followed by the http address and port number these services are listening to.

[[info | Default address:port]]
| If no optional arguments are used (i.e. `--url` or `--wallet-url`), `clsig` attempts to connect to a local `nodsig` or `ksigd` running at localhost `127.0.0.1` and default port `8888`.

## Before you begin

* Install the currently supported version of `clsig`

## Steps
### Connecting to nodsig

```sh
clsig -url http://nodsig-host:8888 COMMAND
```

### Connecting to ksigd

```sh
clsig --wallet-url http://ksigd-host:8888 COMMAND
```
