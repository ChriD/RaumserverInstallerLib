
#include <raumserverInstaller.h>
#include <rlutil/rlutil.h>
#include <libssh/libssh.h> 
#include <libssh/sftp.h>

#ifdef _WIN32
#define strncasecmp _strnicmp
#endif

int verify_knownhost(ssh_session session){
    char *hexa;
    int state;
    char buf[10];
    unsigned char *hash = NULL;
    int hlen;

    state = ssh_is_server_known(session);

    hlen = ssh_get_pubkey_hash(session, &hash);
    if (hlen < 0) {
        return -1;
    }
    switch (state){
    case SSH_SERVER_KNOWN_OK:
        break; /* ok */
    case SSH_SERVER_KNOWN_CHANGED:
        fprintf(stderr, "Host key for server changed : server's one is now :\n");
        ssh_print_hexa("Public key hash", hash, hlen);
        free(hash);
        fprintf(stderr, "For security reason, connection will be stopped\n");
        return -1;
    case SSH_SERVER_FOUND_OTHER:
        fprintf(stderr, "The host key for this server was not found but an other type of key exists.\n");
        fprintf(stderr, "An attacker might change the default server key to confuse your client"
            "into thinking the key does not exist\n"
            "We advise you to rerun the client with -d or -r for more safety.\n");
        return -1;
    case SSH_SERVER_FILE_NOT_FOUND:
        fprintf(stderr, "Could not find known host file. If you accept the host key here,\n");
        fprintf(stderr, "the file will be automatically created.\n");
        /* fallback to SSH_SERVER_NOT_KNOWN behavior */
    case SSH_SERVER_NOT_KNOWN:        
        hexa = ssh_get_hexa(hash, hlen);
        fprintf(stderr, "The server is unknown. Do you trust the host key ?\n");
        fprintf(stderr, "Public key hash: %s\n", hexa);
        //free(hexa);
        //if (fgets(buf, sizeof(buf), stdin) == NULL) {
        //    return -1;
        //}
        //if (strncasecmp(buf, "yes", 3) != 0){
        //    return -1;
        //}
        //fprintf(stderr, "This new key will be written on disk for further usage. do you agree ?\n");
        //if (fgets(buf, sizeof(buf), stdin) == NULL) {
        //    return -1;
        //}
        //if (strncasecmp(buf, "yes", 3) == 0){
        //    if (ssh_write_knownhost(session) < 0) {
        //        free(hash);
        //        fprintf(stderr, "error %s\n", strerror(errno));
        //        return -1;
        //    }
        //}
        ssh_write_knownhost(session);
        
        // its okay....

        break;
    case SSH_SERVER_ERROR:
        free(hash);
        fprintf(stderr, "%s", ssh_get_error(session));
        return -1;
    }
    //free(hash);
    return 0;
}

int authenticate_kbdint(ssh_session session, const char *password) {
    int err;

    err = ssh_userauth_kbdint(session, NULL, NULL);
    while (err == SSH_AUTH_INFO) {
        const char *instruction;
        const char *name;
        char buffer[128];
        int i, n;

        name = ssh_userauth_kbdint_getname(session);
        instruction = ssh_userauth_kbdint_getinstruction(session);
        n = ssh_userauth_kbdint_getnprompts(session);

        if (name && strlen(name) > 0) {
            printf("%s\n", name);
        }

        if (instruction && strlen(instruction) > 0) {
            printf("%s\n", instruction);
        }

        for (i = 0; i < n; i++) {
            const char *answer;
            const char *prompt;
            char echo;

            prompt = ssh_userauth_kbdint_getprompt(session, i, &echo);
            if (prompt == NULL) {
                break;
            }

            if (echo) {
                char *p;

                printf("%s", prompt);

                if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
                    return SSH_AUTH_ERROR;
                }

                buffer[sizeof(buffer)-1] = '\0';
                if ((p = strchr(buffer, '\n'))) {
                    *p = '\0';
                }

                if (ssh_userauth_kbdint_setanswer(session, i, buffer) < 0) {
                    return SSH_AUTH_ERROR;
                }

                memset(buffer, 0, strlen(buffer));
            }
            else {
                if (password && strstr(prompt, "Password:")) {
                    answer = password;
                }
                else {
                    buffer[0] = '\0';

                    if (ssh_getpass(prompt, buffer, sizeof(buffer), 0, 0) < 0) {
                        return SSH_AUTH_ERROR;
                    }
                    answer = buffer;
                }
                if (ssh_userauth_kbdint_setanswer(session, i, answer) < 0) {
                    return SSH_AUTH_ERROR;
                }
            }
        }
        err = ssh_userauth_kbdint(session, NULL, NULL);
    }

    return err;
}

static void error(ssh_session session){
    fprintf(stderr, "Authentication failed: %s\n", ssh_get_error(session));
}

