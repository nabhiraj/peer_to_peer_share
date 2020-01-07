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
#define NPORT 8081   //assuming port will be fixed.
using namespace std;

//don't rely on serialization send data with hand.
//all of these data should have there own mutex because multiple thread can disrupt there functionality.
//thinking about the mutex strucuture of the code.............
bool is_active=false;
bool state=false;
class groups;
class user;
class file_user;
map<string,user*>* userid_to_user;
map<string,groups*>* groupid_to_group;
map<string,list<string>>* userid_to_groupids;
map<string,list<string>>* groupid_to_userids;
//printing all the user and thrier information.
char* ithargument(char* msg,int i);

class file_user{
    public:
        string name;
        string user_id;
        string path;
        bool is_file_complete;
        vector<bool>* parts;//with part of file is present.
        vector<int>* seek_start;
        vector<int>* seek_stop;
      //  vector<char[20]> hash;//this needs to be confirmed what to do.    //will deal later
        int number_of_parts_present;
        int total_number_of_parts;
        //constructor will be created later.
};
class groups{
    public:
    string group_id;
    string group_leader;
    int number_of_members;
    map<string,file_user*>* name_to_files; //name to file mapping for the group
    list<string>* join_requests;
    //group will also cointain request
    groups(string id,string leader){
        group_id=id;
        group_leader=leader;
        number_of_members=0;
        join_requests=new list<string>();
    }
    void show_state(){
        cout<<"group_id :: "<<group_id<<endl;
        cout<<"group leader :: "<<group_leader<<endl;
        cout<<"number of elements :: "<<number_of_members<<endl;
        cout<<"information about file is not displaying right now."<<endl;
        cout<<"the user having join now request is as follow"<<endl;
        //doing some smart work.
        char* s_t=people_want_to_join();
        int i;
        for(i=0;i<join_requests->size();i++){
            cout<<ithargument(s_t,i+1);
        }
    }
    char* people_want_to_join(){        // return the people who want to join in our special format.
        char* temp=new char[join_requests->size()*15];
        temp[0]='x';
        int i=1;
        list<string>::iterator itr;
        for(itr=join_requests->begin();itr!=join_requests->end();itr++){
            string temp2=*itr;
            int j;
            for(j=0;j<temp2.length();j++){
                temp[i]=temp2[j];
                i++;
            }
            temp[i]='\0';
            i++;
        }
        return temp;
    }
    void add_join_req(string req){
        //first we have to check wheter this request exist or not.
        list<string>::iterator itr;
        for(itr=join_requests->begin();itr!=join_requests->end();itr++){
            if(*itr==req){
                return;
            }
        }
        //now we will insert
        join_requests->push_back(req);
    }
    bool accept_req(string req){//five true is succesfull else gives false;
        //first we hapve to find the request.
        list<string>::iterator itr;
        //bool flag=true;
        cout<<"outside th for loop"<<endl;
        for(itr=join_requests->begin();itr!=join_requests->end();itr++){
            cout<<"inside the for loop"<<endl;
            if(*itr==req){
                cout<<"inside the if condition"<<endl;
                //found th iterator
               // flag=false;
                itr->erase();
                cout<<"erased th eelement from requesst list"<<endl;
                //now we need to insert this in actual group.
                //we need to update two data structure.
                //  1. userif_to_group
                //  2. group_to_user
                // we also have increment the number of memebers in the group
                number_of_members++;
                map<string,list<string>>::iterator ug_itr=userid_to_groupids->find(req);
                cout<<"created the iterator and found th list to bee updated 1"<<endl;
                ug_itr->second.push_back(group_id);//error is here
                cout<<"is this printing"<<endl;
                ug_itr=groupid_to_userids->find(group_id);
                cout<<"created the iterator and found the list to be updated 2"<<endl;
                ug_itr->second.push_back(req);
                return true;
            }
        }
        return false;
    }
};
class user{
    public:
        string user_id;
        string port;
        string ip; //ip will be loaded at the time of login. not at the time of creation.
        string passwd;
        bool is_active;
        bool is_group_leader;
        list<string>* owned_group;//group to which this user is the leader
        list<file_user>* files;//file object hold by the user
        //constructor and other methods.
        user(string u,string p){
            user_id=u;
            passwd=p;
            is_active=false;
            is_group_leader=false;
            owned_group=new list<string>();
            files=new list<file_user>();
        }
        void show_state(){
            cout<<"user_id :: "<<user_id<<endl;
            cout<<"password is :: "<<passwd<<endl;
            cout<<"port :: "<<port<<endl;
            cout<<"ip :: "<<ip<<endl;
            if(is_active){
                cout<<"the user is logged in"<<endl;
            }else{
                cout<<"the user is logged out"<<endl;
            }
            if(is_group_leader){
                cout<<"the user is group leader"<<endl;
                cout<<"it is group leader of the following groups."<<endl;
                int i;
                list<string>::iterator itr;
                for(itr=owned_group->begin();itr!=owned_group->end();itr++){
                    cout<<*itr<<endl;
                }
            }else{
                cout<<"not a group leader"<<endl;
            }
            cout<<"files list not showing for now."<<endl;
        }
        void add_his_group(string g){
            owned_group->push_back(g);
            is_group_leader=true;
        }
        void add_his_file(file_user f){
            files->push_back(f);
        }
        bool leave_grp(string grp){
            //first we have to check grp belong to owned_grp if yes delete the group
            //when deleting the group we have remove the group from gtou and utog(difficult)
            //else just delete the entry gtou and utog and count from the group class
            //both entry cahnge in above two line will be diffrent code.
            list<string>::iterator itr;
            for(itr=owned_group->begin();itr!=owned_group->end();itr++){
                if(*itr==grp){
                    cout<<"inside the if condition"<<endl;
                    //the element is the leader therefore deleting the group.
                    map<string,list<string>>::iterator gotu_itr=groupid_to_userids->find(grp);
                    list<string>* target_list=&gotu_itr->second;
                    list<string>::iterator parent_list;
                    for(parent_list=target_list->begin();parent_list!=target_list->end();parent_list++){
                        string temp=*parent_list;//this is the user which has to remope grp from its list
                        map<string,list<string>>::iterator indi;
                        indi=userid_to_groupids->find(temp);
                        list<string>::iterator second_list;
                        list<string>* temp_second=&(indi->second);
                        //now we have to remove grp from this second list.
                        for(second_list=temp_second->begin();second_list!=temp_second->end();second_list++){
                            if(*second_list==grp){
                                temp_second->erase(second_list);
                                break;
                            }
                        }
                    }
                    map<string,groups*>::iterator gg_t= groupid_to_group->find(grp);
                    delete &gg_t->second;//deleting the object.
                    groupid_to_group->erase(grp);
                    groupid_to_userids->erase(grp);
                    return true;
                }
            }
            //just leaving the group
            map<string,list<string>>::iterator utog;
            utog=userid_to_groupids->find(user_id);
            list<string>::iterator utog_gs;//utog->second.begin();
            for(utog_gs=utog->second.begin();utog_gs!=utog->second.end();utog_gs++){
                if(*utog_gs==grp){
                    utog->second.erase(utog_gs);
                    break;
                }
            }
            //now we have to delete the user from the group id.
            map<string,list<string>>::iterator gtou;
            gtou=groupid_to_userids->find(grp);
            list<string>::iterator gtou_us;
            for(gtou_us=gtou->second.begin();gtou_us!=gtou->second.end();gtou_us++){
                if(*gtou_us==user_id){
                    gtou->second.erase(gtou_us);
                    break;
                }
            }
            //now we have to decrease the cout
            groupid_to_group->find(grp)->second->number_of_members--;//decrementing the count of people.
            return true;
        }
};




