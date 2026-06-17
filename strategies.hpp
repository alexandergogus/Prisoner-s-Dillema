#ifndef STRATEGIES_HPP
#define STRATEGIES_HPP

#include "istrategy.hpp"
#include "move.hpp"
#include <random>
#include <bitset>
#include <cmath>
#include <vector>
#include <algorithm>

class Holy : public IStrategy {
public:
    void reset() override {}
    Move getMove(uint64_t, uint64_t) override { return Move::COOPERATE; }
};

class Traitor : public IStrategy {
public:
    void reset() override {}
    Move getMove(uint64_t, uint64_t) override { return Move::DEFECT; }
};

class TitForTat : public IStrategy {
public:
    void reset() override {}
    Move getMove(uint64_t opp_history, uint64_t) override {
        if (opp_history == 0) return Move::COOPERATE;
        return (opp_history & 1) ? Move::DEFECT : Move::COOPERATE;
    }
};

class Friedman : public IStrategy {
private:
    bool ever_defected_ = false;
    int prev_rounds_ = 0;
public:
    void reset() override { ever_defected_ = false; prev_rounds_ = 0; }
    Move getMove(uint64_t opp_history, uint64_t) override {
        int rounds = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++rounds; tmp >>= 1; }
        int current_round = rounds + 1;
        if (current_round == 1 || current_round < prev_rounds_) ever_defected_ = false;
        prev_rounds_ = current_round;
        if (opp_history & 1) ever_defected_ = true;
        return ever_defected_ ? Move::DEFECT : Move::COOPERATE;
    }
};

class RandomStrategy : public IStrategy {
private:
    std::mt19937 rng_;
    std::bernoulli_distribution dist_;
public:
    RandomStrategy() : rng_(std::random_device{}()), dist_(0.5) {}
    void reset() override { rng_.seed(std::random_device{}()); }
    Move getMove(uint64_t, uint64_t) override {
        return dist_(rng_) ? Move::DEFECT : Move::COOPERATE;
    }
};

class Joss : public IStrategy {
private:
    std::mt19937 rng_;
    std::bernoulli_distribution random_defect_;
public:
    Joss() : rng_(std::random_device{}()), random_defect_(0.1) {}
    void reset() override { rng_.seed(std::random_device{}()); }
    Move getMove(uint64_t opp_history, uint64_t) override {
        if (opp_history == 0) return Move::COOPERATE;
        if (opp_history & 1) return Move::DEFECT;
        return random_defect_(rng_) ? Move::DEFECT : Move::COOPERATE;
    }
};

class TitFor2Tat : public IStrategy {
public:
    void reset() override {}
    Move getMove(uint64_t opp_history, uint64_t) override {
        if (opp_history == 0) return Move::COOPERATE;
        return ((opp_history & 3) == 3) ? Move::DEFECT : Move::COOPERATE;
    }
};

class TwoTitsForTat : public IStrategy {
public:
    void reset() override {}
    Move getMove(uint64_t opp_history, uint64_t) override {
        if (opp_history == 0) return Move::COOPERATE;
        return (opp_history & 3) ? Move::DEFECT : Move::COOPERATE;
    }
};

class Pavlov : public IStrategy {
public:
    void reset() override {}
    Move getMove(uint64_t opp_history, uint64_t my_history) override {
        if (opp_history == 0) return Move::COOPERATE;
        bool last_opp = opp_history & 1;
        bool last_me = my_history & 1;
        bool win = (last_me == last_opp);
        bool next = win ? last_me : !last_me;
        return next ? Move::DEFECT : Move::COOPERATE;
    }
};

class GenerousTitForTat : public IStrategy {
private:
    std::mt19937 rng_;
    std::bernoulli_distribution forgive_;
public:
    GenerousTitForTat() : rng_(std::random_device{}()), forgive_(2.0/3.0) {}
    void reset() override { rng_.seed(std::random_device{}()); }
    Move getMove(uint64_t opp_history, uint64_t) override {
        if (opp_history == 0) return Move::COOPERATE;
        if (!(opp_history & 1)) return Move::COOPERATE;
        return forgive_(rng_) ? Move::DEFECT : Move::COOPERATE;
    }
};

class Average64 : public IStrategy {
public:
    void reset() override {}
    Move getMove(uint64_t opp_history, uint64_t) override {
        if (opp_history == 0) return Move::COOPERATE;
        size_t defects = std::bitset<64>(opp_history).count();
        size_t rounds = 64 - __builtin_clzll(opp_history);
        double rate = static_cast<double>(defects) / rounds;
        return (rate <= 0.5) ? Move::COOPERATE : Move::DEFECT;
    }
};

class TidemanChieruzzi : public IStrategy {
private:
    std::mt19937 rng_;
    std::bernoulli_distribution coop_prob_;
public:
    TidemanChieruzzi() : rng_(std::random_device{}()), coop_prob_(0.85) {}
    void reset() override { rng_.seed(std::random_device{}()); }
    Move getMove(uint64_t opp_history, uint64_t my_history) override {
        int opp_defects = std::bitset<64>(opp_history).count();
        int my_defects = std::bitset<64>(my_history).count();
        int rounds = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++rounds; tmp >>= 1; }
        if (rounds == 0) return Move::DEFECT;
        bool last_opp = opp_history & 1;
        if (opp_defects >= 5 && my_defects <= opp_defects - 3) return Move::DEFECT;
        int consecutive_coop = 0;
        uint64_t hist = opp_history;
        while (hist && !(hist & 1)) { ++consecutive_coop; hist >>= 1; }
        if (consecutive_coop >= 3) {
            return coop_prob_(rng_) ? Move::COOPERATE : Move::DEFECT;
        }
        if (last_opp) {
            uint64_t prev_two = (opp_history >> 1) & 3;
            if (prev_two == 0) return Move::COOPERATE;
            return Move::DEFECT;
        }
        return Move::COOPERATE;
    }
};

class Nydegger : public IStrategy {
private:
    static const uint8_t table[64];
public:
    void reset() override {}
    Move getMove(uint64_t opp_history, uint64_t my_history) override {
        int rounds = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++rounds; tmp >>= 1; }
        if (rounds == 0) return Move::COOPERATE;
        if (rounds == 1) return (opp_history & 1) ? Move::DEFECT : Move::COOPERATE;
        if (rounds == 2) return (opp_history & 1) ? Move::DEFECT : Move::COOPERATE;
        int opp3 = opp_history & 0b111;
        int my3 = my_history & 0b111;
        int index = (my3 << 3) | opp3;
        return table[index] ? Move::DEFECT : Move::COOPERATE;
    }
};
const uint8_t Nydegger::table[64] = {
    0,0,0,1, 0,0,1,1,
    0,0,1,1, 0,1,1,1,
    0,1,1,1, 1,1,1,1,
    0,1,1,1, 1,1,1,1,
    0,0,1,1, 1,1,1,1,
    0,0,1,1, 1,1,1,1,
    0,1,1,1, 1,1,1,1,
    1,1,1,1, 1,1,1,1
};

class Grogman : public IStrategy {
private:
    std::mt19937 rng_;
    std::bernoulli_distribution forgive_;
public:
    Grogman() : rng_(std::random_device{}()), forgive_(2.0/7.0) {}
    void reset() override { rng_.seed(std::random_device{}()); }
    Move getMove(uint64_t opp_history, uint64_t my_history) override {
        int round = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++round; tmp >>= 1; }
        if (round < 2) return Move::COOPERATE;
        if (round < 7) return (opp_history & 1) ? Move::DEFECT : Move::COOPERATE;
        bool last_me = my_history & 1;
        bool last_opp = opp_history & 1;
        if (last_me == last_opp) return Move::COOPERATE;
        return forgive_(rng_) ? Move::COOPERATE : Move::DEFECT;
    }
};

class Shubik : public IStrategy {
private:
    int grudge_level_ = 1;
    int retaliation_remaining_ = 0;
    bool in_retaliation_ = false;
public:
    void reset() override {
        grudge_level_ = 1;
        retaliation_remaining_ = 0;
        in_retaliation_ = false;
    }
    Move getMove(uint64_t opp_history, uint64_t my_history) override {
        int round = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++round; tmp >>= 1; }
        if (round == 0) return Move::COOPERATE;
        bool last_me = my_history & 1;
        bool last_opp = opp_history & 1;
        if (in_retaliation_) {
            if (retaliation_remaining_ > 0) {
                retaliation_remaining_--;
                if (retaliation_remaining_ == 0) in_retaliation_ = false;
                return Move::DEFECT;
            }
            in_retaliation_ = false;
        }
        if (!in_retaliation_ && last_opp && !last_me) grudge_level_++;
        if (last_opp) {
            in_retaliation_ = true;
            retaliation_remaining_ = grudge_level_ - 1;
            return Move::DEFECT;
        }
        return Move::COOPERATE;
    }
};

class SteinRapoport : public IStrategy {
public:
    void reset() override {}
    Move getMove(uint64_t opp_history, uint64_t my_history) override {
        int round = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++round; tmp >>= 1; }
        if (round < 4) return Move::COOPERATE;
        int total_rounds = 0;
        tmp = my_history;
        while (tmp) { ++total_rounds; tmp >>= 1; }
        total_rounds = round + 1;
        if (round >= total_rounds - 2) return Move::DEFECT;
        return (opp_history & 1) ? Move::DEFECT : Move::COOPERATE;
    }
};

class Davis : public IStrategy {
public:
    void reset() override {}
    Move getMove(uint64_t opp_history, uint64_t) override {
        int round = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++round; tmp >>= 1; }
        if (round < 10) return Move::COOPERATE;
        return (opp_history != 0) ? Move::DEFECT : Move::COOPERATE;
    }
};

class Graaskamp : public IStrategy {
private:
    bool opponent_is_random_ = false;
    int next_random_defection_turn_ = -1;
    int rounds_played_at_last_check_ = 0;
    std::mt19937 rng_;
    std::uniform_int_distribution<int> dist_;
public:
    Graaskamp() : rng_(std::random_device{}()), dist_(5, 15) {}
    void reset() override {
        opponent_is_random_ = false;
        next_random_defection_turn_ = -1;
        rounds_played_at_last_check_ = 0;
        rng_.seed(std::random_device{}());
    }
    Move getMove(uint64_t opp_history, uint64_t my_history) override {
        int round = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++round; tmp >>= 1; }
        int round_one_indexed = round + 1;
        if (round_one_indexed < 50) {
            if (round == 0) return Move::COOPERATE;
            return (opp_history & 1) ? Move::DEFECT : Move::COOPERATE;
        }
        if (round_one_indexed == 51) return Move::DEFECT;
        if (round_one_indexed <= 56) {
            return (opp_history & 1) ? Move::DEFECT : Move::COOPERATE;
        }
        if (round_one_indexed - rounds_played_at_last_check_ >= 50) {
            int opp_defections = std::bitset<64>(opp_history).count();
            int opp_cooperations = (round_one_indexed - 1) - opp_defections;
            double expected = (round_one_indexed - 1) / 2.0;
            double chi_square = (std::pow(opp_cooperations - expected, 2) / expected) +
                                (std::pow(opp_defections - expected, 2) / expected);
            opponent_is_random_ = (chi_square < 3.841);
            rounds_played_at_last_check_ = round_one_indexed;
        }
        if (opponent_is_random_) return Move::DEFECT;
        bool opponent_is_tft = true;
        if (round_one_indexed >= 2) {
            bool my_last = my_history & 1;
            bool opp_last = opp_history & 1;
            if (opp_last != my_last) opponent_is_tft = false;
        }
        bool opponent_is_clone = (opp_history == my_history);
        if (opponent_is_tft || opponent_is_clone) {
            return (opp_history & 1) ? Move::DEFECT : Move::COOPERATE;
        }
        if (next_random_defection_turn_ == -1) {
            next_random_defection_turn_ = round_one_indexed + dist_(rng_);
        }
        if (round_one_indexed == next_random_defection_turn_) {
            next_random_defection_turn_ = round_one_indexed + dist_(rng_);
            return Move::DEFECT;
        }
        return Move::COOPERATE;
    }
};

class FirstByDowning : public IStrategy {
private:
    int opponent_coop_after_my_coop_ = 0;
    int opponent_coop_after_my_defect_ = 0;
    int total_my_coop_ = 0;
    int total_my_defect_ = 0;
public:
    void reset() override {
        opponent_coop_after_my_coop_ = 0;
        opponent_coop_after_my_defect_ = 0;
        total_my_coop_ = 0;
        total_my_defect_ = 0;
    }
    Move getMove(uint64_t opp_history, uint64_t my_history) override {
        int round = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++round; tmp >>= 1; }
        if (round == 0) {
            total_my_coop_ = 1;
            total_my_defect_ = 1;
            opponent_coop_after_my_coop_ = 0;
            opponent_coop_after_my_defect_ = 0;
            return Move::DEFECT;
        }
        if (round == 1) {
            bool opp_last = opp_history & 1;
            if (!opp_last) opponent_coop_after_my_coop_++;
            total_my_coop_++;
            total_my_defect_++;
            return Move::DEFECT;
        }
        bool my_prev = (my_history >> 1) & 1;
        bool opp_prev = (opp_history >> 1) & 1;
        if (!my_prev) {
            total_my_coop_++;
            if (!opp_prev) opponent_coop_after_my_coop_++;
        } else {
            total_my_defect_++;
            if (!opp_prev) opponent_coop_after_my_defect_++;
        }
        double alpha = opponent_coop_after_my_coop_ / static_cast<double>(total_my_coop_);
        double beta  = opponent_coop_after_my_defect_ / static_cast<double>(total_my_defect_);
        const int R = 3, P = 1, S = 0, T = 5;
        double coop_exp = alpha * R + (1 - alpha) * S;
        double defect_exp = beta * T + (1 - beta) * P;
        if (coop_exp > defect_exp) return Move::COOPERATE;
        if (coop_exp < defect_exp) return Move::DEFECT;
        bool my_last = my_history & 1;
        return my_last ? Move::COOPERATE : Move::DEFECT;
    }
};

class Feld : public IStrategy {
private:
    std::mt19937 rng_;
public:
    Feld() : rng_(std::random_device{}()) {}
    void reset() override { rng_.seed(std::random_device{}()); }
    Move getMove(uint64_t opp_history, uint64_t) override {
        int rounds = 0, opp_defects = 0;
        uint64_t hist = opp_history;
        while (hist) {
            if (hist & 1) opp_defects++;
            rounds++;
            hist >>= 1;
        }
        if (rounds == 0) return Move::COOPERATE;
        bool last_opp = opp_history & 1;
        if (!last_opp) return Move::COOPERATE;
        double prob = static_cast<double>(opp_defects) / rounds;
        std::bernoulli_distribution dist(prob);
        return dist(rng_) ? Move::DEFECT : Move::COOPERATE;
    }
};

class Tullock : public IStrategy {
public:
    void reset() override {}
    Move getMove(uint64_t opp_history, uint64_t) override {
        int rounds = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++rounds; tmp >>= 1; }
        if (rounds < 5) return Move::COOPERATE;
        if (rounds == 5) return Move::DEFECT;
        return (opp_history & 1) ? Move::DEFECT : Move::COOPERATE;
    }
};

class GRASR : public IStrategy {
private:
    int nmov[4] = {0};
    int mmove = 0;
    int igame = 0;
    int n = 0;
    std::mt19937 rng_;
    std::uniform_real_distribution<double> dist_;
public:
    GRASR() : rng_(std::random_device{}()), dist_(0.0, 1.0) {}
    void reset() override {
        for (int i = 0; i < 4; ++i) nmov[i] = 0;
        mmove = 0;
        igame = 0;
        n = 0;
        rng_.seed(std::random_device{}());
    }
    int opponent_score(uint64_t opp_history, uint64_t my_history) {
        int rounds = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++rounds; tmp >>= 1; }
        if (rounds == 0) return 0;
        int score = 0;
        for (int i = 0; i < rounds; ++i) {
            int bitpos = rounds - 1 - i;
            bool opp = (opp_history >> bitpos) & 1;
            bool me  = (my_history  >> bitpos) & 1;
            if (!opp && !me)      score += 3;
            else if (!opp && me)  score += 0;
            else if (opp && !me)  score += 5;
            else                  score += 1;
        }
        return score;
    }
    Move getMove(uint64_t opp_history, uint64_t my_history) override {
        int rounds = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++rounds; tmp >>= 1; }
        int moven = rounds + 1;
        if (moven == 1) return Move::COOPERATE;
        if (moven < 51) return (opp_history & 1) ? Move::DEFECT : Move::COOPERATE;
        if (moven == 51) return Move::DEFECT;
        if (moven >= 52 && moven <= 56) {
            if (moven == 52) return (opp_history & 1) ? Move::DEFECT : Move::COOPERATE;
            int idx = moven - 52;
            int jpick = (opp_history & 1) ? 1 : 0;
            nmov[idx-1] = mmove + jpick;
            mmove = (jpick == 0) ? 2 : 4;
            return (opp_history & 1) ? Move::DEFECT : Move::COOPERATE;
        }
        if (moven == 57) {
            int jscor = opponent_score(opp_history, my_history);
            if (jscor >= 135) {
                int j = nmov[1];
                if (j == 1 || j == 2) {
                    igame = 1;
                    n = static_cast<int>(dist_(rng_) * 10.0 + 5.0);
                    return Move::COOPERATE;
                }
                else if (j == 3) {
                    if (nmov[0] == 2 && nmov[2] == 4 && nmov[3] == 2) igame = 4;
                    else { igame = 1; n = static_cast<int>(dist_(rng_) * 10.0 + 5.0); }
                    return Move::COOPERATE;
                }
                else if (j == 4) {
                    if (nmov[0] == 3 && nmov[2] == 3) igame = 2;
                    else { igame = 1; n = static_cast<int>(dist_(rng_) * 10.0 + 5.0); }
                    return Move::COOPERATE;
                }
                else if (j == 5) {
                    if (nmov[0] == 5 && nmov[2] == 5) igame = 2;
                    else { igame = 1; n = static_cast<int>(dist_(rng_) * 10.0 + 5.0); }
                    return Move::COOPERATE;
                }
            } else {
                igame = 3;
                return Move::DEFECT;
            }
        }
        switch (igame) {
            case 1:
                if (n <= 0) {
                    n = static_cast<int>(dist_(rng_) * 10.0 + 5.0);
                    return Move::DEFECT;
                } else {
                    n--;
                    return (opp_history & 1) ? Move::DEFECT : Move::COOPERATE;
                }
            case 2:
                return (opp_history & 1) ? Move::DEFECT : Move::COOPERATE;
            case 3:
                return Move::DEFECT;
            case 4:
                if (moven >= 118) igame = 2;
                return Move::COOPERATE;
            default:
                return Move::COOPERATE;
        }
    }
};

