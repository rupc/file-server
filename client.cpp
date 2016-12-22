#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;
#define BUFSIZE 1024

#define MAXLEN 1000 

class StringTokenizer {
    public:
        static vector<string> getTokens(const string& str, const string& delimiters = " ") {
            string::size_type lastPos = str.find_first_not_of(delimiters, 0);
            string::size_type pos     = str.find_first_of(delimiters, lastPos);
            vector<string> tokens;
            while( string::npos != pos || string::npos != lastPos) {
                tokens.push_back(str.substr(lastPos, pos - lastPos));
                lastPos = str.find_first_not_of(delimiters, pos);
                pos = str.find_first_of(delimiters, lastPos);
            }
            return tokens;
        }
};
void login_process(int &sockfd );
int main(int argc, char *argv[]) {
	int sockfd;
	int len;
	struct sockaddr_in address;
	char buf_in[BUFSIZE];
	char buf_get[BUFSIZE];
	bool super_flag = false;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr("127.0.0.1");
	address.sin_port = htons(atoi(argv[1]));
	len = sizeof(address);
	if(connect(sockfd, (struct sockaddr *)&address, len) < 0) {
		perror("connect error : ");
		exit(0);
	}
	
	login_process(sockfd);

	while(1) {
		fflush(stdin);
		printf("Enter the command>(quit for exit)");

		/* main part */
		memset(buf_in, '\0', BUFSIZE);
		fgets(buf_in, BUFSIZE, stdin);
		buf_in[strlen(buf_in) - 1] = '\0';
		write(sockfd, buf_in, BUFSIZE);
		if( strncmp(buf_in, "quit", 4) == 0) {
			close(sockfd);
			exit(0);
		} else if( strncmp(buf_in, "get", 3) == 0) {
			int idx = 3;
			while(buf_in[idx++] != '\0');
			string name(&buf_in[4], &buf_in[idx-1]);
			ofstream out(name.c_str());
			while(1) {
				read(sockfd, buf_get, BUFSIZE);
			   	if(strcmp(buf_get, "no") == 0) {
					cout << name << " doesn't exist" << endl;
					break;
				}	
				out << buf_get;
				if(strcmp(buf_get, "noaccess") == 0) {
					cout << "cannot acces this file because you have no permission for it" << endl;
					break;
				}
				if(strcmp(buf_get, "$") == 0) {
					cout << "download succesfully " << name << endl; 
					break;
				}	
			}
		} else if( strncmp(buf_in, "mget", 4) == 0 ) {
			string tmp_s(&buf_in[0], &buf_in[strlen(buf_in)]);
			vector<string> tokens = StringTokenizer::getTokens(tmp_s);
			read(sockfd, buf_get, BUFSIZE);
			if(strcmp(buf_get, "wrongformat") == 0) {
				cout << "check format plz " << endl;
				continue;
			} else if(strcmp(buf_get, "rightformat") == 0) {
				cout << "ready to send files you list" << endl;
			}
			for(int i = 1 ; i < tokens.size(); ++i) {
				read(sockfd, buf_get, BUFSIZE);
				if( strcmp(buf_get, "no") == 0) {
					cout << "can't find " << tokens[i] << " in serve.r" << endl;
				} else if( strcmp(buf_get ,"noaccess") == 0) {
					cout << "you can't access " << tokens[i] << endl;
				} else {
					ofstream out(tokens[i].c_str());
					out << buf_get;
					while(1) {
						read(sockfd, buf_get, BUFSIZE);
						out << buf_get;
						if(strcmp(buf_get, "$") == 0) {
							cout << "download complete" << endl;
							break;
						}
					}
				}
			}
		} else if( strncmp(buf_in, "list", 4) == 0) {
			cout << "server has the follwing papers." << endl;
			cout << "-------------------------------" << endl;
			while(1) {
				read(sockfd, buf_get, BUFSIZE);
				if(strcmp(buf_get, "$") == 0) break;
				else
					cout << buf_get << endl;
			}
			cout << "----------list end ----------" << endl;
		} else if( strncmp(buf_in, "view", 4) == 0) {
			while(1) {
			
				read(sockfd, buf_get, BUFSIZE);
				if(strcmp(buf_get, "no") == 0) {
					cout << "sorry, server do not have that file.." << endl;
					break;
				} else if(strcmp(buf_get, "noaccess") == 0) {
					cout << "you have no permission on it" << endl;
					break;
				}

				cout << "See the text you request " << endl;
				if(strcmp(buf_get, "$") == 0) break;
				else
					cout << buf_get << endl;
			}
		} else if( strncmp(buf_in, "add", 3) == 0) {
			int idx = 3;
			while(buf_in[idx++] != '\0');
			string name(&buf_in[4], &buf_in[idx-1]);
			ifstream in(name.c_str());
			if(in.is_open() == false) {
				cout << "sorry, that file doesn't exist! check it again" << endl;
				write(sockfd, "no", BUFSIZE);
				break;
			} else {
				string text_buf;
				while(getline(in, text_buf)) {
					text_buf += '\n';
					write(sockfd, text_buf.c_str(), BUFSIZE);
				}
				write(sockfd, "$", BUFSIZE);
			}
		
		} else if( strncmp(buf_in, "help", 4) == 0) {
			cout << "commands available are following -- " << endl;
			cout << "------------------------------------" << endl;
			read(sockfd, buf_get, BUFSIZE);
			cout << buf_get << endl;
		} else if( strncmp(buf_in, "grant", 5) == 0) {
			read(sockfd, buf_get, BUFSIZE);
			if( strcmp(buf_get, "notsuper") == 0) {
				cout << "grant command only for super user" << endl;
				continue;
			} else if( strcmp(buf_get, "wrongformat") == 0) {
				cout << "wrong foramt. " << endl;
				continue;
			} else if( strcmp(buf_get, "fail_client") == 0) { 
				cout << "check your ID" << endl;
				continue;
			} else if( strcmp(buf_get, "fail_permission") == 0) {
				cout << "check a permission to be valid" << endl;
				continue;
			}else if( strcmp(buf_get, "success") == 0) {
				read(sockfd, buf_get, BUFSIZE);
				cout << buf_get << endl;
			}
		} else if( strncmp(buf_in, "revoke", 6) == 0) {
			read(sockfd, buf_get, BUFSIZE);
			if( strcmp(buf_get, "notsuper") == 0) {
				cout << "revoke command only for super user" << endl;
		   		continue;		
			} else if( strcmp(buf_get, "wrongformat") == 0) {
			    cout << "wrong foramt " << endl;
				continue;
			} else if( strcmp(buf_get, "fail_client") == 0) {
				cout << "check your ID" << endl;
				continue;
			} else if( strcmp(buf_get ,"success") == 0) {
				read(sockfd, buf_get, BUFSIZE);
				cout << buf_get << endl;
			}
     	} else if( strncmp(buf_in, "fgrant", 6) == 0) {
			read(sockfd, buf_get, BUFSIZE);
			if( strcmp(buf_get, "notsuper") == 0) {
				cout << "grantf command only for super user" << endl;
				continue;
			} else if( strcmp(buf_get, "wrongformat") == 0) {
				cout << "wrong format " << endl;
				continue;
			} else if( strcmp(buf_get, "fail_paper") == 0) {
				cout << "check your title" << endl;
				continue;
			} else if( strcmp(buf_get, "fail_permission") == 0) {
				cout << "check permission to be vaild" << endl;
				continue;
			} else if( strcmp(buf_get, "success") == 0) {
				read(sockfd, buf_get, BUFSIZE);
				cout << buf_get << endl;
			}
		} else if( strncmp(buf_in, "uadd", 7) == 0){
   			read(sockfd, buf_get, BUFSIZE);
			if( strcmp(buf_get, "notsuper") == 0) {
				cout << "Only super user can add client to list" << endl;
				continue;
			} else if( strcmp(buf_get, "wrongformat") == 0) {
				cout << "wrong format " << endl;
				continue;
			} else if( strcmp(buf_get , "fail_permission") == 0) {
				cout << "chekc permission to be vaild" << endl;
				continue;
			} else if( strcmp(buf_get, "success") == 0) {
				read(sockfd, buf_get, BUFSIZE);
				cout << buf_get << endl;
			}
		} else if( strncmp(buf_in, "rename", 6) == 0) {
			read(sockfd, buf_get, BUFSIZE);
			string tmp(&buf_in[0], &buf_in[strlen(buf_in)]);
			vector<string> tokens = StringTokenizer::getTokens(tmp);

			if( strcmp(buf_get, "wrongformat") == 0) {
				cout << "wrong format" << endl;
				continue;
			} else if( strcmp(buf_get, "nofirst") == 0) {
				cout << "first file not in list" << endl;
				continue;
			} else if( strcmp(buf_get, "yessecond") == 0) {
				cout << "second name already in list" << endl;
				continue;
			} else if( strcmp(buf_get, "success") == 0) {
				cout << tokens[1] << " has been changed to " << tokens[2]  << endl;
			}
		
		} else if ( strncmp(buf_in, "del", 3) == 0 ) {
			read(sockfd, buf_get, BUFSIZE);
			string tmp(&buf_in[0], &buf_in[strlen(buf_in)]);
			vector<string> tokens = StringTokenizer::getTokens(tmp);
			if( strcmp(buf_get, "nofile") == 0) {
				cout << "No file(" << tokens[1] << ") Exist!" << endl;
				continue;
			} else if( strcmp(buf_get, "noformat") == 0) {
				cout << "check format : del filename" << endl;
			} else  {
				cout << "delete " << tokens[1] << " successfully." << endl;
			}
		} else {
			read(sockfd, buf_get, BUFSIZE);

			cout << buf_get << endl;
		}
		
	}
	/* main part end */
	close(sockfd);
	exit(0);
}

