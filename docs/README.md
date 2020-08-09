# Cmdr #

Cmdr is a command line terminal emulator client for MRCI host using text input/output. This help administer MRCI host via local or remote TCP connections encrypted with TLS/SSL using the MRCI protocol. It also supports file transfers to/from the client using the GEN_FILE sub-protocol that MRCI understands.

### Usage ###

Cmdr have it's own terminal display so there is no command line switches to pass on start up. Instead, all commands are parsed and processed within it's own terminal interface. Any commands not seen as an internal command for the client itself is passed through to the MRCI host if connected. The client will add a numeric identifer to the end of all host command names that clash with the client commands so there is no chance a client and host command can conflict with each other.  

### Versioning System ###

This application uses a 2 number versioning system: [major].[minor]

* Major - this indicates any changes that cause old configuration or library files to be incompatible.
* Minor - this indicates changes to the code that still maintains compatibility with existing config files or libraries.

Any increments to major resets minor to 0.

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
[tag][appName][mod_instructions][padding]

tag     - 4bytes   - 0x4D, 0x52, 0x43, 0x49 (MRCI)
appName - 32bytes  - UTF8 string (padded with 0x00)
modInst - 128bytes - UTF8 string (padded with 0x00)
padding - 128bytes - string of (0x00)
```

notes:

* **tag** is just a fixed ascii string "MRCI" that indicates to the host that the client is indeed attempting to use the MRCI protocol.

* **appName** is the name of the client application that is connected to the host. It can also contain the client's app version if needed because it doesn't follow any particular standard. This string is accessable to all modules so the commands themselves can be made aware of what app the user is currently using.

* **modInst** is an additional set of command lines that can be passed onto to all module processes when they are intialized. This can be used by certain clients that want to intruct certain modules that might be installed in the host to do certain actions during intialization. This remains constant for as long as the session is active and cannot be changed at any point.

### Host Header ###

```
Format:

[reply][major][minor][tcp_rev][mod_rev][sesId]

reply   - 1byte   - 8bit little endian unsigned int
major   - 2bytes  - 16bit little endian unsigned int
minor   - 2bytes  - 16bit little endian unsigned int
tcp_rev - 2bytes  - 16bit little endian unsigned int
mod_rev - 2bytes  - 16bit little endian unsigned int
sesId   - 28bytes - 224bit sha3 hash
```

notes:

* **reply** is a numeric value that the host returns in it's header to communicate to the client if SSL need to initated or not.

    * reply = 1, means SSL is not required so the client doesn't need to take any further action.
    * reply = 2, means SSL is required to continue so the client needs to send a STARTLS signal.

* **sesId** is the session id. It is a unique 224bit sha3 hash generated against the current date and time of session creation (down to the msec) and the machine id of the host. This can be used by the host or client to uniquely identify the current session or past sessions.

### Async Commands ###

Async commands are 'virtual commands' that this application can encounter at any time while connected to the host. More information about this can be found in the [Async.md](Async.md) document. This application does act on some of the data carried by the async commands but not all of them.

### Build Setup ###

For Linux you need the following packages to successfully build/install:
```
qtbase5-dev
libssl-dev
gcc
make
python3
```

For Windows support you need to have the following applications installed:
```
OpenSSL
Qt5.12 or newer
Python3
```

### Build ###

To build this project from source you just need to run the build.py and then the install.py python scripts. While running the build the script, it will try to find the Qt API installed in your machine according to the PATH env variable. If not found, it will ask you to input where it can find the Qt bin folder where the qmake executable exists or you can bypass all of this by passing the -qt_dir option on it's command line.

while running the install script, it will ask you to input 1 of 3 options:

***local machine*** - This option will install the built application onto the local machine without creating an installer.

***create installer*** - This option creates an installer that can be distributed to other machines to installation. The resulting installer is just a regular .py script file that the target machine can run if it has Python3 insalled. Only Python3 needs to be installed and an internet connection is not required.

***exit*** - Cancel the installation.

-local or -installer can be passed as command line options for install.py to explicitly select one of the above options without pausing for user input.