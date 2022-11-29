#include "automaton.h"
#include <stack>

bool Automaton::get_bit(int mask, int pos) {
    return (mask & (1 << pos)) != 0;
}

char Automaton::get_edge_symbol(pair<char, const vector<int>&> map_pair) {
    return map_pair.first;
}

const vector<int>& Automaton::get_edge_neighbours(pair<char, const vector<int>&> map_pair) {
    return map_pair.second;
}

int Automaton::get_edge_neighbor(pair<char, const vector<int>&> map_pair) {
    auto all_neighbours = get_edge_neighbours(map_pair);
    if (all_neighbours.empty()) {
        throw std::runtime_error("There is no neighbours!");
    }
    return all_neighbours[0];
}

vector<Automaton::Edge> Automaton::get_edges_list(std::set<int> removed) {
    vector<Edge> res;
    for (int v = 0; v < vertices_count; ++v) {
        for (auto edge : edges[v]) {
            for (auto to : Automaton::get_edge_neighbours(edge)) {
                if (removed.count(v) || removed.count(to)) continue;
                res.push_back(Edge(v, to, get_edge_symbol(edge)));
            }
        }
    }
    return res;
}

vector<int> Automaton::get_compressed_list(const vector<Automaton::Edge>& pairs) {
    vector<int> list_of_vertices;
    for (const Edge& edge : pairs) {
        list_of_vertices.push_back(edge.v);
        list_of_vertices.push_back(edge.to);
    }
    std::sort(list_of_vertices.begin(), list_of_vertices.end());
    list_of_vertices.erase(std::unique(list_of_vertices.begin(), list_of_vertices.end()), list_of_vertices.end());
    return list_of_vertices;
}

int Automaton::get_new_vertice_number(const vector<int>& compressed_list, int vertice) {
    return lower_bound(compressed_list.begin(), compressed_list.end(), vertice) - compressed_list.begin();
}

void Automaton::new_terms_after_compression(const vector<int>& compressed_list) {
    std::set<int> new_terms;
    for (auto v : terms) {
        int new_number_of_v = get_new_vertice_number(compressed_list, v);
        new_terms.insert(new_number_of_v);
    }
    terms = new_terms;
}

void Automaton::new_edges_after_compression(const vector<Automaton::Edge>& pairs, const vector<int>& compressed_list) {
    edges.clear();
    edges.resize(vertices_count);
    for (const Edge& edge : pairs) {
        int v = get_new_vertice_number(compressed_list, edge.v);
        int to = get_new_vertice_number(compressed_list, edge.to);
        edges[v][edge.w].push_back(to);
    }
}

void Automaton::compress_and_assign_edges(const vector<Automaton::Edge>& pairs) {
    vector<int> compressed_list = get_compressed_list(pairs);
    vertices_count = compressed_list.size();
    new_terms_after_compression(compressed_list);
    new_edges_after_compression(pairs, compressed_list);
    start = get_new_vertice_number(compressed_list, start);
}

void Automaton::remove_reachless_vertices() {
    std::set<int> removed;
    vector<bitset<MAX_VERTEX>> is_reach(vertices_count, 0);
    for (int v = 0; v < vertices_count; ++v) {
        is_reach[v][v] = 1;
    }
    for (int i = 0; i < vertices_count; ++i) {
        for (int v = 0; v < vertices_count; ++v) {
            for (auto edge : edges[v]) {
                for (int to : Automaton::get_edge_neighbours(edge)) {
                    is_reach[v] |= is_reach[to];
                }
            }
        }
    }
    for (int v = 0; v < vertices_count; ++v) {
        if (!is_reach[start][v]) {
            removed.insert(v);
        }
        bool ok = 0;
        for (int u = 0; u < vertices_count; ++u) {
            if (terms.count(u) && is_reach[v][u]) ok = 1;
        }
        if (!ok) removed.insert(v);
    }
    auto list = get_edges_list(removed);
    compress_and_assign_edges(list);
}

