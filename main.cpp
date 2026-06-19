#include "strategies.hpp"
#include <iostream>
#include <vector>
#include <memory>

static int get_payoff(Move a, Move b) {
    if (a == Move::COOPERATE && b == Move::COOPERATE) return 3;
    if (a == Move::COOPERATE && b == Move::DEFECT) return 0;
    if (a == Move::DEFECT && b == Move::COOPERATE) return 5;
    return 1;
}

int main() {
    std::vector<std::unique_ptr<IStrategy>> strategies;

    strategies.push_back(std::make_unique<Holy>());
    strategies.push_back(std::make_unique<Traitor>());
    strategies.push_back(std::make_unique<TitForTat>());
    strategies.push_back(std::make_unique<Friedman>());
    strategies.push_back(std::make_unique<RandomStrategy>());
    strategies.push_back(std::make_unique<Joss>());
    strategies.push_back(std::make_unique<TitFor2Tat>());
    strategies.push_back(std::make_unique<TwoTitsForTat>());
    strategies.push_back(std::make_unique<Pavlov>());
    strategies.push_back(std::make_unique<GenerousTitForTat>());
    strategies.push_back(std::make_unique<Average64>());
    strategies.push_back(std::make_unique<Shubik>());
    strategies.push_back(std::make_unique<Davis>());
    strategies.push_back(std::make_unique<Graaskamp>());
    strategies.push_back(std::make_unique<GRASR>());

    strategies.push_back(std::make_unique<K31R>());
    strategies.push_back(std::make_unique<K32R>());
    strategies.push_back(std::make_unique<K33R>());
    strategies.push_back(std::make_unique<K35R>());
    strategies.push_back(std::make_unique<K36R>());
    strategies.push_back(std::make_unique<K37R>());
    strategies.push_back(std::make_unique<K38R>());
    strategies.push_back(std::make_unique<K39R>());
    strategies.push_back(std::make_unique<K40R>());
    strategies.push_back(std::make_unique<K41R>());
    strategies.push_back(std::make_unique<K42R>());
    strategies.push_back(std::make_unique<K43R>());
    strategies.push_back(std::make_unique<K44R>());
    strategies.push_back(std::make_unique<K45R>());
    strategies.push_back(std::make_unique<K46R>());
    strategies.push_back(std::make_unique<K47R>());
    strategies.push_back(std::make_unique<K48R>());
    strategies.push_back(std::make_unique<K49R>());
    strategies.push_back(std::make_unique<K50R>());
    strategies.push_back(std::make_unique<K51R>());
    strategies.push_back(std::make_unique<K52R>());
    strategies.push_back(std::make_unique<K53R>());
    strategies.push_back(std::make_unique<K54R>());
    strategies.push_back(std::make_unique<K55R>());
    strategies.push_back(std::make_unique<K58R>());
    strategies.push_back(std::make_unique<K59R>());
    strategies.push_back(std::make_unique<K60R>());
    strategies.push_back(std::make_unique<K61R>());
    strategies.push_back(std::make_unique<K62R>());
    strategies.push_back(std::make_unique<K63R>());
    strategies.push_back(std::make_unique<K64R>());
    strategies.push_back(std::make_unique<K65R>());
    strategies.push_back(std::make_unique<K66R>());
    strategies.push_back(std::make_unique<K67R>());
    strategies.push_back(std::make_unique<K68R>());
    strategies.push_back(std::make_unique<K69R>());
    strategies.push_back(std::make_unique<K70R>());
    strategies.push_back(std::make_unique<K71R>());
    strategies.push_back(std::make_unique<K72R>());
    strategies.push_back(std::make_unique<K73R>());
    strategies.push_back(std::make_unique<K74R>());
    strategies.push_back(std::make_unique<K74RXX>());
    strategies.push_back(std::make_unique<K75R>());
    strategies.push_back(std::make_unique<K76R>());
    strategies.push_back(std::make_unique<K77R>());
    strategies.push_back(std::make_unique<K79R>());
    strategies.push_back(std::make_unique<K80R>());
    strategies.push_back(std::make_unique<K81R>());
    strategies.push_back(std::make_unique<K82R>());
    strategies.push_back(std::make_unique<K83R>());
    strategies.push_back(std::make_unique<K84R>());
    strategies.push_back(std::make_unique<K85R>());
    strategies.push_back(std::make_unique<K86R>());
    strategies.push_back(std::make_unique<K87R>());
    strategies.push_back(std::make_unique<K88R>());
    strategies.push_back(std::make_unique<K89R>());
    strategies.push_back(std::make_unique<K91R>());
    strategies.push_back(std::make_unique<KPavlovC>());

    strategies.push_back(std::make_unique<Adaptive>());
    strategies.push_back(std::make_unique<AdaptorBrief>());
    strategies.push_back(std::make_unique<AdaptorLong>());
    strategies.push_back(std::make_unique<Alternator>());
    strategies.push_back(std::make_unique<EvolvedANN>());
    strategies.push_back(std::make_unique<EvolvedANN5>());
    strategies.push_back(std::make_unique<EvolvedANNNoise05>());
    strategies.push_back(std::make_unique<APavlov2006>());
    strategies.push_back(std::make_unique<APavlov2011>());
    strategies.push_back(std::make_unique<Appeaser>());
    strategies.push_back(std::make_unique<AverageCopier>());
    strategies.push_back(std::make_unique<NiceAverageCopier>());
    strategies.push_back(std::make_unique<FirstByDowning>());
    strategies.push_back(std::make_unique<FirstByFeld>());
    strategies.push_back(std::make_unique<FirstByGrofman>());
    strategies.push_back(std::make_unique<FirstByNydegger>());
    strategies.push_back(std::make_unique<FirstBySteinAndRapoport>());
    strategies.push_back(std::make_unique<FirstByTidemanAndChieruzzi>());
    strategies.push_back(std::make_unique<FirstByTullock>());
    strategies.push_back(std::make_unique<FirstByAnonymous>());
    strategies.push_back(std::make_unique<SecondByChampion>());
    strategies.push_back(std::make_unique<SecondByEatherley>());
    strategies.push_back(std::make_unique<SecondByTester>());
    strategies.push_back(std::make_unique<SecondByGladstein>());
    strategies.push_back(std::make_unique<SecondByBorufsen>());
    strategies.push_back(std::make_unique<BackStabber>());
    strategies.push_back(std::make_unique<DoubleCrosser>());
    strategies.push_back(std::make_unique<BetterAndBetter>());

    const int ROUNDS = 500;
    const int NUM = strategies.size();
    std::vector<int> scores(NUM, 0);

    for (int i = 0; i < NUM; ++i) {
        for (int j = 0; j < NUM; ++j) {
            strategies[i]->reset();
            strategies[j]->reset();

            uint64_t hist_i = 0, hist_j = 0;
            int score_i = 0, score_j = 0;

            for (int r = 0; r < ROUNDS; ++r) {
                Move mi = strategies[i]->getMove(hist_i, hist_j);
                Move mj = strategies[j]->getMove(hist_j, hist_i);

                score_i += get_payoff(mi, mj);
                score_j += get_payoff(mj, mi);

                hist_i = (hist_i << 1) | (mj == Move::DEFECT ? 1ULL : 0ULL);
                hist_j = (hist_j << 1) | (mi == Move::DEFECT ? 1ULL : 0ULL);
            }
            scores[i] += score_i;
            scores[j] += score_j;
        }
    }

    std::cout << "Scores after round-robin tournament (" << ROUNDS << " rounds each):\n";
    for (int i = 0; i < NUM; ++i) {
        std::cout << "Strategy " << i << " : " << scores[i] << "\n";
    }
    return 0;
}
