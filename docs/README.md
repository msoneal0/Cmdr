# Cmdr #

Cmdr is a command line terminal emulator client for MRCI host using text input/output. This helps administer MRCI host via local or remote TCP connections encrypted with TLS/SSL using the MRCI protocol. It also supports file transfers to/from the client using the GEN_FILE sub-protocol that MRCI understands.

### Usage ###

Cmdr have it's own terminal display so there is no command line switches to pass on start up. Instead, all commands are parsed and processed within it's own terminal interface. Any commands not seen as an internal command for the client itself is passed through to the MRCI host if connected. The client will add a numeric identifer to the end of all host command names that clash with the client commands so there is no chance a client and host command can conflict with each other.  

### Versioning System ###

This application uses the typical 3 number versioning system: [major].[minor].[patch]

* Major - this indicates any major changes to the application or changes that render user data of different majors incompatible.
* Minor - this indicates changes to the code that still maintains compatibility with existing user data.
* Patch - this indicates changes that won't require any behaviour changes at all.

Any increments to major resets minor and patch to 0.

### The Protocol ###

This application being a MRCI client uses the MRCI protocol to transport data to and from the host using TCP in a binary data format called MRCI frames. In general, local connections with the host are not encrypted but all connections outside of the host must be encrypted using TLS/SSL (including the local network).

Before any MRCI frames can be transported, both the host and client need basic information about each other. This is done by having the client send a fixed length client header when it successfully connects to the host and the host will reply with it's own fixed length host header, described below.

### MRCI Frame Format ###

```
[type_id][cmd_id][branch_id][data_len][payload]

type_id   - 1byte    - 8bit little endian integer type id of the payload.
cmd_id    - 2bytes   - 16bit little endian integer command id.
branch_id - 2bytes   - 16bit little endian integer branch id.
data_len  - 3bytes   - 24bit little endian integer size of the payload.
payload   - variable - the actual data to be processed.
```

notes:

* A full description of the type id's can be found in the [Type_IDs.md](type_ids.md) document.

* This client call commands by name but the host will assign unique command ids for it's command names and will require the client to use the command id on calling. To track this command id to command name relationship for the host commands, this client will rely on the [ASYNC_ADD_CMD](async.md) and [ASYNC_RM_CMD](async.md) async commands.

* The branch id is an id that can be used to run muliple instances of the same command on the host. Commands sent by a certain branch id will result in data sent back with that same branch id. For now, this client does not do branching; instead all commands sent to the host using branch id 0 only.

### Client Header (This Application) ###

```
[tag][appName][coName]

tag     - 4bytes   - 0x4D, 0x52, 0x43, 0x49 (MRCI)
appName - 134bytes - UTF16LE string (padded with 0x00)
coName  - 272bytes - UTF16LE string (padded with 0x00)
```

notes:

* The **tag** is just a fixed ascii string "MRCI" that indicates to the host that the client is indeed attempting to use the MRCI protocol.

* The **appName** is the name of the client application that is connected to the host. It can also contain the client's app version if needed because it doesn't follow any particular standard.

* The **coName** is the common name of a SSL certificate that is currently installed in the host. Depending on how the host is configured, it can contain more than one installed SSL cert so coName can be used by clients as a way to request which one of the SSL certs to use during the SSL handshake.

### Host Header ###

```
Format:

[reply][major][minor][patch][sesId]

reply - 1byte   - 8bit little endian unsigned int
major - 2bytes  - 16bit little endian unsigned int
minor - 2bytes  - 16bit little endian unsigned int
patch - 2bytes  - 16bit little endian unsigned int
sesId - 28bytes - 224bit sha3 hash
```

notes:

* **reply** is a numeric value that the host returns in it's header to communicate to the client the result of it's evaluation of the client's header.

    * reply = 1, means the client is acceptable and it does not need to take any further action.
    * reply = 2, means the client is acceptable but the host will now send it's Pem formatted SSL cert data in a ```HOST_CERT``` mrci frame just after sending it's header. After receiving the cert, the client will then need to send a STARTTLS signal using this cert.
    * reply = 4, means the host was unable to find the SSL cert associated with the common name sent by the client. The session will auto close at this point.

* **major**, **minor**, **path** these 3 numeric values are the host version number that also use a 3 number versioning system. This can be used by the client to setup backwards compatibility or determine of if supports the host at all. If not supported, the client can simply disconnect form the host.

* **sesId** is the session id. It is a unique 224bit sha3 hash that can be used by the host and client to uniquely identify the current session or past sessions.

### Async Commands ###

Async commands are 'virtual commands' that this application can encounter at any time while connected to the host. More information about this can be found in the [Async.md](Async.md) document. This application does act on some of the data carried by the async commands but not all of them.

### Development Setup ###

Linux Required Packages:
```
qtbase5-dev
libssl-dev
gcc
make
makeself
```

### Build From Source (Linux) ###

Linux_build.sh is a custom script designed to build this project from the source code using qmake, make and makeself. You can pass 2 optional arguments:

1. The path to the QT bin folder in case you want to compile with a QT install not defined in PATH.
2. Path of the output makeself file (usually has a .run extension). If not given, the outfile will be named cmdr-1.0.0.run in the source code folder.

Build:
```
cd /path/to/source/code
sh ./linux_build.sh
```
Install:
```
chmod +x ./cmdr-1.0.0.run
./cmdr-1.0.0.run
```
