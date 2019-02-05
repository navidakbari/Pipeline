#include <iostream>
#include <unistd.h>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h> 
#include <sys/stat.h> 

using namespace std;

#define READ 0
#define WRITE 1
#define BUFSIZE 2048
#define MYFIFO "/tmp/myfifo"

template<size_t N>
string charTostring(char const(&data)[N]){
   return string(data, find(data, data + N, '\0'));
}

void splitByampersan(string& str, vector<string> &arguments){

    size_t current, previous = 0;
    current = str.find('&');

    while (current != string::npos) {
        arguments.push_back(str.substr(previous, current - previous));
        previous = current + 1;
        current = str.find('&', previous);
    }
    arguments.push_back(str.substr(previous, current - previous));
}

string splitByequal(string& str){
    size_t current;
    current = str.find('=');
    return str.substr(current + 1 , str.size());
}

string splitBeforeslash(string& str){
    size_t current;
    current = str.find('/');
    return str.substr(0 , current);
}

string splitAfterslash(string& str){
    size_t current;
    current = str.find('/');
    return str.substr(current + 1 , str.size());
}

void setnames(vector<string> parsedRequest , string &dir , vector<string> &filenames , vector<string> &filters , vector<string> &feildsName){
    for(int i = 0 ; i < parsedRequest.size() ; i++){
        if(parsedRequest[i].find("file=") != string::npos){
            filenames.push_back(splitByequal(parsedRequest[i]));
        }else if(parsedRequest[i].find("dir=") != std::string::npos){
            dir = splitByequal(parsedRequest[i]);
        }else if(parsedRequest[i].find("filter=") != std::string::npos){
            string temp = splitByequal(parsedRequest[i]);
            feildsName.push_back(splitBeforeslash(temp));
            filters.push_back(splitAfterslash(temp));
        }
    }
}

void parseData(string request , vector<string> &filenames , vector<string> &filters , vector<string> &feildsName , string &dir){
    vector<string> parsedRequest;
    splitByampersan(request , parsedRequest);
    setnames(parsedRequest , dir , filenames , filters , feildsName);
}

void readFile(string dir , vector<string> &data){
    ifstream myReadFile;
    myReadFile.open(dir);
    string output;
    
    if (myReadFile.is_open()) {
        while(!myReadFile.eof()){
            getline(myReadFile , output);
            data.push_back(output);
        }
    }      
    myReadFile.close();
}

int findColumn(string data , string feild){
    stringstream temp(data);
    istream_iterator<string> begin(temp);
    istream_iterator<string> end;
    vector<string> vstrings(begin, end);
    for(int i = 0 ;i < vstrings.size() ; i++)
        if(vstrings[i] == feild)
            return i;
    return -1;
}

bool correctData(int column , string filter , string data){
    stringstream temp(data);
    istream_iterator<string> begin(temp);
    istream_iterator<string> end;
    vector<string> vstrings(begin, end);
    if(vstrings[column] == filter)
        return true;
    return false;
}

void filterData(vector<string> data , vector<string> &filtered , vector<string> filters , vector<string> feildsName){
    vector<string> temp;
    for(int i = 0 ; i < filters.size() ; i++){
        int column = findColumn(data[0] , feildsName[i]);
        temp.clear();
        for(int j = 1 ; j < data.size() ; j++){
            if(correctData(column , filters[i] , data[j])){
                temp.push_back(data[j]);
            }
        }
        if(filters.size() > 1){
            string tmp = data[0];
            data.clear();
            data.push_back(tmp);
            for(int j = 0 ; j < temp.size() ; j++){
                data.push_back(temp[j]);
            }
        }
    }
    for(int i = 0 ; i < temp.size() ; i++)
        filtered.push_back(temp[i]);
}

void handelFiles(vector<string> filenames , vector<string> filters , vector<string> feildsName , string dir , vector<string> &filtered){
    string fullDir;
    vector<string> data;
    for(int i = 0 ; i < filenames.size() ; i++){
        fullDir.clear();
        data.clear();
        fullDir = dir + "/" + filenames[i];
        readFile(fullDir , data);
        filterData(data , filtered , filters , feildsName);
    }
}

void sendDatatoPresenter(vector<string> filtered , vector<string> filenames , string dir){
    vector<string> data;
    if(filenames.size()>0){
        string fullDir;
        fullDir = dir + "/" + filenames[0];
        readFile(fullDir , data);
    }
    int fd;
    string request;
    request += data[0];
    request += "&";
    for(int i = 0 ; i < filtered.size() ; i++){
        request += filtered[i];
        if(i < filtered.size() - 1)
            request += "&";
    }
    fd = open(MYFIFO, O_WRONLY);
    write(fd, request.c_str(), request.size());
    close(fd);
}

int main(int argc , char* argv[]){

    vector<string> filenames , filters , feildsName;
    vector<string> filtered;
    string request , dir;
    int bytesRead;
    char message [BUFSIZE];
    int fd;
    bytesRead = read ( atoi(argv[1]), message, BUFSIZE);
    close (atoi(argv[1]));
    request = charTostring(message);
    parseData(request , filenames , filters , feildsName , dir);
    handelFiles(filenames , filters , feildsName , dir , filtered);
    sendDatatoPresenter(filtered , filenames , dir);

    return 0;
}