class K31R : public IStrategy {
public:
    void reset() override {}
    Move getMove(uint64_t opp_history, uint64_t) override {
        if (opp_history == 0) return Move::COOPERATE;
        int rounds = 0, defects = 0;
        uint64_t hist = opp_history;
        while (hist) {
            if (hist & 1) ++defects;
            ++rounds;
            hist >>= 1;
        }
        return (defects * 2 < rounds) ? Move::COOPERATE : Move::DEFECT;
    }
};

class K32R : public IStrategy {
private:
    int C1 = 0, C2 = 0, C3 = 0, C4 = 0;
    int J2 = 0, J1 = 0, I2 = 0, I1 = 0;
    std::mt19937 rng_;
    std::uniform_real_distribution<double> dist_;
public:
    K32R() : rng_(std::random_device{}()), dist_(0.0, 1.0) {}
    void reset() override {
        C1 = C2 = C3 = C4 = 0;
        J2 = J1 = I2 = I1 = 0;
        rng_.seed(std::random_device{}());
    }
    Move getMove(uint64_t opp_history, uint64_t my_history) override {
        int rounds = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++rounds; tmp >>= 1; }
        int M = rounds + 1;
        int move = 0;
        if (M == 1) return Move::COOPERATE;
        int J = (opp_history & 1) ? 1 : 0;
        if (M > 2) {
            I2 = (my_history >> 1) & 1;
            I1 = my_history & 1;
            if (I2 == 0) { if (J == 0) C3++; else C4++; }
            else { if (J == 0) C1++; else C2++; }
        }
        if (M >= 27) {
            int total_def_resp = C1 + C2;
            int total_coop_resp = C3 + C4;
            double threshold1 = (total_def_resp - 1.5 * sqrt(total_def_resp)) / 2.0;
            double threshold2 = (total_coop_resp - 1.5 * sqrt(total_coop_resp)) / 2.0;
            if (!(C1 < threshold1) && !(C4 < threshold2)) {
                move = 1;
                goto update;
            }
        }
        if (M >= 3) J1 = (opp_history >> 1) & 1;
        if (M >= 4) J2 = (opp_history >> 2) & 1;
        if (J1 == J) {
            if (J2 == J1) move = J;
            else {
                double P = 0.9;
                move = J;
                if (dist_(rng_) >= P) move = 1 - J;
            }
        } else {
            double P = (J == 1) ? 0.6 : 0.7;
            move = J;
            if (dist_(rng_) >= P) move = 1 - J;
        }
    update:
        J2 = J1;
        J1 = J;
        I2 = I1;
        I1 = move;
        return (move == 0) ? Move::COOPERATE : Move::DEFECT;
    }
};

class K33R : public IStrategy {
private:
    double coop[4] = {0.0};
    double count[4] = {0.0};
    int last1 = 1;
    int last2 = 1;
    bool twin = true;
public:
    void reset() override {
        for (int i = 0; i < 4; ++i) { coop[i] = 0.0; count[i] = 0.0; }
        last1 = 1; last2 = 1; twin = true;
    }
    Move getMove(uint64_t opp_history, uint64_t my_history) override {
        int rounds = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++rounds; tmp >>= 1; }
        int M = rounds + 1;
        int J = (opp_history & 1) ? 1 : 0;
        if (M == 1) return Move::COOPERATE;
        if (M > 2) {
            int idx_prev = 2 * last2 + last1 + 1;
            int idx0 = idx_prev - 1;
            coop[idx0] += (1 - J);
            count[idx0] += 1.0;
        }
        if (J != last1) twin = false;
        int idx_cur = 2 * last2 + last1 + 1;
        int move = 0;
        if (M <= 22) {
            move = (idx_cur == 1 || idx_cur == 2) ? 1 : 0;
        } else {
            if (twin) move = 0;
            else {
                double p[4];
                for (int i = 0; i < 4; ++i) p[i] = (count[i] > 0.0) ? (coop[i] / count[i]) : 0.0;
                const double CONST[6] = {0.0, 4.0, 6.0, 6.0, 8.0, 12.0};
                const double COEFF[6][4] = {
                    {36.0, 0.0, 0.0, 0.0},
                    {16.0, 12.0, 12.0, 0.0},
                    {0.0, 18.0, 24.0, 0.0},
                    {12.0, 12.0, 9.0, 9.0},
                    {0.0, 16.0, 16.0, 12.0},
                    {0.0, 0.0, 0.0, 48.0}
                };
                double best = -1e9;
                int best_policy = 0;
                for (int pol = 0; pol < 6; ++pol) {
                    double sum = CONST[pol];
                    for (int hist = 0; hist < 4; ++hist) sum += COEFF[pol][hist] * p[hist];
                    if (sum > best) { best = sum; best_policy = pol; }
                }
                switch (best_policy) {
                    case 0: move = 0; break;
                    case 1: move = (idx_cur == 1) ? 1 : 0; break;
                    case 2: move = (idx_cur == 1 || idx_cur == 3) ? 1 : 0; break;
                    case 3: move = (idx_cur == 1 || idx_cur == 2) ? 1 : 0; break;
                    case 4: move = (idx_cur == 1 || idx_cur == 2 || idx_cur == 3) ? 1 : 0; break;
                    case 5: move = 1; break;
                    default: move = 0;
                }
            }
        }
        last2 = last1;
        last1 = move;
        return (move == 0) ? Move::COOPERATE : Move::DEFECT;
    }
};

class K35R : public IStrategy {
private:
    double flack = 0.0;
    std::mt19937 rng_;
    std::uniform_real_distribution<double> dist_;
public:
    K35R() : rng_(std::random_device{}()), dist_(0.0, 1.0) {}
    void reset() override { flack = 0.0; rng_.seed(std::random_device{}()); }
    Move getMove(uint64_t opp_history, uint64_t) override {
        int rounds = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++rounds; tmp >>= 1; }
        int M = rounds + 1;
        if (M == 1) return Move::COOPERATE;
        int J = (opp_history & 1) ? 1 : 0;
        flack = (flack + J) * 0.5;
        double R = dist_(rng_);
        return (flack > R) ? Move::DEFECT : Move::COOPERATE;
    }
};

class K36R : public IStrategy {
private:
    std::mt19937 rng_;
    std::uniform_real_distribution<double> dist_;
public:
    K36R() : rng_(std::random_device{}()), dist_(0.0, 1.0) {}
    void reset() override { rng_.seed(std::random_device{}()); }
    Move getMove(uint64_t opp_history, uint64_t) override {
        int rounds = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++rounds; tmp >>= 1; }
        int M = rounds + 1;
        double probc;
        if (M < 100) probc = 0.1;
        else if (M < 200) probc = 0.05;
        else if (M < 300) probc = 0.15;
        else probc = 0.0;
        double R = dist_(rng_);
        return (R < probc) ? Move::COOPERATE : Move::DEFECT;
    }
};

class K37R : public IStrategy {
private:
    int nd = 0;
public:
    void reset() override { nd = 0; }
    Move getMove(uint64_t opp_history, uint64_t) override {
        int rounds = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++rounds; tmp >>= 1; }
        int M = rounds + 1;
        if (M == 1) return Move::COOPERATE;
        int J = (opp_history & 1) ? 1 : 0;
        nd += J;
        return (5 * nd > M) ? Move::DEFECT : Move::COOPERATE;
    }
};

class K38R : public IStrategy {
private:
    int move = 0;
    int jhis = 0;
public:
    void reset() override { move = 0; jhis = 0; }
    Move getMove(uint64_t opp_history, uint64_t) override {
        int rounds = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++rounds; tmp >>= 1; }
        int M = rounds + 1;
        if (M == 1) return Move::COOPERATE;
        int J = (opp_history & 1) ? 1 : 0;
        if (move == 0) {
            jhis = ((jhis & 3) << 1) | J;
            if (jhis == 0b111) move = 1;
        }
        return (move == 1) ? Move::DEFECT : Move::COOPERATE;
    }
};

class K39R : public IStrategy {
private:
    int STEP = 1, SUBSTP = 1;
    int BOTHD = 0, TITCNT = 0, TATCNT = 0;
    int EVIL = 0, N = 1, F = 0;
    int TOTK = 0, OLDMOV = 0, COUNT = 0, VOLDMV = 0;
    int OK[4] = {0,0,0,0};
public:
    void reset() override {
        STEP = 1; SUBSTP = 1; BOTHD = 0; TITCNT = 0; TATCNT = 0;
        EVIL = 0; N = 1; F = 0; TOTK = 0; OLDMOV = 0; COUNT = 0; VOLDMV = 0;
        for (int i = 0; i < 4; ++i) OK[i] = 0;
    }
    int compute_K(uint64_t opp_history, uint64_t my_history) {
        int K = 0;
        uint64_t o_hist = opp_history;
        uint64_t m_hist = my_history;
        while (o_hist || m_hist) {
            bool opp = (o_hist & 1);
            bool me  = (m_hist & 1);
            if (!me && !opp)      K += 3;
            else if (!me && opp)  K += 0;
            else if (me && !opp)  K += 5;
            else                  K += 1;
            o_hist >>= 1;
            m_hist >>= 1;
        }
        return K;
    }
    Move getMove(uint64_t opp_history, uint64_t my_history) override {
        int M = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++M; tmp >>= 1; }
        ++M;
        if (M == 1) return Move::COOPERATE;
        int J = (opp_history & 1) ? 1 : 0;
        int prev_own = (my_history & 1) ? 1 : 0;
        int K = compute_K(opp_history, my_history);
        if (prev_own + J == 2) BOTHD++;
        if (prev_own + J < 2) BOTHD = 0;
        COUNT--;
        int move = 0;
        VOLDMV = OLDMOV;
        OLDMOV = J;
        if (J == 1) TATCNT++;
        if (EVIL == 0 && J == 1) EVIL = 1;
        bool done = false;
        while (!done) {
            switch (STEP) {
                case 1:
                    switch (SUBSTP) {
                        case 1:
                            COUNT = 10;
                            TATCNT = 0;
                            TITCNT = 0;
                            SUBSTP = 2;
                            break;
                        case 2:
                            if ((VOLDMV + OLDMOV) == 2) move = 1;
                            TITCNT += move;
                            if (COUNT == 0) SUBSTP = 3;
                            done = true;
                            break;
                        case 3: {
                            int OLDSTP = STEP;
                            OK[STEP] = K - TOTK;
                            TOTK = K;
                            SUBSTP = 1;
                            if (TATCNT > 0) {
                                STEP = 1;
                                for (int i1 = 1; i1 <= 2; ++i1)
                                    for (int i2 = 2; i2 <= 3; ++i2)
                                        if (OK[i1] != 0 && OK[i2] != 0 && OK[i1] >= OK[i2] && STEP == i1)
                                            STEP = i2;
                                if (STEP != 3 && OK[STEP+1] == 0 && (TATCNT >= 4 || TITCNT == 0))
                                    STEP++;
                                if (STEP < OLDSTP && BOTHD > 0) STEP = 5;
                            } else {
                                STEP = 4;
                                if (EVIL == 1) STEP = 1;
                                if (EVIL == 0) EVIL = -1;
                            }
                            continue;
                        }
                    }
                    break;
                case 2:
                    switch (SUBSTP) {
                        case 1:
                            COUNT = 10;
                            TATCNT = 0;
                            TITCNT = 0;
                            SUBSTP = 2;
                            break;
                        case 2:
                            if (OLDMOV == 1) move = 1;
                            TITCNT += move;
                            if (COUNT == 0) SUBSTP = 3;
                            done = true;
                            break;
                        case 3: {
                            int OLDSTP = STEP;
                            OK[STEP] = K - TOTK;
                            TOTK = K;
                            SUBSTP = 1;
                            if (TATCNT > 0) {
                                STEP = 1;
                                for (int i1 = 1; i1 <= 2; ++i1)
                                    for (int i2 = 2; i2 <= 3; ++i2)
                                        if (OK[i1] != 0 && OK[i2] != 0 && OK[i1] >= OK[i2] && STEP == i1)
                                            STEP = i2;
                                if (STEP != 3 && OK[STEP+1] == 0 && (TATCNT >= 4 || TITCNT == 0))
                                    STEP++;
                                if (STEP < OLDSTP && BOTHD > 0) STEP = 5;
                            } else {
                                STEP = 4;
                                if (EVIL == 1) STEP = 1;
                                if (EVIL == 0) EVIL = -1;
                            }
                            continue;
                        }
                    }
                    break;
                case 3:
                    switch (SUBSTP) {
                        case 1:
                            COUNT = 10;
                            TATCNT = 0;
                            TITCNT = 0;
                            SUBSTP = 2;
                            break;
                        case 2:
                            move = 1;
                            TITCNT++;
                            if (COUNT == 0) SUBSTP = 3;
                            done = true;
                            break;
                        case 3: {
                            int OLDSTP = STEP;
                            OK[STEP] = K - TOTK;
                            TOTK = K;
                            SUBSTP = 1;
                            if (TATCNT > 0) {
                                STEP = 1;
                                for (int i1 = 1; i1 <= 2; ++i1)
                                    for (int i2 = 2; i2 <= 3; ++i2)
                                        if (OK[i1] != 0 && OK[i2] != 0 && OK[i1] >= OK[i2] && STEP == i1)
                                            STEP = i2;
                                if (STEP != 3 && OK[STEP+1] == 0 && (TATCNT >= 4 || TITCNT == 0))
                                    STEP++;
                                if (STEP < OLDSTP && BOTHD > 0) STEP = 5;
                            } else {
                                STEP = 4;
                                if (EVIL == 1) STEP = 1;
                                if (EVIL == 0) EVIL = -1;
                            }
                            continue;
                        }
                    }
                    break;
                case 4:
                    switch (SUBSTP) {
                        case 1:
                            SUBSTP = 2;
                            move = 1;
                            COUNT = N;
                            TATCNT = 0;
                            done = true;
                            break;
                        case 2:
                            if (COUNT == 0) SUBSTP = 3;
                            done = true;
                            break;
                        case 3:
                            if (TATCNT != 0) {
                                if (F == 0) {
                                    SUBSTP = 4;
                                    if (J == 1) N++;
                                    TATCNT = J;
                                    done = true;
                                } else {
                                    N++;
                                    SUBSTP = 1;
                                    STEP = 1;
                                    continue;
                                }
                            } else {
                                F = 1;
                                SUBSTP = 1;
                                continue;
                            }
                            break;
                        case 4:
                            if (TATCNT <= 4) done = true;
                            else { SUBSTP = 1; STEP = 1; continue; }
                            break;
                    }
                    break;
                case 5:
                    switch (SUBSTP) {
                        case 1:
                            COUNT = 5;
                            SUBSTP = 2;
                        case 2:
                            if (COUNT != 0) done = true;
                            else { SUBSTP = 1; STEP = 1; continue; }
                            break;
                    }
                    break;
            }
        }
        return (move == 1) ? Move::DEFECT : Move::COOPERATE;
    }
};

class K40R : public IStrategy {
private:
    int S = 3;
    int W = 0;
    double Q = 0.8;
    std::mt19937 rng_;
    std::uniform_real_distribution<double> dist_;
public:
    K40R() : rng_(std::random_device{}()), dist_(0.0, 1.0) {}
    void reset() override { S = 3; W = 0; Q = 0.8; rng_.seed(std::random_device{}()); }
    Move getMove(uint64_t opp_history, uint64_t my_history) override {
        int M = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++M; tmp >>= 1; }
        ++M;
        int J = (opp_history & 1) ? 1 : 0;
        int JA = (my_history & 1) ? 1 : 0;
        double R = dist_(rng_);
        int result = JA;
        if (M != 1) goto label505;
        S = 3; W = 0; Q = 0.8;
    label505:
        S = S + 1;
        if (J != 1) goto label510;
        W = W + 1;
        Q = Q / 2.0;
    label510:
        if (M >= 3) goto label520;
        result = 0;
        return (result == 0) ? Move::COOPERATE : Move::DEFECT;
    label520:
        if (J == 1) goto label522;
        goto label530;
    label522:
        W = W + 1;
        if (W > 2 && (W % 3 == 0 || (W - 1) % 3 == 0)) goto label901;
        goto label550;
    label901:
        S = 1;
        Q = Q / 2.0;
        goto label580;
    label530:
        goto label580;
    label550:
        if (R >= Q) goto label560;
        result = 0;
        Q = Q / 2.0;
        return (result == 0) ? Move::COOPERATE : Move::DEFECT;
    label560:
        Q = Q / 2.0;
        result = 1;
        return (result == 0) ? Move::COOPERATE : Move::DEFECT;
    label580:
        if (S == 1 || S == 2) goto label1000;
        if (W > 2 && (W % 3 == 0 || (W - 1) % 3 == 0)) goto label901;
        result = 0;
        return (result == 0) ? Move::COOPERATE : Move::DEFECT;
    label1000:
        result = 1;
        return (result == 0) ? Move::COOPERATE : Move::DEFECT;
    }
};

