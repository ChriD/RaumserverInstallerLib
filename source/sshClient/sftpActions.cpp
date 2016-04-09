
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



        SFTPActions::SFTPActions()
        {           
        }


        SFTPActions::~SFTPActions()
        {
        }


        void SFTPActions::setSessions(ssh_session _sshSession, sftp_session _sftpSession)
        {
            sshSession = _sshSession;
            sftpSession = _sftpSession;
        }


    }
}