#ifndef ICHEATER_STRATEGY_HPP
#define ICHEATER_STRATEGY_HPP

#include "istrategy.hpp"
#include "move.hpp"

class ICheaterStrategy : public IStrategy {
public:
    virtual ~ICheaterStrategy() = default;
    virtual void setOpponentMove(Move opponent_move) = 0;
};

#endif