class K41R : public IStrategy {
private:
    int ICASE = 1;
    int IFORGV = 0;
    int LAST[12] = {0};
public:
    void reset() override {
        ICASE = 1;
        IFORGV = 0;
        for (int i = 0; i < 12; ++i) LAST[i] = 0;
    }
    Move getMove(uint64_t opp_history, uint64_t my_history) override {
        int M = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++M; tmp >>= 1; }
        ++M;
        int J = (opp_history & 1) ? 1 : 0;
        int JA = (my_history & 1) ? 1 : 0;
        int result = JA;
        if (M == 1) return Move::COOPERATE;
        if (ICASE == 1) goto label100;
        if (ICASE == 2) goto label200;
        if (ICASE == 3) goto label300;
    label100:
        result = J;
        ICASE = J + 1;
        goto label400;
    label200:
        result = J;
        ICASE = 3;
        if (J == 1) ICASE = 1;
        goto label400;
    label300:
        result = J;
        if (IFORGV < M) result = 0;
        IFORGV = IFORGV + 20 * J;
        ICASE = 1;
        goto label400;
    label400:
        int LSUM = 0;
        for (int i = 0; i < 12; ++i) LSUM += LAST[i];
        for (int i = 0; i < 11; ++i) LAST[i] = LAST[i+1];
        LAST[11] = J;
        if (LSUM >= 5) result = 1;
        return (result == 0) ? Move::COOPERATE : Move::DEFECT;
    }
};

class K42R : public IStrategy {
private:
    int L3MOV = 0, L3ECH = 0, IDEF = 0, ICOOP = 0;
    int IPICK = 0, I2PCK = 0, J2PCK = 0;
    int MHIST[2][2] = {{0,0},{0,0}};
public:
    void reset() override {
        L3MOV = L3ECH = IDEF = ICOOP = IPICK = I2PCK = J2PCK = 0;
        for (int i = 0; i < 2; ++i) for (int j = 0; j < 2; ++j) MHIST[i][j] = 0;
    }
    Move getMove(uint64_t opp_history, uint64_t my_history) override {
        int rounds = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++rounds; tmp >>= 1; }
        int MOVEN = rounds + 1;
        int JPICK = (opp_history & 1) ? 1 : 0;
        int JA = (my_history & 1) ? 1 : 0;
        int result = JA;
        if (MOVEN == 1) {
            result = 0;
            goto finish;
        }
        if (MOVEN > 2) MHIST[I2PCK][JPICK]++;
        if (IDEF != 0) {
            result = 1;
            goto periodic_check;
        }
        if (IPICK != 0 && JPICK != 0) {
            L3MOV++;
            if (L3MOV >= 3) {
                result = 0;
                L3MOV = 0;
                L3ECH = 0;
                goto periodic_check;
            }
        } else {
            L3MOV = 0;
            if (IPICK == JPICK) L3ECH = 0;
            else {
                if (JPICK == I2PCK && IPICK == J2PCK) {
                    L3ECH++;
                    if (L3ECH >= 3) { L3ECH = 0; L3MOV = 0; ICOOP = 1; }
                } else L3ECH = 0;
            }
        }
        result = JPICK;
    periodic_check:
        if ((MOVEN - 2) % 25 == 0 && MOVEN != 2) {
            IDEF = 0;
            int JNCOP = MHIST[0][0] + MHIST[1][0];
            if (JNCOP > 17) {}
            else if (JNCOP < 8) { if (JNCOP < 3) IDEF = 1; }
            else { if (100 * MHIST[0][0] / JNCOP < 70) IDEF = 1; }
            for (int i = 0; i < 2; ++i) for (int j = 0; j < 2; ++j) MHIST[i][j] = 0;
            if (IDEF != 0) { ICOOP = 0; L3MOV = 0; L3ECH = 0; result = 1; }
        }
        if (ICOOP != 0 && result != 0) { ICOOP = 0; result = 0; }
    finish:
        I2PCK = IPICK;
        J2PCK = JPICK;
        IPICK = result;
        return (result == 0) ? Move::COOPERATE : Move::DEFECT;
    }
};

class K43R : public IStrategy {
private:
    int NCC = 0, NCD = 0, NDC = 0, NDD = 0;
    int KOUNT = 0, MYTWIN = 0, IOLD1 = 0, IOLD2 = 0;
public:
    void reset() override {
        NCC = NCD = NDC = NDD = KOUNT = MYTWIN = IOLD1 = IOLD2 = 0;
    }
    Move getMove(uint64_t opp_history, uint64_t my_history) override {
        int M = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++M; tmp >>= 1; }
        ++M;
        int J = (opp_history & 1) ? 1 : 0;
        int result = IOLD1;
        if (M == 1) {
            result = 0;
            goto finish;
        }
        if (M >= 3) {
            if (IOLD2 == 1) { if (J == 0) NDC++; else NDD++; }
            else { if (J == 0) NCC++; else NCD++; }
        }
        IOLD2 = IOLD1;
        if (M < 16) {
            if (J == 0) result = 0;
            else { if (KOUNT >= 3) result = 0; else { KOUNT++; result = 1; } }
        } else {
            if (M == 17 && J == 1 && NCD == 1 && NDD == 0) MYTWIN = 1;
            if ((NCD * 3) >= (NCC + NCD)) result = 1;
            else {
                if ((M % 4) != 0) result = 0;
                else {
                    if (MYTWIN == 1) result = 0;
                    else { if (NDC >= (M / 12) || NDD == 0) result = 1; else result = 0; }
                }
            }
        }
    finish:
        IOLD1 = result;
        return (result == 0) ? Move::COOPERATE : Move::DEFECT;
    }
};

class K44R : public IStrategy {
private:
    int MC = 0;
    double F = 2.0;
    double AM = 4.0;
    std::mt19937 rng_;
    std::uniform_real_distribution<double> dist_;
public:
    K44R() : rng_(std::random_device{}()), dist_(0.0, 1.0) {}
    void reset() override { MC = 0; F = 2.0; AM = 4.0; rng_.seed(std::random_device{}()); }
    Move getMove(uint64_t opp_history, uint64_t my_history) override {
        int M = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++M; tmp >>= 1; }
        ++M;
        int J = (opp_history & 1) ? 1 : 0;
        int result = (my_history & 1) ? 1 : 0;
        if (M == 1) return Move::COOPERATE;
        if (M < 3) return Move::COOPERATE;
        MC = MC + J;
        if (MC < AM) result = 0;
        else if (MC == AM) result = 1;
        else {
            AM = AM / F;
            MC = 0;
            double R = dist_(rng_);
            result = (R < AM) ? 0 : 1;
        }
        return (result == 0) ? Move::COOPERATE : Move::DEFECT;
    }
};

class K45R : public IStrategy {
private:
    int JOLD = 0;
    int A = 0, B = 0, C = 0, D = 0, E = 0;
public:
    void reset() override {
        JOLD = 0; A = 0; B = 0; C = 0; D = 0; E = 0;
    }
    Move getMove(uint64_t opp_history, uint64_t my_history) override {
        int M = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++M; tmp >>= 1; }
        ++M;
        int J = (opp_history & 1) ? 1 : 0;
        int JA = (my_history & 1) ? 1 : 0;
        int result = JA;
        if (M > 3) {
            if (C == 1) {
                result = J;
            } else if (B == 1) {
                result = 0;
                if (JOLD == 1 && J == 1) result = 1;
                JOLD = J;
            } else if (A == 1) {
                result = 1;
                E = E + 1;
                if (E != 8) {
                    if (!(JOLD == 1 && J == 1)) result = 0;
                    JOLD = J;
                } else {
                    E = 0;
                    JOLD = J;
                }
            } else if (D == 1) {
                if (J == 1) {
                    result = 1;
                    C = 1;
                } else {
                    result = 0;
                    B = 1;
                }
            } else {
                if (J == 1) {
                    result = 0;
                    C = 1;
                } else {
                    result = 0;
                    B = 1;
                }
            }
        } else if (M == 1) {
            JOLD = 0; A = 0; B = 0; C = 0; E = 0;
            result = 1;
        } else if (M == 2) {
            if (J == 1) {
                result = 0;
                D = 1;
            } else {
                result = 0;
                D = 0;
            }
        } else if (M == 3) {
            if (J == 1) {
                result = 0;
                if (D == 1) C = 1;
            } else {
                if (D == 1) {
                    result = 0;
                } else {
                    result = 0;
                    A = 1;
                }
            }
        }
        return (result == 1) ? Move::DEFECT : Move::COOPERATE;
    }
};

class K46R : public IStrategy {
private:
    int NJ = 0;
    std::mt19937 rng;
    std::uniform_real_distribution<double> dist;
public:
    K46R() : rng(std::random_device{}()), dist(0.0, 1.0) {}
    void reset() override {
        NJ = 0;
        rng.seed(std::random_device{}());
    }
    Move getMove(uint64_t opp_history, uint64_t) override {
        int M = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++M; tmp >>= 1; }
        ++M;
        int J = (opp_history & 1) ? 1 : 0;
        if (M == 1) NJ = 0;
        NJ += J;
        if (J == 0) return Move::COOPERATE;
        double P = static_cast<double>(NJ) / (M - 1);
        return (dist(rng) < P) ? Move::DEFECT : Move::COOPERATE;
    }
};

class K47R : public IStrategy {
private:
    int NUM = 2, DEN = 2, RF = 20;
    const int DEF = 1;
    const int COOP = 0;
    int LONG = 1, SHORT = 5;
    int SH2[5] = {1,1,1,1,1};
    int N = 1;
    int MYLAST = 0;
    int MYMOVE = 0;
    std::mt19937 rng;
public:
    K47R() : rng(std::random_device{}()) {}
    void reset() override {
        NUM = 2; DEN = 2; RF = 20;
        LONG = 1; SHORT = 5;
        for (int i = 0; i < 5; ++i) SH2[i] = 1;
        N = 1;
        MYLAST = 0;
        MYMOVE = 0;
        rng.seed(std::random_device{}());
    }
    Move getMove(uint64_t opp_history, uint64_t) override {
        int M = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++M; tmp >>= 1; }
        ++M;
        int J = (opp_history & 1) ? 1 : 0;
        if (M <= RF && J == DEF) {
            RF = M + (20 * NUM) / DEN + 1;
        }
        N = (N % 4) + 1;
        SHORT = SHORT - SH2[N-1];
        if (J == MYLAST) {
            LONG += 1;
            SHORT += 1;
            SH2[N-1] = 1;
        } else {
            SH2[N-1] = 0;
        }
        MYLAST = MYMOVE;
        MYMOVE = J;
        if ((LONG < 0.625 * M) || (SHORT < 3)) MYMOVE = DEF;
        if ((LONG > 0.9 * M) && (SHORT == 5)) MYMOVE = COOP;
        if (M == RF) MYMOVE = DEF;
        if (M >= RF + 2) {
            MYMOVE = COOP;
            NUM = NUM + J;
            DEN = DEN + 1 - J;
            RF = M + (20 * NUM) / DEN + 1;
        }
        return (MYMOVE == 1) ? Move::DEFECT : Move::COOPERATE;
    }
};

static int compute_score(uint64_t my_history, uint64_t opp_history) {
    int score = 0;
    int rounds = 0;
    uint64_t tmp = opp_history;
    while (tmp) { ++rounds; tmp >>= 1; }
    for (int i = rounds - 1; i >= 0; --i) {
        bool my_move = (my_history >> i) & 1;
        bool opp_move = (opp_history >> i) & 1;
        if (!my_move && !opp_move)      score += 3;
        else if (!my_move && opp_move)  score += 0;
        else if (my_move && !opp_move)  score += 5;
        else                            score += 1;
    }
    return score;
}

class K48R : public IStrategy {
private:
    int IARRAY[6];
    const int IPO2[6] = {0, 2, 4, 3, 5, 1};
    int KOLD, K5, KLAST;
    int ICHAN, IPO1, MM;
public:
    K48R() { reset(); }
    void reset() override {
        KOLD = 0; K5 = 0; KLAST = 0;
        for (int i = 1; i <= 5; ++i) IARRAY[i] = 0;
        ICHAN = 1;
        IPO1 = 1;
        MM = 0;
    }
    Move getMove(uint64_t opp_history, uint64_t my_history) override {
        int M = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++M; tmp >>= 1; }
        ++M;
        int J = (opp_history & 1) ? 1 : 0;
        int JA = (my_history & 1) ? 1 : 0;
        (void)JA;
        if (M <= 5) {
            int idx = IPO2[IPO1];
            IARRAY[idx] = J;
            IPO1 = IPO1 + J;
            return (J == 1) ? Move::DEFECT : Move::COOPERATE;
        }
        MM = ((M - 1) % 5) + 1;
        int result = IARRAY[MM];
        if (MM != 1) {
            return (result == 1) ? Move::DEFECT : Move::COOPERATE;
        }
        KOLD = K5;
        int K = compute_score(my_history, opp_history);
        K5 = K - KLAST;
        KLAST = K;
        if (KOLD > K5) {
            ICHAN = -ICHAN;
            IPO1 = IPO1 + ICHAN;
        }
        if (IPO1 >= 1 && IPO1 <= 5) {
            int idx = IPO2[IPO1];
            IARRAY[idx] = IARRAY[idx] + ICHAN;
            IPO1 = IPO1 + ICHAN;
        } else {
            if (IPO1 < 1) IPO1 = 0;
            if (IPO1 > 5) IPO1 = 6;
        }
        result = IARRAY[MM];
        return (result == 1) ? Move::DEFECT : Move::COOPERATE;
    }
};

class K49R : public IStrategy {
private:
    int JDSUM = 0;
    std::mt19937 rng;
    std::uniform_real_distribution<double> dist;
public:
    K49R() : rng(std::random_device{}()), dist(0.0, 1.0) {}
    void reset() override {
        JDSUM = 0;
        rng.seed(std::random_device{}());
    }
    Move getMove(uint64_t opp_history, uint64_t) override {
        int M = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++M; tmp >>= 1; }
        ++M;
        int J = (opp_history & 1) ? 1 : 0;
        if (M == 1) JDSUM = 0;
        if (J == 1) ++JDSUM;
        int JDPC = (100 * JDSUM) / M;
        if (J == 0) return Move::COOPERATE;
        if (J == 1 && JDSUM <= 17) {
            double R = dist(rng);
            return (R >= 0.5) ? Move::DEFECT : Move::COOPERATE;
        }
        if (J == 1 && JDSUM > 17) return Move::DEFECT;
        if (M > 19 && JDPC > 79) return Move::DEFECT;
        if (M > 29 && JDPC > 65) return Move::DEFECT;
        if (M > 39 && JDPC > 39) return Move::DEFECT;
        return Move::COOPERATE;
    }
};

class K50R : public IStrategy {
private:
    std::mt19937 rng;
    std::uniform_real_distribution<double> dist;
public:
    K50R() : rng(std::random_device{}()), dist(0.0, 1.0) {}
    void reset() override {
        rng.seed(std::random_device{}());
    }
    Move getMove(uint64_t opp_history, uint64_t) override {
        int J = (opp_history & 1) ? 1 : 0;
        double R = dist(rng);
        if (J == 0 && R >= 0.9) return Move::DEFECT;
        return Move::COOPERATE;
    }
};

class K51R : public IStrategy {
private:
    int LASTI = 0;
public:
    void reset() override { LASTI = 0; }
    Move getMove(uint64_t opp_history, uint64_t) override {
        int M = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++M; tmp >>= 1; }
        ++M;
        int J = (opp_history & 1) ? 1 : 0;
        int result = 0;
        if (M > 8) {
            result = 0;
            LASTI = LASTI - 1;
            if (LASTI == 3) result = 1;
            if (LASTI > 0) return (result == 1) ? Move::DEFECT : Move::COOPERATE;
            if (J == 1) result = 1;
            if (J == 1) LASTI = 4;
        } else {
            result = 0;
            if (M == 6) result = 1;
            LASTI = 0;
        }
        return (result == 1) ? Move::DEFECT : Move::COOPERATE;
    }
};

class K52R : public IStrategy {
private:
    int D9 = 0, D8 = 0;
    std::mt19937 rng;
    std::uniform_real_distribution<double> dist;
public:
    K52R() : rng(std::random_device{}()), dist(0.0, 1.0) {}
    void reset() override {
        D9 = 0; D8 = 0;
        rng.seed(std::random_device{}());
    }
    Move getMove(uint64_t opp_history, uint64_t) override {
        int M = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++M; tmp >>= 1; }
        ++M;
        int J = (opp_history & 1) ? 1 : 0;
        if (M == 1) { D9 = 0; D8 = 0; }
        D9 = D9 + 1;
        if (J == 0) D9 = 0;
        int result = 0;
        if (D9 >= 2) result = 1;
        if (D9 >= (5 + 3 * D8)) {
            D9 = 0;
            D8 = D8 + 1;
        }
        double R = dist(rng);
        if (R <= 0.05) result = 1 - result;
        return (result == 1) ? Move::DEFECT : Move::COOPERATE;
    }
};