bool Automaton::dfs_finding_cycle_by_letter(int v, char c, vector<char>& used) {
    if (!is_dka()) {
        throw std::runtime_error("Automaton is not DKA!!!");
    }
    used[v] = 1;
    for (auto edge : edges[v]) {
        char w = get_edge_symbol(edge);
        if (w == c) {
            for (int to : get_edge_neighbours(edge)) {
                if (used[to] == 1) {
                    return true;
                } else {
                    if (dfs_finding_cycle_by_letter(to, c, used)) {
                        return true;
                    }
                }
            }
        }
    }
    used[v] = 2;
    return false;
}

bool Automaton::find_cycle_by_letter(char c) {
    vector<char> used(vertices_count, 0);
    return dfs_finding_cycle_by_letter(start, c, used);
}

void Automaton::topological_sort_by_letter(int v, char c, vector<bool>& used, vector<int>& order) {
    used[v] = 1;
    for (auto edge : edges[v]) {
        for (int to : get_edge_neighbours(edge)) {
            if (!used[to]){
                topological_sort_by_letter(to, c, used, order);
            }
        }
    }
    order.push_back(v);
}

int Automaton::get_max_end_by_letter(char c) {
    if (find_cycle_by_letter(c)) {
        return INF;
    }
    vector<int> dp(vertices_count, -1);
    vector<bool> used_for_topsort(vertices_count, 0);
    vector<int> topsort_order;
    for (int v = 0; v < vertices_count; ++v) {
        if (!used_for_topsort[v]) {
            topological_sort_by_letter(v, c, used_for_topsort, topsort_order);
        }
    }
    for (int v : topsort_order) {
        int current_dp_result = -1;
        if (terms.count(v) != 0) current_dp_result = 0;
        for (auto edge : edges[v]) {
            for (int to : get_edge_neighbours(edge)) {
                char w = get_edge_symbol(edge);
                if (w == c) {
                    current_dp_result = std::max(current_dp_result, dp[to] + 1);
                }
            }
        }
        dp[v] = current_dp_result;
    }
    return *std::max_element(dp.begin(), dp.end());
}

int Automaton::get_next(int v, char w) const {
    if (edges[v].count(w) == 0) {
        throw std::runtime_error("No next for vertices_count by symbol" + w);
    }
    return edges[v].at(w)[0];
}

Automaton::Automaton(int vertices_count, int start, const string& alp, const vector<int>& _terms) 
    : vertices_count(vertices_count), start(start), alp(alp) {
    terms = std::set<int> (_terms.begin(), _terms.end());
    edges.resize(vertices_count);
}

Automaton::Automaton(char c) : start(0), alp("") {
    if (c == '1') {
        vertices_count = 1;
        terms = {0};
        edges.resize(vertices_count);
    } else {
        alp += c;
        vertices_count = 2;
        terms = {1};
        edges.resize(vertices_count);
        edges[0][c].push_back(1);
    }
}

Automaton::Automaton(const string& regex) {
    std::stack<Automaton> st;
    for (char cur_sym : regex) {
        if (cur_sym == '+') {
            if (st.empty()) {
                throw std::runtime_error("Incorrect regex");
            }
            Automaton snd = st.top();
            st.pop();

            if (st.empty()) {
                throw std::runtime_error("Incorrect regex");
            }
            Automaton fst = st.top();
            st.pop();

            st.push(fst + snd);
        } else if (cur_sym == '.') {
            if (st.empty()) {
                throw std::runtime_error("Incorrect regex");
            }
            Automaton snd = st.top();
            st.pop();

            if (st.empty()) {
                throw std::runtime_error("Incorrect regex");
            }
            Automaton fst = st.top();
            st.pop();

            st.push(fst - snd);
        } else if (cur_sym == '*') {
            if (st.empty()) {
                throw std::runtime_error("Incorrect regex");
            }
            Automaton fst = st.top();
            st.pop();

            st.push(*fst);
        } else {
            st.push(Automaton(cur_sym));
        }
    }
    if (st.empty() || st.size() > 1) {
        throw std::runtime_error("Incorrect regex");
    }
    *this = st.top();
    to_mpdka();
}