void login_process(int &sockfd) {
	string client_name;
	string password;
	char buf_get[BUFSIZE];
	bool exist_name  = false;
	bool exist_password = false;
	while(1) {
		cout << "ID>";
		cin >> client_name;
		write(sockfd, client_name.c_str(), BUFSIZE);
		read(sockfd, buf_get, BUFSIZE);
		if( strcmp(buf_get, "yes") == 0 ) {
			exist_name = true;
		} else if( strcmp(buf_get, "no") == 0 ) {
			exist_name = false;
		}
		if(exist_name) {
			int wrong_count = 0;
			while(1) {
				cout << "password>";
				cin >> password;
				write(sockfd, password.c_str(), BUFSIZE);
				read(sockfd, buf_get, BUFSIZE);
				if( strcmp(buf_get, "yes") == 0 ) {
					exist_password = true;
				} else if( strcmp(buf_get, "no") == 0) {
					exist_password = false;
				}
				if(exist_password) {
					cout << "ID " << client_name << " login " << endl;
				    return;	
				} else {
					cout << "wrong password" << endl;
					wrong_count++;
					if(wrong_count > 3) {
						cout << "do you forgot your password?" << endl;
						exit(EXIT_FAILURE);
					}
					continue;
				}
			}
		} else {
			cout << "sorry, unidentified ID. ask for server" << endl;
			continue;
			
		}
	}
}