class K53R : public IStrategy {
private:
    int C[10] = {0};
    std::mt19937 rng;
    std::uniform_real_distribution<double> dist;
public:
    K53R() : rng(std::random_device{}()), dist(0.0, 1.0) {}
    void reset() override {
        for (int i = 0; i < 10; ++i) C[i] = 0;
        rng.seed(std::random_device{}());
    }
    Move getMove(uint64_t opp_history, uint64_t) override {
        int M = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++M; tmp >>= 1; }
        ++M;
        int J = (opp_history & 1) ? 1 : 0;
        if (M <= 10) {
            C[M-1] = J;
        } else {
            for (int i = 1; i < 10; ++i) C[i-1] = C[i];
            C[9] = J;
        }
        int D = 0;
        for (int i = 0; i < 10; ++i) if (C[i] == 1) ++D;
        double R = dist(rng);
        bool defect = false;
        if (D >= 9) {
            if (R < 0.94) defect = true;
        } else if (D == 8 || D == 4 || D == 3) {
            if (R < 0.915) defect = true;
        } else if (D == 7 || D == 6 || D == 5 || D == 2) {
            if (R < 0.87) defect = true;
        } else if (D == 1) {
            if (R < 0.23) defect = true;
        } else {
            defect = false;
        }
        return defect ? Move::DEFECT : Move::COOPERATE;
    }
};

class K54R : public IStrategy {
private:
    int OPDEF = 0, STDEF = 0, COOPS = 0, NODEF = 0, ND = 12;
    double DL = 0.20;
    bool OKDEF = true, MYDEF = false;
public:
    void reset() override {
        OPDEF = 0; STDEF = 0; COOPS = 0; NODEF = 0; ND = 12;
        DL = 0.20; OKDEF = true; MYDEF = false;
    }
    Move getMove(uint64_t opp_history, uint64_t) override {
        int M = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++M; tmp >>= 1; }
        ++M;
        int J = (opp_history & 1) ? 1 : 0;
        if (M == 1) return Move::COOPERATE;
        if (M == 20) DL = 0.10;
        if (J == 0) {
            STDEF = 0;
            ++COOPS;
            if (OPDEF > M * DL) goto excessive;
            if (M % ND == 0 && OKDEF) {
                ++NODEF;
                if (NODEF % 6 == 0) --ND;
                if (ND < 1) ND = 1;
                return Move::DEFECT;
            }
            return Move::COOPERATE;
        } else {
            if (M <= 4) return Move::DEFECT;
            ++STDEF;
            ++OPDEF;
            if (MYDEF) OKDEF = false;
            if (OPDEF > M * DL) goto excessive;
            if (STDEF > 2) goto excessive;
            return Move::COOPERATE;
        }
    excessive:
        if (20 * OPDEF <= COOPS * M) return Move::COOPERATE;
        return Move::DEFECT;
    }
};

class K55R : public IStrategy {
private:
    double ALPHA = 1.0, BETA = 0.0;
    int IOLD = 0, QCA = 0, QNA = 0, QCB = 0, QNB = 0, MUTDEF = 0;
public:
    void reset() override {
        ALPHA = 1.0; BETA = 0.0; IOLD = 0;
        QCA = 0; QNA = 0; QCB = 0; QNB = 0; MUTDEF = 0;
    }
    Move getMove(uint64_t opp_history, uint64_t my_history) override {
        int M = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++M; tmp >>= 1; }
        ++M;
        int J = (opp_history & 1) ? 1 : 0;
        int JA = (my_history & 1) ? 1 : 0;
        int result = JA;
        if (M == 1) {
            result = 0;
        } else if (M > 2) {
            if (IOLD == 0) {
                if (J == 0) ++QCA;
                ++QNA;
                ALPHA = static_cast<double>(QCA) / QNA;
            } else {
                if (J == 0) ++QCB;
                ++QNB;
                BETA = static_cast<double>(QCB) / QNB;
            }
        }
        IOLD = result;
        double POLC = 6.0 * ALPHA - 9.0 * BETA - 2.0;
        double POLALT = 4.0 * ALPHA - 6.0 * BETA - 1.0;
        if (POLC >= 0.0 && POLC >= POLALT) {
            result = 0;
        } else if (POLC >= 0.0 && POLC < POLALT) {
            result = 1 - result;
        } else if (POLALT >= 0.0) {
            result = 1 - result;
        } else {
            result = 1;
            if (J == 0 || IOLD == 0) {
                MUTDEF = 0;
            } else {
                ++MUTDEF;
                if (MUTDEF > 3) result = 0;
            }
        }
        return (result == 1) ? Move::DEFECT : Move::COOPERATE;
    }
};

class K58R : public IStrategy {
private:
    int KAM = 0, NPHA = 0;
public:
    void reset() override { KAM = 0; NPHA = 0; }
    Move getMove(uint64_t opp_history, uint64_t my_history) override {
        int M = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++M; tmp >>= 1; }
        ++M;
        if (M == 1) { KAM = 0; NPHA = 0; }
        if (KAM > 6) return Move::DEFECT;
        if (NPHA >= 1) {
            --NPHA;
            if (NPHA == 0) return Move::DEFECT;
            return Move::COOPERATE;
        }
        if ((M / 18) * 18 == M && KAM > 2) --KAM;
        if ((M / 6) * 6 != M) return Move::COOPERATE;
        int K = compute_score(my_history, opp_history);
        if (K < M) {
            KAM += 2;
        } else if (K * 10 < M * 15) {
            KAM += 1;
        } else if (K < M * 2) {
            KAM += 1;
        } else if (K * 10 < M * 25) {
            KAM += 1;
        } else {
            return Move::COOPERATE;
        }
        NPHA = 2;
        return Move::DEFECT;
    }
};

class K59R : public IStrategy {
private:
    int PAST = 0, NICE1 = 0, NICE2 = 0, TOTCOP = 0, TOTDEF = 0;
    double GOOD = 1.0, BAD = 0.0;
    const int COOP = 0, DEFECT = 1;
public:
    void reset() override {
        PAST = 0; NICE1 = 0; NICE2 = 0; TOTCOP = 0; TOTDEF = 0;
        GOOD = 1.0; BAD = 0.0;
    }
    Move getMove(uint64_t opp_history, uint64_t my_history) override {
        int M = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++M; tmp >>= 1; }
        ++M;
        int J = (opp_history & 1) ? 1 : 0;
        int JA = (my_history & 1) ? 1 : 0;
        int result = JA;
        if (M == 1) {
            GOOD = 1.0; BAD = 0.0; PAST = 0;
            TOTCOP = 0; TOTDEF = 0; NICE1 = 0; NICE2 = 0;
            result = COOP;
        } else if (M > 2) {
            if (PAST == DEFECT) {
                if (J == COOP) ++NICE2;
                ++TOTDEF;
                BAD = static_cast<double>(NICE2) / TOTDEF;
            } else {
                if (J == COOP) ++NICE1;
                ++TOTCOP;
                GOOD = static_cast<double>(NICE1) / TOTCOP;
            }
        }
        PAST = result;
        double C = 6.0 * GOOD - 8.0 * BAD - 2.0;
        double ALT = 4.0 * GOOD - 5.0 * BAD - 1.0;
        if (C >= 0.0 && C >= ALT) {
            result = COOP;
        } else if (C >= 0.0 && C < ALT) {
            result = 1 - result;
        } else if (ALT >= 0.0) {
            result = 1 - result;
        } else {
            result = DEFECT;
        }
        return (result == DEFECT) ? Move::DEFECT : Move::COOPERATE;
    }
};

class K60R : public IStrategy {
private:
    int ID = 0;
public:
    void reset() override { ID = 0; }
    Move getMove(uint64_t opp_history, uint64_t my_history) override {
        int M = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++M; tmp >>= 1; }
        ++M;
        int J = (opp_history & 1) ? 1 : 0;
        int result = J;
        if (M == 1) {
            ID = 0;
            result = 0;
        } else if (ID == 1) {
            result = 1;
        } else {
            result = J;
            int K = compute_score(my_history, opp_history);
            if ((M == 11 && K < 23) ||
                (M == 21 && K < 53) ||
                (M == 31 && K < 83) ||
                (M == 41 && K < 113) ||
                (M == 51 && K < 143) ||
                (M == 101 && K < 293)) {
                ID = 1;
                result = 1;
            }
        }
        return (result == 1) ? Move::DEFECT : Move::COOPERATE;
    }
};

class K61R : public IStrategy {
private:
    int ICOOP = 0;
    std::mt19937 rng;
    std::uniform_real_distribution<double> dist;
public:
    K61R() : rng(std::random_device{}()), dist(0.0, 1.0) {}
    void reset() override {
        ICOOP = 0;
        rng.seed(std::random_device{}());
    }
    Move getMove(uint64_t opp_history, uint64_t) override {
        int M = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++M; tmp >>= 1; }
        ++M;
        int J = (opp_history & 1) ? 1 : 0;
        if (M == 1) {
            ICOOP = 0;
            return Move::COOPERATE;
        }
        if (J == 0) ++ICOOP;
        if (M <= 10) return Move::COOPERATE;
        int result = J;
        if (M <= 25) return (result == 1) ? Move::DEFECT : Move::COOPERATE;
        result = 0;
        double COPRAT = static_cast<double>(ICOOP) / M;
        double R = dist(rng);
        if (J == 1 && COPRAT < 0.6 && R > COPRAT) result = 1;
        return (result == 1) ? Move::DEFECT : Move::COOPERATE;
    }
};

class K62R : public IStrategy {
private:
    int JOLD = 0;
    int IRAN = 0;
    std::mt19937 rng;
    std::uniform_real_distribution<double> dist;
public:
    K62R() : rng(std::random_device{}()), dist(0.0, 1.0) {}
    void reset() override {
        JOLD = 0;
        IRAN = 0;
        rng.seed(std::random_device{}());
    }
    Move getMove(uint64_t opp_history, uint64_t) override {
        int M = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++M; tmp >>= 1; }
        ++M;
        int J = (opp_history & 1) ? 1 : 0;
        if (M == 1) {
            double R = dist(rng);
            IRAN = static_cast<int>(23 * R) + 1;
            JOLD = 0;
            return Move::COOPERATE;
        }
        int result = 0;
        if (M == IRAN) {
            result = 1;
            double R = dist(rng);
            IRAN = static_cast<int>(23 * R) + M + 1;
        } else if (JOLD == 1 && J == 1) {
            result = 1;
        }
        JOLD = J;
        return (result == 1) ? Move::DEFECT : Move::COOPERATE;
    }
};

class K63R : public IStrategy {
private:
    int ik = 1;
public:
    void reset() override { ik = 1; }
    Move getMove(uint64_t opp_history, uint64_t) override {
        int M = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++M; tmp >>= 1; }
        ++M;
        if (M == 1) ik = 1;
        ik = 1 - ik;
        return (ik == 1) ? Move::DEFECT : Move::COOPERATE;
    }
};

class K64R : public IStrategy {
private:
    int A[2][2] = {{0,0},{0,0}};
    int X = 1, Y = 1;
    int E = 0, F = 0;
public:
    void reset() override {
        for (int i = 0; i < 2; ++i)
            for (int j = 0; j < 2; ++j)
                A[i][j] = 0;
        X = 1; Y = 1;
        E = 0; F = 0;
    }
    Move getMove(uint64_t opp_history, uint64_t) override {
        int M = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++M; tmp >>= 1; }
        ++M;
        int J = (opp_history & 1) ? 1 : 0;
        if (M == 1) {
            E = 0; F = 0;
            X = 1; Y = 1;
            return Move::COOPERATE;
        }
        int result = (A[X-1][Y-1] >= 0) ? 0 : 1;
        if (J == 0) A[X-1][Y-1] += 1;
        else A[X-1][Y-1] -= 1;
        X = J + 1;
        Y = result + 1;
        if (J == 0) ++E;
        else ++F;
        int P = E - F;
        if (P < 0) P = -P;
        if (M > 40 && (10 * P < M)) result = 1;
        return (result == 1) ? Move::DEFECT : Move::COOPERATE;
    }
};

class K65R : public IStrategy {
private:
    int LASTD = 0, DIFF = 0, TOTD = 0;
public:
    void reset() override {
        LASTD = 0; DIFF = 0; TOTD = 0;
    }
    Move getMove(uint64_t opp_history, uint64_t) override {
        int M = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++M; tmp >>= 1; }
        ++M;
        int J = (opp_history & 1) ? 1 : 0;
        if (M == 1) {
            LASTD = 0; DIFF = 0; TOTD = 0;
            return Move::COOPERATE;
        }
        if (TOTD >= 10) return Move::DEFECT;
        if (J == 0) return Move::COOPERATE;
        ++TOTD;
        if (TOTD >= 10) return Move::DEFECT;
        if (LASTD == 0) {
            LASTD = M;
            return Move::COOPERATE;
        }
        DIFF = M - LASTD;
        if (DIFF <= 4) {
            TOTD = 10;
            return Move::DEFECT;
        }
        LASTD = M;
        return Move::COOPERATE;
    }
};

class K66R : public IStrategy {
private:
    int D = 0;
    int J2 = -3;
public:
    void reset() override {
        D = 0; J2 = -3;
    }
    Move getMove(uint64_t opp_history, uint64_t) override {
        int M = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++M; tmp >>= 1; }
        ++M;
        int J = (opp_history & 1) ? 1 : 0;
        D += J;
        double RR = static_cast<double>(D) / M;
        J2 = J2 - 1 + 3 * J;
        if (J2 > 10) J2 = 10;
        if (J2 < -5) J2 = -5;
        if (M < 3) return Move::COOPERATE;
        if (J2 < 3) return Move::COOPERATE;
        if (M > 10) {
            if (RR >= 0.15) return Move::DEFECT;
            else return Move::COOPERATE;
        }
        J2 = -1;
        return Move::DEFECT;
    }
};

class K67R : public IStrategy {
private:
    int S = 0, AD = 5, FD = 0, C = 0;
    double NO = 0, NK = 1, AK = 1;
    std::mt19937 rng;
    std::uniform_real_distribution<double> dist;
public:
    K67R() : rng(std::random_device{}()), dist(0.0, 1.0) {}
    void reset() override {
        S = 0; AD = 5; FD = 0; C = 0;
        NO = 0; NK = 1; AK = 1;
        rng.seed(std::random_device{}());
    }
    Move getMove(uint64_t opp_history, uint64_t my_history) override {
        int M = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++M; tmp >>= 1; }
        ++M;
        int J = (opp_history & 1) ? 1 : 0;
        int JA = (my_history & 1) ? 1 : 0;
        int result = JA;
        int K = compute_score(my_history, opp_history);
        int L = compute_score(opp_history, my_history);
        if (M == 1) {
            S = 0; AD = 5; FD = 0; C = 0;
            NO = 0; NK = 1; AK = 1;
        }
        if (FD == 2) {
            FD = 0;
            NO = (NO * NK + 3 - 3 * J + 2 * result - result * J) / (NK + 1);
            NK = NK + 1;
        }
        if (FD == 1) {
            FD = 2;
            AD = (AD * AK + 3 - 3 * J + 2 * result - result * J) / (AK + 1);
            AK = AK + 1;
        }
        if (J == 0) {
            S = 0;
            ++C;
        } else {
            ++S;
        }
        result = 0;
        if (std::abs(FD - 1.5) != 0.5) {
            if (K >= 2.25 * M) {
                double P = 0.95 - (AD + NO - 5) / 15.0 + 1.0 / (M * M) - J / 4.0;
                if (!(dist(rng) <= P)) {
                    result = 1;
                    FD = 1;
                }
            } else if (K >= 1.75 * M) {
                double P = 0.25 + static_cast<double>(C) / M - S * 0.25 + (K - L) / 100.0 + 4.0 / M;
                if (!(dist(rng) <= P)) {
                    result = 1;
                }
            } else {
                result = J;
            }
        }
        return (result == 1) ? Move::DEFECT : Move::COOPERATE;
    }
};

class K68R : public IStrategy {
private:
    int J2 = 0, J1 = 0;
    std::mt19937 rng;
    std::uniform_real_distribution<double> dist;
public:
    K68R() : rng(std::random_device{}()), dist(0.0, 1.0) {}
    void reset() override {
        J2 = 0; J1 = 0;
        rng.seed(std::random_device{}());
    }
    Move getMove(uint64_t opp_history, uint64_t) override {
        int M = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++M; tmp >>= 1; }
        ++M;
        int J = (opp_history & 1) ? 1 : 0;
        int result = 0;
        if (M == 1) {
            J2 = 0; J1 = 0;
            result = 0;
        } else {
            if (J1 * J == 1) {
                if (dist(rng) < 0.75) result = 1;
                else result = 0;
            } else if (J2 * 2 + J1 + J * 2 + J == 1) {
                result = 1;
            } else if (J2 * 2 + J1 * 2 + J == 1) {
                if (dist(rng) < 0.5) result = 1;
                else result = 0;
            } else {
                result = 0;
            }
        }
        J2 = J1;
        J1 = J;
        return (result == 1) ? Move::DEFECT : Move::COOPERATE;
    }
};

class K69R : public IStrategy {
private:
    int S = 1, F = 0, D = 0, C = 0;
    std::mt19937 rng;
    std::uniform_real_distribution<double> dist;
public:
    K69R() : rng(std::random_device{}()), dist(0.0, 1.0) {}
    void reset() override {
        S = 1; F = 0; D = 0; C = 0;
        rng.seed(std::random_device{}());
    }
    Move getMove(uint64_t opp_history, uint64_t) override {
        int M = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++M; tmp >>= 1; }
        ++M;
        int J = (opp_history & 1) ? 1 : 0;
        int result = 0;
        if (M == 1) {
            S = 1; F = 0; D = 0; C = 0;
            result = 0;
            return Move::COOPERATE;
        }
        if (J == 0) ++C;
        if (S == 1) {
            if (dist(rng) < 0.1) {
                S = 5;
                result = 1;
            } else {
                if (J == 0) D = 0;
                else ++D;
                if (D > 20) {
                    S = 3;
                    result = 0;
                    D = 0;
                } else if (C < 0.7 * (M - 3)) {
                    S = 2;
                    result = 1;
                } else {
                    result = J;
                }
            }
        } else if (S == 2) {
            if (J == 0) D = 0;
            else ++D;
            if (D > 10) {
                S = 3;
                result = 1;
            } else {
                result = 1;
            }
        } else if (S == 3) {
            if (J == 0) D = 0;
            else ++D;
            if (D > 20) {
                S = 3;
                result = 0;
                D = 0;
            } else {
                result = J;
            }
        } else if (S == 4) {
            if (J == 0) {
                S = 1;
                result = 0;
            } else {
                ++F;
                if (F > 3) {
                    S = 3;
                    result = 0;
                    D = 0;
                } else {
                    result = 1;
                }
            }
        } else if (S == 5) {
            S = 4;
            if (J == 0) D = 0;
            else ++D;
            if (D > 20) {
                S = 3;
                result = 0;
                D = 0;
            } else if (C < 0.7 * (M - 3)) {
                S = 2;
                result = 1;
            } else {
                result = J;
            }
        }
        return (result == 1) ? Move::DEFECT : Move::COOPERATE;
    }
};

