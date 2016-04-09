
#include "sshClient/sftpActions.h"


namespace RaumserverInstaller
{
    namespace SSHClient
    {
       
        /*

        // TODO : @@@
        //rc = sftp_mkdir(sftp, "helloworld", S_IRWXU);
        rc = sftp_mkdir(sftp, "helloworld", 0777);
        if (rc != SSH_OK)
        {
            if (sftp_get_error(sftp) != SSH_FX_FILE_ALREADY_EXISTS)
            {
                fprintf(stderr, "Can't create directory: %s\n",
                    ssh_get_error(my_ssh_session));
                return;
            }
        }

        sftp_free(sftp);
        */


        /*
        ifstream fin("file.doc", ios::binary);
            if (fin) {
              fin.seekg(0, ios::end);
              ios::pos_type bufsize = fin.tellg(); // get file size in bytes
              fin.seekg(0); // rewind to beginning of file

              char* buf = new char[bufsize];
              fin.read(buf, bufsize); // read file contents into buffer

              sftp_write(file, buf, bufsize); // write to remote file
            }
        */



        SFTPActions::SFTPActions()
        {  
            stopThreads = false;
        }


        SFTPActions::~SFTPActions()
        {
            cancelActions();
        }


        void SFTPActions::cancelActions()
        {
            stopThreads = true;
            if (copyDirThreadObject.joinable())
            {                
                copyDirThreadObject.join();
            }
            stopThreads = false;
        }


        void SFTPActions::copyDir(std::string _clientDir, std::string _remoteDir, bool _sync)
        {
            // start a thread which will handle the copying of the directory to the remote computer
            // there are signals for file copying
            copyDirThreadObject = std::thread(&SFTPActions::copyDirThread, this, _clientDir, _remoteDir);
            if (_sync)
                copyDirThreadObject.join();
        }


        void SFTPActions::copyDirThread(std::string _clientDir, std::string _remoteDir)
        {
            bool allFilesProcessed = false;
            
            // get all files we have to copy 
            TinyDirCpp::TinyDirCpp  tinyDirCpp;

            // create remote directory
            auto filesToCopy = tinyDirCpp.getFiles(_clientDir);

            /*
            tinydir_dir dir;
            if (tinydir_open(&dir, _clientDir.c_str()) == -1)
            {
                // oje.....
            }

            while (dir.has_next)
            {
                tinydir_file file;
                if (tinydir_readfile(&dir, &file) == -1)
                {
                   // ujee
                }

                printf("%s", file.name);
                if (file.is_dir)
                {
                    // 
                    printf("/");
                }
                printf("\n");

                tinydir_next(&dir);
            }


            tinydir_close(&dir);
            */

            // run through files directory and start copying files one by one

            


            //while (!stopThreads && !allFilesProcessed)
            {
                // TODO:
                
            }
        }


        bool SFTPActions::copyFile(std::string _clientFile, std::string _remoteFile)
        {
            // this is a sync method which will copy a file to a remote destination with a specific buffer size
            return false;
        }


        bool SFTPActions::sessionsExists()
        {
            if (sshSession == nullptr || sftpSession == nullptr)
                return false;
            return true;
        }


        void SFTPActions::setSessions(ssh_session _sshSession, sftp_session _sftpSession)
        {
            sshSession = _sshSession;
            sftpSession = _sftpSession;
        }


    }
}