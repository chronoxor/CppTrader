/*!
    \file order.cpp
    \brief Order implementation
    \author Ivan Shynkarenka
    \date 31.07.2017
    \copyright MIT License
*/

#include "trader/matching/order.h"

namespace CppTrader {
namespace Matching {

ErrorCode Order::Validate() const noexcept
{
    // Validate order Id
    assert((Id > 0) && "Order Id must be greater than zero!");
    if (Id == 0)
        return ErrorCode::ORDER_ID_INVALID;

    // Validate order quantity
    assert((Quantity >= LeavesQuantity) && "Order quantity must be greater than or equal to order leaves quantity!");
    if (Quantity < LeavesQuantity)
        return ErrorCode::ORDER_QUANTITY_INVALID;
    assert((LeavesQuantity > 0) && "Order leaves quantity must be greater than zero!");
    if (LeavesQuantity == 0)
        return ErrorCode::ORDER_QUANTITY_INVALID;

    // Validate market order
    if (IsMarket())
    {
        assert((IsIOC() || IsFOK()) && "Market order must have 'Immediate-Or-Cancel' or 'Fill-Or-Kill' parameter!");
        if (!IsIOC() && !IsFOK())
            return ErrorCode::ORDER_PARAMETER_INVALID;
        assert(!IsIceberg() && "Market order cannot be 'Iceberg'!");
        if (IsIceberg())
            return ErrorCode::ORDER_PARAMETER_INVALID;
    }

    // Validate limit order
    if (IsLimit())
    {
        assert(!IsSlippage() && "Limit order cannot have slippage parameter!");
        if (IsSlippage())
            return ErrorCode::ORDER_PARAMETER_INVALID;
    }

    // Validate stop order
    if (IsStop() || IsTrailingStop())
    {
        assert(!IsAON() && "Stop order cannot have 'All-Or-None' parameter!");
        if (IsAON())
            return ErrorCode::ORDER_PARAMETER_INVALID;
        assert(!IsIceberg() && "Stop order cannot be 'Iceberg'!");
        if (IsIceberg())
            return ErrorCode::ORDER_PARAMETER_INVALID;
    }

    // Validate stop-limit order
    if (IsStopLimit() || IsTrailingStopLimit())
    {
        assert(!IsSlippage() && "Stop-limit order cannot have slippage!");
        if (IsSlippage())
            return ErrorCode::ORDER_PARAMETER_INVALID;
    }

    // Validate trailing order
    if (IsTrailingStop() || IsTrailingStopLimit())
    {
        assert((TrailingDistance != 0) && "Trailing stop order must have non zero distance to the market!");
        if (TrailingDistance == 0)
            return ErrorCode::ORDER_PARAMETER_INVALID;

        if (TrailingDistance > 0)
        {
            assert(((TrailingStep >= 0) && (TrailingStep < TrailingDistance)) && "Trailing step must be less than trailing distance!");
            if ((TrailingStep < 0) || (TrailingStep >= TrailingDistance))
                return ErrorCode::ORDER_PARAMETER_INVALID;
        }
        else
        {
            assert(((TrailingDistance <= -1) && (TrailingDistance >= -1000)) && "Trailing percentage distance must be in the range [0.01, 100%] (from -1 down to -10000)!");
            if ((TrailingDistance > -1) || (TrailingDistance < -1000))
                return ErrorCode::ORDER_PARAMETER_INVALID;
            assert(((TrailingStep <= 0) && (TrailingStep > TrailingDistance)) && "Trailing step must be less than trailing distance!");
            if ((TrailingStep > 0) || (TrailingStep <= TrailingDistance))
                return ErrorCode::ORDER_PARAMETER_INVALID;
        }
    }

    return ErrorCode::OK;
}

} // namespace Matching
} // namespace CppTrader