void print_state(){
    //first we will show all the user
    //then we will show all the gropu
    //map<string,user>::iterator itr;
    cout<<"the user are as follow"<<endl;
    //for(itr=userid_to_user->begin();itr!=userid_to_user->end();itr++ ){
    //    cout<<endl<<endl;
    //    itr->second.show_state();
    //    cout<<endl<<endl;
    //    //std::next(itr,1);
    //}
    //trying another way to iterate over map.
    for (auto i : *userid_to_user){
        cout<<endl<<endl;
        i.second->show_state();
        cout<<endl<<endl;
    }
   // map<string,groups>::iterator itr2;
    cout<<"the groups are as follow "<<endl;
    //for(itr2=groupid_to_group->begin();itr2!=groupid_to_group->end();itr2++ ){
    //    cout<<endl<<endl;
    //    itr2->second.show_state();
    //    cout<<endl<<endl;
       // std::next(itr,1);
    //}
    for (auto i : *groupid_to_group){
        cout<<endl<<endl;
        i.second->show_state();
        cout<<endl<<endl;
    }
    cout<<"now lets see the connection between the following group"<<endl;
    cout<<"printing groupid_to_userid"<<endl;
    map<string,list<string>>::iterator i;
    for(i=groupid_to_userids->begin();i!=groupid_to_userids->end();i++){
        //now we have to print the list
        cout<<"the group is "<<i->first<<endl;
        cout<<"and user which are part of it is"<<endl;
        list<string>::iterator j;
        for(j=i->second.begin();j!=i->second.end();j++){
            cout<<*j<<" ";
        }
        cout<<endl;
    }
    cout<<"now printing userid to group"<<endl;
    for(i=userid_to_groupids->begin();i!=userid_to_groupids->end();i++){
        //now we have to print the list
        cout<<"the group is "<<i->first<<endl;
        cout<<"and user which are part of it is"<<endl;
        list<string>::iterator j;
        for(j=i->second.begin();j!=i->second.end();j++){
            cout<<*j<<" ";
        }
        cout<<endl;
    }
}












