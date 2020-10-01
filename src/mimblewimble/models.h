// Copyright(C) 2011 - 2020 The Litecoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef LITECOIN_MIMBLEWIMBLE_MODELS_H
#define LITECOIN_MIMBLEWIMBLE_MODELS_H

#include <serialize.h>
#include <libmw/libmw.h>
#include <vector>
#include <memory>

struct CMWBlock
{
    using CPtr = std::shared_ptr<CMWBlock>;

    libmw::BlockRef m_block;

    CMWBlock() = default;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        if (ser_action.ForRead()) {
            // Deserialize
            std::vector<uint8_t> bytes;
            READWRITE(bytes);

            if (!bytes.empty()) {
                m_block = libmw::DeserializeBlock(std::move(bytes));
            }
        } else {
            // Serialize
            if (!IsNull()) {
                std::vector<uint8_t> bytes = libmw::SerializeBlock(m_block);
                READWRITE(bytes);
            } else {
                READWRITE(std::vector<uint8_t>{});
            }
        }
    }

    bool IsNull() const noexcept { return m_block.pBlock == nullptr; }
    void SetNull() noexcept { m_block = libmw::BlockRef{ nullptr }; }
};

struct CMWTx
{
    using CPtr = std::shared_ptr<CMWTx>;

    libmw::TxRef m_transaction;

    CMWTx() = default;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        if (ser_action.ForRead()) {
            // Deserialize
            std::vector<uint8_t> bytes;
            READWRITE(bytes);

            if (!bytes.empty()) {
                m_transaction = libmw::DeserializeTx(std::move(bytes));
            }
        } else {
            // Serialize
            if (!IsNull()) {
                std::vector<uint8_t> bytes = libmw::SerializeTx(m_transaction);
                READWRITE(bytes);
            } else {
                READWRITE(std::vector<uint8_t>{});
            }
        }
    }

    bool IsNull() const noexcept { return m_transaction.pTransaction == nullptr; }
};

#endif // LITECOIN_MIMBLEWIMBLE_MODELS_H