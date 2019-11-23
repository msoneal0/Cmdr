#ifndef LONG_TXT_H
#define LONG_TXT_H

#define TXT_SaveBookmark "\
create or update a bookmark given using the -name argument. you can pass the optional -force \
argument to overwrite an existing bookmark without a confirmation question. you can also pass \
-addr and/or -port to set the bookmark parameters; if any of these are not passed, it is \
backfilled with the currently set address/port."

#define TXT_ListBookmarks "\
this list all available bookmarks that can be used with the -load option when connecting to a \
host."

#define TXT_DeleteBookmark "\
permanently delete the bookmark given in -name."

#define TXT_SeeBookmark "\
view all of the parameters currently set in a bookmark given by the -name argument."

#define TXT_Connect "\
connect to the host passed by -addr and -port. example: connect -addr 127.0.0.1 -port 3548. you can \
also pass -save to save this host address/port to a bookmark for later use with the -load so you \
don't have to re-enter this information everytime. note: the -save option overwrites the \
bookmark if it exists."

#define TXT_Quit "\
exit the client and end the current session if connected to a host."

#define TXT_Help "\
list all available commands in this interface. if connected to a host, this will list all host \
commands as well. pass the optional cmd_name parameter to see more detailed information about the \
command."

#define TXT_About "\
display application usage information, along with it's version number and supported MRCI protocol \
version. you can pass the cmd_name of another command to display more information specific to it. \
example: about connect"

#define TXT_SeeParams "\
view the current session's parameters and connection status information (as seen by this \
application)."

#define TXT_Term "\
this will terminate all actions currently being performed by the host command."

#define TXT_Halt "\
this will halt all actions currently being performed by the host command. this will not \
terminate the command; just stop what it is currently doing until told to resume."

#define TXT_Resume "\
this will resume the actions the previously halted host command is currently doing."

#define TXT_EndSession "\
this diconnects the client from the currently connected host. this will inturn end the session \
at the host side. warning: this will inturpt any running command at the host."

#define TXT_SetColors "\
this sets the colors that this application use when displaying text/error output as well as the \
background color. -text is for regular text output, -error for error output and the -bg for the \
background. each of these arguments take an RGB color code that must begin with a #. pass an empty \
parameter on any of these arguments to reset it to the default. example: -text or -text ''"

#define TXT_SetFont "\
set the font family for all text displayed by this application given in -name and font size given in \
-size. it's highly recommended to use monospaced font. don't forget to add quotes if passing a font \
family name that contain spaces. pass an empty parameter on any of these arguments to reset it to the \
default. example: -name or -name ''"

#define TXT_SetMaxLines "\
set or view the maximum amount of lines of text that can be displayed in this application. pass -value to \
to set this limit or leave it out to display what the limit is currently set at. this has a minimum value \
of 50 and a maxmum of 100000."

#endif // LONG_TXT_H
