#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <pthread.h>

#include <iostream>
#include <cstdlib>
#include <string>
#include <fstream>
#include <list>
#include <algorithm>
#include <vector>
#include <map>

using namespace std;

#define MAXLEN 20
#define BUFSIZE 1024
string papers_path = "./paper_list/";
string client_path = "./clientinfo/";
string access_path = "./accessinfo/";
enum PERMISSION {
    NONE, GENERAL, SPECIAL, SUPER
};
#define PERSIZE 4
/*
To use pthread library in Code::Blocks
add -lpthread line option to "Setting -> Compiler and Debugger -> Linker Setting"
*/
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
class client_ele {
    public:
        client_ele(string a_id, string a_password) {
            id = a_id;
            password = a_password;
        }
        string id;
        string password;
        int credential;
    string get_password() { return password;}
    string get_id() {return id;}
    friend class client_list;
    friend bool operator == (client_ele e1, client_ele e2);
    friend ostream &operator <<(ostream &out, client_ele &e);
    friend bool operator == (client_ele e1, string e2);
};
bool operator == (client_ele e1, client_ele e2) {
    if(e1.id == e2.id)
        return true;
    else
        return false;
}
bool operator == (client_ele e1, string e2) {
    if(e1.id == e2)
        return true;
    else
        return false;
}
ostream &operator <<(ostream &out, client_ele &e) {
    out << "ID : " << e.id << "\tpasswd : " << e.password << endl;
}
class client_list {
    protected:
        vector<client_ele> clientls;
    public:
        client_list() {
            string clientInfoPath = client_path;
            clientInfoPath += "clientlist";
            ifstream in(clientInfoPath.c_str());
            while(!in.eof()) {
                string tmp_id, tmp_passwd;
                in >> tmp_id >> tmp_passwd;
                client_ele tmp_cli(tmp_id, tmp_passwd);
                clientls.push_back(tmp_cli);
            }
        }

        void write_client_list(string cli_path) {
            cli_path += "clientlist";
            ofstream out(cli_path.c_str());
            for(int i = 0 ; i < clientls.size(); ++i) {
                out << clientls[i].id << " " << clientls[i].password << endl;
            }
        }
        string get_password(string client_name) {
            vector<client_ele>::iterator c_iter = find(clientls.begin(), clientls.end(), client_name);
            if(c_iter != clientls.end())
                return c_iter->get_password();
            else
                return string("");
        }
        void add_client(client_ele cl) {
            vector<client_ele>::iterator c_iter = find(clientls.begin(), clientls.end(), cl);
            if(c_iter == clientls.end())
                clientls.push_back(cl);
            else
                cout << "it already exist in client list " << endl;
        }
        bool IsInCli_list(string e) {
            vector<client_ele>::iterator c_iter = find(clientls.begin(), clientls.end(), e);
            if(c_iter == clientls.end())
                return false;
            else
                return true;
        }

        void print_list() {
            vector<client_ele>::iterator c_iter = clientls.begin();
            for(; c_iter != clientls.end(); c_iter++)
                cout << *c_iter;
        }
};

list<string> papers;
vector<string> cmd_list;
client_list cli_object;

class access_manage :public client_list {
    map<string, int> client_permission;
    map<string ,int> file_permission;

    PERMISSION perm_for_client;
    PERMISSION perm_for_file;
public:

    access_manage() {
        read_client_access(access_path);
        read_file_access(access_path);


    }
    bool delete_file(string file_name) {
        map<string, int>::iterator f_iter = file_permission.find(file_name);
        if(f_iter == file_permission.end()) {
            return false;
        } else {
            file_permission.erase(f_iter);
            write_file_access(access_path);
            return true;
        }
    }
    void update_filename_for_rename(string oldfile, string newfile) {
        map<string, int>::iterator f_iter = file_permission.find(oldfile);
        int tmp = f_iter->second;
        file_permission[newfile] = tmp;
        file_permission.erase(f_iter);

        this->write_file_access(access_path);

    }
    void add_new_client_and_permission(string client_name, int per) {
        client_permission[client_name] = per;
    }
    void add_new_paper_and_permission(string file_name, int per) {
        file_permission[file_name] = per;
    }
    void write_client_access(string client_access_path) {
        client_access_path += "access";
        ofstream out(client_access_path.c_str());
        map<string, int>::iterator o_iter = client_permission.begin();
        for(; o_iter != client_permission.end(); ++o_iter) {
            out << o_iter->first << " " << o_iter->second << endl;
        }
    }
    void read_client_access(string client_access_path) {
        client_access_path += "access";
        ifstream in(client_access_path.c_str());
        string name;
        int per;
        while(!in.eof()) {
            in >> name >> per;
            client_permission[name] = per;
        }
    }
    void write_file_access(string file_access_path) {
        file_access_path += "file_access";
        ofstream out(file_access_path.c_str());
        map<string, int>::iterator o_iter = file_permission.begin();
        for(; o_iter != file_permission.end(); ++o_iter) {
            out << o_iter->first << " " << o_iter->second << endl;
        }
    }
    void read_file_access(string file_access_path) {
        file_access_path += "file_access";
        ifstream in(file_access_path.c_str());
        string name;
        int per;
        while(!in.eof()) {
            in >> name >> per;
            file_permission[name] = per;
        }
    }
    bool change_permission_for_client(string client_name, int a_permission) {
        bool exist = false;
        a_permission = a_permission % PERSIZE;

        exist = IsInCli_list(client_name);
        if(!exist) {
            cout << "not in list";
            return false;
        }
        else {
            vector<client_ele>::iterator c_iter = find(clientls.begin(), clientls.end(), client_name);
            client_permission[c_iter->get_id()] = a_permission;
            write_client_access(access_path);
        }
    }
    bool change_permission_for_file(string file_name, int a_permission) {
        bool exist = false;
        a_permission = a_permission % PERSIZE;
        map<string, int>::iterator check_iter = file_permission.find(file_name);
        if(check_iter == file_permission.end()) {
            return exist;
        } else {
            exist = true;
            check_iter->second = a_permission;
            return exist;
        }
    }
    bool doesHeHaveAPermissionOnThatFile(string client_name, string file_name) {
        bool doesit = false;
        map<string, int>::iterator NameCheck_iter = client_permission.find(client_name);
        map<string, int>::iterator FileCheck_iter = file_permission.find(file_name);
        if(NameCheck_iter->second >= FileCheck_iter->second) {
            doesit = true;
        }
        return doesit;
    }
    void print_client_permission() {
        map<string, int> ::iterator c_iter = client_permission.begin();
        for(;c_iter != client_permission.end(); ++c_iter) {
            cout << c_iter->first << " " << aux_print(c_iter->second) << endl;

        }
    }
    void print_file_permission() {
        map<string, int> :: iterator f_iter = file_permission.begin();
        for(;f_iter != file_permission.end(); ++f_iter)
            cout << f_iter->first << " " << aux_print(f_iter->second) << endl;
    }
    string aux_print(int a_permission) {
        string res;
        switch(a_permission % PERSIZE) {
        case 0:
            res = "NONE";
            break;
        case 1:
            res = "GENERAL";
            break;
        case 2:
            res = "SPECIAL";
            break;
        case 3:
            res = "SUPER";
            break;
        default:
            res = "WHAHISIT?";
            break;
        }
        return res;
    }
    int IntValue_for_permission(string per) {
        if(per == "NONE") return 0;
        else if(per == "GENERAL") return 1;
        else if(per == "SPECIAL") return 2;
        else if(per == "SUPER") return 3;
        else return -1;
    }

};

