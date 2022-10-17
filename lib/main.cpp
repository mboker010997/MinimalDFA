#include <iostream>
#include "automaton.h"

using namespace std;

int main() {
    freopen("input.txt", "r", stdin);
    Automaton aut;
    cin >> aut;
    aut.to_mpdka();
    // cout << "\n\n";
    cout << aut;
    return 0;
}