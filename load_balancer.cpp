#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h> 
#include <sys/stat.h> 

using namespace std;

#define READ 0
#define WRITE 1
#define MYFIFO "/tmp/myfifo"

char* toArray(int number){
    char *numberArray = (char *)malloc(sizeof(char)*10);
    sprintf(numberArray,"%d", number);
    return numberArray;
}

void deletWhitespaces(string& str){
    for(int i=0; i<str.length(); i++)
        if(str[i] == ' ') 
            str.erase(i,1);
}

void splitBydash(string& str, vector<string> &arguments){

    size_t current, previous = 0;
    current = str.find('-');

    while (current != std::string::npos) {
        arguments.push_back(str.substr(previous, current - previous));
        previous = current + 1;
        current = str.find('-', previous);
    }
    arguments.push_back(str.substr(previous, current - previous));
}

string splitAfterequal(string& str){
    size_t current;
    current = str.find('=');
    return str.substr(current + 1 , str.size());
}

string splitBeforequal(string& str){
    size_t current;
    current = str.find('=');
    return str.substr(0 , current);
}

bool trueCommand(string str){
    size_t dash = std::count(str.begin(), str.end(), '-');
    size_t equal = std::count(str.begin(), str.end(), '=');
    if(dash < 1 || equal < 2)
        return false;
    return true;
}

void read_directory(const string& name, vector<string>& v)
{
    DIR* dirp = opendir(name.c_str());
    struct dirent * dp;
    if((dp = readdir(dirp)) == NULL){
        cout << "wrong directory" << endl;
        return;
    }

    while ((dp = readdir(dirp)) != NULL) {
        if( strcmp(dp->d_name , ".") == 0 || strcmp(dp->d_name , ".." ) == 0 || strcmp(dp->d_name , ".DS_Store") == 0)
            continue;
        v.push_back(dp->d_name);
    }
    closedir(dirp);
}

void creatWorkerprocess(int prc_cnt , vector <string> filesname , string dir , vector<string> feildsName , vector<string> filters){
    string request;
    int numOffiles , countUsedfile = 0;
    numOffiles = filesname.size();

    int filesPerprocess = (numOffiles / prc_cnt);
    int modeFileperProcess = numOffiles % prc_cnt;
    for(int i = 0 ; i < prc_cnt ; i++){
            
        int fd[2];
        pid_t pid;
        
        if(pipe(fd) == -1){
            cout << "pipe fail\n";
            break;
        }

        if((pid = fork()) < 0){
            cout << "fork fail\n";
            break;
        }
        
        if(pid == 0){
            close(fd[WRITE]);
            char *args[] = {"./worker" , toArray(fd[READ]) , NULL};
            execvp(args[0], args);
            close ( fd[READ]);
        }else{
            request.clear();
            close(fd[READ]);
            request.append("dir=");
            request.append(dir);
            request.append("&");

            if(modeFileperProcess > 0){
                for(int j = 0 ; j < filesPerprocess + 1 ; j++ ){
                    request.append("file=");
                    request.append(filesname[countUsedfile]);
                    request.append("&");
                    countUsedfile++;
                }
                modeFileperProcess--;
            }else{
                for(int j = 0 ; j < filesPerprocess ; j++ ){
                    request.append("file=");
                    request.append(filesname[countUsedfile]);
                    request.append("&");
                    countUsedfile++;
                }
            }

            for(int k = 0 ; k < feildsName.size() ; k++){
                request.append("filter=");
                request.append(feildsName[k]);
                request.append("/");
                request.append(filters[k]);
                if( k < feildsName.size() - 1)
                    request.append("&");
            }

            write (fd[WRITE], request.c_str(), request.size());
            close (fd[WRITE]);
            wait(&pid);
        }
    }
}

void creatPresenterprocess(string sortingValue, string sortType , int prc_cnt){
    pid_t pid;
    int fd;
    string request = "";
    if((pid = fork()) < 0)
        cout << "fork fail\n";
    
    if(pid == 0){
        char *args[] = {"./presenter" , NULL};
        execvp(args[0], args);
    }else{
        if(sortingValue != "")
            request = string(toArray(prc_cnt))+ "&" + sortingValue + "&" + sortType;
        else 
            request = string(toArray(prc_cnt));

        mkfifo(MYFIFO, 0666);
        fd = open(MYFIFO, O_WRONLY);
        write(fd, request.c_str(), request.size());
        close(fd);
    }
}

void setArguments(vector<string> arguments , vector <string> &feildsName , vector <string> &filters  , string &sortingValue , string &sortType , int &prc_cnt , string &dir){

    for(int i = 0 ; i < arguments.size() ; i++){
        if(arguments[i].find("prc_cnt") != std::string::npos){
            prc_cnt = stoi(splitAfterequal(arguments[i]));
        }else if(arguments[i].find("dir") != std::string::npos){
            dir = splitAfterequal(arguments[i]);
        }else if(arguments[i].find("descend") != std::string::npos || arguments[i].find("ascend") != std::string::npos){
            sortType = splitAfterequal(arguments[i]);
            sortingValue = splitBeforequal(arguments[i]);
        }else{
            feildsName.push_back(splitBeforequal(arguments[i]));
            filters.push_back(splitAfterequal(arguments[i]));
        }
    }
}

void run(string userCommand){
    vector<string> arguments;
	string sortType = "" , sortingValue = "" , dir;
    int prc_cnt = 0;
    vector <string> filesname , feildsName , filters;
    
    deletWhitespaces(userCommand);
    arguments.clear();
    splitBydash(userCommand , arguments);
    setArguments(arguments , feildsName, filters , sortingValue , sortType , prc_cnt , dir);
    read_directory(dir , filesname);
    creatPresenterprocess(sortingValue , sortType , prc_cnt);
    creatWorkerprocess(prc_cnt , filesname , dir , feildsName , filters);
}

void inputModel(){
    cout << "your input should be in this type: " << endl;
    cout << "<FIELD NAME>=<CORRESPONDING FILTERING VALUE>-<SORTING VALUE>=< ascend/descend>-<'prc_cnt'>=<N>-<'dir'>=<RELATIVE DATASET ADDRESS>"<<endl;
}

int main(){
    string userCommand;
    
    inputModel();
    
    while(true){
        getline(cin,userCommand);
        if(userCommand == "quit")
            break;
        if(trueCommand(userCommand)){
            run(userCommand);
        }else{
            cout << "wrong command" << endl;
            inputModel();
        }
    }
}