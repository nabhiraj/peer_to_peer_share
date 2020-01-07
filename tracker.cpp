#include<iostream>
#include<list>
#include<vector>
#include<map>
#include<string>
#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<unistd.h>
#include<pthread.h>
#define NPORT 8081
using namespace std;
int output_state;
//create the printing routine and other related routine.
struct s_arg{
    int connection_fd;
    sockaddr_in connection_address;
};
int numbe_of_files;
pthread_mutex_t myconmutex=PTHREAD_MUTEX_INITIALIZER;
char* ithargument(char* msg,int i){
    char* result=new char[100];
    int t=0;
    int j=0;
    if(i==1){
        j=1;
        while(msg[j]!='\0'){
            result[t]=msg[j];
            t++;
            j++;
        }
        result[t]='\0';
    }else{
        // for ith index we have to pass i-1 null charecter.
        int count=i-1;
        while(count>0){
            if(msg[j]=='\0'){
                count--;
            }
            j++;
        }
        //this t is of our intrest;
        while(msg[j]!='\0'){
            result[t]=msg[j];
            t++;
            j++;
        }
        result[t]='\0';
    }
    return result;
}

class user{
    public:
    bool log_in;
    string userr;
    string pass;
    string ip;
    string port;
    user(string u,string p){
        userr=u;
        pass=p;
    }
    //print the state of thee user
    void u_print(){
        if(log_in){
            cout<<"the user is logged in"<<endl;
        }else{
            cout<<"the user is not loggeed in "<<endl;
        }
        cout<<"user :: "<<userr<<endl;
        cout<<"pass :: "<<pass<<endl;
        cout<<"the length of password is "<<pass.length()<<endl;
        cout<<"ip :: "<<ip<<endl;
        cout<<"port :: "<<port<<endl;
    }
};

class my{
    public:
    string user_id;
    string path;
    my(string u,string path){
        user_id=u;
        this->path=path;
    }
    void m_print(){
        cout<<"the user id is "<<user_id<<endl;
        cout<<"the path of the file is "<<path<<endl;
    }
};
map<string,vector<my*>> file_to_user;
map<string,user*> utou;
void print_every(){
    //first print the data of every user fro utou.
    //then print the content of file_to_user.
    cout<<"printing every user"<<endl;
    auto itr=utou.begin();
    for( ;itr!=utou.end();itr++){
        itr->second->u_print();
    }
    cout<<"priting the file structure"<<endl;
    auto itr2=file_to_user.begin();
    vector<my*>* t;
    for(;itr2!=file_to_user.end();itr2++){
        t=&(itr2->second);
        //now print the content of vector.
        cout<<"this information is about thee file :: "<<itr2->first<<endl;
        auto i=t->begin();
        for( ;i!=t->end();i++){
                (*i)->m_print();
        }
    }
}
user* find_user(string u){
    auto itr=utou.find(u);
    if(itr==utou.end()){
        return NULL;
    }else{
        return itr->second;
    }
}
bool is_loged(string u){
    user* temp=find_user(u);
    if(temp==NULL){
        return false;
    }else{
        if(temp->log_in){
            return true;
        }else{
            return false;
        }
    }
}
//this vector cointains the list of user
vector<my*>* file_exist(string f){
    auto itr=file_to_user.find(f);
    if(itr==file_to_user.end()){
        return NULL;
    }else{
        return &itr->second;
    }
}

//uploading the file is done
bool upload_file(string user,string file,string path){//will be used in upload.
    vector<my*>* temp=file_exist(file);
    my* m=new my(user,path);
    if(temp==NULL){//a new file
        vector<my*> tempp;
        tempp.push_back(m);
        file_to_user.insert(pair<string,vector<my*>>(file,tempp));
        numbe_of_files++;
    }else{//file is already present
        //what is user is also already present
        for(auto i=temp->begin();i!=temp->end();i++){
            if((*i)->user_id==user){
                return false;
            }
        }
        temp->push_back(m);
    }
    return true;
}