bool login_process(int &client_sockfd, client_list &cli_list, string &client_name) {

    char buf_in[BUFSIZE];
    int err_count = 0;
    bool yeah = false;
    while(1) { // ID check;
           read(client_sockfd, buf_in, BUFSIZE);
           cout << "client entered ID : " << buf_in << endl;
           bool outer_flag = false;
           string t_id(buf_in);
           bool exist = cli_list.IsInCli_list(t_id);
           if(exist) {
               write(client_sockfd, "yes", BUFSIZE);
               while(1) { // password check
                   read(client_sockfd, buf_in, BUFSIZE);
                   bool corr_password;
                   string tmp_pass = cli_list.get_password(t_id);
                    if(strcmp(buf_in, tmp_pass.c_str()) == 0)
                       corr_password = true;
                   else corr_password = false;
                   if(corr_password) {
                       write(client_sockfd, "yes", BUFSIZE);
                       outer_flag = true;
                       yeah = true;
                       client_name =  t_id;
                       break;
                   } else {
                       err_count++;
                       write(client_sockfd, "no", BUFSIZE);
                       if(err_count > 3) return yeah;
                   }
               }
           } else {
               write(client_sockfd, "no", BUFSIZE);
           }
           if(outer_flag) break;
    }
    return yeah;
}

