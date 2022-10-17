#include "automaton.h"
#include <stack>

bool Automaton::get_bit(int mask, int pos) {
    return (mask & (1 << pos)) != 0;
}

vector<Automaton::Edge> Automaton::get_edges_list(std::set<int> removed) {
    vector<Edge> res;
    for (int v = 0; v < vertex; ++v) {
        for (const Automaton::Edge& edge : edges[v]) {
            int v = edge.v;
            int to = edge.to;
            if (removed.count(v) || removed.count(to)) continue;
            res.push_back(edge);
        }
    }
    return res;
}

void Automaton::compress_and_assign_edges(const vector<Automaton::Edge>& pairs) {
    vector<int> xs;
    for (const Edge& edge : pairs) {
        xs.push_back(edge.v);
        xs.push_back(edge.to);
    }
    std::sort(xs.begin(), xs.end());
    xs.erase(std::unique(xs.begin(), xs.end()), xs.end());
    vertex = xs.size();
    std::set<int> new_terms;
    for (auto v : terms) {
        int u = lower_bound(xs.begin(), xs.end(), v) - xs.begin();
        new_terms.insert(u);
    }
    terms = new_terms;
    edges.clear();
    edges.resize(vertex);
    for (const Edge& edge : pairs) {
        int v = lower_bound(xs.begin(), xs.end(), edge.v) - xs.begin();
        int to = lower_bound(xs.begin(), xs.end(), edge.to) - xs.begin();
        edges[v].push_back(Edge(v, to, edge.w));
    }
    start = lower_bound(xs.begin(), xs.end(), start) - xs.begin();
}

void Automaton::remove_extra_vertices() {
    std::set<int> removed;
    vector<bitset<MAX_VERTEX>> is_reach(vertex, 0);
    for (int v = 0; v < vertex; ++v) {
        is_reach[v][v] = 1;
    }
    for (int i = 0; i < vertex; ++i) {
        for (int v = 0; v < vertex; ++v) {
            for (const Edge& edge : edges[v]) {
                int to = edge.to;
                is_reach[v] |= is_reach[to];
            }
        }
    }
    for (int v = 0; v < vertex; ++v) {
        if (!is_reach[start][v]) {
            removed.insert(v);
        }
        bool ok = 0;
        for (int u = 0; u < vertex; ++u) {
            if (terms.count(u) && is_reach[v][u]) ok = 1;
        }
        if (!ok) removed.insert(v);
    }
    auto list = get_edges_list(removed);
    compress_and_assign_edges(list);
}

// void Automaton::build_go() {
//     go.clear();
//     go.resize(vertex);
//     for (int v = 0; v < vertex; ++v) {
//         for (const Edge& edge : edges[v]) {
//             int to = edge.to;
//             char w = edge.w;
//             go[v][w] = to;
//         }
//     }
// }

Automaton::Automaton(int vertex, int start, const string& alp, const vector<int>& _terms) 
    : vertex(vertex), start(start), alp(alp) {
    terms = std::set<int> (_terms.begin(), _terms.end());
    edges.resize(vertex);
}

Automaton::Automaton(char c, const string& alp = "abc") : start(0), alp(alp) {
    if (c == '1') {
        vertex = 1;
        terms = {0};
        edges.resize(vertex);
    } else {
        vertex = 2;
        terms = {1};
        edges.resize(vertex);
        edges[0].push_back(Edge(0, 1, c));
    }
}