class K70R : public IStrategy {
private:
    int JZ = 0;
    std::mt19937 rng;
    std::uniform_real_distribution<double> dist;
public:
    K70R() : rng(std::random_device{}()), dist(0.0, 1.0) {}
    void reset() override {
        JZ = 0;
        rng.seed(std::random_device{}());
    }
    Move getMove(uint64_t opp_history, uint64_t) override {
        int M = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++M; tmp >>= 1; }
        ++M;
        int J = (opp_history & 1) ? 1 : 0;
        int result;
        if (M == 1) JZ = 0;
        if (JZ == J) {
            result = JZ;
        } else {
            result = 0;
            if (dist(rng) > 0.2) result = 1;
        }
        JZ = result;
        return (result == 1) ? Move::DEFECT : Move::COOPERATE;
    }
};

class K71R : public IStrategy {
private:
    int IA = 0, IB = 0;
public:
    void reset() override { IA = 0; IB = 0; }
    Move getMove(uint64_t opp_history, uint64_t) override {
        int M = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++M; tmp >>= 1; }
        ++M;
        int J = (opp_history & 1) ? 1 : 0;
        int result;
        if (M == 1) {
            IA = 0; IB = 0;
            result = 0;
        } else if (M == 2) {
            result = 1;
            if (J == 1) result = 0;
        } else {
            if (J == 0) {
                ++IA;
                if (IA == 2) {
                    result = 1;
                    IA = 0;
                } else {
                    result = 0;
                }
            } else {
                ++IB;
                if (IB == 2) {
                    result = 1;
                    IB = 0;
                } else {
                    result = 0;
                }
            }
        }
        return (result == 1) ? Move::DEFECT : Move::COOPERATE;
    }
};

class K72R : public IStrategy {
private:
    int JOLD = 0, JCOUNT = 0;
    std::mt19937 rng;
    std::uniform_real_distribution<double> dist;
public:
    K72R() : rng(std::random_device{}()), dist(0.0, 1.0) {}
    void reset() override {
        JOLD = 0; JCOUNT = 0;
        rng.seed(std::random_device{}());
    }
    Move getMove(uint64_t opp_history, uint64_t) override {
        int M = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++M; tmp >>= 1; }
        ++M;
        int J = (opp_history & 1) ? 1 : 0;
        if (M == 1) JCOUNT = 0;
        JOLD = J;
        if (JOLD == 1) ++JCOUNT;
        int N = 1;
        if (JOLD == 1 && M > 10) N = static_cast<int>(std::log(static_cast<double>(M)));
        double threshold = static_cast<double>(N * JCOUNT) / M;
        int result = (dist(rng) <= threshold) ? 1 : 0;
        return (result == 1) ? Move::DEFECT : Move::COOPERATE;
    }
};

class K73R : public IStrategy {
private:
    int IAGGD = 4, IDUNU = 0, IDUNB = 0, IPAYB = 8, ITEST = 1, IPOST = 0;
public:
    void reset() override {
        IAGGD = 4; IDUNU = 0; IDUNB = 0; IPAYB = 8; ITEST = 1; IPOST = 0;
    }
    Move getMove(uint64_t opp_history, uint64_t my_history) override {
        int M = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++M; tmp >>= 1; }
        ++M;
        int J = (opp_history & 1) ? 1 : 0;
        int K = compute_score(my_history, opp_history);
        if (M == 1) {
            IAGGD = 4; IDUNU = 0; IDUNB = 0; IPAYB = 8; ITEST = 1; IPOST = 0;
        }
        int result = IPOST;
        if (J != ITEST) return (result == 1) ? Move::DEFECT : Move::COOPERATE;
        if (ITEST == 1) ++IDUNU;
        if (ITEST == 0) ++IDUNB;
        if ((IDUNU >= IAGGD) || (IDUNB >= IPAYB)) {
            IDUNU = 0; IDUNB = 0; IPOST = 0;
            if (J == 1) IPOST = 1;
            result = IPOST;
            ITEST = (IPOST == 0) ? 1 : 0;
            if (ITEST == 0) {
                IPAYB = static_cast<int>(1.6667f * (IAGGD + 1));
            } else {
                IAGGD = IAGGD - 3 + (K / M);
                if (IAGGD <= 0) IAGGD = 1;
            }
        }
        return (result == 1) ? Move::DEFECT : Move::COOPERATE;
    }
};

class K74R : public IStrategy {
private:
    double ALPHA = 1.0, BETA = 0.3;
    int IOLD = 0, QCA = 0, QNA = 0, QCB = 0, QNB = 0;
    int JSW = 0, JS4 = 0, JS11 = 0, JR = 0, JL = 0, JT = 0, JSM = 1;
public:
    void reset() override {
        ALPHA = 1.0; BETA = 0.3; IOLD = 0; QCA = 0; QNA = 0; QCB = 0; QNB = 0;
        JSW = 0; JS4 = 0; JS11 = 0; JR = 0; JL = 0; JT = 0; JSM = 1;
    }
    Move getMove(uint64_t opp_history, uint64_t my_history) override {
        int M = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++M; tmp >>= 1; }
        ++M;
        int J = (opp_history & 1) ? 1 : 0;
        int JA = (my_history & 1) ? 1 : 0;
        int result = JA;
        if (M == 1) {
            ALPHA = 1.0; BETA = 0.3; IOLD = 0; QCA = 0; QNA = 0; QCB = 0; QNB = 0;
            JSW = 0; JS4 = 0; JS11 = 0; JR = 0; JL = 0; JT = 0; JSM = 1;
            result = 0;
        }
        if (JR == 1) {
            result = 1;
            return (result == 1) ? Move::DEFECT : Move::COOPERATE;
        }
        if (M > 2) {
            if (IOLD == 1) {
                if (J == 0) ++QCB;
                ++QNB;
                BETA = static_cast<double>(QCB) / QNB;
                QCB = static_cast<int>(QCB * 0.8);
                QNB = static_cast<int>(QNB * 0.8);
            } else {
                if (J == 0) ++QCA;
                ++QNA;
                ALPHA = static_cast<double>(QCA) / QNA;
                QCA = static_cast<int>(QCA * 0.8);
                QNA = static_cast<int>(QNA * 0.8);
            }
        }
        IOLD = result;
        if (M == 37) goto check_random;
        if (M > 37) goto compute_policies;
        if (M == 1) goto compute_policies;
        if (J == JL) ++JSM;
        if (JSM >= 3) JS4 = 1;
        if (JSM >= 11) JS11 = 1;
        if (J != JL) ++JSW;
        JSM = 1;
        JT += J;
    compute_policies:
        {
            double POLC = 6.0 * ALPHA - 8.0 * BETA - 2.0;
            double POLALT = 4.0 * ALPHA - 5.0 * BETA - 1.0;
            if (POLC == 0.0) {
                if (POLC >= POLALT) result = 0;
            } else if (POLALT >= 0.0) {
                result = 1 - result;
            } else {
                result = 1;
            }
        }
        JL = J;
        return (result == 1) ? Move::DEFECT : Move::COOPERATE;
    check_random:
        if (JS4 == 0) goto compute_policies;
        if (JS11 == 1) goto compute_policies;
        if (JT <= 10) goto compute_policies;
        if (JT >= 26) goto compute_policies;
        if (JSW >= 26) goto compute_policies;
        JR = 1;
        result = 1;
        return (result == 1) ? Move::DEFECT : Move::COOPERATE;
    }
};

class K74RXX : public IStrategy {
private:
    double ALPHA = 1.0, BETA = 0.3;
    int IOLD = 0, QCA = 0, QNA = 0, QCB = 0, QNB = 0;
    int JSW = 0, JS4 = 0, JS11 = 0, JR = 0, JL = 0, JT = 0, JSM = 1;
    int k74dummy = 0;
public:
    void reset() override {
        ALPHA = 1.0; BETA = 0.3; IOLD = 0; QCA = 0; QNA = 0; QCB = 0; QNB = 0;
        JSW = 0; JS4 = 0; JS11 = 0; JR = 0; JL = 0; JT = 0; JSM = 1;
        k74dummy = 0;
    }
    Move getMove(uint64_t opp_history, uint64_t my_history) override {
        int M = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++M; tmp >>= 1; }
        ++M;
        int J = (opp_history & 1) ? 1 : 0;
        int JA = (my_history & 1) ? 1 : 0;
        int result = JA;
        if (M == 1) {
            ALPHA = 1.0; BETA = 0.3; IOLD = 0; QCA = 0; QNA = 0; QCB = 0; QNB = 0;
            JSW = 0; JS4 = 0; JS11 = 0; JR = 0; JL = 0; JT = 0; JSM = 1;
            k74dummy = 0;
            result = 0;
        }
        if (JR == 1) {
            result = 1;
            k74dummy = 1;
            return (result == 1) ? Move::DEFECT : Move::COOPERATE;
        }
        if (M > 2) {
            if (IOLD == 1) {
                if (J == 0) ++QCB;
                ++QNB;
                BETA = static_cast<double>(QCB) / QNB;
                QCB = static_cast<int>(QCB * 0.8);
                QNB = static_cast<int>(QNB * 0.8);
            } else {
                if (J == 0) ++QCA;
                ++QNA;
                ALPHA = static_cast<double>(QCA) / QNA;
                QCA = static_cast<int>(QCA * 0.8);
                QNA = static_cast<int>(QNA * 0.8);
            }
        }
        IOLD = k74dummy;
        if (M == 37) goto check_random;
        if (M > 37) goto compute_policies;
        if (M == 1) goto compute_policies;
        if (J == JL) ++JSM;
        if (JSM >= 3) JS4 = 1;
        if (JSM >= 11) JS11 = 1;
        if (J != JL) ++JSW;
        JSM = 1;
        JT += J;
    compute_policies:
        {
            double POLC = 6.0 * ALPHA - 8.0 * BETA - 2.0;
            double POLALT = 4.0 * ALPHA - 5.0 * BETA - 1.0;
            if (POLC == 0.0) {
                if (POLC >= POLALT) {
                    result = 0;
                    k74dummy = 0;
                }
            } else if (POLALT >= 0.0) {
                result = 1 - result;
                k74dummy = result;
            } else {
                result = 1;
                k74dummy = 1;
            }
        }
        JL = J;
        return (result == 1) ? Move::DEFECT : Move::COOPERATE;
    check_random:
        if (JS4 == 0) goto compute_policies;
        if (JS11 == 1) goto compute_policies;
        if (JT <= 10) goto compute_policies;
        if (JT >= 26) goto compute_policies;
        if (JSW >= 26) goto compute_policies;
        JR = 1;
        result = 1;
        k74dummy = 1;
        return (result == 1) ? Move::DEFECT : Move::COOPERATE;
    }
};

class K75R : public IStrategy {
private:
    int HIST[4][2];
    int IBURN = 0, ID[2] = {0,0}, IDEF = 0, ITWIN = 0, ISTRNG = 0, ICOOP = 0;
    int ITRY = 0, IRDCHK = 0, IRAND = 0, IPARTY = 1, IND = 0, MY = 0, INDEF = 5, IOPP = 0;
    double PROB = 0.2;
    std::mt19937 rng;
    std::uniform_real_distribution<double> dist;
public:
    K75R() : rng(std::random_device{}()), dist(0.0, 1.0) {}
    void reset() override {
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 2; ++j)
                HIST[i][j] = 0;
        IBURN = 0; ID[0] = 0; ID[1] = 0; IDEF = 0; ITWIN = 0; ISTRNG = 0;
        ICOOP = 0; ITRY = 0; IRDCHK = 0; IRAND = 0; IPARTY = 1; IND = 0;
        MY = 0; INDEF = 5; IOPP = 0; PROB = 0.2;
        rng.seed(std::random_device{}());
    }
    Move getMove(uint64_t opp_history, uint64_t my_history) override {
        int M = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++M; tmp >>= 1; }
        ++M;
        int J = (opp_history & 1) ? 1 : 0;
        int JA = (my_history & 1) ? 1 : 0;
        int result = JA;
        double R = dist(rng);
        if (M == 1) {
            for (int i = 0; i < 4; ++i)
                for (int j = 0; j < 2; ++j)
                    HIST[i][j] = 0;
            IBURN = 0; ID[0] = 0; ID[1] = 0; IDEF = 0; ITWIN = 0; ISTRNG = 0;
            ICOOP = 0; ITRY = 0; IRDCHK = 0; IRAND = 0; IPARTY = 1; IND = 0;
            MY = 0; INDEF = 5; IOPP = 0; PROB = 0.2;
            result = 0;
        }
        if (IRAND == 1) {
            IRDCHK = IRDCHK + J * 4 - 3;
            if (IRDCHK >= 11) {
                IRAND = 2;
                ICOOP = 2;
                result = 0;
            } else {
                result = 1;
            }
            return (result == 1) ? Move::DEFECT : Move::COOPERATE;
        }
        IOPP += J;
        HIST[IND][J] += 1;
        if (M == 15 || (M % 15 == 0 && IRAND != 2)) {
            if (!(HIST[0][0] / static_cast<double>(M - 2) >= 0.8) &&
                (IOPP * 4 >= M - 2 && IOPP * 4 <= 3 * M - 6)) {
                double row[4], col[2];
                for (int i = 0; i < 4; ++i) row[i] = HIST[i][0] + HIST[i][1];
                for (int j = 0; j < 2; ++j) {
                    double sum = 0;
                    for (int i = 0; i < 4; ++i) sum += HIST[i][j];
                    col[j] = sum;
                }
                double chi2 = 0.0;
                for (int i = 0; i < 4; ++i) {
                    for (int j = 0; j < 2; ++j) {
                        double ex = row[i] * col[j] / (M - 2);
                        if (ex > 1.0) {
                            chi2 += std::pow(HIST[i][j] - ex, 2) / ex;
                        }
                    }
                }
                if (chi2 > 3.0) {
                    IRAND = 1;
                    result = 1;
                    return (result == 1) ? Move::DEFECT : Move::COOPERATE;
                }
            }
        }
        if (ITRY == 1 && J == 1) IBURN = 1;
        if (M <= 37 && J == 0) ++ITWIN;
        if (M == 38 && J == 1) ++ITWIN;
        if (M >= 39 && ITWIN == 37 && J == 1) ITWIN = 0;
        if (ITWIN == 37) {
            result = 0;
            --ITRY;
            --ICOOP;
            goto update;
        }
        IDEF = IDEF * J + J;
        if (IDEF >= 20) {
            ID[IPARTY-1] = ID[IPARTY-1] + 1;
            result = 1;
            goto update;
        }
        IPARTY = 3 - IPARTY;
        ID[IPARTY-1] = ID[IPARTY-1] * J + J;
        if (ID[IPARTY-1] >= INDEF) {
            ID[IPARTY-1] = 0;
            ++ISTRNG;
            if (ISTRNG == 8) INDEF = 3;
            result = 0;
            --ITRY;
            --ICOOP;
            goto update;
        }
        if (ICOOP >= 1) {
            result = 0;
            --ITRY;
            --ICOOP;
            goto update;
        }
        if (M < 37 || IBURN == 1) {
            if (J == 0) {
                result = 0;
                --ITRY;
                --ICOOP;
                goto update;
            } else {
                ID[IPARTY-1] = ID[IPARTY-1] + 1;
                result = 1;
                goto update;
            }
        }
        if (M == 37 || R <= PROB) {
            ITRY = 2;
            ICOOP = 2;
            PROB += 0.05;
            result = 1;
        } else {
            if (J == 0) {
                result = 0;
                --ITRY;
                --ICOOP;
            } else {
                ID[IPARTY-1] = ID[IPARTY-1] + 1;
                result = 1;
            }
        }
    update:
        IND = 2 * MY + J + 1;
        MY = result;
        return (result == 1) ? Move::DEFECT : Move::COOPERATE;
    }
};

class K76R : public IStrategy {
private:
    bool PATSY = true;
    int DC = 0, MDC = 0, G = 1;
public:
    void reset() override {
        PATSY = true; DC = 0; MDC = 0; G = 1;
    }
    Move getMove(uint64_t opp_history, uint64_t) override {
        int M = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++M; tmp >>= 1; }
        ++M;
        int J = (opp_history & 1) ? 1 : 0;
        if (M == 1) {
            PATSY = true; DC = 0; MDC = 0; G = 1;
            return Move::DEFECT;
        }
        if (!PATSY) return (J == 1) ? Move::DEFECT : Move::COOPERATE;
        if (J == 1) {
            PATSY = false;
            return Move::COOPERATE;
        }
        ++DC;
        if (G == 0) ++MDC;
        G = 0;
        if (static_cast<double>(MDC) / (DC + 1) >= 0.5) G = 1;
        return (G == 1) ? Move::DEFECT : Move::COOPERATE;
    }
};

