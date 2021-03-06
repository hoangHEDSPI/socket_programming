#include "client.h"

int recv_file(int ,char*, char*);
char *recv_msg(int conn_sock);
int send_msg(int conn_sock, char *message, int msg_len);
int send_eof_msg(int conn_sock);
void enter_path_file();
int create_folder(char* folder_name);

int recv_file(int sock, char* file_name, char* newfilename) {
	char send_str [MAX_SEND_BUF]; 
	int f; 
	ssize_t sent_bytes, rcvd_bytes, rcvd_file_size;
	int recv_count; 
	char recv_str[MAX_RECV_BUF]; 
    char error[MAX_RECV_BUF] = "File not found\n";
	size_t send_strlen; 
	sprintf(send_str, "%s\n", file_name); 
	send_strlen = strlen(send_str); 
	if( (sent_bytes = send_msg(sock, file_name, send_strlen)) < 0 ) {
		perror("send error");
		return -1;
	}
	if ( (f = open(newfilename, O_WRONLY|O_CREAT, 0644)) < 0 ) {
		perror("error creating file");
		return -1;
	}
    
	recv_count = 0; 
	rcvd_file_size = 0; 

	while ( (rcvd_bytes = recv(sock, recv_str, MAX_RECV_BUF, 0)) > 0 ) {
        if (strcmp(recv_str, error) == 0) {
            printf("File not found in server\n");
            break;
        }
        recv_count++;
		rcvd_file_size += rcvd_bytes;
		if (write(f, recv_str, rcvd_bytes) < 0 ) {
			perror("error writing to file");
			return -1;
		}
        if(rcvd_bytes < 256) break;

	}
	close(f); /* close file*/
	printf("Client Received: %d bytes in %d recv(s)\n", rcvd_file_size, recv_count);
	return rcvd_file_size;
}

int delete_file(int conn_sock, char* wanna_delete_filename) {
    return 0;
}

int create_folder(char* folder_name) {
    return mkdir(folder_name, 0755);
}

void create_sub_client_folder(char* sub_folder_name) {
    DIR *dir = opendir(sub_folder_name);
	if (dir) {
		printf("Existed folder\n");
		closedir(dir);
	}
	else if (ENOENT == errno) {
    /* Directory does not exist. */
		create_folder(sub_folder_name);
	}
	else {
    /* opendir() failed for some other reason. */
		printf("Something was wrong\n");
	}
}

int copyfile(char* infilename, char* outfileDir) {
    // FILE* infile; //File handles for source and destination.
    // FILE* outfile;
    // char outfilename[1000];

    // infile = fopen(infilename, "r"); // Open the input and output files.
    // if (infile == NULL) {
    //   printf("%s not found\n", infilename);
    //   return 1;
    // }
    // sprintf(outfilename, "%s/%s", outfileDir, basename(infilename));

    // outfile = fopen(outfilename, "w");
    char* copy_command  = malloc(sizeof(char)*BUFF_SIZE);
    strcat(copy_command, "cp ");
    strcat(copy_command, " ");
    strcat(copy_command, infilename);
    strcat(copy_command, " ");
    strcat(copy_command, outfileDir);
    system(copy_command);
}

void delete_file_on_client_side(char* filename) {
	int f;
	char * errmsg_notfound = "File not found\n";

	if ((f=open(filename, O_RDONLY)) < 0) {
		perror(filename);
	}
	else {
		remove(filename);
	}
}



char *recv_msg(int conn_sock){
	int ret, nLeft, msg_len, index = 0;
	char recv_data[WINDOW_SIZE], *data;
	// receive the length of message
	int bytes_received = recv(conn_sock, &msg_len, sizeof(int), MSG_WAITALL);
	if (bytes_received <= 0){
		return NULL;
	}
	nLeft = msg_len;
	data = (char*) malloc(msg_len);
	memset(data, 0, msg_len);
	index = 0;
	
	//receives message from server
	while(nLeft > 0){
		ret = recv(conn_sock, recv_data, WINDOW_SIZE, 0);
		if(ret == -1){
			break;
		}
		memcpy(data + index, recv_data, ret);
		index += ret;
		nLeft -= ret;
	}
	data[msg_len] = '\0';
	return data;
}

