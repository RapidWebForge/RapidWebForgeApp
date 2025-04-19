#ifndef TRANSACTIONOPERATION_H
#define TRANSACTIONOPERATION_H

#include "../node-operation/nodeoperation.h"
#include "../transaction/transaction.h"

struct TransactionOperation
{
    OperationType type;
    Transaction transaction;

    // Constructor
    TransactionOperation(OperationType opType, const Transaction &transactionRef)
        : type(opType)
        , transaction(transactionRef)
    {}
};

#endif // TRANSACTIONOPERATION_H