int authenticate_console(ssh_session session){
    int rc;
    int method;
    char password[128] = { 0 };
    char *banner;

    // Try to authenticate
    rc = ssh_userauth_none(session, NULL);
    if (rc == SSH_AUTH_ERROR) {
        error(session);
        return rc;
    }

    method = ssh_auth_list(session);
    while (rc != SSH_AUTH_SUCCESS) {
        // Try to authenticate with public key first
        if (method & SSH_AUTH_METHOD_PUBLICKEY) {
            rc = ssh_userauth_autopubkey(session, NULL);
            if (rc == SSH_AUTH_ERROR) {
                error(session);
                return rc;
            }
            else if (rc == SSH_AUTH_SUCCESS) {
                break;
            }
        }

        // Try to authenticate with keyboard interactive";
        /*
        if (method & SSH_AUTH_METHOD_INTERACTIVE) {
            rc = authenticate_kbdint(session, NULL);
            if (rc == SSH_AUTH_ERROR) {
                error(session);
                return rc;
            }
            else if (rc == SSH_AUTH_SUCCESS) {
                break;
            }
        }

        if (ssh_getpass("Password: ", password, sizeof(password), 0, 0) < 0) {
            return SSH_AUTH_ERROR;
        }*/

        // Try to authenticate with password        
        if (method & SSH_AUTH_METHOD_PASSWORD) {
            rc = ssh_userauth_password(session, NULL, password);
            if (rc == SSH_AUTH_ERROR) {
                error(session);
                return rc;
            }
            else if (rc == SSH_AUTH_SUCCESS) {
                break;
            }
        }
    }

    banner = ssh_get_issue_banner(session);
    if (banner) {
        printf("%s\n", banner);
        //free(banner);
    }

    return rc;
}


ssh_session connect_ssh(const char *host, const char *user, int verbosity){
    ssh_session session;
    int auth = 0;

    session = ssh_new();
    if (session == NULL) {
        return NULL;
    }

    if (user != NULL){
        if (ssh_options_set(session, SSH_OPTIONS_USER, user) < 0) {
            ssh_disconnect(session);
            return NULL;
        }
    }

    if (ssh_options_set(session, SSH_OPTIONS_HOST, host) < 0) {
        return NULL;
    }
    ssh_options_set(session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
    if (ssh_connect(session)){
        fprintf(stderr, "Connection failed : %s\n", ssh_get_error(session));
        ssh_disconnect(session);
        return NULL;
    }
    if (verify_knownhost(session)<0){
        ssh_disconnect(session);
        return NULL;
    }
    auth = authenticate_console(session);
    if (auth == SSH_AUTH_SUCCESS){
        return session;
    }
    else if (auth == SSH_AUTH_DENIED){
        fprintf(stderr, "Authentication failed\n");
    }
    else {
        fprintf(stderr, "Error while authenticating : %s\n", ssh_get_error(session));
    }
    ssh_disconnect(session);
    return NULL;
}


int main()
{
    RaumserverInstaller::RaumserverInstaller    *raumServerInstaller;
    std::string result;

    
    /*

    // TODO: @@@ try ssh device reboot
    ssh_session session;
    ssh_channel channel;
    char buffer[256];
    int nbytes;
    int rc;

    session = connect_ssh("10.0.0.2", "root", 0);
    channel = ssh_channel_new(session);;
    rc = ssh_channel_open_session(channel);

    rc = ssh_channel_request_exec(channel, "reboot");

    nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
    while (nbytes > 0) {
        fwrite(buffer, 1, nbytes, stdout);       
        nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
    }

    ssh_channel_send_eof(channel);
    ssh_channel_close(channel);
    ssh_channel_free(channel);
    ssh_free(session);




    RaumserverInstaller::SSHClient::SSHClient client;

    client.setOption(ssh_options_e::SSH_OPTIONS_HOST, "10.0.0.2");
    client.setAuth("root", "");
    client.connectSSH();
    client.executeCommand("reboot -f", result);
    client.closeSSH();
    */
    raumServerInstaller = new RaumserverInstaller::RaumserverInstaller();

    raumServerInstaller->init();
    raumServerInstaller->initLogObject(Log::LogType::LOGTYPE_INFO);
    raumServerInstaller->initDiscover();

       
    auto adapterList = raumServerInstaller->getNetworkAdapterList();
    for (auto item : adapterList)
    {
        std::cout << item.name + " - " + item.fullName;
        std::cout << std::endl;
    }    

    std::int16_t    idx;
    std::cout << "Adapter IDX: ";
    std::cin >> idx;

    raumServerInstaller->setNetworkAdapter(adapterList[idx]);
    raumServerInstaller->startDiscoverDevicesForInstall();    

    rlutil::getkey();

    RaumserverInstaller::DeviceInformation deviceInfo;

    deviceInfo.name = "Dummmy";
    deviceInfo.ip = "10.0.0.1";
    deviceInfo.type = RaumserverInstaller::DeviceType::DT_UPNPDEVICE_RAUMFELD;

    //raumServerInstaller->startInstallToDevice(deviceInfo);

    //rlutil::getkey();

    deviceInfo.ip = "10.0.0.201";
    deviceInfo.hardwareType = "3";
    raumServerInstaller->startInstallToDevice(deviceInfo);

    //deviceInfo.ip = "10.0.0.201";
    //deviceInfo.hardwareType = "3";
    //raumServerInstaller->startInstallToDevice(deviceInfo);

    rlutil::getkey();

    //delete raumServerInstaller;
    //raumServerInstaller = new RaumserverInstaller::RaumserverInstaller();/
    //raumServerInstaller->startRemoveFromDevice(deviceInfo);
   // rlutil::getkey();

    delete raumServerInstaller;

    return 0;
}