class K77R : public IStrategy {
private:
    int JSTR = 3, KTRY = 0, KEXP[5] = {100,100,100,100,100}, KI = 0;
public:
    void reset() override {
        JSTR = 3; KTRY = 0;
        for (int i = 0; i < 5; ++i) KEXP[i] = 100;
        KI = 0;
    }
    Move getMove(uint64_t opp_history, uint64_t my_history) override {
        int M = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++M; tmp >>= 1; }
        ++M;
        int J = (opp_history & 1) ? 1 : 0;
        int JA = (my_history & 1) ? 1 : 0;
        (void)JA; // silence unused warning
        double R = static_cast<double>(rand()) / RAND_MAX;
        if (M == 1) {
            JSTR = 3; KTRY = 0;
            for (int i = 0; i < 5; ++i) KEXP[i] = 100;
            KI = 0;
        }
        if (KTRY >= 20) {
            int ISCORE = compute_score(my_history, opp_history);
            KEXP[JSTR-1] = ISCORE - KI;
            if (JSTR < 5 && KEXP[JSTR] <= KEXP[JSTR-1]) ++JSTR;
            else if (JSTR > 1 && KEXP[JSTR-2] <= KEXP[JSTR-1]) --JSTR;
            KI = ISCORE;
            KTRY = 0;
        }
        ++KTRY;
        int result;
        switch (JSTR) {
            case 1: result = 0; break;
            case 2: result = (J == 0) ? 0 : (R <= 0.75 ? 1 : 0); break;
            case 3: result = J; break;
            case 4: result = (J == 1) ? 1 : (R <= 0.75 ? 0 : 1); break;
            case 5: result = 1; break;
            default: result = 0;
        }
        return (result == 1) ? Move::DEFECT : Move::COOPERATE;
    }
};

class K79R : public IStrategy {
private:
    int JBACK[5] = {0};
public:
    void reset() override {
        for (int i = 0; i < 5; ++i) JBACK[i] = 0;
    }
    Move getMove(uint64_t opp_history, uint64_t) override {
        int M = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++M; tmp >>= 1; }
        ++M;
        int J = (opp_history & 1) ? 1 : 0;
        if (M == 1) {
            for (int i = 0; i < 5; ++i) JBACK[i] = 0;
            return Move::COOPERATE;
        }
        int result;
        if (M < 6) {
            result = J;
        } else {
            int sum = 0;
            for (int i = 0; i < 5; ++i) sum += JBACK[i];
            result = (sum >= 3) ? 1 : 0;
        }
        for (int i = 0; i < 4; ++i) JBACK[i] = JBACK[i+1];
        JBACK[4] = J;
        return (result == 1) ? Move::DEFECT : Move::COOPERATE;
    }
};

class K80R : public IStrategy {
private:
    int MODE = 0, INOD = 0, INOC = 0;
    double TEST = 0.0;
public:
    void reset() override {
        MODE = 0; INOD = 0; INOC = 0; TEST = 0.0;
    }
    Move getMove(uint64_t opp_history, uint64_t) override {
        int M = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++M; tmp >>= 1; }
        ++M;
        int J = (opp_history & 1) ? 1 : 0;
        if (M == 1) {
            MODE = 0; INOD = 0; INOC = 0; TEST = 0.0;
            return Move::COOPERATE;
        }
        if (MODE == 1) return Move::DEFECT;
        if (J == 1) {
            ++INOD;
            INOC = M - INOD;
            TEST = std::pow(1.6667, INOD) * std::pow(0.882, INOC);
            if (TEST >= 5.0) MODE = 1;
        }
        return (MODE == 1) ? Move::DEFECT : Move::COOPERATE;
    }
};

class K81R : public IStrategy {
private:
    double L4[8][2];
    int T0, T4, T5, T6, T8, T9, D4, A, B, S1;
    double X[8];
    std::mt19937 rng;
    std::uniform_real_distribution<double> dist;
public:
    K81R() : rng(std::random_device{}()), dist(0.0, 1.0) {}
    void reset() override {
        for (int c = 0; c < 8; ++c) {
            L4[c][0] = 0; L4[c][1] = 0; X[c] = 0;
        }
        T0 = 0; T4 = 0; T5 = 0; T6 = 25; T8 = 0; T9 = 5; D4 = 0;
        A = 0; B = 0; S1 = 0;
        rng.seed(std::random_device{}());
    }
    Move getMove(uint64_t opp_history, uint64_t my_history) override {
        int M = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++M; tmp >>= 1; }
        ++M;
        int J = (opp_history & 1) ? 1 : 0;
        int JA = (my_history & 1) ? 1 : 0;
        int result = JA;
        int K = compute_score(my_history, opp_history);
        int L = compute_score(opp_history, my_history);
        double R = dist(rng);
        (void)R;

        if (M == 1) { reset(); result = 0; }

        bool done = false;
        while (!done) {
            if (M < T9) {
                result = J;
                done = true;
                break;
            }
            if (T5 > 7) T5 -= 8;
            if (J == 0) L4[T5][0] += 1;
            if (T9 == 9 && T0 == 1) {
                if (J != 1) {
                    int T2 = 0;
                    while (true) {
                        if (M > 80 + T2 && M < 140 + T2) { result = 1; break; }
                        else if (M >= 140 + T2 && M <= 180 + T2) { result = 0; break; }
                        T2 += 100;
                    }
                } else {
                    T0 = 0;
                    continue;
                }
                done = true;
                break;
            }
            if (J != 1) {
                if (T8 >= 0 && T8 < 6) {
                    T8 = 0;
                    if (L > K + T6) { result = J; done = true; break; }
                    D4 = T4; if (D4 > 7) D4 -= 8;
                    double A1 = L4[D4][0], A2 = L4[D4][1]; if (A2 == 0) A2 = 1;
                    double A3 = A1 / A2;
                    A = int(3 * A3); B = A + int(A3) + 1;
                    for (int c = 0; c < 4; ++c) X[c] = A;
                    for (int c = 4; c < 8; ++c) X[c] = B;
                    int E0=5,E1=6,E2=7,E3=8, F0=3,F1=4,F2=7,F3=8, L900=1;
                    if (T4 > 4) T4 -= 4; T4 *= 2;
                    for (int c = 1; c <= 8; ++c) {
                        int d = T4;
                        if (c==E0||c==E1||c==E2||c==E3) d = T4+1;
                        if (d==9) d=1; if (d>7) d-=8;
                        double a1 = L4[d][0], a2 = L4[d][1]; if (a2==0) a2=1;
                        double a3 = a1 / a2;
                        int ai = int(3*a3), bi = ai + int(a3) + 1;
                        if (c==F0||c==F1||c==F2||c==F3) X[c-1] += bi;
                        else X[c-1] += ai;
                    }
                    E0=3;E1=4; F0=2;F2=6; L900=2;
                    if (T4 > 4) T4 -= 4; T4 *= 2;
                    for (int c = 1; c <= 8; ++c) {
                        int d = T4;
                        if (c==E0||c==E1||c==E2||c==E3) d = T4+1;
                        if (d==9) d=1; if (d>7) d-=8;
                        double a1 = L4[d][0], a2 = L4[d][1]; if (a2==0) a2=1;
                        double a3 = a1 / a2;
                        int ai = int(3*a3), bi = ai + int(a3) + 1;
                        if (c==F0||c==F1||c==F2||c==F3) X[c-1] += bi;
                        else X[c-1] += ai;
                    }
                    int best = 0; int bestIdx = 0;
                    for (int c=0; c<8; ++c) if (X[c] > best) { best = X[c]; bestIdx = c+1; }
                    result = (bestIdx >= 5) ? 1 : 0;
                    done = true;
                    break;
                } else if (T8 > 0) {
                    T8 = -200;
                }
                result = 0;
                T8 += 1;
                done = true;
                break;
            } else {
                T8 += 1;
                if (T8 >= 8 && T8 <= 9) { result = 0; done = true; break; }
                else if (T8 > 1) T8 = 1;
                if (L > K + T6) { result = J; done = true; break; }
                D4 = T4; if (D4 > 7) D4 -= 8;
                double A1 = L4[D4][0], A2 = L4[D4][1]; if (A2 == 0) A2 = 1;
                double A3 = A1 / A2;
                A = int(3 * A3); B = A + int(A3) + 1;
                for (int c = 0; c < 4; ++c) X[c] = A;
                for (int c = 4; c < 8; ++c) X[c] = B;
                int E0=5,E1=6,E2=7,E3=8, F0=3,F1=4,F2=7,F3=8, L900=1;
                if (T4 > 4) T4 -= 4; T4 *= 2;
                for (int c = 1; c <= 8; ++c) {
                    int d = T4;
                    if (c==E0||c==E1||c==E2||c==E3) d = T4+1;
                    if (d==9) d=1; if (d>7) d-=8;
                    double a1 = L4[d][0], a2 = L4[d][1]; if (a2==0) a2=1;
                    double a3 = a1 / a2;
                    int ai = int(3*a3), bi = ai + int(a3) + 1;
                    if (c==F0||c==F1||c==F2||c==F3) X[c-1] += bi;
                    else X[c-1] += ai;
                }
                E0=3;E1=4; F0=2;F2=6; L900=2;
                if (T4 > 4) T4 -= 4; T4 *= 2;
                for (int c = 1; c <= 8; ++c) {
                    int d = T4;
                    if (c==E0||c==E1||c==E2||c==E3) d = T4+1;
                    if (d==9) d=1; if (d>7) d-=8;
                    double a1 = L4[d][0], a2 = L4[d][1]; if (a2==0) a2=1;
                    double a3 = a1 / a2;
                    int ai = int(3*a3), bi = ai + int(a3) + 1;
                    if (c==F0||c==F1||c==F2||c==F3) X[c-1] += bi;
                    else X[c-1] += ai;
                }
                int best = 0, bestIdx = 0;
                for (int c=0; c<8; ++c) if (X[c] > best) { best = X[c]; bestIdx = c+1; }
                result = (bestIdx >= 5) ? 1 : 0;
                done = true;
                break;
            }
        }

        T5 = T4;
        if ((M / 10) * 10 == M) {
            for (int c = 0; c < 8; ++c) L4[c][0] *= 9;
            T6 += 1;
        }
        if (T4 > 7) T4 -= 8;
        if (M > 3) L4[T4][1] += 1;
        if (T4 > 4) T4 -= 4;
        T4 = T4 * 2 + result;

        return (result == 1) ? Move::DEFECT : Move::COOPERATE;
    }
};

class K82R : public IStrategy {
private:
    int I5 = 0, I3 = 0, I2 = 0, I1 = 0;
    double X = 0.75, D4 = 0.0;
    std::mt19937 rng;
    std::uniform_real_distribution<double> dist;
public:
    K82R() : rng(std::random_device{}()), dist(0.0, 1.0) {}
    void reset() override {
        I5 = 0; I3 = 0; I2 = 0; I1 = 0;
        X = 0.75; D4 = 0.0;
        rng.seed(std::random_device{}());
    }
    Move getMove(uint64_t opp_history, uint64_t) override {
        int M = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++M; tmp >>= 1; }
        ++M;
        int J = (opp_history & 1) ? 1 : 0;
        int result = J;
        double R = dist(rng);
        if (M == 1) {
            X = 0.75; I5 = 0; D4 = 0.0;
            I2 = 0; I3 = 1;
            result = 0;
            return Move::COOPERATE;
        }
        I5 += J;
        D4 += J;
        if (J == 0 && I5 > 1) {
            if (I5 > 5) {
                I5 = 0; I1 = 0;
                result = 0;
                goto finish;
            }
            I5 = 0;
        }
        if (M < 30) goto finish;
        if (I3 == 0) {
            I2 = 0; I3 = 1;
            goto finish;
        }
        if (std::abs(D4 / (M - 1.0) - 0.5) < 0.1) X -= 0.2;
        if (I2 == 1) {
            if (J == 0) {
                X -= 0.05;
                if (X < 0.0) X = 0.0;
                I2 = 0;
                if (X >= 0.3) goto finish;
            } else {
                X += 0.15;
                if (X > 1.0) X = 1.0;
                goto finish;
            }
        }
        if (R > X) {
            result = 1;
            I1 = 1;
        } else {
            I2 = I1;
        }
    finish:
        return (result == 1) ? Move::DEFECT : Move::COOPERATE;
    }
};

class K83R : public IStrategy {
private:
    int JHIS[5];
    int JTOT = 0, MCNT = 1;
    std::mt19937 rng;
    std::uniform_real_distribution<double> dist;
public:
    K83R() : rng(std::random_device{}()), dist(0.0, 1.0) {}
    void reset() override {
        for (int i = 0; i < 5; ++i) JHIS[i] = 0;
        JTOT = 0; MCNT = 1;
        rng.seed(std::random_device{}());
    }
    Move getMove(uint64_t opp_history, uint64_t) override {
        int M = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++M; tmp >>= 1; }
        ++M;
        int J = (opp_history & 1) ? 1 : 0;
        double R = dist(rng);
        if (M <= 5) {
            if (M == 1) { JTOT = 0; MCNT = 1; }
            JHIS[M-1] = J;
            JTOT += J;
            return Move::COOPERATE;
        }
        JTOT = JTOT - JHIS[MCNT-1] + J;
        JHIS[MCNT-1] = J;
        ++MCNT;
        if (MCNT > 5) MCNT = 1;
        int result = 0;
        if (R * 25 < JTOT * JTOT - 1) result = 1;
        return (result == 1) ? Move::DEFECT : Move::COOPERATE;
    }
};

class K84R : public IStrategy {
private:
    int ISIG, JQ, JDR;
    double DS, FJD, FM;
public:
    void reset() override {
        ISIG = 0; JQ = 0; JDR = 0;
        DS = 0.0; FJD = 0.0; FM = 0.0;
    }
    Move getMove(uint64_t opp_history, uint64_t my_history) override {
        int M = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++M; tmp >>= 1; }
        ++M;
        int JP = (opp_history & 1) ? 1 : 0;
        int JA = (my_history & 1) ? 1 : 0;
        int result = JA;

        int IS = compute_score(my_history, opp_history);
        int JS = compute_score(opp_history, my_history);

        if (M == 1) {
            ISIG = 0; DS = 0.0; JQ = 0; FJD = 0.0; JDR = 0; FM = 0.0;
            result = 1;
            if (IS - JS - DS - 5 * JDR * (JDR - 1) / 2 >= 0) result = 0;
            JQ = JP;
            return (result == 1) ? Move::DEFECT : Move::COOPERATE;
        }

        if (JP == 1) FJD += 1.0;
        if (ISIG == 1) {
            ISIG = M;
            JQ = 0;
            JDR = 0;
            DS = IS - JS;
        } else {
            FM = M;
            if (JQ == 0 && JP == 1) JDR += 1;
        }

        if (IS - JS - DS - 5 * JDR * (JDR - 1) / 2 >= 0) result = 0;

        if (result == 1) {
            if ((JQ - JP) < 0 || (M - ISIG) < 10) {
                JQ = JP;
                return Move::DEFECT;
            }
            double fm_minus_1 = FM - 1.0;
            if (fm_minus_1 <= 0.0) {
                JQ = JP;
                return Move::DEFECT;
            }
            if (std::abs(FJD - fm_minus_1 / 2.0) < 1.5 * std::sqrt(fm_minus_1)) {
                JQ = JP;
                return Move::DEFECT;
            }
            ISIG = 1;
            JQ = JP;
            result = 0;
        } else {
            JQ = JP;
        }

        return (result == 1) ? Move::DEFECT : Move::COOPERATE;
    }
};

class K85R : public IStrategy {
private:
    int J2, J4, J8, J0, F4, F8, F0, F1, C, D, T, I1, I2, I3, I4;
public:
    void reset() override {
        J2 = 0; J4 = 0; J8 = 0; J0 = 0;
        F4 = 0; F8 = 0; F0 = 0;
        F1 = 0; C = 0; D = 0; T = 0;
        I1 = 0; I2 = 0; I3 = 0; I4 = 0;
    }
    Move getMove(uint64_t opp_history, uint64_t my_history) override {
        int M = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++M; tmp >>= 1; }
        ++M;
        int J = (opp_history & 1) ? 1 : 0;
        int JA = (my_history & 1) ? 1 : 0;
        int result = JA;
        if (M == 1) {
            J2 = 0; J4 = 0; J8 = 0; J0 = 0;
            F4 = 0; F8 = 0; F0 = 0;
            F1 = 0; C = 0; D = 0; T = 0;
            I1 = 0; I2 = 0; I3 = 0; I4 = 0;
            result = 0;
            goto L900;
        }
        {
            double J5 = J0 / 1e7;
            int J3 = static_cast<int>(J5);
            double J8d = J5 - J3;
            J8 = static_cast<int>(J8d * 1e7);
            double F5 = F0 / 1e7;
            int F3 = static_cast<int>(F5);
            double F8d = F5 - F3;
            F8 = static_cast<int>(F8d * 1e7);
            J0 = J8 * 10 + 5;
            F0 = F8 * 10 + 5;
        }
        if (F1 == 0) {
            if (J == 0) ++I3;
            else ++I4;
        } else {
            if (J == 0) ++I1;
            else ++I2;
        }
        if (M > 20) {
            double I5 = I1 + 1e-6;
            double I6 = I2 + 1e-6;
            double X8 = I3 + 1e-6;
            double I8d = I4 + 1e-6;
            double A = I5 / I6;
            double B = X8 / I8d;
            if (!(A > 1.5 || A < 0.5 || B > 1.5 || B < 0.5)) goto L910;
        }
        if (T == 1) goto L920;
        if (J0 == 11111111) goto L920;
        if (C == 1) goto L980;
        {
            double Z1 = J0 / 100.0;
            int Z2 = static_cast<int>(Z1);
            double Z3 = Z1 - Z2;
            J2 = static_cast<int>(Z3 * 100);
            if (M <= 30 && J2 == 11) goto L390;
        }
        {
            double Z4 = J0 / 10000.0;
            int Z5 = static_cast<int>(Z4);
            double Z6 = Z4 - Z5;
            J4 = static_cast<int>(Z6 * 10000);
            double W8 = F0 / 10000.0;
            int Z8 = static_cast<int>(W8);
            double Z9 = W8 - Z8;
            F4 = static_cast<int>(Z9 * 10000);
            if (J4 == 1011 && F4 == 111) goto L930;
        }
        {
            double Y1 = I2 + I4;
            double Y2 = I1 + I2 + 3;
            if (Y1 >= Y2) goto L910;
        }
        if (result == 1 && J == 0) goto L940;
        if (D == 1) goto L995;
        if (result == 0 && J == 0) goto L900;
        if (result == 0 && J == 1) goto L910;
        if (result != 0) goto L950;
    L900:
        F1 = result;
        result = 0;
        return Move::COOPERATE;
    L910:
        F1 = result;
        result = 1;
        D = 0;
        return Move::DEFECT;
    L920:
        T = 1;
        result = J;
        return (result == 1) ? Move::DEFECT : Move::COOPERATE;
    L930:
        C = 1;
        goto L981;
    L940:
        F1 = result;
        result = 1;
        D = 0;
        return Move::DEFECT;
    L950:
        F1 = result;
        result = 1;
        D = 1;
        return Move::DEFECT;
    L980:
        C = 0;
    L981:
        F1 = result;
        result = 0;
        return Move::COOPERATE;
    L995:
        F1 = result;
        result = 0;
        D = 0;
        return Move::COOPERATE;
    L390:
        if (J != 0) goto L400;
        goto L900;
    L400:
        goto L910;
    }
};