bool is_ug_mapped(string s_u,string s_g){
    map<string,list<string>>::iterator itr;
    itr=userid_to_groupids->find(s_u);
    if(itr==userid_to_groupids->end()){
        cout<<s_u<<" is part of no group"<<endl;
        return false;
    }else{
        list<string>* temp_list=&(itr->second);
        //now if this list cointain s_g then return true other return false.
        list<string>::iterator list_itr;
        for(list_itr=temp_list->begin();list_itr!=temp_list->end();list_itr++){
            if(*list_itr==s_g){
                return true;
            }
        }
        return false;
    }
}
vector<string> ip_of_tracker;//this is already inisialized
user* get_user(string u){
    map<string,user*>::iterator itr;
    itr=userid_to_user->find(u);
    if(itr==userid_to_user->end()){
        return NULL;
    }else{
        return itr->second;
    }
}
groups* get_grp(string g){
    map<string,groups*>::iterator itr;
    itr=groupid_to_group->find(g);
    if(itr==groupid_to_group->end()){
        return NULL;
    }else{
        return itr->second;
    }
}
bool is_user(string u){
    if(get_user(u)==NULL){
        return false;
    }else{
        return true;
    }
}
bool is_grp(string g){
    if(get_grp(g)==NULL){
        return false;
    }else{
        return true;
    }
}