void print_papers() {
    list<string>::iterator p_iter = papers.begin();
    for(; p_iter != papers.end(); ++p_iter) {
        cout << *p_iter << endl;
    }

}
// error check must be done by caller
void rename_file(string old_file, string new_file) {
    list<string>::iterator l_iter = find(papers.begin(), papers.end(), old_file);
    *l_iter = new_file;
    ofstream out_paper_list("papers");
    // update papers list file to reflect new file name
    l_iter = papers.begin();
    for(; l_iter != papers.end(); ++l_iter) {
        out_paper_list << *l_iter << endl;
    }


    // update real file name by using syscall mv
    string first_arg = "./" + papers_path + "/" + old_file;
    string second_arg = "./" + papers_path + "/" + new_file;
    string arg = "mv " + first_arg + " " + second_arg;
    system(arg.c_str());

}
void papersInit(ifstream &in) {
    string title;
    while(getline(in, title)) {
        papers.push_back(title);
    }
}
void cmdInit() {
    ifstream cmdin("cmdlist");
    string tmp;
    while(getline(cmdin, tmp)) {
        cmd_list.push_back(tmp);
    }
}
void early_sockstuff(int &server_sockfd, sockaddr_in& server_address, int &server_len, char *port, int &state) {
	unlink("server_socket");
	server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	bzero(&server_address, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(6666); // needs to be updated later
	server_len = sizeof(server_address);
	state = bind(server_sockfd, (struct sockaddr *)&server_address, server_len);
	if(state == -1) {
		perror("bind error : ");
		exit(0);
	}
	state = listen(server_sockfd, 5);
	if(state == -1) {
		perror("listen error : ");
		exit(0);
	}
}
string getTitle(int cmdlen, char *buf_a) {
    int idx = cmdlen;
    while(buf_a[idx++] != '\0');
    string name(&buf_a[cmdlen+1], &buf_a[idx-1]);
    return name;
}
bool isInList(string &name) {
    list<string>::iterator l_iter = find(papers.begin(), papers.end(), name);
    if(l_iter == papers.end()) return false;
    else return true;
}
void SendingFile(string &name, int &client_sockfd) { // function that sends file named "name" to a client who has client_sockfd number
    string targetfile = papers_path;
    targetfile += name;
    ifstream in_text(targetfile.c_str());
    string text_buf;
    while(getline(in_text, text_buf)) {
        text_buf += "\n";
        write(client_sockfd, text_buf.c_str(), BUFSIZE);
    }
    write(client_sockfd, "$", BUFSIZE);
}
void ReceevingFile(string &name, int &client_sockfd) {
    string targetfile = papers_path;
    targetfile += name;
    ofstream out_text(targetfile.c_str());
    char text_buf[BUFSIZE];
    string msg = "server received sended file successfully";
    papers.push_back(name);


    while(1) {
        read(client_sockfd,text_buf, BUFSIZE);


        if(strcmp(text_buf, "$") == 0) {
            cout << msg << endl;
            break;
        }
        out_text << text_buf;
    }
}


bool clientinfo_init(int &sock) {
    string path = client_path;
    path += "clientlist";
    ifstream in(path.c_str());

}
/* void printAvailableCommand();*/


void *thread_request_process(void *arg) {

    int client_sockfd = *(int *)arg;
    char buf[BUFSIZE];
    string client_name;
    bool login = login_process(client_sockfd, cli_object, client_name);
    bool super_flag = false;

    if( strcmp(client_name.c_str(), "super") == 0)
        super_flag = true;

    access_manage am;
    if(!login) {
        cout << "login failure!" << endl;
        return 0;
    }
    cout << client_name << "get in and server is wating to receive a message" << endl;
    while(1) {
			memset(buf, '0', BUFSIZE);
			if( read(client_sockfd, buf, BUFSIZE) <= 0) {
				close(client_sockfd);
				break;
			}
			if (strncmp(buf, "get", 3) == 0) {
                string file_name = getTitle(3, buf); // second argument
                bool exist = isInList(file_name);
                cout << "request text : " << file_name << endl;
                if(exist == false) {
                    write(client_sockfd, "no", BUFSIZE);
                } else {

                    if(am.doesHeHaveAPermissionOnThatFile(client_name, file_name)) {
                        SendingFile(file_name, client_sockfd);
                    } else {
                        cout << client_name << "don't have permission for " << file_name << endl;
                        write(client_sockfd, "noaccess", BUFSIZE);
                    }
                }
			} else if ( strncmp(buf, "mget", 4) == 0 ) {
                string tmp(&buf[0], &buf[strlen(buf)]);
                vector<string> tokens = StringTokenizer::getTokens(tmp);
                bool exist;
                if(tokens.size() < 2) {
                    write(client_sockfd, "wrongformat", BUFSIZE);
                    continue;
                } else {
                    write(client_sockfd, "rightformat", BUFSIZE);
                }
                for(int i = 1; i < tokens.size(); ++i) {
                    exist = isInList(tokens[i]);
                    if(exist == false) {

                        write(client_sockfd, "no", BUFSIZE);
                    } else {
                        if(am.doesHeHaveAPermissionOnThatFile(client_name, tokens[i])) {
                            SendingFile(tokens[i], client_sockfd);
                        } else {
                            write(client_sockfd, "noacces", BUFSIZE);
                        }
                    }
                }

			} else if (strncmp(buf, "list", 4) == 0) {
                list<string>::iterator l_iter = papers.begin();
                for(; l_iter != papers.end(); ++l_iter) {
                    write(client_sockfd, (*l_iter).c_str(), BUFSIZE);
                }
                write(client_sockfd, "$", BUFSIZE);
			} else if (strncmp(buf, "view", 4) == 0 ) {
                string file_name = getTitle(4, buf);
                bool exist = isInList(file_name);
                if(exist == false) {
                    write(client_sockfd, "no", BUFSIZE);
                } else {
                    if(am.doesHeHaveAPermissionOnThatFile(client_name, file_name)) {
                        SendingFile(file_name, client_sockfd);
                    } else {
                        write(client_sockfd, "noaccess", BUFSIZE);
                    }

                }
			} else if (strncmp(buf, "add", 3) == 0) {
                string name = getTitle(3, buf);
                bool exist = isInList(name);
                if(exist == true) {
                    write(client_sockfd,"already exist", BUFSIZE);
                } else {
                    char check[BUFSIZE];
                    read(client_sockfd, check, BUFSIZE);
                    if(strcmp(check, "no") == 0)
                        break;
                    ReceevingFile(name, client_sockfd);
                    am.add_new_paper_and_permission(name, 1);
                    am.write_file_access(access_path);
                }
			} else if ( strncmp(buf, "madd", 4) == 0 ) {

			} else if (strncmp(buf, "del", 3) == 0) {
                string tmp(&buf[0], &buf[strlen(buf)]);
                vector<string> tokens = StringTokenizer::getTokens(tmp);
                if(tokens.size() != 2) {
                    write(client_sockfd, "noformat", BUFSIZE);
                    continue;
                }
                if(isInList(tokens[1]) == false) {
                    write(client_sockfd, "nofile", BUFSIZE);
                    continue;
                }

                list<string>::iterator i_iter = find(papers.begin(), papers.end(), tokens[1]);
                papers.erase(i_iter);
                i_iter = papers.begin();
                string p_path = "./papers";
                ofstream out(p_path.c_str());
                for(; i_iter != papers.end(); ++i_iter) {
                    out << *i_iter << endl;
                }
                am.delete_file(tokens[1]);
                string arg = "./paper_list/"+tokens[1];
                string cmd = "rm -f ";
                cmd += arg;
                system(cmd.c_str());

                write(client_sockfd, "yes", BUFSIZE);

			} else if (strncmp(buf, "help", 4) == 0) {
			    string cmdbuf = "";
			    for(int i = 0 ; i < cmd_list.size(); ++i) {
                    cmdbuf += cmd_list[i] + "\n";

			    }
                write(client_sockfd, cmdbuf.c_str(), BUFSIZE);
			} else if (strncmp(buf, "quit", 4) == 0) {
				write(client_sockfd, "bye bye", 8);
				close(client_sockfd);
				break;
			} else if (strncmp(buf, "grant", 5) == 0) {
			    if(super_flag == false) {
                    write(client_sockfd, "notsuper", BUFSIZE);
                    continue;
			    }

			    string tmp(&buf[0], &buf[strlen(buf)]);
                vector<string> tokens = StringTokenizer::getTokens(tmp);

                if(tokens.size() != 3) {
                    write(client_sockfd, "wrongformat", BUFSIZE);
                    continue;
                }
                // check client
                cout << tokens[1] << endl;
                if(cli_object.IsInCli_list(tokens[1]) == false) {
                    write(client_sockfd, "fail_client", BUFSIZE);
                    continue;
                }
                // check if permission type is valid
                cout << tokens[2] << endl;

                if(  (strcmp(tokens[2].c_str(), "NONE") != 0) &&
                     ( strcmp(tokens[2].c_str(),"GENERAL") != 0) &&
                     ( strcmp(tokens[2].c_str(), "SPECIAL") != 0))

                   {
                     write(client_sockfd, "fail_permission", BUFSIZE);
                     continue;
                   }
                int per_int = am.IntValue_for_permission(tokens[2]);

                am.change_permission_for_client(tokens[1], per_int);
                am.write_client_access(access_path);

                write(client_sockfd, "success", BUFSIZE);
                string result_str = tokens[1];
                result_str += " now have " + tokens[2];

                write(client_sockfd, result_str.c_str(), BUFSIZE);
			} else if( strncmp(buf, "fgrant", 6) == 0 ) {
                if(super_flag == false) {
                    write(client_sockfd, "notsuper", BUFSIZE);
                    continue;
			    }
			    string tmp(&buf[0], &buf[strlen(buf)]);
                vector<string> tokens = StringTokenizer::getTokens(tmp);

                if(tokens.size() != 3) {
                    write(client_sockfd, "wrongformat", BUFSIZE);
                    continue;
                }
                if(isInList(tokens[1]) == false) {
                    write(client_sockfd, "fail_paper", BUFSIZE);
                    continue;
                }

                if(  (strcmp(tokens[2].c_str(), "NONE") != 0) &&
                     ( strcmp(tokens[2].c_str(),"GENERAL") != 0) &&
                     ( strcmp(tokens[2].c_str(), "SPECIAL") != 0))

                   {
                     write(client_sockfd, "fail_permission", BUFSIZE);
                     continue;
                   }
                int per_int = am.IntValue_for_permission(tokens[2]);
                am.change_permission_for_file(tokens[1], per_int);
                am.write_file_access(access_path);
                write(client_sockfd, "success", BUFSIZE);
                string result_str = tokens[1];
                result_str += " now have " + tokens[2];
                write(client_sockfd, result_str.c_str(), BUFSIZE);
			} else if( strncmp(buf, "revoke", 6) == 0) {
                if(super_flag == false) {
                    write(client_sockfd, "notsuper", BUFSIZE);
                    continue;
			    }
			    string tmp(&buf[0], &buf[strlen(buf)]);
                vector<string> tokens = StringTokenizer::getTokens(tmp);
                if(tokens.size() != 2) {
                    write(client_sockfd, "wrongformat", BUFSIZE);
                    continue;
                }
                if(cli_object.IsInCli_list(tokens[1]) == false) {
                    write(client_sockfd, "fail_client", BUFSIZE);
                    continue;
                }
                am.change_permission_for_client(tokens[1], 0);
                am.write_client_access(access_path);
                write(client_sockfd, "success", BUFSIZE);
                string result_str = tokens[1];
                result_str += " now lose his permission!";
                write(client_sockfd, result_str.c_str(), BUFSIZE);

			} else if( strncmp(buf, "uadd", 4) == 0) {
                 if(super_flag == false) {
                    write(client_sockfd, "notsuper", BUFSIZE);
                    continue;
			    }
			    string tmp(&buf[0], &buf[strlen(buf)]);
                vector<string> tokens = StringTokenizer::getTokens(tmp);
                if(tokens.size() != 4) {
                    write(client_sockfd, "wrongformat", BUFSIZE);
                    continue;
                }
                if(  (strcmp(tokens[3].c_str(), "NONE") != 0) &&
                     ( strcmp(tokens[3].c_str(),"GENERAL") != 0) &&
                     ( strcmp(tokens[3].c_str(), "SPECIAL") != 0)) {

                     write(client_sockfd, "fail_permission", BUFSIZE);
                     continue;
                 }
                int per_int = am.IntValue_for_permission(tokens[3]);
                am.add_new_client_and_permission(tokens[1], per_int);
                am.write_client_access(access_path);

                client_ele cli_tmp(tokens[1], tokens[2]);
                cli_object.add_client(cli_tmp);
                cli_object.write_client_list(client_path);

                write(client_sockfd, "success", BUFSIZE);
                string result_str = tokens[1];
                result_str += " has been registered successfully !";
                write(client_sockfd, result_str.c_str(), BUFSIZE);
            } else if( strncmp(buf, "rename", 6) == 0) {
                string tmp(&buf[0], &buf[strlen(buf)]);
                vector<string> tokens = StringTokenizer::getTokens(tmp);

                if(tokens.size() != 3) {
                    write(client_sockfd, "wrongformat", BUFSIZE);
                    continue;
                }
                // error check for rename_file();
                if(!isInList(tokens[1])) {
                   write(client_sockfd, "nofirst", BUFSIZE);
                   continue;
                }
                if(isInList(tokens[2])) {
                    write(client_sockfd, "yessecond", BUFSIZE);
                    continue;
                }
                write(client_sockfd,"success", BUFSIZE);
                rename_file(tokens[1], tokens[2]);
                am.update_filename_for_rename(tokens[1], tokens[2]);

            } else
				write(client_sockfd, buf, BUFSIZE);
    }
    pthread_exit(0);
}
int main(int argc, char *argv[]) {
	int server_sockfd, client_sockfd;
	int server_len, client_len;
	struct sockaddr_in server_address, client_address;
	int state;

    ifstream in("papers");
    papersInit(in);
    cmdInit();
    print_papers();
	early_sockstuff(server_sockfd, server_address, server_len, argv[1], state);

    cli_object.print_list();
    access_manage am;
    am.print_client_permission();
    am.print_file_permission();
	while(1) {
		client_len = sizeof(client_address);
		client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_address, (socklen_t *)&client_len);
		if (client_sockfd == -1) {
			perror("Accept error : ");
			exit(0);
		}

        int res;
        pthread_t request_thread;
        res = pthread_create(&request_thread, NULL, thread_request_process, (void *)&client_sockfd);

        if(res != 0) {
            perror("Thread creation failed");
            exit(EXIT_FAILURE);
        }
	}
	close(client_sockfd);


}

