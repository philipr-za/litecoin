#pragma once

#include <mw/common/Traits.h>
#include <mw/models/crypto/Commitment.h>

//
// Represents coins being pegged in, i.e. moved from canonical chain to the extension block.
//
class PegInCoin : public Traits::ISerializable, public Traits::IPrintable
{
public:
    PegInCoin() = default;
    PegInCoin(const uint64_t amount, const Commitment& commitment)
        : m_amount(amount), m_commitment(commitment) { }
    PegInCoin(const uint64_t amount, Commitment&& commitment)
        : m_amount(amount), m_commitment(std::move(commitment)) { }

    bool operator==(const PegInCoin& rhs) const noexcept
    {
        return m_amount == rhs.m_amount && m_commitment == rhs.m_commitment;
    }

    uint64_t GetAmount() const noexcept { return m_amount; }
    const Commitment& GetCommitment() const noexcept { return m_commitment; }

    //
    // Serialization/Deserialization
    //
    IMPL_SERIALIZABLE(PegInCoin);
    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(m_amount);
        READWRITE(m_commitment);
    }

    std::string Format() const noexcept final
    {
        return std::string("PegInCoin(commitment: ") + m_commitment.Format() + ", amount: " + std::to_string(m_amount) + ")";
    }

private:
    uint64_t m_amount;
    Commitment m_commitment;
};