/*
    ------------------------------------  NOTEE -----------------------------------
    the first thing which any tracker will do is to check any othre tracker is active
    if so it will take all thee data from that other tracker.
       lets put group to user and user to group mapping by hash table.
        one hash table to link user_id to user class.
        one hash table to link group_id to group class.
*/
struct s_arg{
    int connection_fd;
    sockaddr_in connection_address;
};
//syncronise_trackers(char* mssag);//to be build in near future.
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
void print_everything(){
    //this willl all content till now.
}
void* mkr(void* argu){
    char d='d';
    char e='e';
    //feching the value from the argument.
    s_arg* arguments=(s_arg*)argu;
    int connectiond_fd=arguments->connection_fd;
    sockaddr_in address_of_client=arguments->connection_address;
    string ip_of_client=inet_ntoa(address_of_client.sin_addr);//we have to convert this long format into string.
    int client_port=ntohs(address_of_client.sin_port);
    cout<<"the ip address of the client is "<<ip_of_client<<endl;
    cout<<"the port number of client is "<<client_port<<endl;
    //here all the language will be build.
    char mssag_chahe[100];//we will send this messsage chache as it is to other tracker.
    int i=0;
    int ei=recv(connectiond_fd,mssag_chahe,100,0);//how to extract argumeents from it.
    if(ei==-1){
        cout<<"the error number is "<<errno<<endl;
    }
    cout<<"the value of ei is "<<ei<<endl;
    char command=mssag_chahe[0];
    mssag_chahe[i]=command;
    i++;
    if(command=='c'){//creating the user.
        cout<<"creating the user"<<endl;
        char* a=ithargument(mssag_chahe,1);
        char* b=ithargument(mssag_chahe,2);
        string my_user=a;
        string my_pass=b;
        //checking this user exist or not.
        if(userid_to_user->find(my_user)!=userid_to_user->end()){
            //the user alreeady exist
            cout<<"the user already exist"<<endl;                  //the error case.
            //in this case we will send the error message;
            char m='e';
            if(state){
                ei=send(connectiond_fd,&m,1,0);
                if(ei==-1){
                    cout<<"error in writing "<<endl;
                }
            }
        }else{
        //creating entry for this user.
            cout<<"the value of usser is "<<my_user<<endl;
            cout<<"the value of password is "<<my_pass<<endl;
            user* temp=new user(my_user,my_pass);               //the normal case.
            //userid_to_user[my_user]=temp;
            userid_to_user->insert(pair<string,user*>(my_user,temp));
            //also need to start user to gruoup data strucuree.
            list<string> random_name;
            userid_to_groupids->insert(pair<string,list<string>>(my_user,random_name));//littl less sure about this.
            cout<<"the user is created"<<endl;
            char m='d';
            if(state){
                ei=send(connectiond_fd,&m,1,0);
                if(ei==-1){
                    cout<<"error in writing "<<endl;
                }
            }
        }
        delete a;
        delete b;
    }else if(command=='l'){//login of user.  //we should also send the server side port of the clint
        cout<<"logging in the user"<<endl;
        char* a=ithargument(mssag_chahe,1);//user
        char* b=ithargument(mssag_chahe,2);//pass
        char* c=ithargument(mssag_chahe,3);//port
        string my_user=a;
        string my_pass=c;
        string my_port=b;
        char m1='d';
        char m2='e';
        //now we have to check the pass correspondig to the user.
        //first check whether the user exist or not
        map<string,user*>::iterator itr;
        itr=userid_to_user->find(my_user);
        if(itr==userid_to_user->end()){
            //eroor case
            cout<<"error case"<<endl;
            cout<<"the user does not exist"<<endl;
            if(state){
                send(connectiond_fd,&m2,1,0);
            }
        }else{
            user* temp=itr->second;
            cout<<"my password is "<<my_pass<<"and the stl"<<my_pass.length()<<endl;
            cout<<"password stored is "<<temp->passwd<<"lngth si "<<temp->passwd.length()<<endl;
            int i;
            for(i=0;i<temp->passwd.length();i++){
                cout<<(int)temp->passwd[i]<<endl;
            }
            if(my_pass==temp->passwd){
                //correct condition
                cout<<"this is the correct case"<<endl;
                temp->is_active=true;
                temp->ip=ip_of_client;//we have save the ip address
                temp->port=my_port;
                //temp->port=itoa(client_port);//we have to create itoa port can we use sprintf;
                //we do not need this port we need the port of the server part of the client.
                if(state){
                    send(connectiond_fd,&m1,1,0);
                }
            }else{
                //error condition
                cout<<"the credentials does not match"<<endl;
                cout<<"error case"<<endl;
                if(state){
                    send(connectiond_fd,&m2,1,0);
                }
            }
            /*
            itr=userid_to_user->find(my_user);
            if(itr==userid_to_user->end()){
                cout<<"now it is not present"<<endl;
            }else{
                cout<<"still present baby"<<endl;
            }*/
        }
        delete a;
        delete b;
        delete c;
    }else if(command=='g'){//create group.
        cout<<"creating the group"<<endl;
        //this can be tricy.
        char* a=ithargument(mssag_chahe,1);//user id
        char* b=ithargument(mssag_chahe,2);//group id  //the leader of the group.
        string my_userr=a;
        string my_groupid=b;
        //char d='d';
        //char e='e';
        map<string,user*>::iterator itr_u;
        map<string,groups*>::iterator itr_g;
        // map<string,user*>::iterator itr;
        //itr=userid_to_user->find(my_user);
        itr_u=userid_to_user->find(my_userr);
        itr_g=groupid_to_group->find(my_groupid);
        cout<<"the user id is "<<my_userr<<endl;
        cout<<"thee group id is "<<my_groupid<<endl;
        if(itr_u==userid_to_user->end()||itr_g!=groupid_to_group->end()||itr_u->second->is_active==false){// user not found error case
            //this is error condition.
            if(state){
                send(connectiond_fd,&e,1,0);
            }
        }else{
            cout<<"correct condition"<<endl;
            groups* temp_g=new groups(my_groupid,my_userr);
            cout<<"the constructor is created"<<endl;
            itr_u->second->add_his_group(my_groupid);
            cout<<"add_hit_group routine runned"<<endl;
            groupid_to_group->insert(pair<string,groups*>(my_groupid,temp_g));
            cout<<"groupid to groupid tablee is updated"<<endl;
            list<string> just_random;
            groupid_to_userids->insert(pair<string,list<string>>(my_groupid,just_random));
            temp_g->add_join_req(my_userr);
            cout<<"hakuna "<<endl;
            temp_g->accept_req(my_userr);//the problem is here
            cout<<"matata"<<endl;
            if(state){
                send(connectiond_fd,&d,1,0);
            }
        }
        delete a;
        delete b;
    }else if(command=='j'){//join group.
        cout<<"joining the group"<<endl;
        //this can only send the request to join the group
        char* a=ithargument(mssag_chahe,1);//user id
        char* b=ithargument(mssag_chahe,2);//group id
        string my_user=a;
        string my_grp=b;
        groups* target_group=get_grp(my_grp);
        if(target_group==NULL||is_user(my_user)==false||is_ug_mapped(my_user,my_grp)==true||get_user(my_user)->is_active==false){
            //we need method to find two user and group is mapped for not.
            cout<<"error condition"<<endl;
            if(state){
                send(connectiond_fd,&e,1,0);
            }
        }else{
            //correct condition.
            cout<<"correct condition"<<endl;
            //we have to add this user to the request list of the group.
            //target_group->join_requests->push_back(my_user);
            target_group->add_join_req(my_user);
            if(state){
                send(connectiond_fd,&d,1,0);
            }
        }
        delete a;
        delete b;//compil and check this condition.
    }else if(command=='x'){//leave group.              //this is not working right now.
        cout<<"leaving the group"<<endl;
        //the user should be present
        //the group should be present 
        //the user and group should be mapped.
        char* a=ithargument(mssag_chahe,1);//user id
        char* b=ithargument(mssag_chahe,2);//group id
        string my_user=a;
        string my_grp=b;
        //finding the user required.
        user* usr=userid_to_user->find(my_user)->second;
        cout<<"call to leave group started"<<endl;
        usr->leave_grp(my_grp);
        cout<<"call to leave group ended"<<endl;
        if(state){
                send(connectiond_fd,&d,1,0);
            }
        delete a;
        delete b;
    }else if(command=='y'){//list pending group.
        cout<<"list pending request"<<endl;
        //just send the group id
        char* a=ithargument(mssag_chahe,1);//group id
        string my_grp=a;
        //now we need to get that format.
        //people_want_to_join()
        //error handeling not done.
        //the group should exist.
        
        groups* grp=groupid_to_group->find(my_grp)->second;
        int ss=grp->join_requests->size();
        char* msg=grp->people_want_to_join();//will it allocate the space already ?
        if(state){
                send(connectiond_fd,&ss,1,0);
                send(connectiond_fd,msg,ss,0);
            }
        delete[] msg;
        delete a;
    }else if(command=='a'){//accept grouop joining request.
        cout<<"accepting the group request"<<endl;
        //user should be group leader
        //target user should be in the request list.

    }else if(command=='n'){//list all group in the network.
        cout<<"list all the gruops int the network"<<endl;
        //simple command
    }else if(command=='f'){//list all the sharabel file in the group.
        cout<<"list all thee files in the group"<<endl;
        //this one requires some thinking.
    }else if(command=='u'){//upload the file.   
        cout<<"upload the file"<<endl;
    }else if(command=='o'){//logout
        cout<<"logout from the system"<<endl;
    }else if(command=='d'){//download
        cout<<"download the file"<<endl;
        //only have to send the information.
    }else if(command=='s'){//stop shareing          //doubt in this application.
        cout<<"stop shareing"<<endl;        //this one we are not going to impliment.
    }else if(command=='i'){//change the state to active
        cout<<"change the state of the tracker to active"<<endl;
        state=true;
    }else if(command=='t'){
        cout<<"command to print the state of all the data structure"<<endl;
        print_state();
    }else{
        cout<<"this command is not known"<<endl;
    }
    //the zone of communication ends heere.
    close(connectiond_fd);
    delete arguments;
    pthread_exit(NULL);
    return NULL;
}
int main(){
    is_active=true; // with multiple tracker this should be false;

    /*
    //first we will populate the ip of tracker from the file to the vector.


    //------------------ populating the vector -------------------------------
            //while populating it does not have to populate its own ip
                //how to find your own ip
            //we are assuming that ip of only other trackers are there in the file.

    //------------------- done populating the vector --------------------------


    //----------------- checking any old traker is active-----------------------
    //{::doga::}
    bool is_old_active=false;
    string target_ip;//ip of the parent tracker

    //--------------------- done checking -----------------------------------------
    if(is_old_active){
            //take all the meta data from the old server.
            //if any error comes in between then then retry from here {::doga::}
                    //make is_old_active as false;



                    //last line
                    is_active=true;
    }else{
        is_active=true;
    }
*/
    if(is_active){
        //all the remaining code will come here.
        //build up the language.
                        //code here..............
    //starting the server.
    //inisializign all the maps.
    userid_to_groupids=new map<string,list<string>>();
    groupid_to_userids=new map<string,list<string>>();
    userid_to_user=new map<string,user*>();
    groupid_to_group=new map<string,groups*>();

    //(*userid_to_groupids)["power"]=
    int server_socket_fd=socket(AF_INET,SOCK_STREAM,0);
    if(server_socket_fd<0){
        cout<<"error in creating the socket"<<endl;
        return 0;
    }
    //creating the socket address.
    sockaddr_in server_sockt_address;
    server_sockt_address.sin_port=htons(NPORT);
    server_sockt_address.sin_family=AF_INET;
    server_sockt_address.sin_addr.s_addr=INADDR_ANY;
    int ei;
    ei=bind(server_socket_fd,(sockaddr*)&server_sockt_address,sizeof(server_sockt_address));
    if(ei==-1){
        cout<<"error in binding the socket"<<endl;
        cout<<"the error number if "<<errno<<endl;
    }
    ei=listen(server_socket_fd,15);
    if(ei==-1){
        cout<<"error in listening to socket"<<endl;
    }
    sockaddr_in temp_connection_addr;
    int temp_connection;
    int temp_connection_len;
    vector<pthread_t> thread_vector;
    pthread_t temp_thread_id;
    s_arg* my_argument;
    while(1){
       // temp_connection=new int();
       // temp_connection_addr=new sockaddr_in();
       cout<<"waiting for thread connection"<<endl;
        temp_connection=accept(server_socket_fd,(sockaddr*)&temp_connection_addr,(socklen_t*)&temp_connection_len);
        //now we will start the new thread.
        //we need to send two thigs to the thread
                // 1. the fd of the connection socket  int
                // 2. the address of the connection.   sockaddr_in
        if(temp_connection<0){
            cout<<"error in accept method"<<endl;
            cout<<"error number is "<<errno<<endl;
            break;//a potencial target for goto stateement.
        }
        my_argument=new s_arg();
        my_argument->connection_fd=temp_connection;
        my_argument->connection_address=temp_connection_addr;
        pthread_create(&temp_thread_id,NULL,mkr,(void*)my_argument);//new thread is created at this point
        thread_vector.push_back(temp_thread_id);
    }


    //lets catch all the thread here.
    int i_s=thread_vector.size();
    int i;
    for(i=0;i<i_s;i++){
        pthread_join(thread_vector[i],NULL);
    }


    }else{
        cout<<"due to some reason the tracker is not in active state."<<endl;
    }
    //control may never reach here.
    //here i will be deleting all the maps.
    //before it we have to delete all the pointers pointed by it.
    return 0;
}

