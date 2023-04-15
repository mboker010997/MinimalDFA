#include "automaton.h"
#include "gtest/gtest.h"
#include <fstream>


TEST(Construction_Test, Automaton_by_Regex) {
    string s = "ab";
    EXPECT_THROW(Automaton aut(s), std::runtime_error);

    s = "ab.";
    EXPECT_NO_THROW(Automaton aut(s));

    s = "a*";
    EXPECT_NO_THROW(Automaton aut(s));

    s = "a*.";
    EXPECT_THROW(Automaton aut(s), std::runtime_error);

    s = "ab+*bc+.ac+*."; // (a + b)*(b + c)(a + c)*
    EXPECT_NO_THROW(Automaton aut(s));
}

TEST(MPDKA_TEST, MPDKA_TEST) {
    string s = "ab+*bc+.ac+*.";
    Automaton aut(s);
    aut.to_mpdka();
    cout << aut;
    EXPECT_EQ(aut.get_vertices_count(), 4);
}

TEST(TASK_TEST, TASK_TEST) {
    const int INF = 1e9;
    string s = "ab.";
    Automaton aut(s);
    EXPECT_EQ(aut.get_max_end_by_letter('b'), 1);
    EXPECT_EQ(aut.get_max_end_by_letter('a'), 0);

    s = "ab+*bc+.ac+*.";
    aut = Automaton(s);
    EXPECT_EQ(aut.get_max_end_by_letter('a'), INF);
    EXPECT_EQ(aut.get_max_end_by_letter('b'), INF);
    EXPECT_EQ(aut.get_max_end_by_letter('c'), INF);
}