int Automaton::get_vertices_count() const {
    return vertices_count;
}

int Automaton::get_start() const {
    return start;
}

string Automaton::get_alp() const {
    return alp;
}

void Automaton::add_edge(int v, int to, char w) {
    edges[v][w].push_back(to);
}

void Automaton::remove_eps_edges() {
    vector<bitset<MAX_VERTEX>> is_reach(vertices_count, 0);
    vector<vector<Edge>> eps_edges(vertices_count);
    for (int v = 0; v < vertices_count; ++v) {
        for (auto edge : edges[v]) {
            char w = get_edge_symbol(edge);
            for (int to : get_edge_neighbours(edge)) {
                if (is_eps(w)) {
                    eps_edges[v].push_back(Edge(v, to, w));
                }
            }
        }
        is_reach[v][v] = 1;
    }

    for (int i = 0; i < vertices_count; ++i) {
        for (int v = 0; v < vertices_count; ++v) {
            for (const Edge& edge : eps_edges[v]) {
                int to = edge.to;
                is_reach[v] |= is_reach[to];
            }
        }
    }

    vector<map<char, vector<int>>> new_edges(vertices_count);
    for (int v = 0; v < vertices_count; ++v) {
        for (int u = 0; u < vertices_count; ++u) {
            if (is_reach[v][u]) {
                for (auto edge : edges[u]) {
                    char w = get_edge_symbol(edge);
                    for (int to : get_edge_neighbours(edge)) {
                        if (!is_eps(w)) {
                            new_edges[v][w].push_back(to);
                        }
                    }
                }
                if (terms.count(u)) terms.insert(v);
            }
        }
    }
    edges = new_edges;
    remove_reachless_vertices();
}

bool Automaton::is_dka() const {
    for (int v = 0; v < vertices_count; ++v) {
        std::set<char> used;
        for (auto edge : edges[v]) {
            char w = get_edge_symbol(edge);
            if (is_eps(w) || used.count(w)) return false;
            used.insert(w);
        }
    }
    return true;
}

void Automaton::to_dka() {
    if (is_dka()) {
        return;
    }
    remove_eps_edges();
    vector<Edge> list;
    std::queue<int> q;
    q.push((1 << start));
    start = (1 << start);
    std::set<int> used;
    std::set<int> new_terms;
    while (!q.empty()) {
        int mask = q.front();
        q.pop();
        if (used.count(mask)) continue;
        for (int v = 0; v < vertices_count; ++v) {
            if (terms.count(v) && get_bit(mask, v)) {
                new_terms.insert(mask);
                break;
            }
        }
        used.insert(mask);
        std::map<char, int> delta;
        for (int v = 0; v < vertices_count; ++v) {
            if (get_bit(mask, v)) {
                for (auto edge : edges[v]) {
                    char w = get_edge_symbol(edge);
                    for (int to : get_edge_neighbours(edge)) {
                        delta[w] |= (1 << to);
                    }
                }
            }
        }
        for (auto el : delta) {
            list.push_back(Edge(mask, el.second, el.first));
            q.push(el.second);
        }
    }
    terms = new_terms;
    compress_and_assign_edges(list);
}

void Automaton::to_pdka() {
    if (!is_dka()) to_dka();
    auto list = get_edges_list();
    int stok = vertices_count;
    bool is_need_stok = 0;
    for (int v = 0; v < vertices_count; ++v) {
        std::set<char> used;
        for (auto edge : edges[v]) {
            used.insert(get_edge_symbol(edge));
        }
        for (char c : alp) {
            if (!used.count(c)) {
                is_need_stok = 1;
                list.push_back(Edge(v, stok, c));
            }
        }
    }
    if (is_need_stok) {
        for (char c : alp) {
            list.push_back(Edge(stok, stok, c));
        }
    }
    compress_and_assign_edges(list);
}

