#pragma once

#include <dbwrapper.h>
#include <mw/interfaces/db_interface.h>

namespace MWEB {

class DBBatch : public mw::IDBBatch
{
public:
    DBBatch(CDBWrapper* pDB, const std::shared_ptr<CDBBatch>& pBatch)
        : m_pDB(pDB), m_pBatch(pBatch) {}

    void Write(const std::string& key, const std::vector<uint8_t>& value) final
    {
        m_pBatch->Write(key, value);
    }

    void Erase(const std::string& key) final
    {
        m_pBatch->Erase(key);
    }

    void Commit() final
    {
        m_pDB->WriteBatch(*m_pBatch);
    }

private:
    CDBWrapper* m_pDB;
    std::shared_ptr<CDBBatch> m_pBatch;
};

class DBIterator : public mw::IDBIterator
{
public:
    DBIterator(CDBIterator* pIterator)
        : m_pIterator(std::unique_ptr<CDBIterator>(pIterator)) {}

    void Seek(const std::string& key) final
    {
        m_pIterator->Seek(key);
    }

    void Next() final
    {
        m_pIterator->Next();
    }

    bool GetKey(std::string& key) const final
    {
        return m_pIterator->GetKey(key);
    }

    bool Valid() const final
    {
        return m_pIterator->Valid();
    }

private:
    std::unique_ptr<CDBIterator> m_pIterator;
};

class DBWrapper : public mw::IDBWrapper
{
public:
    DBWrapper(CDBWrapper* pDB) : m_pDB(pDB) {}

    bool Read(const std::string& key, std::vector<uint8_t>& value) const final // MW: TODO - Should support serializable object instead of vector?
    {
        return m_pDB->Read(key, value);
    }

    std::unique_ptr<mw::IDBIterator> NewIterator() final
    {
        return std::unique_ptr<mw::IDBIterator>(new MWEB::DBIterator(m_pDB->NewIterator()));
    }

    std::unique_ptr<mw::IDBBatch> CreateBatch() final
    {
        return std::unique_ptr<mw::IDBBatch>(new MWEB::DBBatch(m_pDB, std::make_shared<CDBBatch>(*m_pDB)));
    }

private:
    CDBWrapper* m_pDB;
};

} // namespace MWEB