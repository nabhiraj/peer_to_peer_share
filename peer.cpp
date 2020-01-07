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
#include<algorithm>
#include<fcntl.h>
#include <cmath>
#include<semaphore.h>
#define PACKT_SIZE 100
#define CHUNK_SIZE_B 524288
#define PAR_DOWN 8
using namespace std;

//packet cointains the messagee
int port_of_server_part;
string current_user;
bool dpeu_flag=false;   //download pe upload flag.
//our semaphore for downloading.
sem_t my_counting_semaphore;

//----------------- structure for maintaining downloading --------------------
class permanet_store{
    public:
    string file_path;
    int file_size;//in bytes
    string bit_map;
    permanet_store(string fp,int fs,string bm){
        file_path=fp;
        file_size=fs;
        bit_map=bm;
    }
    void print_me(){
        cout<<"file path :: "<<file_path<<endl;
        cout<<"file size :: "<<file_size<<endl;
        cout<<"bit_map :: "<<bit_map<<endl;
    }
};
class download_space{ //alocal array for this will be enough.
    public:
    string ip;
    int port;
    int size_of_file; //will be querry form client.
    string bit_map;   //will be querry from client.
    //constructor
    download_space(string ip,string port){
        this->ip=ip;
        this->port=atoi(port.c_str());
        size_of_file=0;
    }
    void print_me(){
        cout<<"ip :: "<<ip<<endl;
        cout<<"port :: "<<port<<endl;
        cout<<"size_of_file :: "<<size_of_file<<endl;
        cout<<"bit map ::"<<bit_map<<endl;
    }
};
map<string,permanet_store> file_stored;
//-----------------------------------------------------------------------------
void print_every(){
    cout<<"the user currently logged in is "<<current_user<<endl;
    cout<<"printing all the files uploaded in the system"<<endl;
    for(auto i=file_stored.begin();i!=file_stored.end();i++){
        cout<<"name :: "<<i->first<<endl;
        i->second.print_me();
    }
}
void download_space_arr_p(vector<download_space>* arr){                             //my printing routine.
    int i;
    for(i=0;i<arr->size();i++){
        (*arr)[i].print_me();
    }
}
//-----------------------------------------------------------------------------------
string itoa(int x){
    string temp;
    while(x){
        //temp.append((char)(x%10+48));
        temp.push_back(x%10+48);
        x=x/10;
    }
    reverse(temp.begin(),temp.end());
    return temp;
}
bool send_no_reply_packet(char* packet,sockaddr_in tar_addr){
    bool r;
    int c_socket=socket(AF_INET,SOCK_STREAM,0);
    connect(c_socket,(sockaddr*)&tar_addr,sizeof(tar_addr));
    int e=send(c_socket,packet,PACKT_SIZE,0);
    if(e==-1){
        cout<<"error in sending packer"<<endl;
    }
    char result;
    recv(c_socket,&result,1,0);
    if(result=='d'){
        cout<<"operation completed successfully"<<endl;
        r=true;
    }else{
        cout<<"error in completing the operation"<<endl;
        r=false;
    }
    close(c_socket);
    return r;
}
//all message cointain 2 or 3 arguments.
//therfore we will be useing overloaded methods.
void messge_creater(char* buf,char com,string a,string b){
    buf[0]=com;
    int i=1;
    int j;
    for(j=0;j<a.length();j++){
        buf[i]=a[j];
        i++;
    }
    buf[i]='\0';
    i++;
    for(j=0;j<b.length();j++){
        buf[i]=b[j];
        i++;
    }
    buf[i]='\0';
}
void messge_creater(char* buf,char com,string a,string b,string c){
    buf[0]=com;
    int i=1;
    int j;
    for(j=0;j<a.length();j++){
        buf[i]=a[j];
        i++;
    }
    buf[i]='\0';
    i++;
    for(j=0;j<b.length();j++){
        buf[i]=b[j];
        i++;
    }
    buf[i]='\0';
    i++;
    for(j=0;j<c.length();j++){
        buf[i]=c[j];
        i++;
    }
    buf[i]='\0';
}
struct file_download_data{
    string ip;
    int port;
    string name_of_file;
    string my_destination_path;
    int seek_start;
    int seek_end;
};
pthread_mutex_t mymutex=PTHREAD_MUTEX_INITIALIZER;
void* downloading_thread(void* argu){
    //lets put a mutex in this method.
    //pthread_mutex_lock(&mymutex);
    sem_wait(&my_counting_semaphore);
    //cout<<"aquired the lock"<<endl;
  //  cout<<"starting the thread"<<endl;
    file_download_data* myd=(file_download_data*)argu;
    //here comes all the logic.
    char* msg=new char[100];
    int er;
    messge_creater(msg,'c',myd->name_of_file,itoa(myd->seek_start),itoa(myd->seek_end));
   // cout<<"messgae is created"<<endl;
    int sfd=socket(AF_INET,SOCK_STREAM,0);
    if(sfd<0){
        cout<<"erro in creating the socket"<<endl;
    }
    //creating the address.
    sockaddr_in this_address;
    this_address.sin_family=AF_INET;
    this_address.sin_port=htons(myd->port);
    this_address.sin_addr.s_addr=inet_addr(myd->ip.c_str());
    //address created
    er=connect(sfd,(sockaddr*)&this_address,sizeof(this_address));
    if(er<0){
        cout<<"erro in connecting to the socket"<<endl;
    }
    er=send(sfd,msg,100,0);
    if(er<0){
        cout<<"error in sending the data"<<endl;
        cout<<errno<<endl;
    }
    //now we have recive the real data.
    char* data=new char[(myd->seek_end)-(myd->seek_start)];


    //er=recv(sfd,data,(myd->seek_end)-(myd->seek_start),0);
    //we have to reciev this data in small parths.
    int k=0;
    int count=(myd->seek_end)-(myd->seek_start);
    while(count>8*1024){
        er=recv(sfd,data+k,8*1024,0);
        if(er<0){
            cout<<"error in reciving "<<endl;
        }
        count=count-8*1024;
        k=k+8*1024;
    }
    if(count>0){
        er=recv(sfd,data+k,count,0);
        if(er<0){
            cout<<"error in reciving 2 "<<endl;
        }
        count=0;
    }

    if(er<0){
        cout<<"error in reciving the messge"<<endl;
    }
    //now write this data into target file
    FILE* myfile=fopen(myd->my_destination_path.c_str(),"r+");
    fseek(myfile,myd->seek_start,SEEK_SET);
    //cout<<"inisial value of fseek is "<<ftell(myfile)<<endl;
    er=fwrite(data,1,(myd->seek_end)-(myd->seek_start),myfile);
    //cout<<"final value of feek at right "<<ftell(myfile)<<endl;
    if(er<0){
        cout<<"error in writing to the file "<<endl;
    }
    fclose(myfile);
    delete[] data;
    delete[] msg;
    //cout<<"about to leve the lock"<<endl;
    sem_post(&my_counting_semaphore);
    //pthread_mutex_unlock(&mymutex);
    pthread_exit(NULL);
    return NULL;
}
void* perfect_c(void* v){
    string ip_of_t;
    int port_of_t;
    vector<download_space> my_downloadspace;
    //later we will do file handeling----------------------------------
    cout<<"enter the ip of tracker"<<endl;
    cin>>ip_of_t;
    cout<<"port of tracker"<<endl;
    cin>>port_of_t;
    //---------------------------------------------------------------
    //now write the client.
    //int c_socket=socket(AF_INET,SOCK_STREAM,0); //we will create a new connection for every new command
    //create the address.
    sockaddr_in my_ser_addr;
    my_ser_addr.sin_port=htons(port_of_t);
    my_ser_addr.sin_family=AF_INET;
    my_ser_addr.sin_addr.s_addr=inet_addr(ip_of_t.c_str());
    int command_connection;
    int er;
    //address is set
    cout<<"the client is ready to work."<<endl;
    char msg_buf[100];
    char temp_data;
    string comm;
    //sending the input for making output state true
    command_connection=socket(AF_INET,SOCK_STREAM,0);
    er=connect(command_connection,(sockaddr*)&my_ser_addr,sizeof(my_ser_addr));
    if(er<0){
        cout<<"error in creating the connection"<<endl;
    }
    temp_data='i';
    er=send(command_connection,&temp_data,1,0);
    if(er<0){
        cout<<"error in sending the data"<<endl;
    }
    //command i does not expect andy return value.
    close(command_connection);
    cout<<"start entering command from here"<<endl;
    //--------------------------------------------
    while(1){
        cin>>comm;
        if(comm=="create_user"){
            //userid and password
            //code used is c
            cout<<"enter userid"<<endl;
            string u;
            string p;
            cin>>u;
            cout<<"enter the password"<<endl;
            cin>>p;
            messge_creater(msg_buf,'c',u,p);
            send_no_reply_packet(msg_buf,my_ser_addr);
        }else if(comm=="loggin_user"){
            //user id    port of server part   password
            //code used is l
            //port_of_server_part
            string u;
            string p;
            string my_port;
            cout<<"enter userid"<<endl;
            cin>>u;
            cout<<"enter the password"<<endl;
            cin>>p;
            //setting value to my port
            my_port=itoa(port_of_server_part);
            cout<<"actual port is "<<port_of_server_part<<endl;
            cout<<"the port which is converted by me is "<<my_port<<endl;
            messge_creater(msg_buf,'l',u,my_port,p);
            if(send_no_reply_packet(msg_buf,my_ser_addr)){
                current_user=u;
            }
        }else if(comm=="logout"){
            //user id.
            //code used is o
            messge_creater(msg_buf,'o',current_user," ");
            if(send_no_reply_packet(msg_buf,my_ser_addr)){
                current_user="";
            }
        }else if(comm=="upload"){           //this needs to change littl bit.
            //user, name of file    path of file
            //code is u
            cout<<"enter the name of the file"<<endl;
            string name_f;
            cin>>name_f;
            cout<<"enter the path of the file"<<endl;
            string path_f;
            cin>>path_f;
            messge_creater(msg_buf,'u',current_user,name_f,path_f);
            int fd=open(path_f.c_str(),O_RDONLY);
            if(send_no_reply_packet(msg_buf,my_ser_addr)&&fd>0){
                //we need to put the path in our structure. //no need to do sanity check.
                //we need to do file preprocessing.
                //int fd=open(path_f.c_str(),O_RDONLY);
                    int my_file_size=lseek(fd,0,SEEK_END);
                    close(fd);
                    int num_of_chunks=int(ceil((double)my_file_size/CHUNK_SIZE_B));
                    string bit_map;
                    cout<<"num of chunks "<<num_of_chunks<<endl;
                    for(int i=0;i<num_of_chunks;i++){
                        bit_map.push_back('1');
                    }
                    permanet_store p(path_f,my_file_size,bit_map);
                    //file_stored[name_f]=p; //we have to insert by hand.
                    file_stored.insert(pair<string,permanet_store>(name_f,p));//fingers crossed.
                    cout<<"printing everything "<<endl;
                    print_every();
                
            }else{
                cout<<"error in uploading"<<endl;
            }
        }else if(comm=="list_file"){ //good communication requred               //now only good work is left.
            //list all the file in the tracker
            //code is y
            messge_creater(msg_buf,'y'," "," ");
            //we have to create our own socket
            command_connection=socket(AF_INET,SOCK_STREAM,0);
            if(command_connection<0){
                cout<<"error in creating the socket"<<endl;
            }
            er=connect(command_connection,(sockaddr*)&my_ser_addr,sizeof(my_ser_addr));
            send(command_connection,msg_buf,100,0);
            int number_of_files;
            er=recv(command_connection,&number_of_files,sizeof(number_of_files),0);
            number_of_files=ntohs(number_of_files);
            cout<<"number of files is "<<number_of_files<<endl;
            int i;
            for(i=0;i<number_of_files;i++){
                //first recv the size of the string.
                int ss;
                er=recv(command_connection,&ss,sizeof(ss),0);
                ss=ntohs(ss);
                char* target_top=new char[ss+1];
                er=recv(command_connection,target_top,ss,0);
                cout<<target_top<<endl;
            }
            cout<<"all files printed"<<endl;
            close(command_connection);
        }else if(comm=="download_file"){//good communication required
            //user file
            //code is d
            if(current_user!=""){
                string f;//this is the file_name.
                string my_complete_path;
                cout<<"enter the name of the file"<<endl;
                cin>>f;
                cout<<"enter the complete path where the file should be stored in this machine"<<endl;
                cin>>my_complete_path;
                command_connection=socket(AF_INET,SOCK_STREAM,0);
                if(command_connection<0){
                    cout<<"error in creating the socket "<<endl;
                }
                er=connect(command_connection,(sockaddr*)&my_ser_addr,sizeof(my_ser_addr));
                messge_creater(msg_buf,'d',current_user,f);
                er=send(command_connection,msg_buf,100,0);
                char r;
                er=recv(command_connection,&r,1,0);
                if(r=='e'){
                    cout<<"the requested is not constructed correctly"<<endl;
                }else if(r=='d'){
                    dpeu_flag=true;  ///making the flag as true.

                    char re_ip[100];
                    char re_port[100];
                    int active_clients;
                    er=recv(command_connection,&active_clients,sizeof(active_clients),0);
                    active_clients=ntohs(active_clients);
                    int i;
                    for(i=0;i<active_clients;i++){
                        int size_of_ip;
                        int size_of_port;
                        er=recv(command_connection,&size_of_ip,sizeof(size_of_ip),0);                     //extracted information from tracker
                        size_of_ip=ntohs(size_of_ip);
                        er=recv(command_connection,&size_of_port,sizeof(size_of_port),0);
                        size_of_port=ntohs(size_of_port);
                        er=recv(command_connection,re_ip,size_of_ip,0);
                        er=recv(command_connection,re_port,size_of_port,0);
                        download_space d(re_ip,re_port);
                        my_downloadspace.push_back(d);
                    }



                    //now we will talk to indiviual clints 
                    int tt_soc;
                    sockaddr_in tt_addr;
                    string target_ip;
                    int target_port;
                    cout<<"number of logged in client with the file is "<<active_clients<<endl;
                    for(i=0;i<active_clients;i++){
                        target_ip=my_downloadspace[i].ip;
                        target_port=my_downloadspace[i].port;
                        cout<<"target ip is"<<target_ip<<endl;
                        cout<<"target port is "<<target_port<<endl;
                        tt_soc=socket(AF_INET,SOCK_STREAM,0);
                        if(tt_soc<0){
                            cout<<"error in connection with the socket"<<endl;
                        }
                        //construction the address
                        tt_addr.sin_family=AF_INET;
                        tt_addr.sin_addr.s_addr=inet_addr(target_ip.c_str());
                        tt_addr.sin_port=htons(target_port);
                        er=connect(tt_soc,(sockaddr*)&tt_addr,sizeof(tt_addr));
                        if(er<0){
                            cout<<"error is happening at the time of connect"<<endl;
                        }
                        //communication starts here...
                        messge_creater(msg_buf,'v',f,"");
                        er=send(tt_soc,msg_buf,100,0);
                        if(er<0){
                            cout<<"error in sending the message"<<endl;
                            cout<<"the error number is "<<errno<<endl;
                        }
                        //now we will just recive the data.
                        int ss_of_file;
                        er=recv(tt_soc,&ss_of_file,sizeof(ss_of_file),0);                          //extracted information from other client.
                        if(er<0){
                            cout<<"error in recv a the message"<<endl;
                            cout<<"the error number is "<<errno<<endl;
                        }
                        ss_of_file=ntohl(ss_of_file);//hope it work crossing th finger
                        int ll_of_bitmap;
                        er=recv(tt_soc,&ll_of_bitmap,sizeof(ll_of_bitmap),0);
                        if(er<0){
                            cout<<"error in recv b the message"<<endl;
                            cout<<"the error number is "<<errno<<endl;
                        }
                        ll_of_bitmap=ntohs(ll_of_bitmap);
                        //now we will recive the bitmap.
                        char* tt_bitmap=new char[ll_of_bitmap];
                        er=recv(tt_soc,tt_bitmap,ll_of_bitmap,0);
                        if(er<0){
                            cout<<"error in recv c the message"<<endl;
                            cout<<"the error number is "<<errno<<endl;
                        }
                        //now we need to place the things in there place.
                        my_downloadspace[i].bit_map=tt_bitmap;
                        my_downloadspace[i].size_of_file=ss_of_file;
                        //communication ends here.
                        close(tt_soc);
                    }
                    //cout<<"printing the state after clien communication"<<endl;
                    //print_every();
                    //cout<<"printing download array"<<endl;
                    //download_space_arr_p(&my_downloadspace);
                    //cout<<"printintg done"<<endl;


                                                                                                                
                    //do littl debugging printing before executing ...
                    int number_of_chunks=my_downloadspace[0].bit_map.length();
                    int d_file_size=my_downloadspace[0].size_of_file;
                    int current_chunk=0;
                    int s_start=0;
                    int s_end=0;
                    int current_client=0;
                    char* commin_data=NULL;
                    FILE* target_file;
                    char a='0';
                    //we have to create a file of required size
                    /*
                    cout<<"creating the file of requred size"<<endl;
                    target_file=fopen(my_complete_path.c_str(),"w");
                    for(i=0;i<d_file_size;i++){
                        er=fwrite(&a,1,1,target_file);
                    }
                    fclose(target_file);
                    */
                    target_file=fopen(my_complete_path.c_str(),"w");
                    fclose(target_file);
                    //cout<<"done with creating the file"<<endl;                                                 //actual transfer needs to be done.
                    vector<pthread_t> myvv;
                    //cout<<"number of active seeders is "<<active_clients<<endl;
                    while(current_chunk<number_of_chunks){
                        if(current_chunk==number_of_chunks-1){//case for the last chunk
                            //cout<<"the last chunk"<<endl;
                            s_start=current_chunk*512*1024;                                                  
                            s_end=my_downloadspace[0].size_of_file;
                        }else{//normal case
                            //cout<<"not the last chunk"<<endl;
                            s_start=current_chunk*512*1024;
                            s_end=(current_chunk+1)*512*1024;
                        }
                        if(my_downloadspace[current_client].bit_map[current_chunk]=='1'){
                            //cout<<"the client have the chunk"<<endl;
                            //now to thread we need to give following things 
                            //ip and port where we have to communicate
                            //name of the file which we need to get
                            //target destination of the file
                            //seek value of the file.
                            //cout<<"inisializing the thread"<<endl;
                            file_download_data* arg=new file_download_data();
                            arg->ip=my_downloadspace[current_client].ip;
                            arg->port=my_downloadspace[current_client].port;
                            arg->name_of_file=f;
                            arg->my_destination_path=my_complete_path;
                            arg->seek_start=s_start;
                            arg->seek_end=s_end;
                            pthread_t temp;
                            //cout<<"declared all the variables"<<endl;
                            pthread_create(&temp,NULL,downloading_thread,arg); //executin downloading thread from here.
                            //cout<<"generated thee thread"<<endl;
                            myvv.push_back(temp);
                            //cout<<"pushed th thread id in the vector"<<endl;
                            //now we need to start the thread.

                            //till here we have succesfully downloaded the at least one chunk 










                            if(dpeu_flag==true){                                                                            //creting the structure and uploading in trackr.
                                dpeu_flag=false;
                              //  cout<<"creting the structure"<<endl;
                                messge_creater(msg_buf,'u',current_user,f,my_complete_path);
                                if(send_no_reply_packet(msg_buf,my_ser_addr)){
                                    string bit_map;
                                    int num_of_chunks=my_downloadspace[current_client].bit_map.length();
                                    int myfsize=my_downloadspace[current_client].size_of_file;
                                    for(int i=0;i<num_of_chunks;i++){
                                        bit_map.push_back('0');
                                    }
                                    permanet_store p(my_complete_path,myfsize,bit_map);
                                    file_stored.insert(pair<string,permanet_store>(f,p));//fingers crossed.
                                }else{
                                    cout<<"error in uploading from download area"<<endl;
                                }
                            }
                            //cout<<"updating the structure"<<endl;
                            file_stored.find(f)->second.bit_map[current_chunk]='1';//updating the structure 
                            //update structure in client.
                            //setting upload information here













                            current_client=(current_client+1)%active_clients;
                            current_chunk++;
                        }else{
                            //cout<<"the client does not have the chunk"<<endl;
                            current_client=(current_client+1)%active_clients;
                        }

                    }
                    //all the threads have to join here.
                    int j;
                    for(j=0;j<myvv.size();j++){
                        pthread_join(myvv[j],NULL);
                    }
                    myvv.clear();
                    my_downloadspace.clear();
                    cout<<"done with downloading"<<endl;
                }else{
                    cout<<"something went wrong"<<endl;
                }
                close(command_connection);
                //now work with other clients and extract information from them.
                //my_download_space is the vector of intrest.
            }else{
                cout<<"please loging first "<<endl;
            }
        }else if(comm=="give_log"){
            //server will print the log
            //code is q.
            cout<<"requesting server to print the log"<<endl;
            command_connection=socket(AF_INET,SOCK_STREAM,0);
            er=connect(command_connection,(sockaddr*)&my_ser_addr,sizeof(my_ser_addr));
            if(er<0){   
                cout<<"error in creating the connection"<<endl;
            }
            temp_data='q';
            er=send(command_connection,&temp_data,1,0);
            if(er<0){
                cout<<"error in sending the data"<<endl;
            }
            //command i does not expect andy return value.
            close(command_connection);
        }else if(comm=="stop_sharing"){
            //cout<<"code for stop sharing a file "<<endl;
            string mm_file;
            cout<<"enter the name of the file"<<endl;
            cin>>mm_file;
            messge_creater(msg_buf,'s',current_user,mm_file);
            //does we have this file.
            auto ii=file_stored.find(mm_file);
            if(ii==file_stored.end()){
                cout<<"this file does not exist"<<endl;
            }else{
                file_stored.erase(ii->first);
                send_no_reply_packet(msg_buf,my_ser_addr);
                cout<<"file file is no longer beging shared from your system"<<endl;
            }
        }else{
            cout<<"this command is not known."<<endl;
        }

        

    }
    pthread_exit(NULL);
    return NULL;
}


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