void Automaton::to_mpdka() {
    to_pdka();
    vector<int> cl(vertices_count, 0);
    for (auto v : terms) {
        cl[v] = 1;
    }   
    for (int i = 0; i < vertices_count; ++i) {
        vector<pair<pair<int, std::map<char, int>>, int>> cmp(vertices_count);
        for (int v = 0; v < vertices_count; ++v) {
            cmp[v].first.first = cl[v];
            cmp[v].second = v;
            for (auto edge : edges[v]) {
                char c = get_edge_symbol(edge);
                for (int to : get_edge_neighbours(edge)) {
                    (cmp[v].first.second)[c] = cl[to];
                }
            }
        }
        std::sort(cmp.begin(), cmp.end());
        cl[cmp[0].second] = 0;
        for (int j = 1; j < vertices_count; ++j) {
            cl[cmp[j].second] = cl[cmp[j - 1].second];
            if (cmp[j].first != cmp[j - 1].first) cl[cmp[j].second]++;
        }
    }
    start = cl[start];
    std::set<int> new_terms;
    for (int v : terms) {
        new_terms.insert(cl[v]);
    }
    terms = new_terms;
    auto list = get_edges_list();
    vector<Edge> new_list;
    for (auto edge : list) {
        int v = edge.v;
        int to = edge.to;
        char c = edge.w;
        new_list.push_back(Edge(cl[v], cl[to], c));
    }
    std::sort(new_list.begin(), new_list.end());
    new_list.erase(std::unique(new_list.begin(), new_list.end()), new_list.end());
    compress_and_assign_edges(new_list);
}


bool operator<(const Automaton::Edge& fst, const Automaton::Edge& snd) {
    if (fst.v < snd.v) {
        return true;
    } else if (fst.v == snd.v && fst.to < snd.to) {
        return true;
    } else if (fst.v == snd.v && fst.to == snd.to && fst.w < snd.w) {
        return true;
    }
    return false;
}

bool operator==(const Automaton::Edge& fst, const Automaton::Edge& snd) {
    return fst.v == snd.v && fst.to == snd.to && fst.w == snd.w;
}

bool operator<=(const Automaton::Edge& fst, const Automaton::Edge& snd) {
    return fst < snd || fst == snd;
}

bool operator>=(const Automaton::Edge& fst, const Automaton::Edge& snd) {
    return snd <= fst;
}

bool operator>(const Automaton::Edge& fst, const Automaton::Edge& snd) {
    return snd < fst;
}

bool operator!=(const Automaton::Edge& fst, const Automaton::Edge& snd) {
    return !(snd == fst);
}

string union_strings(const string& fst, const string& snd) {
    std::set<char> set_of_symbols;
    for (char c : fst) {
        set_of_symbols.insert(c);
    }
    for (char c : snd) {
        set_of_symbols.insert(c);
    }
    string result = "";
    for (char c : set_of_symbols) {
        result += c;
    }
    return result;
}

Automaton operator+(const Automaton& fst, const Automaton& snd) {
    Automaton result;
    const int fst_shift = 1;
    const int snd_shift = fst.get_vertices_count() + 1;
    result.vertices_count = 2 + fst.get_vertices_count() + snd.get_vertices_count();
    result.edges.resize(result.vertices_count);
    result.start = 0;
    result.add_edge(result.start, fst.start + fst_shift, Automaton::eps);
    result.add_edge(result.start, snd.start + snd_shift, Automaton::eps);

    result.terms = {result.vertices_count - 1};
    int fst_term = *fst.terms.begin();
    int snd_term = *snd.terms.begin();
    int res_term = *result.terms.begin();
    result.add_edge(fst_term + fst_shift, res_term, Automaton::eps);
    result.add_edge(snd_term + snd_shift, res_term, Automaton::eps);

    result.alp = union_strings(fst.get_alp(), snd.get_alp());

    for (int fst_vertex = 0; fst_vertex < fst.get_vertices_count(); ++fst_vertex) {
        for (auto edge : fst.edges[fst_vertex]) {
            char w = Automaton::get_edge_symbol(edge);
            for (int to : Automaton::get_edge_neighbours(edge)) {
                result.add_edge(fst_vertex + fst_shift, to + fst_shift, w);
            }
        }
    }

    for (int snd_vertex = 0; snd_vertex < snd.get_vertices_count(); ++snd_vertex) {
        for (auto edge : snd.edges[snd_vertex]) {
            char w = Automaton::get_edge_symbol(edge);
            for (int to : Automaton::get_edge_neighbours(edge)) {
                result.add_edge(snd_vertex + snd_shift, to + snd_shift, w);
            }
        }
    }

    return result;
}

