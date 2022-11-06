#include <iostream>
#include "automaton.h"
#include <string>

using namespace std;

int main() {
    string s;
    cin >> s;
    char c;
    cin >> c;
    string alp = "abc";
    Automaton aut(s, alp);
    aut.to_pdka();
    cout << aut << '\n';
    // cout << "Answer: " << aut.get_max_end_by_letter(c);
    return 0;
}