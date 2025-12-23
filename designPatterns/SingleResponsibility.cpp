#include <iostream>
#include <string>
#include <vector>
#include <fstream>

using namespace std;

class journal {
public:
    journal(const string &title) : title(title) {}
    void add_entry(const string entry) {
        entries.push_back(entry);
    }
    void show_journal() {
        cout << "Title: " << title << endl;
        for(auto& i:entries) {
            cout << i << endl;
        }
    }
    vector<string>& get_entries(void) {
        return entries;
    }
    string& get_title(void) {
        return title;
    }
    void save(const string file_name) {
        ofstream ofs(file_name);
        for(auto& i: entries)
            ofs << i << endl;
    }
private:
    string title;
    vector<string> entries;
};

class persist {
public:
    void save(journal& journal, const string file_name) {
        cout << "Saving " << file_name << endl;
        ofstream ofs(file_name);
        for(auto& i: journal.get_entries())
            ofs << i << endl;
    }
};
int main()
{
    journal j{"DearDiary"};
    j.add_entry("Entry 1");
    j.add_entry("Entry 2");
    j.add_entry("Entry 3");
    j.show_journal();
    
    persist pm;
    pm.save(j, "/Users/ppadmoh/workspace/C++/patterns/" + j.get_title() + ".txt");
    return 0;
}
