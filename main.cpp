#include <iostream>
#include <vector>
#include <unordered_map>

using namespace std;

class Transaction {
    public: 
        void ReadInput(); 
        void RedoPhase();
        void UndoPhase();
        void ShowDataItems();
        
    private:
    	// vector<pair<int, pair<char,int> > > transactions; 
        vector<int> commit;
        vector<pair<int, tuple<char,char,int,int> > > logs;  // { T_id, { Operation, DataObject, PreviousValue, NewValue }}
        // wal suggests buffer logs array must be updated before putting record into transactions array. 
        unordered_map<char, int> dataitem;
        vector<int> undolist; // tid of transactions to undo
        unordered_map<char, int> recovery_data;  // modified by redo or undo phase
};

void Transaction::ShowDataItems() {
    cout<<endl;
    for(auto it:dataitem) {
        cout<<"DataItems before recovery: "<<it.first<<": "<<it.second<<endl;
    }
    for (auto it:recovery_data){
        cout<<"DataItems after recovery: "<<it.first<<": "<<it.second<<endl;
    }
}

void Transaction::ReadInput() {
    int num_data_items; 
    cin>>num_data_items;
    char data_item_name;
    char operation;
    int data_item_initial_value;
    int t_id;
    int new_value;
    while(num_data_items-->0){
        cin>>data_item_name;
        cin>>data_item_initial_value;
        dataitem[data_item_name] = data_item_initial_value;   
        recovery_data[data_item_name] = data_item_initial_value;
    }

    // handle all logs during input
    while(!cin.eof())   // take all the input till end of file.
    {
        cin>>operation;
        if ( operation == 'S') {
            cin>>t_id;
            logs.push_back({ t_id, { 'S', NULL, NULL , NULL } });
        }   
        else if ( operation == 'R' ) {
            cin>>t_id;
            cin>>data_item_name;
        }
        else if ( operation == 'W') {
            cin>>t_id;
            cin>>data_item_name;
            cin>>new_value;
            logs.push_back({ t_id, { 'W' , data_item_name, dataitem[data_item_name], new_value }});
            dataitem[data_item_name] = new_value;
        }
        else if ( operation == 'C') {
            cin>>t_id;
            commit.push_back(t_id);
            logs.push_back({ t_id, { 'C', NULL, NULL, NULL } });
        }
    }

}
// push the uncommited transactions into undol list overall
void Transaction::RedoPhase() { 
    for ( auto it:logs ){
        if(get<0>(it.second) == 'W') {
            recovery_data[get<1>(it.second)] = get<3>(it.second);
        }
        else if ( get<0>(it.second) == 'S'){
            undolist.push_back(it.first);
        }
        else if (get<0>(it.second) == 'C') {
            undolist.erase(remove(undolist.begin(), undolist.end(), it.first ));
        }
    }
}

void Transaction::UndoPhase() {
    // iterate the logs array reverse and for all t_id in undo list rollback the transactions.
    int t_id;
    while(undolist.size()>0){
        t_id = undolist.back();
        undolist.pop_back();
        for(auto it = logs.rbegin(); it != logs.rend(); it++) {
            cout<<it->first<<" ";
            if(it->first == t_id && get<0>(it->second) == 'S') {
                logs.push_back({ t_id, { 'A', NULL, NULL, NULL } });
            }
            else if(it->first == t_id && get<0>(it->second) == 'W'){
                recovery_data[get<1>(it->second)] = get<2>(it->second);
                //write a redo-log -- optional;
            }
        }
    }
}

int main() {

    Transaction T;

    T.ReadInput();
    T.RedoPhase();
    T.UndoPhase();
    T.ShowDataItems();

    return 0;
}