Automaton::Automaton(const string& regex, const string& alp = "abc") : alp(alp) {
    std::stack<Automaton> st;
    for (char cur_sym : regex) {
        if (cur_sym == '+') {
            if (st.empty()) {
                throw std::runtime_error("Incorrect regex");
            }
            Automaton& snd = st.top();
            st.pop();

            if (st.empty()) {
                throw std::runtime_error("Incorrect regex");
            }
            Automaton& fst = st.top();
            st.pop();

            st.push(fst + snd);
        } else if (cur_sym == '.') {
            if (st.empty()) {
                throw std::runtime_error("Incorrect regex");
            }
            Automaton& snd = st.top();
            st.pop();

            if (st.empty()) {
                throw std::runtime_error("Incorrect regex");
            }
            Automaton& fst = st.top();
            st.pop();

            st.push(fst - snd);
        } else if (cur_sym == '*') {
            if (st.empty()) {
                throw std::runtime_error("Incorrect regex");
            }
            Automaton& fst = st.top();
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
}

int Automaton::get_vertex() const {
    return vertex;
}

int Automaton::get_start() const {
    return start;
}

string Automaton::get_alp() const {
    return alp;
}

vector<int> Automaton::get_all_next(int v, char c) {
    vector<int> res;
    for (const Edge& edge : edges[v]) {
        int to = edge.to;
        char w = edge.w;
        if (w == c) res.push_back(to);
    }
    return res;
}

void Automaton::add_edge(int v, int to, char w) {
    edges[v].push_back(Edge(v, to, w));
}

void Automaton::remove_eps_edges() {
    vector<bitset<MAX_VERTEX>> is_reach(vertex, 0);
    vector<vector<Edge>> eps_edges(vertex);
    for (int v = 0; v < vertex; ++v) {
        for (const Edge& edge : edges[v]) {
            int to = edge.to;
            char w = edge.w;
            if (is_eps(w)) {
                eps_edges[v].push_back(edge);
            }
        }
        is_reach[v][v] = 1;
    }

    for (int i = 0; i < vertex; ++i) {
        for (int v = 0; v < vertex; ++v) {
            for (const Edge& edge : eps_edges[v]) {
                int to = edge.to;
                is_reach[v] |= is_reach[to];
            }
        }
    }

    vector<vector<Edge>> new_edges(vertex);
    for (int v = 0; v < vertex; ++v) {
        for (int u = 0; u < vertex; ++u) {
            if (is_reach[v][u]) {
                for (const Edge& edge : edges[u]) {
                    int to = edge.to;
                    char w = edge.w;
                    if (!is_eps(w)) {
                        new_edges[v].push_back(Edge(v, to, w));
                    }
                }
                if (terms.count(u)) terms.insert(v);
            }
        }
    }
    edges = new_edges;
    remove_extra_vertices();
}

bool Automaton::is_dka() const {
    std::set<char> used;
    for (int v = 0; v < vertex; ++v) {
        for (const Edge& edge : edges[v]) {
            if (is_eps(edge.w) || used.count(edge.w)) return false;
            used.insert(edge.w);
        }
    }
    return true;
}

void Automaton::to_dka() {
    if (is_dka()) return;
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
        for (int v = 0; v < vertex; ++v) {
            if (terms.count(v) && get_bit(mask, v)) {
                new_terms.insert(mask);
                break;
            }
        }
        used.insert(mask);
        std::map<char, int> delta;
        for (int v = 0; v < vertex; ++v) {
            if (get_bit(mask, v)) {
                for (const Edge& edge : edges[v]) {
                    delta[edge.w] |= (1 << edge.to);
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
    // build_go();
}

void Automaton::to_pdka() {
    if (!is_dka()) to_dka();
    auto list = get_edges_list();
    int stok = vertex;
    bool ok = 0;
    for (int v = 0; v < vertex; ++v) {
        std::set<char> used;
        for (const Edge& edge : edges[v]) {
            used.insert(edge.w);
        }
        for (char c : alp) {
            if (!used.count(c)) {
                ok = 1;
                list.push_back(Edge(v, stok, c));
            }
        }
    }
    if (!ok) return;
    for (char c : alp) {
        list.push_back(Edge(stok, stok, c));
    }
    compress_and_assign_edges(list);
    // build_go();
}

void Automaton::to_mpdka() {
    to_pdka();
    vector<int> cl(vertex, 0);
    for (auto v : terms) {
        cl[v] = 1;
    }   
    for (int i = 0; i < vertex; ++i) {
        vector<pair<pair<int, std::map<char, int>>, int>> cmp(vertex);
        for (int v = 0; v < vertex; ++v) {
            cmp[v].first.first = cl[v];
            cmp[v].second = v;
            for (const Edge& edge : edges[v]) {
                int to = edge.to;
                char c = edge.w;
                (cmp[v].first.second)[c] = cl[to];
            }
        }
        std::sort(cmp.begin(), cmp.end());
        cl[cmp[0].second] = 0;
        for (int j = 1; j < vertex; ++j) {
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
    pair<pair<int, int>, char> ff = {{fst.v, fst.to}, fst.w};
    pair<pair<int, int>, char> ss = {{snd.v, snd.to}, snd.w};
    return ff < ss;
}

bool operator==(const Automaton::Edge& fst, const Automaton::Edge& snd) {
    pair<pair<int, int>, char> ff = {{fst.v, fst.to}, fst.w};
    pair<pair<int, int>, char> ss = {{snd.v, snd.to}, snd.w};
    return ff == ss;
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

Automaton operator+(const Automaton& fst, const Automaton& snd) {
    Automaton result;
    const int fst_shift = 1;
    const int snd_shift = fst.get_vertex() + 1;
    result.vertex = 2 + fst.get_vertex() + snd.get_vertex();
    result.edges.resize(result.vertex);
    result.start = 0;
    result.add_edge(result.start, fst.start + fst_shift, Automaton::eps);
    result.add_edge(result.start, snd.start + snd_shift, Automaton::eps);

    result.terms = {result.vertex - 1};
    int fst_term = *fst.terms.begin();
    int snd_term = *snd.terms.begin();
    int res_term = *result.terms.begin();
    result.add_edge(fst_term + fst_shift, res_term, Automaton::eps);
    result.add_edge(snd_term + snd_shift, res_term, Automaton::eps);

    for (int fst_vertex = 0; fst_vertex < fst.get_vertex(); ++fst_vertex) {
        for (auto edge : fst.edges[fst_vertex]) {
            int to = edge.to;
            char w = edge.w;
            result.add_edge(fst_vertex + fst_shift, to + fst_shift, w);
        }
    }

    for (int snd_vertex = 0; snd_vertex < snd.get_vertex(); ++snd_vertex) {
        for (auto edge : snd.edges[snd_vertex]) {
            int to = edge.to;
            char w = edge.w;
            result.add_edge(snd_vertex + snd_shift, to + snd_shift, w);
        }
    }

    return result;
}

Automaton operator-(const Automaton& fst, const Automaton& snd) {
    Automaton result;
    result.start = 0;
    const int fst_shift = 0;
    const int snd_shift = fst.get_vertex();
    result.vertex = fst.get_vertex() + snd.get_vertex();
    result.edges.resize(result.vertex);

    result.terms = {result.vertex - 1};
    int fst_term = *fst.terms.begin();
    int snd_term = *snd.terms.begin();
    result.add_edge(fst_term + fst_shift, snd.start + snd_shift, Automaton::eps);

    for (int fst_vertex = 0; fst_vertex < fst.get_vertex(); ++fst_vertex) {
        for (auto edge : fst.edges[fst_vertex]) {
            int to = edge.to;
            char w = edge.w;
            result.add_edge(fst_vertex + fst_shift, to + fst_shift, w);
        }
    }

    for (int snd_vertex = 0; snd_vertex < snd.get_vertex(); ++snd_vertex) {
        for (auto edge : snd.edges[snd_vertex]) {
            int to = edge.to;
            char w = edge.w;
            result.add_edge(snd_vertex + snd_shift, to + snd_shift, w);
        }
    }

    return result;
}

Automaton operator*(const Automaton& fst) {
    Automaton result;
    result.start = 0;
    result.terms = {0};
    result.vertex = fst.get_vertex() + 1;
    int fst_term = *fst.terms.begin();
    const int fst_shift = 1;
    result.add_edge(result.start, fst.start + fst_shift, Automaton::eps);
    result.add_edge(fst_term + fst_shift, result.start, Automaton::eps);

    for (int fst_vertex = 0; fst_vertex < fst.get_vertex(); ++fst_vertex) {
        for (auto edge : fst.edges[fst_vertex]) {
            int to = edge.to;
            char w = edge.w;
            result.add_edge(fst_vertex + fst_shift, to + fst_shift, w);
        }
    }

    return result;
}

std::istream& operator>>(std::istream& in, Automaton& aut) {
    in >> aut.vertex >> aut.start >> aut.alp;
    int k;
    in >> k;
    for (int i = 0; i < k; ++i) {
        int el;
        in >> el;
        aut.terms.insert(el);
    }
    int m;
    in >> m;
    aut.edges.resize(aut.vertex);
    for (int i = 0; i < m; ++i) {
        int v, to;
        char w;
        in >> v >> to >> w;
        aut.add_edge(v, to, w);
    }
    return in;
}

std::ostream& operator<<(std::ostream& out, const Automaton& aut) {
    out << aut.vertex << ' ' << aut.start << '\n';
    for (int v = 0; v < aut.vertex; ++v) {
        if (aut.terms.count(v)) {
            out << v << ' ';
        }
    }
    out << '\n';
    for (int v = 0; v < aut.vertex; ++v) {
        for (auto edge : aut.edges[v]) {
            out << v << ' ' << edge.to << ' ' << edge.w << '\n';
        }
    }
    return out;
}