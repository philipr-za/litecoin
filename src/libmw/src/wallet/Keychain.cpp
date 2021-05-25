#include <mw/wallet/Keychain.h>
#include <mw/crypto/Hasher.h>
#include <mw/common/Logger.h>

#define RESTORE_WINDOW 100

MW_NAMESPACE

bool Keychain::RewindOutput(const Output& output, mw::Coin& coin) const
{
    SecretKey t = Hashed(EHashTag::DERIVE, output.Ke().Mul(GetScanSecret()));
    if (t[0] != output.GetViewTag()) {
        return false;
    }

    PublicKey B = output.Ko().Sub(Hashed(EHashTag::OUT_KEY, t));

    // Check if B belongs to wallet
    uint32_t index = 0;
    if (!IsSpendPubKey(B, index)) {
        return false;
    }

    StealthAddress wallet_addr = GetStealthAddress(index);
    Deserializer hash64(Hash512(t).vec());
    SecretKey r = hash64.Read<SecretKey>();
    uint64_t value = output.GetMaskedValue() ^ hash64.Read<uint64_t>();
    BigInt<16> n = output.GetMaskedNonce() ^ hash64.ReadVector(16);

    if (Commitment::Switch(r, value) != output.GetCommitment()) {
        return false;
    }

    // Calculate Carol's sending key 's' and check that s*B ?= Ke
    SecretKey s = Hasher(EHashTag::SEND_KEY)
                      .Append(wallet_addr.A())
                      .Append(wallet_addr.B())
                      .Append(value)
                      .Append(n)
                      .hash();
    if (output.Ke() != wallet_addr.B().Mul(s)) {
        return false;
    }

    SecretKey private_key = Crypto::AddPrivateKeys(
        Hashed(EHashTag::OUT_KEY, t),
        GetSpendKey(index)
    );

    coin.features = output.GetFeatures().Get();
    coin.address_index = index;
    coin.key = boost::make_optional(BlindingFactor(private_key.data()));
    coin.blind = boost::make_optional(BlindingFactor(r.data()));
    coin.amount = value;
    coin.commitment = output.GetCommitment();

    return true;
}

StealthAddress Keychain::GetStealthAddress(const uint32_t index) const
{
    assert(index < ULONG_MAX);
    m_addressIndexCounter = std::max(m_addressIndexCounter, index);

    PublicKey Bi = PublicKey::From(GetSpendKey(index));
    PublicKey Ai = Bi.Mul(m_scanSecret);

    return StealthAddress(Ai, Bi);
}

SecretKey Keychain::GetSpendKey(const uint32_t index) const
{
    assert(index < ULONG_MAX);

    SecretKey mi = Hasher(EHashTag::ADDRESS)
        .Append<uint32_t>(index)
        .Append(m_scanSecret)
        .hash();

    return Crypto::AddPrivateKeys(m_spendSecret, mi);
}

bool Keychain::IsSpendPubKey(const PublicKey& spend_pubkey, uint32_t& index_out) const
{
    // Ensure pubkey cache is fully populated
    while (m_pubkeys.size() <= ((size_t)m_addressIndexCounter + RESTORE_WINDOW)) {
        LOG_INFO_F("Size: {}, Counter: {}", m_pubkeys.size(), m_addressIndexCounter);
        uint32_t pubkey_index = (uint32_t)m_pubkeys.size();
        PublicKey pubkey = PublicKey::From(GetSpendKey(pubkey_index));

        m_pubkeys.insert({ std::move(pubkey), pubkey_index });
    }

    auto iter = m_pubkeys.find(spend_pubkey);
    if (iter != m_pubkeys.end()) {
        index_out = iter->second;
        return true;
    }

    return false;
}

END_NAMESPACE