class K86R : public IStrategy {
private:
    int IOPPNT[999];
public:
    void reset() override {
        for (int i = 0; i < 999; ++i) IOPPNT[i] = 0;
    }
    Move getMove(uint64_t opp_history, uint64_t my_history) override {
        int M = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++M; tmp >>= 1; }
        ++M;
        int J = (opp_history & 1) ? 1 : 0;
        int JA = (my_history & 1) ? 1 : 0;
        IOPPNT[M-1] = J;
        int MYOLD = JA;
        if (M <= 2) return Move::COOPERATE;
        if (M <= 7) return (J == 1) ? Move::DEFECT : Move::COOPERATE;
        int IPREV7 = 0;
        for (int i = M - 7; i <= M - 1; ++i) IPREV7 += IOPPNT[i-1];
        int result;
        if (MYOLD == 0) {
            result = (IPREV7 > 2) ? 1 : 0;
        } else {
            result = (IPREV7 > 1) ? 1 : 0;
        }
        return (result == 1) ? Move::DEFECT : Move::COOPERATE;
    }
};

class K87R : public IStrategy {
private:
    int Z = 0, H = 0;
    double Q6 = 0.5;
    std::mt19937 rng;
    std::uniform_real_distribution<double> dist;
public:
    K87R() : rng(std::random_device{}()), dist(0.0, 1.0) {}
    void reset() override {
        Z = 0; H = 0; Q6 = 0.5;
        rng.seed(std::random_device{}());
    }
    Move getMove(uint64_t opp_history, uint64_t) override {
        int M = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++M; tmp >>= 1; }
        ++M;
        int J = (opp_history & 1) ? 1 : 0;
        double R = dist(rng);
        if (M == 1) {
            Z = 0; Q6 = 0.5; H = 0;
            return Move::COOPERATE;
        }
        int S = 2 * J + H + 1;
        if (Z == 1) goto L630;
        if (J == 0) goto L692;
        Z = 1;
    L630:
        if (S > 1) {
            if (S == 4) Q6 = 0.74 * Q6 + 0.104;
            else Q6 = 0.5 * Q6;
        } else {
            Q6 = Q6 * 0.57 + 0.43;
        }
        H = 1;
        if (R > Q6) {
            return Move::DEFECT;
        } else {
            goto L692;
        }
    L692:
        H = 0;
        return Move::COOPERATE;
    }
};

class K88R : public IStrategy {
private:
    int MMC = 0, LMV = 0, MP = 0, MMV = 0, MP2 = 0, MMD = 1, DFLG = 0;
    double PRC = 0.0, PRD = 0.0;
    std::mt19937 rng;
    std::uniform_real_distribution<double> dist;
public:
    K88R() : rng(std::random_device{}()), dist(0.0, 1.0) {}
    void reset() override {
        MMC = 0; LMV = 0; MP = 0; MMV = 0; MP2 = 0; MMD = 1; DFLG = 0;
        PRC = 0.0; PRD = 0.0;
        rng.seed(std::random_device{}());
    }
    Move getMove(uint64_t opp_history, uint64_t) override {
        int M = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++M; tmp >>= 1; }
        ++M;
        int J = (opp_history & 1) ? 1 : 0;
        double R = dist(rng);
        if (M == 1) {
            MMC = 0; LMV = 0; MP = 0; MMV = 0; MP2 = 0; MMD = 1; DFLG = 0;
            return Move::COOPERATE;
        }
        if (M >= 2) {
            if (MMV == 0) {
                ++MMC;
                MP += J;
                PRC = static_cast<double>(MP) / MMC;
            } else {
                ++MMD;
                MP2 += J;
                PRD = static_cast<double>(MP2) / MMD;
            }
        }
        int result = 0;
        if (M <= 4) {
            result = 0;
        } else if (J == 1 && DFLG == 0) {
            DFLG = 1;
            result = 0;
        } else {
            if (MMV == 0 && R < PRC) result = 1;
            if (MMV == 1 && R < PRD) result = 1;
        }
        MMV = LMV;
        LMV = result;
        return (result == 1) ? Move::DEFECT : Move::COOPERATE;
    }
};

class K89R : public IStrategy {
private:
    int SC[7], SL[7], TM[7];
    int ST[5], GT[5];
    int CN, CSRC, MYLM, HLM;
    std::mt19937 rng;
    std::uniform_real_distribution<double> dist;
public:
    K89R() : rng(std::random_device{}()), dist(0.0, 1.0) {}
    void reset() override {
        for (int i = 1; i <= 6; ++i) {
            SC[i] = 0;  SL[i] = 1;  TM[i] = 0;
        }
        for (int i = 0; i < 5; ++i) { ST[i] = 0; GT[i] = 0; }
        CN = 10;  CSRC = 5;  MYLM = 1;  HLM = 0;
        rng.seed(std::random_device{}());
    }
    Move getMove(uint64_t opp_history, uint64_t my_history) override {
        int M = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++M; tmp >>= 1; }
        ++M;
        int J = (opp_history & 1) ? 1 : 0;
        int JA = (my_history & 1) ? 1 : 0;
        int result = JA;

        if (M == 1) {
            reset();
            result = 0;
            return Move::COOPERATE;
        }

        int CODE = CN / 10;
        if (CODE < 1 || CODE > 6) {
            result = 0;
            return (result == 1) ? Move::DEFECT : Move::COOPERATE;
        }

        if (10 * CODE == CN)
            SC[CODE] = compute_score(my_history, opp_history);

        if (SL[CODE] == 1) {
            ++CN;
            ++TM[CODE];
            switch (CODE) {
                case 1: result = 0; break;
                case 2: result = 1; break;
                case 3: result = 1 - MYLM; MYLM = result; break;
                case 4: result = (J == 1) ? 1 : 0; break;
                case 5: result = (J == 1 && HLM == 1) ? 1 : 0; HLM = J; break;
                case 6: {
                    int SGT = 0;
                    for (int i = 1; i <= 5; ++i) {
                        ST[i-1] = SC[i+1] - SC[i];
                        SGT += ST[i-1];
                        GT[i-1] += ST[i-1];
                    }
                    double MEAN = static_cast<double>(SGT) / CSRC;
                    double AMEAN = 9.0 * MEAN / 10.0;
                    CSRC = 0;
                    for (int i = 1; i <= 5; ++i) {
                        if (SL[i] == 1) {
                            if (ST[i-1] < AMEAN) SL[i] = 0;
                        } else {
                            if (10.0 * GT[i-1] / TM[i] > AMEAN) SL[i] = 1;
                        }
                        if (SL[i] == 1) ++CSRC;
                    }
                    CN = 10;
                    result = 0;
                } break;
                default: result = 0;
            }
        } else {
            CN += 10;
        }

        return (result == 1) ? Move::DEFECT : Move::COOPERATE;
    }
};

class K91R : public IStrategy {
private:
    double X, PX, Y, PY, Z, PZ, W, PW;
    double QC[4], QN[4];
    int IPOL[11][4];
    int IOLD, N;
public:
    K91R() {
        reset();
    }
    void reset() override {
        X = 0.999; PX = 0.001;
        Y = 0.001; PY = 0.999;
        Z = 0.999; PZ = 0.001;
        W = 0.001; PW = 0.999;
        QC[0] = 1.999; QC[1] = 1.999; QC[2] = 0.001; QC[3] = 0.001;
        QN[0] = 2; QN[1] = 2; QN[2] = 2; QN[3] = 2;
        int ipol_data[44] = {0,0,0,0, 1,1,1,1,1,1,1, 0,1,1,1, 0,0,0,1,1,1,1, 0,0,0,1, 0,0,1,0,0,1,1, 0,0,1,0,0,1,0,0,1,0,1};
        int idx = 0;
        for (int i = 0; i < 11; ++i) {
            for (int j = 0; j < 4; ++j) {
                IPOL[i][j] = ipol_data[idx++];
            }
        }
        IOLD = 0;
        N = 0;
    }
    Move getMove(uint64_t opp_history, uint64_t my_history) override {
        int M = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++M; tmp >>= 1; }
        ++M;
        int J = (opp_history & 1) ? 1 : 0;
        int JA = (my_history & 1) ? 1 : 0;
        int result = JA;
        if (M == 1) {
            IOLD = 0;
            N = 0;
            result = 0;
            return Move::COOPERATE;
        }
        if (M > 2) {
            if (J == 0) QC[N] += 1;
            QN[N] += 1;
            switch (N) {
                case 0: X = QC[0] / QN[0]; PX = 1 - X; break;
                case 1: Z = QC[1] / QN[1]; PZ = 1 - Z; break;
                case 2: Y = QC[2] / QN[2]; PY = 1 - Y; break;
                case 3: W = QC[3] / QN[3]; break;
            }
        }
        double E[11];
        E[0] = (3*Z) / (Z + PX);
        E[1] = (3*(Y*Z + W*PZ) + 5*Z*PX + PX*PZ) / (Y*Z + W*PZ + PX + Z*PX + PX*PZ);
        E[2] = (3*W*Y + 5*W*PX + PX*PZ) / (W*Y + 2*W*PX + PX*PZ);
        E[3] = (3*W*PY + 5*Z*PX + PX*PY) / (W*PY + PX*PY + Z*PX + PX*PY);
        E[4] = (3*Z + 5*X*Z + Z*PX) / (1 - X*Y - W*PX + 2*Z);
        E[5] = (8*W*Z + Z*PX) / (2*W*Z + W*PY + Z*PX);
        E[6] = (3*Z*PY + 5*X*Z + Z*PY) / (2*Z*PY + PW*PY + X*Z);
        E[7] = (3*(Y*Z + W*PZ) + 5*(Z*PW + W*X) + 1 - X*Y - Z*PY) / (Y*Z + W*PZ + 2 - 2*X*Y - W*PX + Z*PW + W*X - Z*PY);
        E[8] = (3*W*Y + 5*W + 1 - X*Y - Z*PY) / (2*W + 1 - X*Y - Z*PY);
        E[9] = (3*W*PY + 5*(Z*PW + W*X) + PY) / (PY + Z*PW + W*X + PY);
        E[10] = (5*W + PY) / (W + PY);
        int IBEST = 0;
        double BESTE = E[0];
        for (int i = 1; i < 11; ++i) {
            if (E[i] > BESTE) {
                BESTE = E[i];
                IBEST = i;
            }
        }
        N = 2 * IOLD + J;
        result = IPOL[IBEST][N];
        IOLD = result;
        return (result == 1) ? Move::DEFECT : Move::COOPERATE;
    }
};

class KPavlovC : public IStrategy {
private:
    int last_own = 0;
public:
    void reset() override { last_own = 0; }
    Move getMove(uint64_t opp_history, uint64_t my_history) override {
        int M = 0;
        uint64_t tmp = opp_history;
        while (tmp) { ++M; tmp >>= 1; }
        ++M;
        int J = (opp_history & 1) ? 1 : 0;
        int JA = (my_history & 1) ? 1 : 0;
        (void)JA;
        if (M == 1) {
            last_own = 0;
            return Move::COOPERATE;
        }
        int result = (J == last_own) ? 0 : 1;
        last_own = result;
        return (result == 1) ? Move::DEFECT : Move::COOPERATE;
    }
};

class Adaptive : public IStrategy {
private:
    std::vector<bool> initial_plays;
    int move_count;
    int scoreC;
    int scoreD;

public:
    Adaptive(const std::vector<bool>& init = {}) {
        if (init.empty()) {
            initial_plays.assign(11, false);
            for (int i = 6; i < 11; ++i) initial_plays[i] = true;
        } else {
            initial_plays = init;
        }
        reset();
    }

    void reset() override {
        move_count = 0;
        scoreC = 0;
        scoreD = 0;
    }

    Move getMove(uint64_t opp_history, uint64_t my_history) override {
        if (move_count > 0) {
            bool my_last = my_history & 1;
            bool opp_last = opp_history & 1;
            int payoff = 0;

            if (!my_last && !opp_last) payoff = 3;      // C vs C
            else if (!my_last && opp_last) payoff = 0;  // C vs D
            else if (my_last && !opp_last) payoff = 5;  // D vs C
            else payoff = 1;                            // D vs D

            if (!my_last) scoreC += payoff;
            else scoreD += payoff;
        }

        Move move;
        if (move_count < (int)initial_plays.size()) {
            move = initial_plays[move_count] ? Move::DEFECT : Move::COOPERATE;
        } else {
            move = (scoreC > scoreD) ? Move::COOPERATE : Move::DEFECT;
        }

        ++move_count;
        return move;
    }
};

class AbstractAdaptor : public IStrategy {
protected:
    double s;
    double perr;
    double delta[2][2];
    std::mt19937 rng;
    std::uniform_real_distribution<double> dist;

    static double heaviside(double x) {
        return (x >= 0.0) ? 1.0 : 0.0;
    }

public:
    AbstractAdaptor(double perr = 0.01)
        : s(0.0), perr(perr), rng(std::random_device{}()), dist(0.0, 1.0) {
    }

    void reset() override {
        s = 0.0;
        rng.seed(std::random_device{}());
    }

    Move getMove(uint64_t opp_history, uint64_t my_history) override {
        if (my_history != 0 || opp_history != 0) {
            bool my_last = my_history & 1;
            bool opp_last = opp_history & 1;
            int my_idx = my_last ? 1 : 0;
            int opp_idx = opp_last ? 1 : 0;
            s += delta[my_idx][opp_idx];
        }

        double p = perr + (1.0 - 2.0 * perr) *
                         (heaviside(s + 1.0) - heaviside(s - 1.0));

        double r = dist(rng);
        return (r < p) ? Move::COOPERATE : Move::DEFECT;
    }
};

class AdaptorBrief : public AbstractAdaptor {
public:
    AdaptorBrief() : AbstractAdaptor(0.01) {
        delta[0][0] =  0.0;          // (C, C)
        delta[0][1] = -1.001505;     // (C, D)
        delta[1][0] =  0.992107;     // (D, C)
        delta[1][1] = -0.638734;     // (D, D)
    }
};

class AdaptorLong : public AbstractAdaptor {
public:
    AdaptorLong() : AbstractAdaptor(0.01) {
        delta[0][0] =  0.0;          // (C, C)
        delta[0][1] =  1.888159;     // (C, D)
        delta[1][0] =  1.858883;     // (D, C)
        delta[1][1] = -0.995703;     // (D, D)
    }
};

class Alternator : public IStrategy {
public:
    void reset() override {}

    Move getMove(uint64_t /* opp_history */, uint64_t my_history) override {
        if (my_history == 0) {
            return Move::COOPERATE;
        }
        bool last_own_defect = my_history & 1ULL;
        return last_own_defect ? Move::COOPERATE : Move::DEFECT;
    }
};

static inline int get_move_at(uint64_t history, int round) {
    return (history >> round) & 1ULL;
}

static inline int count_defections(uint64_t history) {
    int count = 0;
    while (history) {
        count += history & 1ULL;
        history >>= 1;
    }
    return count;
}

static inline int count_cooperations(uint64_t history) {
    int total = 0;
    uint64_t tmp = history;
    while (tmp) { ++total; tmp >>= 1; }
    return total - count_defections(history);
}

static inline int history_length(uint64_t history) {
    int len = 0;
    while (history) { ++len; history >>= 1; }
    return len;
}

