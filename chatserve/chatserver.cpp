#include"chatserver.h"
#define LOGIN 0
#define CHAT 1
#define QUIT 2
chatServer::chatServer(const char*ip,int port,int numsize):stop(false){
    sfd=socket(AF_INET,SOCK_STREAM,0);
    if(sfd<0){
        perror("socket error");
        return ;
    }
    struct sockaddr_in sin;
    sin.sin_addr.s_addr=inet_addr(ip);
    sin.sin_family=AF_INET;
    sin.sin_port=htons(port);

    if(bind(sfd,((struct sockaddr*)&sin),sizeof(sin))<0){
        perror("bind error");
        return ;
    }
    if(listen(sfd,128)<0){
        perror("listen error");
        return ;
    }
    startthreadpool(numsize);
}

void chatServer::run(){
    
    while(1){
        struct sockaddr_in cin;
        socklen_t len=sizeof(cin);
        int clinet_fd=accept(sfd,(struct sockaddr*)&cin,&len);
        if(clinet_fd<0){
            perror("accept error");
            continue;
        }
        addtask([cin,clinet_fd,this]{
            while(1){
                this->handleClient(clinet_fd,cin);
            }
        });

    }
}
void chatServer::addtask(function<void()>task){
    {
        unique_lock<mutex> lock(mtx_task);
        tasks.push(task);
    }
    task_cv.notify_one();
}
void chatServer::handleClient(int clinetfd,struct sockaddr_in cin){
    MSG msg;
    char buffer[sizeof(MSG)];

    while(1){
        int recvlen=recv(clinetfd,buffer,sizeof(buffer),0);
        if(recvlen<0){
            //表示客户下线
           auto it=client_vec.begin();
           while(it!=client_vec.end()){
                if(it->cfd==clinetfd){
                    client_vec.erase(it);
                    
                    break;
                }
                ++it;
           }
           close(clinetfd);
           break;

        }
        msg.reserialize(string(buffer,recvlen));

        switch(msg.type){
            case LOGIN:
            {
            unique_lock<mutex> lock(client_mtx);
            Client new_client;
            new_client.cfd=clinetfd;
            new_client.client=cin;
            client_vec.push_back(new_client);
            sprintf(msg.text,"-----%s加入聊天室-----",msg.name);
            broadcast(msg);
            break;
            
        }
            
            case CHAT:
            {
            unique_lock<mutex> lock(client_mtx);
            sprintf(buffer,"%s:%s",msg.name,msg.text);
            broadcast(msg,clinetfd);

        }

            case QUIT:
            {
            unique_lock<mutex> lock(client_mtx);
            sprintf(msg.text,"-----%s退出聊天室-----",msg.name);
            broadcast(msg);
            for(auto it=client_vec.begin();it!=client_vec.end();it++){
                if(it->cfd==clinetfd){
                    client_vec.erase(it);
                }
            }
            close(clinetfd);
            break;
            }
            defualt:{
                cout<<"消息类型有误"<<endl;
            }

        }

    }
}

void chatServer::broadcast(MSG&msg,int eculate){
    string data;
    data=msg.serialize();
    for(auto it=client_vec.begin();it!=client_vec.end();it++){
        if(it->cfd!=eculate){
            send(it->cfd,data.c_str(),sizeof(data),0);
        }
    }


}

void chatServer::startthreadpool(int numsize){
    for(int i=0;i<numsize;i++){
        workers.emplace_back([this]{
            while(1){
                function<void()> task;
                {
                    unique_lock<mutex>lock(mtx_task);
                    task_cv.wait(lock,[this]{
                        return stop||!tasks.empty();
                    });
                    if(tasks.empty()&&stop){
                        return ;
                    }
                    task=move(tasks.front());
                    tasks.pop();
                }
                task();
                
            }
        });
    }
}