int send_msg(int conn_sock, char *message, int msg_len){
	int bytes_sent;
	//send the length of the message to server
	bytes_sent = send(conn_sock, &msg_len, sizeof(int), 0);
	if(bytes_sent <= 0){
		return -1;
	}

	// send the message to server
	bytes_sent = send(conn_sock, message, msg_len, 0);
	if (bytes_sent <= 0){
		return -1;
	}
	return 0;
}

int send_eof_msg(int conn_sock){
	int msg_len = 0;
	int bytes_sent = send(conn_sock, &msg_len, sizeof(int), 0);
	if(bytes_sent <= 0){
		return -1;
	}
	return 0;
}

void enter_path_file() {
	printf("\nEnter path file :");
}

void set_null(char **ptr) {
    *ptr = NULL;
}

void cmdLOUT (char *str) {
	char code[CODE_LEN + 1];
	char pass[PASS_LEN + 1];

	sscanf(str, "%s", code);

	if (strcmp(code, "LOUT")) return;
	
	printf("Enter your password: ");
	__fpurge(stdin);
	scanf("%s", pass);

	strcat(str, " ");
	strncat(str, pass, PASS_LEN);
}

// Check exit
_Bool wannaExit (char *buff) {
	if (buff[0] == '\0' || buff[0] == '\n') return 1;
	else return 0;
}

// Check arguments is valid or not. If valid ip -> *serv_ip, port -> &serv_port
void validArguments (int argc, char *argv[], char *serv_ip, int *serv_port) {
	if (argc > 2) {
		// Check valid ip address
		struct sockaddr_in tmp_addr;
		if (inet_pton(AF_INET, argv[1], &(tmp_addr.sin_addr)) == 0) {
			printf("IP Address is invalid\n!");
			exit(EXIT_FAILURE);
		} else {
			strcpy(serv_ip, argv[1]);
		}

		// Check valid port
		int i;
		char *port_str = argv[2];
		for (i = 0; port_str[i] != '\0'; i++) {
			if (!isdigit(port_str[i])) {
				printf("Port is invalid\n!");
				exit(EXIT_FAILURE);
			}
		}
		if (port_str[i] == '\0') *serv_port = atoi(port_str);
	} else {
		printf("(ERROR) To few arguments!\n");
		exit(EXIT_FAILURE);
	}
}