//creating a user
bool create_user(string u,string p){
    if(find_user(u)==NULL){
        user* t=new user(u,p);
        utou.insert(pair<string,user*>(u,t));
        return true;
    }else{
        return false;
    }
}

bool login_usr(string u,string pass,string ip,string port){
    if(is_loged(u)){
        return false;
    }else{
        user* t=find_user(u);
        if(t==NULL){
            return false;
        }
        if(t->pass==pass){
            t->ip=ip;
            t->port=port;
            t->log_in=true;
        }else{
            cout<<"password did not match"<<endl;
            return false;
        }
        return true; 
    }
    return true;
}
bool logout_usr(string u){
    if(is_loged(u)){
        user* t=find_user(u);
        t->log_in=false;
        return true;
    }else{
        return false;
    }
}

/*          this should be done directly there only.
char* list_all_files(){
    //:) there is no group.
    auto itr=file_to_user.begin();
    for(;itr!=file_to_user.end();itr++){

    }
}*/

//download should also be done in the thread only.

void* mkr(void* argu){
    pthread_mutex_lock(&myconmutex);
    char d='d';
    char e='e';
    int er;
    s_arg* arguments=(s_arg*)argu;
    int connectiond_fd=arguments->connection_fd;
    sockaddr_in address_of_client=arguments->connection_address;
    string ip_of_client=inet_ntoa(address_of_client.sin_addr);
    int client_port=ntohs(address_of_client.sin_port);
    cout<<"the ip address of the client is "<<ip_of_client<<endl;
    cout<<"the port number of client is "<<client_port<<endl;
    char mssag_chahe[100];
    int i=0;
    int ei=recv(connectiond_fd,mssag_chahe,100,0);
    if(ei==-1){
        cout<<"the error number is "<<errno<<endl;
    }
    char command=mssag_chahe[0];
    //here we will run the commands
    if(command=='c'){ //method is prepared
        char* a=ithargument(mssag_chahe,1);//user
        char* b=ithargument(mssag_chahe,2);//pass
        string my_user=a;
        string my_pass=b;
            if(create_user(my_user,my_pass)){
                if(output_state){
                    er=send(connectiond_fd,&d,1,0);
                    if(er<0){
                        cout<<"error in sending message"<<endl;
                    }
                }
                cout<<"user created with user id "<<my_user<<endl;
            }else{
                cout<<"error in creating user"<<endl;
                if(output_state){
                    er=send(connectiond_fd,&e,1,0);
                    if(er<0){
                        cout<<"error in sending error message"<<endl;
                    }
                }
            }
        delete[] a;
        delete[] b;
    }else if(command=='l'){//login    //method is prepared
        char* a=ithargument(mssag_chahe,1);//user
        char* c=ithargument(mssag_chahe,2);//server port
        char* b=ithargument(mssag_chahe,3);//password
        string my_user=a;
        string my_pass=b;
        string my_server_port=c;
            if(login_usr(my_user,my_pass,ip_of_client,my_server_port)){
                //successfull logiin
                if(output_state){
                    er=send(connectiond_fd,&d,1,0);
                    if(er<0){
                        cout<<"error in sending message"<<endl;
                    }
                }
                cout<<my_user<<" logged in "<<endl;
            }else{
                cout<<"unsuccesfull login"<<endl;
                //unscsessfull login
                if(output_state){
                    er=send(connectiond_fd,&e,1,0);
                    if(er<0){
                        cout<<"error in sending error message"<<endl;
                    }
                }
            }
        delete[] a;
        delete[] b;
        delete[] c;
    }else if(command=='o'){//logout   //method is prepared
        char* a=ithargument(mssag_chahe,1);//user
        string my_user;
        my_user=a;
        if(logout_usr(my_user)){
            if(output_state){
                    er=send(connectiond_fd,&d,1,0);
                    if(er<0){
                        cout<<"error in sending message"<<endl;
                    }
                }
                cout<<my_user<<" logging out"<<endl;
        }else{
            if(output_state){
                    er=send(connectiond_fd,&e,1,0);
                    if(er<0){
                        cout<<"error in sending error message"<<endl;
                    }
                }
        }
        delete[] a;
    }else if(command=='u'){//upload     //method is prepared           //needs to do some modification.
        char* a=ithargument(mssag_chahe,1);//user name.
        char* b=ithargument(mssag_chahe,2);//name of the file.
        char* c=ithargument(mssag_chahe,3);//path of the file.    //i give garbage value to this field
        string my_user=a;
        string file_name=b;
        string my_path=c;
            if(upload_file(my_user,file_name,my_path)){
                //upload succesfull
                if(output_state){
                    er=send(connectiond_fd,&d,1,0);
                    if(er<0){
                        cout<<"error in sending message"<<endl;
                    }
                }
                cout<<my_user<<" has uploaded file named "<<file_name<<endl;
            }else{
                //upload unsuccesfull
                cout<<"error occured in uploading the file "<<endl;
                if(output_state){
                    er=send(connectiond_fd,&e,1,0);
                    if(er<0){
                        cout<<"error in sending error message"<<endl;
                    }
                }
            }
        delete[] a;
        delete[] b;
        delete[] c;
        //later commands will be implimented with the files.
    }else if(command=='y'){
        if(output_state){
            int data_to_send=htons(numbe_of_files);
            er=send(connectiond_fd,&data_to_send,sizeof(data_to_send),0);
            if(er<0){
                cout<<"error in sending the number of files"<<endl;
            }
            for(auto i=file_to_user.begin();i!=file_to_user.end();i++){
                int ss=i->first.length();
                const char* sd=i->first.c_str();
                cout<<"sending .. "<<sd<<endl;
                //we also need to send the size of the string.
                int ss_h=htons(ss);
                er=send(connectiond_fd,&ss_h,sizeof(ss_h),0);
                if(er<0){
                    cout<<"error in sending the size of the string "<<endl;
                }
                //sending the size of the string is over.
                er=send(connectiond_fd,sd,ss,0);//sending the name of the file.
                if(er<0){
                    cout<<"error in sending the name of the file."<<endl;
                }
            }
            cout<<"information transfered successfully"<<endl;
        }
    }else if(command=='d'){//download       //manege here
        //user and filename.
        if(output_state){
            char* a=ithargument(mssag_chahe,1);//user name.
            char* b=ithargument(mssag_chahe,2);//name of the file.
            string u=a;
            string f_name=b;
            vector<my*>* temp_file_u_vector=file_exist(f_name);
            if(!is_loged(u)||temp_file_u_vector==NULL){
                er=send(connectiond_fd,&e,1,0);
            }else{
                er=send(connectiond_fd,&d,1,0);
                //find number of user login and having the file.
                int i;
                int user_count=0;
                for(i=0;i<temp_file_u_vector->size();i++){
                    if(is_loged((*temp_file_u_vector)[i]->user_id)){
                        user_count++;
                    }
                }
                //we got the user count.
                int data_to_send=htons(user_count);
                er=send(connectiond_fd,&data_to_send,sizeof(data_to_send),0);        //sending the number of usser.
                for(i=0;i<temp_file_u_vector->size();i++){
                    if(is_loged((*temp_file_u_vector)[i]->user_id)){
                        string t_u=(*temp_file_u_vector)[i]->user_id;
                        //send the information related to this user... i.e ip and port
                        string ip_to_send=utou.find(t_u)->second->ip;
                        string port_to_send=utou.find(t_u)->second->port;
                        int size_of_ip=ip_to_send.length();
                        int size_of_port=port_to_send.length();
                        size_of_ip=htons(size_of_ip);
                        size_of_port=htons(size_of_port);
                        const char* ipc=ip_to_send.c_str();
                        const char* pc=port_to_send.c_str();
                        //all information is retrieved...
                        er=send(connectiond_fd,&size_of_ip,sizeof(size_of_ip),0);
                        er=send(connectiond_fd,&size_of_port,sizeof(size_of_port),0);
                        er=send(connectiond_fd,ipc,ntohs(size_of_ip),0);
                        er=send(connectiond_fd,pc,ntohs(size_of_port),0);
                    }
                }
                cout<<"sending downloading information about "<<f_name<<" to "<<u<<endl;
            }
            delete[] a;
            delete[] b;
        }
    }else if(command=='s'){//stop
        char* my_user=ithargument(mssag_chahe,1);//user_if
        char* my_file=ithargument(mssag_chahe,2);//file name.
        string m_user=my_user;
        string m_file=my_file;
        vector<my*>* t = file_exist(m_file);
        if(t==NULL){
            cout<<"error in removing file "<<endl;
            send(connectiond_fd,&e,1,0);
        }else{
            bool falg=false;
            for(auto i=t->begin();i!=t->end();i++){
                if((*i)->user_id==m_user){
                    falg=true;
                    t->erase(i);
                    if(t->size()==0){
                        //now we have to erase the map entry
                        file_to_user.erase(m_file);
                    }
                    break;
                }
            }
            if(falg){
                er=send(connectiond_fd,&d,1,0);
                cout<<"succefully removed the file "<<m_file<<" for user "<<m_user<<endl;
            }else{
                cout<<"error in removing the file"<<endl;
                er=send(connectiond_fd,&e,1,0);//the user does not have that file.
            }
        }
        delete[] my_user;
        delete[] my_file;
    }else if(command=='i'){//change the output state.
        cout<<"set the output state to true"<<endl;
        output_state=true;
    }else if(command=='q'){
        //print the state of all the variables.
        cout<<"printing the state of the current machine."<<endl;
        print_every();
    }else{
        cout<<"this command is not known"<<endl;
    }
    //end of the place to run the command.
    close(connectiond_fd);
    delete arguments;
    pthread_mutex_unlock(&myconmutex);
    pthread_exit(NULL);
    return NULL;
}
int main(){
    output_state=false;
    numbe_of_files=0;
    cout<<"enter the port number for the server "<<endl;
    int serverport; //the port to which the server is listenning.
    //serverport=NPORT;
    cin>>serverport;
    int server_socker=socket(AF_INET,SOCK_STREAM,0);
    sockaddr_in ser_addr;
    ser_addr.sin_family=AF_INET;
    ser_addr.sin_port=htons(serverport);
    ser_addr.sin_addr.s_addr=INADDR_ANY;
    int e=bind(server_socker,(sockaddr*)&ser_addr,sizeof(ser_addr));
    if(e==-1){
        cout<<"error in binding the address"<<endl;
    }
    e=listen(server_socker,10);
    if(e==-1){
        cout<<"error in listening"<<endl;
    }
    

    // an experiment
    sockaddr_in temp_connection_addr;
    int temp_connection;
    int temp_connection_len;
    vector<pthread_t> thread_vector;
    pthread_t temp_thread_id;
    s_arg* my_argument;
    while(1){
       cout<<"waiting for thread connection"<<endl;
        temp_connection=accept(server_socker,(sockaddr*)&temp_connection_addr,(socklen_t*)&temp_connection_len);
        if(temp_connection<0){
            cout<<"error in accept method"<<endl;
            cout<<"error number is "<<errno<<endl;
            break;
        }
        my_argument=new s_arg();
        my_argument->connection_fd=temp_connection;
        my_argument->connection_address=temp_connection_addr;
        pthread_create(&temp_thread_id,NULL,mkr,(void*)my_argument);
        thread_vector.push_back(temp_thread_id);
    }


    //lets catch all the thread here.
    int i_s=thread_vector.size();
    int i;
    for(i=0;i<i_s;i++){
        pthread_join(thread_vector[i],NULL);
    }

    // end of my experimenet
    
    return 0;
}