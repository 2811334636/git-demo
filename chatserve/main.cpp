#include"chatserver.h"

using namespace std;
int main(int argc,const char*argv[]){
    if(argc<3){
        cout<<"蠢猪，输入ip地址和端口号"<<endl;
    }
    chatServer server(argv[1],atoi(argv[2]),5);
    server.run();
//aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
//bbbbbbb
    return 0;
}