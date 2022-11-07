#include <vector>
#include <string>
#include <bitset>
#include <set>
#include <queue>
#include <map>
#include <algorithm>
#include <iostream>

using std::vector;
using std::pair;
using std::string;
using std::bitset;
using std::cout;
using std::map;
using std::set;

const vector<int> empty_vector;
const int INF = 1e9;

class Automaton {
private:

    struct Edge {
        int v, to;
        char w;

        Edge() = default;

        Edge(int v, int to, const char& w) : v(v), to(to), w(w) {}
    };

    static const size_t MAX_VERTEX = 100;
    static const char eps = '_';

    static bool is_eps(char c) {
        return c == eps;
    }

    int start;
    int vertex;
    string alp;
    vector<map<char, vector<int>>> edges;
    std::set<int> terms;

    static char get_edge_symbol(pair<char, const vector<int>&> map_pair);

    static const vector<int>& get_edge_neighbours(pair<char, const vector<int>&> map_pair);

    static int get_edge_neighbor(pair<char, const vector<int>&> map_pair);

    static bool get_bit(int mask, int pos);

    vector<Edge> get_edges_list(std::set<int> removed = std::set<int>());

    void compress_and_assign_edges(const vector<Edge>& pairs);

    void remove_reachless_vertices();

    bool dfs_finding_cycle_by_letter(int v, char c, vector<char>& used);

    void topological_sort_by_letter(int v, char c, vector<bool>& used, vector<int>& order);

public:
    Automaton() = default;

    Automaton(int vertex, int start, const string& alp, const vector<int>& _terms);

    Automaton(char c, const string& alp);

    Automaton(const string& regex, const string& alp);

    int get_vertex() const;

    int get_start() const;

    int get_next(int v, char w) const;

    string get_alp() const;

    void add_edge(int v, int to, char w);

    void remove_eps_edges();

    bool is_dka() const;

    void to_dka();

    void to_pdka();

    void to_mpdka();

    bool find_cycle_by_letter(char c);

    int get_max_end_by_letter(char c);

    friend class Node;

    friend bool operator<(const Automaton::Edge& fst, const Automaton::Edge& snd);
    friend bool operator==(const Automaton::Edge& fst, const Automaton::Edge& snd);
    friend bool operator<=(const Automaton::Edge& fst, const Automaton::Edge& snd);
    friend bool operator>=(const Automaton::Edge& fst, const Automaton::Edge& snd);
    friend bool operator>(const Automaton::Edge& fst, const Automaton::Edge& snd);
    friend bool operator!=(const Automaton::Edge& fst, const Automaton::Edge& snd);

    friend Automaton operator+(const Automaton& fst, const Automaton& snd);
    friend Automaton operator-(const Automaton& fst, const Automaton& snd);
    friend Automaton operator*(const Automaton& fst);

    friend std::ostream& operator<<(std::ostream& out, const Automaton& aut);
    friend std::istream& operator>>(std::istream& in, Automaton& aut);
};