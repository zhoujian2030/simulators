/*
 * File.h
 *
 *  Created on: May 18, 2017
 *      Author: j.zhou
 */

#ifndef FILE_H
#define FILE_H

#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

namespace cm {

    typedef enum {
        FILE_SUCC,
        FILE_ERR,
        FILE_WAIT
    } SocketErrCode;

    typedef enum
    {
        FILE_OPEN,			// Open, fail if does not exist.
        FILE_CREATE,		// Create, overwrite existing.
        FILE_OPEN_CREATE  	// Attempt to open, if does not exist create it.
    } OpenMode;

    // Seek modes
    typedef enum
    {
        F_SEEK_CURRENT = -2,
        F_SEEK_END     = -1,
        F_SEEK_BEGIN   = 0
    } FileSeek;

    typedef enum
    {
        FILE_READ_ONLY,
        FILE_WRITE_ONLY,
        FILE_READ_WRITE,
        FILE_READ_WRITE_APPEND
    } AccessType;

    typedef enum
    {
        OWNER_READ                  = S_IRUSR,
        OWNER_READ_WRITE            = S_IRUSR | S_IWUSR,
        OWNER_EXECUTE               = S_IXUSR,
        GROUP_READ                  = S_IRGRP,
        GROUP_READ_WRITE            = S_IRGRP | S_IWGRP,
        GROUP_EXECUTE               = S_IXGRP,
        OWNER_AND_GROUP_READ_WRITE  = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP,
        OTHER_READ                  = S_IROTH,
        OTHER_READ_WRITE            = S_IROTH | S_IWOTH,
        OTHER_EXECUTE               = S_IXOTH,
        ALL_READ                    = S_IRUSR | S_IRGRP | S_IROTH,
        ALL_READ_WRITE              = S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP | S_IWOTH,
        ALL_EXECUTE                 = S_IXUSR | S_IXGRP | S_IXOTH
    } FileMode;

    class File {
    public:
        // Open a file or create a file if not exists
        //  filename: the file name to be opened/created
        //  mode: FILE_OPEN - open, fail if does not exist.
        //        FILE_CREATE - Create, overwrite existing.
        //        FILE_OPEN_CREATE - Attempt to open, if does not exist create it.
        //  accessType: FILE_READ_ONLY - open the file read only, used with FILE_OPEN
        //              FILE_WRITE_ONLY - create/open the file write only
        //              FILE_READ_WRITE - open/create file, read and write
        //              FILE_READ_WRITE_APPEND - open/create the file and write from the end, NOT used with FILE_CREATE
        //
        // Exception:
        //  IoException - if fail to create/open file
        File(const std::string filename, OpenMode mode, AccessType accessType);

        // Open a file or create a file if not exists
        //  filename: the file name to be opened/created
        //  mode: FILE_OPEN - open, fail if does not exist.
        //        FILE_CREATE - Create, overwrite existing.
        //        FILE_OPEN_CREATE - Attempt to open, if does not exist create it.
        //  accessType: FILE_READ_ONLY - open the file read only, used with FILE_OPEN
        //              FILE_WRITE_ONLY - create/open the file write only
        //              FILE_READ_WRITE - open/create file, read and write
        //              FILE_READ_WRITE_APPEND - open/create the file and write from the end, NOT used with FILE_CREATE
        //
        // Exception:
        //  IoException - if fail to create/open file
        //  invalid_argument - if filePath is null pointer
        File(const char* filename, OpenMode mode, AccessType accessType);

        virtual ~File();

        // check if the file is opened success
        //
        // return true if opened success
        bool checkState();

        virtual int write(const char* theBuffer, int numOfBytesToWrite, int& numberOfBytesWritten);
        virtual int read(char* theBuffer, int buffSize, int& numOfBytesRead);
        virtual long seek(long thePos);
        virtual void close();

    protected:
        void open();

        int m_fd;
        std::string m_filename;
        OpenMode m_mode;
        AccessType m_accessType;
    };

}

#endif

