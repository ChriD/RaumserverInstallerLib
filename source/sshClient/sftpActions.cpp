
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
        }


        SFTPActions::~SFTPActions()
        {
        }


        void SFTPActions::cancelActions()
        {
            // TODO: cancel all actions
            //
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