static std::vector<double> compute_features(uint64_t my_history, uint64_t opp_history) {
    int my_len = history_length(my_history);
    int opp_len = history_length(opp_history);

    double opp_first_c = 0.0, opp_first_d = 0.0;
    if (opp_len >= 1) {
        int first = get_move_at(opp_history, opp_len - 1);
        if (first == 0) opp_first_c = 1.0; else opp_first_d = 1.0;
    }

    double opp_second_c = 0.0, opp_second_d = 0.0;
    if (opp_len >= 2) {
        int second = get_move_at(opp_history, opp_len - 2);
        if (second == 0) opp_second_c = 1.0; else opp_second_d = 1.0;
    }

    double my_prev_c = 0.0, my_prev_d = 0.0;
    if (my_len >= 1) {
        int prev = get_move_at(my_history, 0);
        if (prev == 0) my_prev_c = 1.0; else my_prev_d = 1.0;
    }

    double my_prev2_c = 0.0, my_prev2_d = 0.0;
    if (my_len >= 2) {
        int prev2 = get_move_at(my_history, 1);
        if (prev2 == 0) my_prev2_c = 1.0; else my_prev2_d = 1.0;
    }

    double opp_prev_c = 0.0, opp_prev_d = 0.0;
    if (opp_len >= 1) {
        int prev = get_move_at(opp_history, 0);
        if (prev == 0) opp_prev_c = 1.0; else opp_prev_d = 1.0;
    }

    double opp_prev2_c = 0.0, opp_prev2_d = 0.0;
    if (opp_len >= 2) {
        int prev2 = get_move_at(opp_history, 1);
        if (prev2 == 0) opp_prev2_c = 1.0; else opp_prev2_d = 1.0;
    }

    double total_opp_c = static_cast<double>(count_cooperations(opp_history));
    double total_opp_d = static_cast<double>(count_defections(opp_history));
    double total_my_c  = static_cast<double>(count_cooperations(my_history));
    double total_my_d  = static_cast<double>(count_defections(my_history));
    double round_num   = static_cast<double>(my_len);

    return std::vector<double>{
        opp_first_c, opp_first_d, opp_second_c, opp_second_d,
        my_prev_c, my_prev_d, my_prev2_c, my_prev2_d,
        opp_prev_c, opp_prev_d, opp_prev2_c, opp_prev2_d,
        total_opp_c, total_opp_d, total_my_c, total_my_d,
        round_num
    };
}

static double activate(const std::vector<double>& bias,
                       const std::vector<std::vector<double>>& hidden_weights,
                       const std::vector<double>& output_weights,
                       const std::vector<double>& inputs) {
    std::vector<double> hidden_values(bias.size());
    for (size_t i = 0; i < bias.size(); ++i) {
        double sum = bias[i];
        for (size_t j = 0; j < inputs.size(); ++j)
            sum += hidden_weights[i][j] * inputs[j];
        hidden_values[i] = std::max(0.0, sum);
    }
    double output = 0.0;
    for (size_t i = 0; i < output_weights.size(); ++i)
        output += hidden_values[i] * output_weights[i];
    return output;
}

static void split_weights(const std::vector<double>& weights,
                          int num_features,
                          int num_hidden,
                          std::vector<std::vector<double>>& i2h,
                          std::vector<double>& h2o,
                          std::vector<double>& bias) {
    i2h.resize(num_hidden, std::vector<double>(num_features));
    int idx = 0;
    for (int i = 0; i < num_hidden; ++i)
        for (int j = 0; j < num_features; ++j)
            i2h[i][j] = weights[idx++];
    h2o.resize(num_hidden);
    for (int i = 0; i < num_hidden; ++i) h2o[i] = weights[idx++];
    bias.resize(num_hidden);
    for (int i = 0; i < num_hidden; ++i) bias[i] = weights[idx++];
}

class ANN : public IStrategy {
protected:
    int num_features;
    int num_hidden;
    std::vector<std::vector<double>> input_to_hidden;
    std::vector<double> hidden_to_output;
    std::vector<double> bias;

public:
    ANN(int features, int hidden, const std::vector<double>& weights)
        : num_features(features), num_hidden(hidden) {
        split_weights(weights, features, hidden, input_to_hidden, hidden_to_output, bias);
    }

    void reset() override {}

    Move getMove(uint64_t opp_history, uint64_t my_history) override {
        auto features = compute_features(my_history, opp_history);
        double output = activate(bias, input_to_hidden, hidden_to_output, features);
        return (output > 0.0) ? Move::COOPERATE : Move::DEFECT;
    }
};

class EvolvedANN : public ANN {
public:
    EvolvedANN() : ANN(17, 10, get_weights()) {}

private:
    static std::vector<double> get_weights() {
        static std::vector<double> w = {
            3.459899447605539, 2.1318404265386586, 13.17966613895038, -6.192108007790158,
            -0.37086706789610013, -0.3896626810316768, -0.3866099292858918, -0.5945768318137592,
            8.657276286604098, -0.31582965913711214, -12.77263491874852, -3.7221902888177283,
            0.7700315120556411, -33.026063114130366, -0.6766419579791423, -0.9319070818093544,
            -12.214457623430176, -0.7005949704263965, -0.9248704722582742, -0.30575483808901427,
            -0.6882322381266356, 0.492463682676912, 0.4162176461180324, 2.2349377906341243,
            -4.828561111548519, -1.9512825291316083, 15.234656539966528, 13.201167624183022,
            2.128673097211623, 2.92411276919898, -0.7547167660636005, 1.199314577481091,
            0.06760026684238629, 0.7832939406545589, -177.49932309404323, 6.89500655807993,
            -0.08284645051120698, -0.799724698184292, -8.207688126907367, 17.450767180301746,
            -19.54570476880868, 23.570589723652876, 7.443462688149636, -4.262126715956444,
            51.789933932119446, -0.9595303137934914, -0.015387143246637383, -1.6996871126483133,
            0.5326556419474413, -0.8690658866365495, -0.747871482057276, -0.1990663384836313,
            0.7376303253285281, 19.692218213944003, -2.920745672290689, -0.2877252920885854,
            -5.351299019427598, 1.1722532499353777, 1.3902788805171062, 0.38849134405949914,
            -5.729822642840242, -4.641822939824729, 0.5691745664669388, 0.30905426295392857,
            -8.85980449965398, -4.732837361082687, 0.8831193558694339, -0.9520915826182503,
            -0.5355473390079997, -22.037852229409136, -0.9596756645256569, 0.15632020854402362,
            -24.710657392303975, 21.00195680553553, -6.099731246259528, -0.9426766932743909,
            -0.7194608598270196, 3.4908551710867917, 0.1371367081336668, -0.9960642361403127,
            -0.23395001734734366, 0.7974681865981025, 1.1178545864789984, 0.0309085368898055,
            0.3709257972027509, -0.7705906687715782, 0.3106563294379545, -0.006111882508708552,
            0.01778595332796895, 0.5677955352244695, -0.04300357455768222, -0.8378512149555155,
            0.4517674874175419, 0.026356153000395066, 8.559746666840512, 60.1036518587203,
            -11.066045796708, 1.9547952088765457, -1.2572770384601373, -3.1565358568060065,
            -0.39428165742075283, -7.976304408878465, -12.979182222761667, -0.8903476091382874,
            -0.044046900475693686, -2290.612969360032, 3.1361101611630904, -4.744165899359072,
            1.5590913900932555, 23.589552731167824, 3.696503591016147, -49.0193733334204,
            -0.31155127179562386, -12.893291340674148, 3.355621906094085, -1124.6168527572167,
            -8.611905802129927, -5.495486462660128, 9.317002862904996, -0.928102034629926,
            0.9862671193437127, 71.6360157474848, -41.78727928552422, -0.08938630210606524,
            -25.475912160872333, -1.6296570038831701, 3.721506342123227, 1.2616295894267517,
            -2.16674011456336, 1.0608792593103447, -1.5348732560098246, 0.6488189032682272,
            -9.554624561640432, 0.9982451738644897, -3.585518099583287, 2.3410371683096507,
            -1.059363823436213, -36.03382554076086, 49.400216254399005, -0.09560108506061127,
            22.265326907988467, -1167.4125713033582, -827.3412289305065, -0.2817962724984171,
            -12.585799415544116, -0.4968822378372789, 81.10836010264876, 0.7230863554676401,
            -3.7241902674476655, -0.9757756717170544, -68.02893974316859, -2.2396567795647564,
            0.07066110212550569, -1.250920612229347, -0.046887193108303915, -0.7680639795702753,
            17.937616126439604, -1.1019678164169133, 0.07411014749198457, 2.056144561078099,
            -0.9166833148022522, -27.361514430051557, -2.3877636883795264, 2.2300576943504535,
            447.438860571402, 0.45131006835733695, 10.58847351640523, 1.9849158213808964,
            -1574.2103557260862, -0.7371599837780478, 0.9086264191508042, -0.7828307535556679,
            0.3300464039361075, 1.81242958242284, -3.610267708304238, -0.973651955724061,
            0.1209299242691169, 2.9843573498979894, -0.11277293056015156, -0.7439895632451479,
            -3.8649914641316685, 0.9782080851048618, -5.053463725816831, 3.5891164827308604,
            -3.7958700956913, -2.8572114118106247, -0.41818946926149336, 1.4695517340149908,
            -0.026854015704181288, -0.02788909596479816
        };
        return w;
    }
};

class EvolvedANN5 : public ANN {
public:
    EvolvedANN5() : ANN(17, 5, get_weights()) {}

private:
    static std::vector<double> get_weights() {
        static std::vector<double> w = {
            -28.102635339566508, -5.270221138740612, -1.7991915039829207, -19.860573976774578,
            -24.60513164187047, 3.8162913045444635, 0.023473769583907095, -54.50321528122049,
            10.003539037251969, -2.3346147693972115, 16.73844151591633, 0.3100810247981438,
            -169.33492203029917, -5.724230282870263, 0.8526201446384842, -0.22605058147685014,
            -17.835641849307482, -0.7043679829900225, -10.486007034199204, -3.0550187074781925,
            -0.6810531857496793, -4453.796912952781, 33810.53059746947, -10848.78737197365,
            -118.11626381871724, 93.18879667509046, -16083.650736874622, -13.725703495018438,
            50.10494647994167, -7.604207807021347, 90.77373268627657, 1.595074916653774,
            2.7557812139573015, -39715.109221929466, 28.904911685140643, -0.43899423549864697,
            5.768080489913274, 5.295125125995148, -2.546634014137431, -238767.20745174124,
            -3.5528156876545527, -4.9527347193332965, 1115.2695575158439, 4.174668501799942,
            -14.649495628121237, -23.25321447104678, 0.5500236215768699, -20.73180030629891,
            0.5694594098639599, 9.126335853167467, 3.566801760870098, 1337.557828195395,
            7235.478182255781, 73866.16381328272, -857.4430471718431, -924.9591765734541,
            -3518.967885192465, -1.9067958478325906, -6.633454070601698, -14.521354381912227,
            -2885.6120623623256, 22500.261088902847, -2.151057666676581, -1132.4844452804136,
            -320738.9350600944, 95.16925312662866, -5656.2165411116075, -45086.65321702537,
            -27.563351028422737, -2.61387586136827, 371.56363217654854, -303.4084110871267,
            6.983609633015089, -47.030329656598646, -32.801150369212415, 106.5993554145673,
            -1488.6548395800542, -76.76127924999216, 228.98568852459917, 3.1681704936401016,
            7.847547722960641, -17.600408577355783, -10.176187185243615, -2.8861364567007923,
            -6.112911717298348, -335.90148548509376, 52.34158411280463, 1272.9680157062762,
            22.881750122081684, 533.1837365484016, -0.6762316764182235, 4.4063523628928465,
            4.139696013585834, 124.12127527435491, -128.06733876616389
        };
        return w;
    }
};

class EvolvedANNNoise05 : public ANN {
public:
    EvolvedANNNoise05() : ANN(17, 5, get_weights()) {}

private:
    static std::vector<double> get_weights() {
        static std::vector<double> w = {
            23.884262150181755, -1818.8117704211384, -99.98875916610936, -0.03961610388820535,
            -6.210054298897063, 3.7052379169262717, -0.2341738759745093, -181.13859219885555,
            12.13317672298101, -0.49669501894086343, -1.0749425884840627, 10.74864948953037,
            -0.2896667901231506, -16.22712292450597, -1.7178383009508598, 0.03221417142047274,
            -158.1666203488453, -14.63669956068759, 15.408582826806054, 5.844929636776999,
            4.566735602054217, 3.2997530921332476, -1.9670038978333584, 11.7498294264678,
            13.681065767064851, 6.221555978655614, -6.577090380820985, -5.280239152793359,
            -3.708452340749404, 0.9946864682020832, -4.969083580503034, 0.37099841885684537,
            -4.663632521892934, 0.5739444442201613, -97.24526798384726, -18.898938443720873,
            2.836962511054863, 1426.696564885826, -61.660716521684535, -96.9853369128562,
            -35.811382478532956, 21.74704795347286, -43208.50415025884, -30.91261823654964,
            556.8238912039677, -1.2029710853005269, 86.95493903070172, -10502.293376143582,
            -2.0466530653599544, 22.16892386331129, -155.02893455429202, -28.409090647029252,
            -3.391747333401208, -13.869670523479188, 13.919971713037201, 10.27621997230116,
            0.8687219298082444, 5.274915939989237, -103.56226257273838, -11.47523804695477,
            8.100576862445774, -0.5353167277104594, -9.108755782674894, 0.8155938130504109,
            -7.262915116612636, -0.7403289719769353, -1.7023773645551308, 1.0574026713510245,
            -2732.0241659942394, -5.435067280962746, -188.24943501427512, 0.37886585953090285,
            -67.01235198117197, -121.1206837158425, -83.84782446726592, 45.31005125030543,
            11.069886195948996, -169.35773604933806, 31.675914286375498, -311.2006747809748,
            49.093586385973545, -5.819783407611689, -88.52645028711315, -284.2358463873679,
            0.6989482567897998, 156.74810964228283, 1.8064093223587379, 3.4228475977181265,
            -125.93039228074147, -25.169489233728665, 115.98479433268457, 21.27858612035085,
            0.2744006209426808, 2.899956014713407, -53.98506935427509
        };
        return w;
    }
};

enum class OppClass {
    UNKNOWN,
    COOPERATIVE,
    ALLD,
    STFT,
    PAVLOVD,
    RANDOM
};

class APavlov2006 : public IStrategy {
private:
    OppClass opponent_class;
    int round;

public:
    APavlov2006() : opponent_class(OppClass::UNKNOWN), round(0) {}

    void reset() override {
        opponent_class = OppClass::UNKNOWN;
        round = 0;
    }

    Move getMove(uint64_t opp_history, uint64_t my_history) override {
        int my_len = 0;
        uint64_t tmp = my_history;
        while (tmp) { ++my_len; tmp >>= 1; }
        round = my_len;

        if (round < 6) {
            if (opp_history & 1ULL) return Move::DEFECT;
            return Move::COOPERATE;
        }

        if (round % 6 == 0) {
            uint64_t last6 = opp_history & 0x3FULL;
            if (last6 == 0x00ULL) {
                opponent_class = OppClass::COOPERATIVE;
            } else if (last6 == 0x3FULL) {
                opponent_class = OppClass::ALLD;
            } else if (last6 == 0x2AULL) {  // D,C,D,C,D,C -> bits: 101010
                opponent_class = OppClass::STFT;
            } else if (last6 == 0x36ULL) {  // D,D,C,D,D,C -> bits: 110110
                opponent_class = OppClass::PAVLOVD;
            } else {
                opponent_class = OppClass::RANDOM;
            }
        }

        bool opp_last_defect = opp_history & 1ULL;

        switch (opponent_class) {
            case OppClass::RANDOM:
            case OppClass::ALLD:
                return Move::DEFECT;

            case OppClass::STFT:
                if (round % 6 == 0 || round % 6 == 1) return Move::COOPERATE;
                return opp_last_defect ? Move::DEFECT : Move::COOPERATE;

            case OppClass::PAVLOVD:
                if (round % 6 == 0) return Move::DEFECT;
                return Move::COOPERATE;

            case OppClass::COOPERATIVE:
                return opp_last_defect ? Move::DEFECT : Move::COOPERATE;

            default:
                return Move::COOPERATE;
        }
    }
};

class APavlov2011 : public IStrategy {
private:
    OppClass opponent_class;
    int round;

public:
    APavlov2011() : opponent_class(OppClass::UNKNOWN), round(0) {}

    void reset() override {
        opponent_class = OppClass::UNKNOWN;
        round = 0;
    }

    Move getMove(uint64_t opp_history, uint64_t my_history) override {
        int my_len = 0;
        uint64_t tmp = my_history;
        while (tmp) { ++my_len; tmp >>= 1; }
        round = my_len;

        if (round < 6) {
            if (opp_history & 1ULL) return Move::DEFECT;
            return Move::COOPERATE;
        }

        if (round % 6 == 0) {
            uint64_t last6 = opp_history & 0x3FULL;
            int d_count = 0;
            for (int i = 0; i < 6; ++i) {
                if (last6 & (1ULL << i)) ++d_count;
            }

            if (last6 == 0x00ULL) {
                opponent_class = OppClass::COOPERATIVE;
            } else if (d_count >= 4) {
                opponent_class = OppClass::ALLD;
            } else if (d_count == 3) {
                opponent_class = OppClass::STFT;
            } else {
                opponent_class = OppClass::RANDOM;
            }
        }

        bool opp_last_defect = opp_history & 1ULL;

        switch (opponent_class) {
            case OppClass::RANDOM:
            case OppClass::ALLD:
                return Move::DEFECT;

            case OppClass::STFT: {
                bool opp_second_last_defect = (opp_history >> 1) & 1ULL;
                if (opp_last_defect && opp_second_last_defect) return Move::DEFECT;
                return Move::COOPERATE;
            }

            case OppClass::COOPERATIVE:
                return opp_last_defect ? Move::DEFECT : Move::COOPERATE;

            default:
                return Move::COOPERATE;
        }
    }
};

class Appeaser : public IStrategy {
public:
    void reset() override {}

    Move getMove(uint64_t opp_history, uint64_t my_history) override {
        if (my_history == 0) {
            return Move::COOPERATE;
        }

        bool last_opp_defect = opp_history & 1ULL;
        bool last_my_defect  = my_history & 1ULL;

        if (last_opp_defect) {
            return last_my_defect ? Move::COOPERATE : Move::DEFECT;
        } else {
            return last_my_defect ? Move::DEFECT : Move::COOPERATE;
        }
    }
};

#endif
