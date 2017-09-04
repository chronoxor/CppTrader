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
    assert((Quantity > 0) && "Order quantity must be greater than zero!");
    if (Quantity == 0)
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
    if (IsStop())
    {
        assert(!IsAON() && "Stop order cannot have 'All-Or-None' parameter!");
        if (IsAON())
            return ErrorCode::ORDER_PARAMETER_INVALID;
        assert(!IsIceberg() && "Stop order cannot be 'Iceberg'!");
        if (IsIceberg())
            return ErrorCode::ORDER_PARAMETER_INVALID;
    }

    // Validate stop limit order
    if (IsStopLimit())
    {
        assert(!IsSlippage() && "Stop-limit order cannot have slippage!");
        if (IsSlippage())
            return ErrorCode::ORDER_PARAMETER_INVALID;
    }

    return ErrorCode::OK;
}

} // namespace Matching
} // namespace CppTrader