int main(int argc, const char* argv[]) {

    /*Initialize*/
    char dest[BUFF_SIZE];
    int inner_loop = 0;
    char username[BUFF_SIZE];
    memset(username, '\0', sizeof(username));

    if (argc < 3) {
        printf("Invalid Arugment !\n");
        return 0;
    }
    strcpy(serv_ip_child, argv[1]);
    memset(&server_addr, 0, sizeof(server_addr)); 
    conn_sock = socket(AF_INET,SOCK_STREAM,0);
	
    server_addr.sin_family = AF_INET;
	serv_port = (in_port_t) strtol(argv[2], &endptr, 10);
    server_addr.sin_port = htons(serv_port);
	
	server_addr.sin_addr.s_addr = inet_addr(serv_ip_child);
	
	if(connect(conn_sock, (struct sockaddr*)&server_addr, sizeof(struct sockaddr)) < 0){
		printf("\nError!Can not connect to sever! Client exit imediately! ");
		return 0;
	}

    // Register - Login - Logout Menu
    int login_option, choice, upload_option, download_option;
    while (1) {
        printf("*************************************************************\n");
        printf("*************************Welcome*****************************\n");
        printf("*******                                          ************\n");
        printf("*******               1. REGISTER                 ***********\n");
        printf("*******               2. LOGIN                    ***********\n");
        printf("*******                                           ***********\n");
        printf("*************************************************************\n");
        printf("*************************************************************\n\n\n");
        printf("Please choose 1 if you want to create a new account\n");
        printf("Or choose 2 if you already have an account\n");
        printf("Enter your choice: ");
        scanf("%d", &login_option);
        __fpurge(stdin);
        switch(login_option) {
            case 1:
                system("clear");
                memset(dest, '\0', sizeof(dest));
                printf("\nOK. Please enter this bellow form to complete your register: \n");
                printf("                ************* RESG ***************");
                __fpurge(stdin);
                bzero(buff, BUFF_SIZE);
                strcpy(buff, "RESG ");
                printf("\nUsername : ");
                __fpurge(stdin);
                fgets(dest, BUFF_SIZE, stdin);
                dest[strlen(dest)-1] = '\0';
                strcpy(username, dest);
                strcat(buff, dest);
                strcat(buff, " ");
                printf("\nPassword : ");
                __fpurge(stdin);
                fgets(dest, BUFF_SIZE, stdin);

                strcat(buff, dest);
                printf("buff = %s", buff);
                buff[strlen(buff) - 1] = '\0';

                if (wannaExit(buff)) return;
                msg_len = strlen(buff) + 1;

                if( send_msg(conn_sock, buff, msg_len) == -1) {
                    printf("Connection closed!\n");
                    break;
                }
                
                bytes_received = recv(conn_sock, buff, BUFF_SIZE-1, 0);
                if(bytes_received <= 0){
                    printf("\nError!Cannot receive data from sever!\n");
                    break;
                }

                buff[bytes_received] = '\0';
                // printf("\nReply from server: %s\n", buff);
                break;
            case 2:
                system("clear");
                memset(dest, '\0', sizeof(dest));

                printf("OK. Please enter this bellow form to complete your login: \n");
                printf("             *******   LGIN   *******");
                __fpurge(stdin);
                bzero(buff, BUFF_SIZE);
                strcpy(buff, "LGIN ");
                printf("\nUsername : ");
                __fpurge(stdin);
                fgets(dest, BUFF_SIZE, stdin);
                dest[strlen(dest)-1] = '\0';
                strcpy(username, dest);
                strcat(buff, dest);
                strcat(buff, " ");
                printf("\nPassword : ");
                __fpurge(stdin);
                fgets(dest, BUFF_SIZE, stdin);

                strcat(buff, dest);
                printf("buff = %s", buff);
                //fgets(buff, BUFF_SIZE, stdin);
                buff[strlen(buff) - 1] = '\0';

                if (wannaExit(buff)) return;
                msg_len = strlen(buff) + 1;

                if( send_msg(conn_sock, buff, msg_len) == -1) {
                    printf("Connection closed!\n");
                    break;
                }
                
                bytes_received = recv(conn_sock, buff, BUFF_SIZE-1, 0);
                if(bytes_received <= 0){
                    printf("\nError!Cannot receive data from sever!\n");
                    break;
                }

                buff[bytes_received] = '\0';
                // printf("\nReply from server: %s\n", buff);
                printf("------------------------------------------------------------------------------\n");
                if (buff[0] == '1' && buff[1] == '0') {
                    system("clear");
                    printf("Welcome to our cloud\n\n\nPlease look at this bellow menu and select your choice\n\n");
                    do {
                        printf("\n\n");
                        printf("        ------------------------------------------------\n");
                        printf("        |1. Upload your files into cloud server        |\n");
                        printf("        |2. Download files from cloud server           |\n");
                        printf("        |3. Delete a file                              |\n");
                        printf("        |4. Create a new sub-folder                    |\n");
                        printf("        |5. Show other users                           |\n");
                        printf("        |6. Download files from other users            |\n");
                        printf("        |7. Upload files into sub-folder               |\n");
                        printf("        |8. Logout                                     |\n");
                        printf("        ------------------------------------------------\n");
                        printf("\n");
                        printf("Please take your choice: ");
                        scanf("%d", &choice);
                        __fpurge(stdin);
                        switch(choice) {
                            /*Upload option*/
                            case 1:
                                system("clear");
                                inner_loop = 0;
                                while(1){
                                    printf("\nEnter 1 if you want to upload single file\n");
                                    printf("Enter 2 if you want to choose a difference option\n");
                                    printf("And, what is your choice : \n");
                                    printf("            1 or 2          \n");
                                    printf("Enter here: ");
                                    scanf("%d", &upload_option);
                                    __fpurge(stdin);
                                    if (upload_option == 1) {
                                        bytes_transfered = 0;
                                        // input file link
                                        enter_path_file();
                                        memset(buff,'\0', BUFF_SIZE);
                                        fgets(buff, BUFF_SIZE, stdin);
                                        buff[strlen(buff) - 1] = '\0';
                                        if (strlen(buff) == 0) break;
                                        strcpy(filelink, buff);

                                        if((fp = fopen(filelink, "rb")) == NULL){
                                            printf("Error: File not found\n");
                                            continue;
                                        }else{
                                            
                                            strcpy(filename, rindex(filelink, '/') + 1);
                                            extended_filename = (char*) malloc(sizeof(char)*BUFF_SIZE);
                                            bzero(extended_filename,BUFF_SIZE);
                                            strcat(extended_filename, "1");
                                            strcat(extended_filename,filename);
                                            printf("extended filename: %s", extended_filename);
                                            if(send_msg(conn_sock, extended_filename, strlen(extended_filename)) == -1){
                                                printf("Hard!\n");
                                                continue;
                                            }
                                            printf("filename = %s\n", filename);
                                            fflush(stdin);
                                            data = recv_msg(conn_sock);
                                            printf("error number :%s\n", data);
                                            errnum = atoi(data);
                                            if(errnum == 1){		// if file is existent on server
                                                printf("Error: File is existent on server\n");
                                            }else if(errnum == 0){		// if there is no error
                                                while(errnum == 0){		// until there is an error, keep reading from file
                                                    memset(buff,'\0', BUFF_SIZE);
                                                    if(fread(buff, BUFF_SIZE, 1, fp) == 1){
                                                        if(send_msg(conn_sock, buff, sizeof(buff)) == -1){
                                                            break;
                                                        }
                                                        printf("Uploaded: %.2lf MB\n", (bytes_transfered += sizeof(buff)) / (1024*1024));
                                                    }else{
                                                        if(send_msg(conn_sock, buff, sizeof(buff)) == -1){
                                                            break;
                                                        }
                                                        printf("Uploaded: %.2lf MB\n", (bytes_transfered += sizeof(buff)) / (1024*1024));
                                                        
                                                        data = recv_msg(conn_sock);
                                                        errnum = atoi(data);

                                                        if(send_eof_msg(conn_sock) == -1){
                                                            break;
                                                        }
                                                    }
                                                    data = recv_msg(conn_sock);
                                                    errnum = atoi(data);
                                                }
                                                if(errnum == -1){
                                                    printf("\nSuccessful Uploading\n");
                                                    // syn here
                                                    char* saved_username = malloc(sizeof(char)*50);
                                                    strcpy(saved_username, username);
                                                    strcat(saved_username, "_client/");
                                                    copyfile(filename, saved_username);
                                                    printf("syn successfuly");

                                                }else{
                                                    printf("\nError: File uploading is interupted\n");
                                                }
                                            }
                                        }
                                        fclose(fp);
                                    
                                    }
                                    else if (upload_option == 2) {
                                    break;
                                    
                                }
                                
                                }
                                
                            break;
                            
                            /*Download option*/
                            case 2:
                                // system("clear");
                                inner_loop = 0;
                                download_filename = malloc(sizeof(char)*BUFF_SIZE);
                                new_download_filename = malloc(sizeof(char)*BUFF_SIZE);
                                printf("Enter filename you want to download : ");
                                scanf("%s", download_filename);
                                printf("Enter new filename you want to save : ");
                                scanf("%s", new_download_filename);
                                extended_filename = (char*) malloc(sizeof(char)*BUFF_SIZE);
                                bzero(extended_filename,BUFF_SIZE);
                                strcat(extended_filename, "2");
                                strcat(extended_filename,download_filename);
                                recv_file(conn_sock,extended_filename, new_download_filename);
                                break;
                            case 3:
                                system("clear");
                                inner_loop = 0;
                                wanna_delete_filename = malloc(sizeof(char)*BUFF_SIZE);
                                printf("Enter filename you want to delete : ");
                                scanf("%s", wanna_delete_filename);
                                extended_filename = (char*) malloc(sizeof(char)*BUFF_SIZE);
                                bzero(extended_filename,BUFF_SIZE);
                                strcat(extended_filename, "3");
                                strcat(extended_filename,wanna_delete_filename);
                                // recv_file(conn_sock,extended_filename, new_download_filename);
                                // TODO
                                if(send_msg(conn_sock, extended_filename, strlen(extended_filename)) == -1){
                                    printf("Hard!\n");
                                    continue;
                                }
                                data = recv_msg(conn_sock);
                                // printf("error number :%s\n", data);
                                errnum = atoi(data);
                                if (errnum == 1) {
                                    printf("Error : This file doesn not exist on server\n");
                                    break;
                                } else if (errnum == 0) {
                                    printf("Deleted successfully\n. This file is no longer exits on server.\n");
                                    // syn delete here
                                    char* file_dir = (char*)malloc(sizeof(char)*50);
                                    strcat(file_dir, username);
                                    strcat(file_dir, "_client/");
                                    strcat(file_dir, wanna_delete_filename);
                                    delete_file_on_client_side(file_dir);
                                    break;
                                }
                                break;
                            case 4:
                                system("clear");
                                folder_name = malloc(sizeof(char)*BUFF_SIZE);
                                printf("Enter a new folder name: ");
                                scanf("%s", folder_name);
                                extended_filename = (char*) malloc(sizeof(char)*BUFF_SIZE);
                                bzero(extended_filename,BUFF_SIZE);
                                strcat(extended_filename, "4");
                                strcat(extended_filename,folder_name);
                                if(send_msg(conn_sock, extended_filename, strlen(extended_filename)) == -1){
                                    printf("Hard!\n");
                                    continue;
                                }
                                data = recv_msg(conn_sock);
                                printf("error number :%s\n", data);
                                errnum = atoi(data);
                                if (errnum == 1) {
                                    printf("Error : Folder_name exists on server\n");
                                    break;
                                } else if (errnum == 0) {
                                    char* saved_username = malloc(sizeof(char)*50);
                                    strcpy(saved_username, username);
                                    strcat(saved_username, "_client/");
                                    strcat(saved_username, folder_name);
                                    // printf("saved_username = %s len = %d\n", saved_username, strlen(saved_username));
                                    create_sub_client_folder(saved_username);
                                    printf("A new folder was created successfuly\n");
                                    break;
                                } else if (errnum == 2) {
                                    printf("Something was wrong\n");
                                    break;
                                }
                                break;
                            case 5:
                                system("clear");
                                printf("List of all user_names on server\n");
                                f = fopen(account_file_name, "r");
                                
                                int i = 0;
                                
                                while(!(feof(f))) {
                                    user_name = malloc(sizeof(char)*30);
                                    user_password = malloc(sizeof(char)*30);
		                            fscanf(f, "%s %s",user_name, user_password);
                                    printf("---------------------------\n");
                                    printf("|%d. %s\n", i+1, user_name);
                                    printf("---------------------------\n");
		                            if (user_name[0] == '\0' || user_password[0] == '\0') break;
                                    i += 1;
                                }
                                fclose(f);
                                
                                other_user_name = malloc(sizeof(char)*BUFF_SIZE);
                                printf("Enter an username that you want to see: ");
                                scanf("%s", other_user_name);
                                
                                extended_filename = (char*) malloc(sizeof(char)*BUFF_SIZE);
                                
                                bzero(extended_filename,BUFF_SIZE);
                                strcat(extended_filename, "5");
                                strcat(extended_filename,other_user_name);
                                
                                if(send_msg(conn_sock, extended_filename, strlen(extended_filename)) == -1){
                                    printf("Hard!\n");
                                    continue;
                                }

                                data = recv_msg(conn_sock);
                                printf("----------------------------\n");
                                printf("All the files of %s\n", other_user_name);
                                printf("----------------------------\n");
                                printf("----------------------------\n");
                                printf("%s\n", data);
                                printf("----------------------------\n");
                                break;
                            
                            case 6:
                                system("clear");
                                printf("List of all user_names on server\n");
                                
                                f = fopen(account_file_name, "r");
                            
                                int j = 0;
                                    
                                while(!(feof(f))) {
                                    user_name = malloc(sizeof(char)*30);
                                    user_password = malloc(sizeof(char)*30);
                                    fscanf(f, "%s %s",user_name, user_password);
                                    printf("---------------------------\n");
                                    printf("|%d. %s\n", j+1, user_name);
                                    printf("---------------------------\n");
                                    if (user_name[0] == '\0' || user_password[0] == '\0') break;
                                    j += 1;
                                }
                                fclose(f);
                
                                    other_user_name = malloc(sizeof(char)*BUFF_SIZE);
                                    printf("Enter an username that you want to see: ");
                                    scanf("%s", other_user_name);
                                    system("clear");

                                    
                                    extended_filename = (char*) malloc(sizeof(char)*BUFF_SIZE);
                                    
                                    bzero(extended_filename,BUFF_SIZE);
                                    strcat(extended_filename, "5");
                                    strcat(extended_filename,other_user_name);
                                    
                                    if(send_msg(conn_sock, extended_filename, strlen(extended_filename)) == -1){
                                        printf("Hard!\n");
                                        continue;
                                    }
                                    free(extended_filename);

                                    data = recv_msg(conn_sock);
                                    printf("----------------------------\n");
                                    printf("All the files of %s\n", other_user_name);
                                    printf("----------------------------\n");
                                    printf("----------------------------\n");
                                    printf("%s\n", data);
                                    printf("----------------------------\n");

                                    download_filename = malloc(sizeof(char)*BUFF_SIZE);
                                    new_download_filename = malloc(sizeof(char)*BUFF_SIZE);
                                    printf("Enter filename you want to download : ");
                                    scanf("%s", download_filename);
                                    printf("Enter new filename you want to save : ");
                                    scanf("%s", new_download_filename);
                                    extended_filename = (char*) malloc(sizeof(char)*BUFF_SIZE);
                                    bzero(extended_filename,BUFF_SIZE);
                                    strcat(extended_filename, "6");
                                    strcat(extended_filename, other_user_name);
                                    strcat(extended_filename, "/");
                                    strcat(extended_filename,download_filename);

                                    recv_file(conn_sock,extended_filename, new_download_filename);
                                    
                                break;
                            



                            case 7:
                                bytes_transfered = 0;
                                char* local_filename = malloc(sizeof(char)*BUFF_SIZE);
                                        // input file link
                                        folder_name = malloc(sizeof(char)*BUFF_SIZE);
                                        printf("Enter folder that you want to upload file into: ");
                                        scanf("%s", folder_name);
                                        // fflush(stdin);
                                        printf("Enter filename that you want to upload: ");
                                        scanf("%s", local_filename);
                                        printf("local filename = %s", local_filename);
                                        // memset(buff,'\0', BUFF_SIZE);
                                        // fgets(buff, BUFF_SIZE, stdin);
                                        // buff[strlen(buff) - 1] = '\0';
                                        // if (strlen(buff) == 0) break;
                                        strcpy(filelink, local_filename);
                                        printf("file link = %s", filelink);
                                        if((fp = fopen(filelink, "rb")) == NULL){
                                            printf("Error: File not found\n");
                                            continue;
                                        }else{
                                            
                                            strcpy(filename, rindex(filelink, '/') + 1);
                                            extended_filename = (char*) malloc(sizeof(char)*BUFF_SIZE);
                                            bzero(extended_filename,BUFF_SIZE);
                                            strcat(extended_filename, "1");
                                            strcat(extended_filename, folder_name);
                                            strcat(extended_filename, "/");
                                            strcat(extended_filename,filename);
                                            printf("extended filename: %s", extended_filename);
                                            if(send_msg(conn_sock, extended_filename, strlen(extended_filename)) == -1){
                                                printf("Hard!\n");
                                                continue;
                                            }
                                            printf("filename = %s\n", filename);
                                            fflush(stdin);
                                            data = recv_msg(conn_sock);
                                            printf("error number :%s\n", data);
                                            errnum = atoi(data);
                                            if(errnum == 1){		// if file is existent on server
                                                printf("Error: File is existent on server\n");
                                            }else if(errnum == 0){		// if there is no error
                                                while(errnum == 0){		// until there is an error, keep reading from file
                                                    memset(buff,'\0', BUFF_SIZE);
                                                    if(fread(buff, BUFF_SIZE, 1, fp) == 1){
                                                        if(send_msg(conn_sock, buff, sizeof(buff)) == -1){
                                                            break;
                                                        }
                                                        printf("Uploaded: %.2lf MB\n", (bytes_transfered += sizeof(buff)) / (1024*1024));
                                                    }else{
                                                        if(send_msg(conn_sock, buff, sizeof(buff)) == -1){
                                                            break;
                                                        }
                                                        printf("Uploaded: %.2lf MB\n", (bytes_transfered += sizeof(buff)) / (1024*1024));
                                                        
                                                        data = recv_msg(conn_sock);
                                                        errnum = atoi(data);

                                                        if(send_eof_msg(conn_sock) == -1){
                                                            break;
                                                        }
                                                    }
                                                    data = recv_msg(conn_sock);
                                                    errnum = atoi(data);
                                                }
                                                if(errnum == -1){
                                                    
                                                    // syn here
                                                    char* saved_username = malloc(sizeof(char)*50);
                                                    strcpy(saved_username, username);
                                                    strcat(saved_username, "_client/");
                                                    strcat(saved_username, folder_name);
                                                    strcat(saved_username, "/");
                                                    copyfile(filename, saved_username);
                                                    // printf("\nSuccessful Uploading\n");
                                                    printf("Uploaded successfuly");

                                                }else{
                                                    printf("\nError: File uploading is interupted\n");
                                                }
                                            }
                                        }
                                        fclose(fp);
                                    
                                    
                                system("clear");
                                // TODO
                                break;
                            
                            
                            
                            
                            
                            
                            case 8:
                                system("clear");
                                memset(dest, '\0', sizeof(dest));
                                printf("You wanna logout (y/n): \n");
                                fgets(dest, BUFF_SIZE, stdin);

                                //printf("*******   LOUT  ***********");
                                __fpurge(stdin);
                                bzero(buff, BUFF_SIZE);
                                dest[strlen(dest)-1] = '\0';
                                //fgets(buff, BUFF_SIZE, stdin);
                                if (strcmp(dest, "y") == 0) {
                                    char pass[PASS_LEN + 1];
                                    // memset(dest, '\0', sizeof(dest));
                                    strcpy(buff, "LOUT");
                                    strcat(buff, " ");
                                    strcat(buff, username);
                                    printf("Enter your password: ");
                                    __fpurge(stdin);
                                    scanf("%s", pass);

                                    strcat(buff, " ");
                                    strncat(buff, pass, PASS_LEN);

                                    msg_len = strlen(buff) + 1;
                                    printf("buff  = %s\n", buff);

                                    if (send_msg(conn_sock, buff, msg_len) == -1) {
                                        printf("Connection closed!\n");
                                        break;
                                    }

                                    //receive echo reply
                                    bytes_received = recv(conn_sock, buff, BUFF_SIZE-1, 0);
                                    if(bytes_received <= 0){
                                        printf("\nError!Cannot receive data from sever!\n");
                                        break;
                                    }

                                    buff[bytes_received] = '\0';
                                    printf("\nReply from server: %s\n", buff);
                                    if (buff[0] == '2' && buff[1] == '0')
                                        inner_loop = 1;
                                    break;
                                    
                                }
                                else 
                                    break;
                            default:
                                printf("Wrong choice.\n");
                                break;
                        }
                    } while (inner_loop == 0);
                }
                break;
            default:
                printf("Wrong choice. Make sure your choice is 1 or 2\n");
                break;
        }
    }
       
    close(conn_sock);
    return 0;
}
