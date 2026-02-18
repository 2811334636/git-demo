#include<myheader.h>
#include <vector> //用于存储多个线程
#include <thread> //线程的创建及管理
#include <queue> //用于存储多个任务的队列
#include <condition_variable> //条件变量头文件
#include <mutex> //互斥锁头文件
#include <functional>
using namespace std;
class chatServer{
    public:
    struct MSG{
        int type;
        char text[128];
        char name[20];

        string serialize(){
        string data;
        data.append(reinterpret_cast<char*>(&type),sizeof(type));
        data.append(reinterpret_cast<char*>(name),sizeof(name));
        data.append(reinterpret_cast<char*>(text),sizeof(text));
        return data;
        }

        void reserialize(const string &data){
            size_t offset=0;
            memcpy(&type,&data+offset,sizeof(type));
            offset+=sizeof(type);
            memcpy(&name,&data+offset,sizeof(name));
            offset+=sizeof(name);
            memcpy(&text,&data+offset,sizeof(text));


        }
    };
    struct Client{
        int cfd;
        struct sockaddr_in client;
    };

    void run();
    chatServer(const char*ip,int port,int numsize);
    ~chatServer();
    void handleClient(int clinetfd,struct sockaddr_in cin);
    void broadcast(MSG&msg,int eculate=-1);

    private:
    mutex mtx_task;
    vector<thread>workers;
    queue<function<void()>>tasks;
    condition_variable task_cv;
    bool stop;
    
    int sfd;
    vector<Client>client_vec;
    mutex client_mtx;

    void errlog(const char*msg);
    void addtask(function<void()>task);
    void startthreadpool(int numsize);


};