Automaton operator-(const Automaton& fst, const Automaton& snd) {
    Automaton result;
    result.start = fst.start;
    const int fst_shift = 0;
    const int snd_shift = fst.get_vertices_count();
    result.vertices_count = fst.get_vertices_count() + snd.get_vertices_count();
    result.edges.resize(result.vertices_count);

    int fst_term = *fst.terms.begin();
    int snd_term = *snd.terms.begin();
    result.terms = {snd_term + snd_shift};
    result.add_edge(fst_term + fst_shift, snd.start + snd_shift, Automaton::eps);

    result.alp = union_strings(fst.get_alp(), snd.get_alp());

    for (int fst_vertex = 0; fst_vertex < fst.get_vertices_count(); ++fst_vertex) {
        for (auto edge : fst.edges[fst_vertex]) {
            char w = Automaton::get_edge_symbol(edge);
            for (int to : Automaton::get_edge_neighbours(edge)) {
                result.add_edge(fst_vertex + fst_shift, to + fst_shift, w);
            }
        }
    }

    for (int snd_vertex = 0; snd_vertex < snd.get_vertices_count(); ++snd_vertex) {
        for (auto edge : snd.edges[snd_vertex]) {
            char w = Automaton::get_edge_symbol(edge);
            for (int to : Automaton::get_edge_neighbours(edge)) {
                result.add_edge(snd_vertex + snd_shift, to + snd_shift, w);
            }
        }
    }

    return result;
}

Automaton operator*(const Automaton& fst) {
    Automaton result;
    result.start = 0;
    result.terms = {0};
    result.vertices_count = fst.get_vertices_count() + 1;
    result.edges.resize(result.vertices_count);
    int fst_term = *fst.terms.begin();
    const int fst_shift = 1;
    result.add_edge(result.start, fst.start + fst_shift, Automaton::eps);
    result.add_edge(fst_term + fst_shift, result.start, Automaton::eps);

    result.alp = fst.get_alp();

    for (int fst_vertex = 0; fst_vertex < fst.get_vertices_count(); ++fst_vertex) {
        for (auto edge : fst.edges[fst_vertex]) {
            char w = Automaton::get_edge_symbol(edge);
            for (int to : Automaton::get_edge_neighbours(edge)) {
                result.add_edge(fst_vertex + fst_shift, to + fst_shift, w);
            }
        }
    }

    return result;
}

std::istream& operator>>(std::istream& in, Automaton& aut) {
    in >> aut.vertices_count >> aut.start >> aut.alp;
    int k;
    in >> k;
    for (int i = 0; i < k; ++i) {
        int el;
        in >> el;
        aut.terms.insert(el);
    }
    int m;
    in >> m;
    aut.edges.resize(aut.vertices_count);
    for (int i = 0; i < m; ++i) {
        int v, to;
        char w;
        in >> v >> to >> w;
        aut.add_edge(v, to, w);
    }
    return in;
}

std::ostream& operator<<(std::ostream& out, const Automaton& aut) {
    out << aut.vertices_count << ' ' << aut.start << '\n';
    for (int v = 0; v < aut.vertices_count; ++v) {
        if (aut.terms.count(v)) {
            out << v << ' ';
        }
    }
    out << '\n';
    for (int v = 0; v < aut.vertices_count; ++v) {
        for (auto edge : aut.edges[v]) {
            char w = Automaton::get_edge_symbol(edge);
            for (int to : Automaton::get_edge_neighbours(edge)) {
                out << v << ' ' << to << ' ' << w << '\n';
            }
        }
    }
    return out;
}