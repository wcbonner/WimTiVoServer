// C:\Documents and Settings\Wim\My Documents\ss\EPCPalletReader\res>"C:\Program Files (x86)\Windows Kits\8.0\bin\x86\mc.EXE" C:\Users\Wim\Documents\SS\WimTiVoServer\WimTiVoServerEventLog.mc
//
//  Values are 32 bit values laid out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//


//
// Define the severity codes
//


//
// MessageId: CAT_ONE
//
// MessageText:
//
// Category #1
//
#define CAT_ONE                          ((WORD)0x00000001L)

//
// MessageId: CAT_TWO
//
// MessageText:
//
// Category #2
//
#define CAT_TWO                          ((WORD)0x00000002L)

//
// MessageId: WIMSWORLD_EVENT_GENERIC
//
// MessageText:
//
// %1
//
#define WIMSWORLD_EVENT_GENERIC          ((DWORD)0x40000100L)

//
// MessageId: WIMSWORLD_EVENT_INSTALL
//
// MessageText:
//
// Application %1 has been installed
//
#define WIMSWORLD_EVENT_INSTALL          ((DWORD)0x40000101L)

//
// MessageId: WIMSWORLD_EVENT_REMOVE
//
// MessageText:
//
// Application %1 has been removed
//
#define WIMSWORLD_EVENT_REMOVE           ((DWORD)0x40000102L)

//
// MessageId: WIMSWORLD_EVENT_START
//
// MessageText:
//
// Application %1 has been started
//
#define WIMSWORLD_EVENT_START            ((DWORD)0x40000103L)

//
// MessageId: WIMSWORLD_EVENT_APPLICATION_HELP
//
// MessageText:
//
// Application %1 is helping %2
//
#define WIMSWORLD_EVENT_APPLICATION_HELP ((DWORD)0x40000104L)

//
// MessageId: WIMSWORLD_EVENT_SERVICE_HELP
//
// MessageText:
//
// Service %1 is helping %2
//
#define WIMSWORLD_EVENT_SERVICE_HELP     ((DWORD)0x40000105L)

//
// MessageId: MSG_TWO
//
// MessageText:
//
// Application %1 is about to exit.
// (The Event data is the log handle which is about to be closed.)
//
#define MSG_TWO                          ((DWORD)0x80000106L)