struct argu2{
    string ip;
    int port;
    int socket_connectionid;
};

void* my_server_thread(void* o){
    //cout<<"is control reaching on the server thread"<<endl;
    argu2* oo=(argu2*)o;
    string client_ip=oo->ip;
    int client_port=oo->port;
    int connection_id=oo->socket_connectionid;
    //now the code of clients server part will come here.
    char msg[100];
    int er;
    er=recv(connection_id,msg,100,0);
    if(er<0){
        cout<<"error in recive method"<<endl;
        close(connection_id);
        return NULL;
    }
    char command=msg[0];
    if(command=='v'){//just asking for vector.
        //takes only one input name of the file.
        //cout<<"inide the vector command"<<endl;
        char* file_name=ithargument(msg,1);
        string f_name=file_name;
        //now the task is to return the vector and size of the file. by searching from the permanent structure.
        permanet_store* p=&(file_stored.find(f_name)->second);
        string temp_bitmap=p->bit_map;
        int temp_sizze=p->file_size;
        //first we will send the file size
        //then clustr size i.e number of element in the bitmap
        //then we will send the bitmap.
        int data_to_send;
        data_to_send=htonl(temp_sizze);
        //cout<<"the value of size sent from server part is "<<temp_sizze<<endl;
        er=send(connection_id,&data_to_send,sizeof(data_to_send),0);
        data_to_send=htons(temp_bitmap.length());
        er=send(connection_id,&data_to_send,sizeof(data_to_send),0);
        const char* vec=temp_bitmap.c_str();
        er=send(connection_id,vec,temp_bitmap.length(),0);
        delete[] file_name;
    }else if(command=='c'){//wants the chunk.
        //takes three argument name of the file and seek start and seek end value.
        char* f_name=ithargument(msg,1);//name of file.  //this is not name this is the path.
        char* myseek_start=ithargument(msg,2);
        char* myseek_end=ithargument(msg,3);
        string myfile=f_name;
        int s_start=atoi(myseek_start);
        int s_end=atoi(myseek_end);
        //need to find the orignam path with the help of this name.
        string file_path=file_stored.find(f_name)->second.file_path;
        //we can also be sucking here.
        cout<<"sending the chunk from "<<s_start<<" to "<<s_end<<endl;
        char* t_buf=new char[512*1024+1];
        FILE* f=fopen(file_path.c_str(),"r");
        er=fseek(f,s_start,SEEK_SET);
        if(er<0){
            cout<<"erro in fseek"<<endl;
        }
        //cout<<"actual inisial value of fseek is "<<ftell(f)<<endl;
        er=fread(t_buf,sizeof(char),s_end-s_start,f);
        //cout<<"actual final value of fseek is "<<ftell(f)<<endl;
        if(er<0){
            cout<<"error in reading the file."<<endl;
        }


        //now we have to send the content of t_buf in 8KB one by one.
        int count=s_end-s_start;
        int k=0;
        while(count>8*1024){
            er=send(connection_id,t_buf+k,8*1024,0);
            if(er<0){
                cout<<"error in sending the data"<<endl;
            }
            count=count-8*1024;
            k=k+8*1024;
        }
        if(count>0){
            er=send(connection_id,t_buf+k,count,0);
            if(er<0){
                cout<<"error in sending the data"<<endl;
            }
            count=0;
        }
        //i think data has been sent.

        //er=send(connection_id,t_buf,s_end-s_start,0);
        if(er<0){
            cout<<"error in sending the actual data"<<endl;
        }
        fclose(f);
        //close(fd);
        //close(jc);
        delete[] t_buf;
        delete[] f_name;
        delete[] myseek_end;
        delete[] myseek_start;
    }else{
        cout<<"this command is not known man"<<endl;
    }
    close(connection_id);
    delete oo;
    pthread_exit(NULL);
    return NULL;
}
//almost no printing is done on the server side.
int main(){
    //int port_of_server_part;
    //ask for the server port from the user only.
    //inisializing the semaphore.
    sem_init(&my_counting_semaphore,0,PAR_DOWN);
    cout<<"enter the port of server part"<<endl;
    cin>>port_of_server_part;
    pthread_t client_s;
    int e=pthread_create(&client_s,NULL,perfect_c,NULL);//running the client part of the client
    if(e!=0){
        cout<<"error in creating the thread"<<endl;
    }
    //now write the server part of the client
    cout<<"running the server part."<<endl;
    int server_part_connection=socket(AF_INET,SOCK_STREAM,0);
    if(server_part_connection<0){
        cout<<"error in creating the socket"<<endl;
    }
    sockaddr_in c_server_addr;
    c_server_addr.sin_family=AF_INET;
    c_server_addr.sin_port=htons(port_of_server_part);
    c_server_addr.sin_addr.s_addr=INADDR_ANY;
    e=bind(server_part_connection,(sockaddr*)&c_server_addr,sizeof(c_server_addr));
    if(e<0){
        cout<<"error in binding with the port and ip"<<endl;
    }
    e=listen(server_part_connection,10);
    if(e<0){
        cout<<"error in listening the connection"<<endl;
    }
    //now accecpt the connection and launch a new thread.
    sockaddr_in len_addr;
    int len;
    pthread_t temp_thread;
    argu2* t_arg;
    while(1){
        cout<<"waiting for the connection"<<endl;
        e=accept(server_part_connection,(sockaddr*)&len_addr,(socklen_t*)&len);
        if(e<0){
            cout<<"error in accepting the connection"<<endl;
        }else{
            cout<<"accepting a new connection"<<endl;
        }
        t_arg=new argu2();
        t_arg->ip=inet_ntoa(len_addr.sin_addr);
        t_arg->port=ntohs(len_addr.sin_port);
        t_arg->socket_connectionid=e;
        cout<<"creating a new thread"<<endl;
        pthread_create(&temp_thread,NULL,my_server_thread,(void*)t_arg);
    }

    //may be if i join the thread things will become normal.
    pthread_join(client_s,NULL);//wait for client.
    //destroying the semaphore
    sem_destroy(&my_counting_semaphore);
    return 0;
}