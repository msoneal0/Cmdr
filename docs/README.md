# Cmdr #

Cmdr is a command line terminal emulator client for MRCI host using text input/output. This helps administer MRCI host via local or remote TCP connections encrypted with TLS/SSL using the MRCI protocol. It also supports file transfers to/from the client using the GEN_FILE sub-protocol that MRCI understands.

### Usage ###

Cmdr has it's own user interface and terminal display so there is no command line switches to pass on start up. Instead, all commands are parsed and processed within it's own terminal interface. Any commands not seen as an internal command for the client itself is passed through to the MRCI host if connected. Users have the option to start the command line with a '#' to bypass any of the client's internal commands in cases when there is overlap with a host command. 

### Versioning System ###

This application uses the typical 3 number versioning system: [major].[minor].[patch]

* Major - this indicates any major changes to the code of the application that renders versions of different majors incompatible with each other.
* Minor - this indicates only minor changes to the code that may require a few conditional blocks to maintain compatibility.
* Patch - this indicates changes that won't require any behaviour changes at all to maintain compatibility.

Any increments to major resets minor and patch to 0. The MRCI host/protocol also uses the 3 number version system seperate from this client's own version number.

### The Protocol ###

This application being a MRCI client uses the MRCI protocol to transport data to and from the host using TCP in a binary data format called MRCI frames. In general, local connections with the host are not encrypted but all connects outside of the host must be encrypted using TLS/SSL.

Before any MRCI frames can be transported, the host need to be made aware of the version of the host that this application supports and this application also needs to be made aware of the host current version. This is done by sending a fixed length header containing information about this application to the host when it successfully connects. the host will then reply with it's own fixed length host header.

### MRCI Frame Format ###

```
Format:

[type_id][cmd_id][data_len][payload]

type_id  - 1byte    - 8bit little endian integer type id of the payload.
cmd_id   - 2bytes   - 16bit little endian integer command object id.
data_len - 3bytes   - 24bit little endian integer size of the payload.
payload  - variable - the actual data to be processed.
```

A full description of the type id's can be found in the [Type_IDs.md](Type_IDs.md) document.

### Client Header (This Application) ###

```
Format: 

[tag][major][minor][patch][appName][coName]

tag     - 4bytes   - 0x4D, 0x52, 0x43, 0x49 (MRCI)
major   - 2bytes   - 16bit little endian unsigned int
minor   - 2bytes   - 16bit little endian unsigned int
patch   - 2bytes   - 16bit little endian unsigned int
appName - 128bytes - UTF16LE string (padded with spaces)
coName  - 272bytes - UTF16LE string (padded with spaces)
```

notes:

* The **tag** is just a fixed ascii string "MRCI" that indicates to the host that the client is indeed attempting to use the MRCI protocol.

* **major**, **minor**, **patch** is the version number of the MRCI protocol that the client understands. Do not send the client's own version number.

* The **appName** is the name of the client application that is connected to the host. It can also contain the client's app version if needed because it doesn't follow any particular standard.

* The **coName** is the common name of a SSL certificate that is currently installed in the host. Depending on how the host is configured, it can contain more than one installed SSL cert so coName can be used by clients as a way to request which one of the certs to use during the SSL handshake.

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

    * reply = 1, means the client version is acceptable and it does not need to take any further action.
    * reply = 2, means the client version is acceptable but the host will now send it's Pem formatted SSL cert data in a ```HOST_CERT``` mrci frame just after sending it's header. After receiving the cert, the client will then need to send a STARTTLS signal using this cert.
    * reply = 3, means the client version is not supported by the host and the session will end shortly.
    * reply = 4, means the host was unable to load the SSL cert associated with the common name sent by the client. The session will auto close at this point.

* **sesId** is the session id. It is a unique 224bit sha3 hash generated against the current date of session creation (down to the msec) and the machine id. This can be used to uniquely identify the current session and past sessions.

### ASync Commands ###

Async commands are host only 'virtual commands' that this application can encounter at any time while connected to the host. More information about this can be found in the [Async.md](Async.md) document. This application does act on some of the data carried by the async commands but not all of them.

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
