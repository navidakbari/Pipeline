#include <iostream>
#include <fstream>
#include <stdio.h> 
#include <string> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sstream>
#include <sys/types.h> 
#include <unistd.h>
#include <vector>

using namespace std;

#define MYFIFO "/tmp/myfifo"
#define BUFSIZE 2048

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

void setData(string request , string &sortingValue , string &sortType , int &prc_cnt){
    vector<string> arguments;
    splitByampersan(request , arguments);
    if(arguments.size() > 1){
        prc_cnt = atoi(arguments[0].c_str());
        sortingValue = arguments[1];
        sortType = arguments[2];
    }else{
        prc_cnt = atoi(arguments[0].c_str());
    }
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

void sort(vector<string> answer , string sortingValue , string header){
    int column = findColumn(header , sortingValue);
    //sort(answer.begin() , answer.end() , compareInterval());   
}

void getDatafromWorker(int prc_cnt , vector<string> &answer , string sortType , string sortingValue , string &header){
    int count = 0;
    string data;
    vector<string> datas;

    for(int i = 0 ; i < prc_cnt ; i++){
        datas.clear();
        ifstream file(MYFIFO);
        getline(file , data);
        splitByampersan(data , datas);
        header = datas[0];
        for(int j = 0 ; j < datas.size() ; j++){
            if(j != 0)
                answer.push_back(datas[j]);
        }
        file.close();
    }
}

void showData(vector <string> answer , string sortType){
    if(sortType == "descend")
        for(int i = answer.size() - 1 ; i >= 0; i--)
            cout << answer[i] << endl;
    else
        for(int i = 0 ; i < answer.size(); i++)
            cout << answer[i] << endl;
}

int main(){ 
    
    string sortingValue , sortType , header;
    int prc_cnt , fd;
    string request;
    vector <string> answer;
    
    ifstream file(MYFIFO);
    getline(file , request);
    setData(request , sortingValue , sortType , prc_cnt);
    file.close();
    getDatafromWorker(prc_cnt , answer , sortType , sortingValue , header);
    //sort(answer);
    showData(answer , sortType);

    return 0; 
}