// Copyright (c) 2021 The Litecoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <mw/node/BlockBuilder.h>
#include <mw/node/Node.h>
#include <mw/node/validation/BlockValidator.h>

#include <test_framework/Miner.h>
#include <test_framework/TestMWEB.h>

BOOST_FIXTURE_TEST_SUITE(TestBlockBuilder, MWEBTestingSetup)

BOOST_AUTO_TEST_CASE(BlockBuilder)
{
    auto pDatabase = GetDB();

    mw::CoinsViewDB::Ptr db_view = mw::Node::Init(GetDataDir(), mw::Header::CPtr{nullptr}, pDatabase);
    mw::CoinsViewCache::Ptr cached_view = std::make_shared<mw::CoinsViewCache>(db_view);

    test::Miner miner(GetDataDir());

    ///////////////////////
    // Mine Block 1
    ///////////////////////
    test::Tx block1_tx1 = test::Tx::CreatePegIn(1000);
    auto block1 = miner.MineBlock(150, { block1_tx1 });
    cached_view->ApplyBlock(block1.GetBlock());

    ///////////////////////
    // Mine Block 2
    ///////////////////////
    test::Tx block2_tx1 = test::Tx::CreatePegIn(500);
    auto block2 = miner.MineBlock(151, {block2_tx1});
    cached_view->ApplyBlock(block2.GetBlock());

    ///////////////////////
    // Flush View
    ///////////////////////
    auto pBatch = pDatabase->CreateBatch();
    cached_view->Flush(pBatch);
    pBatch->Commit();

    ///////////////////////
    // BlockBuilder
    ///////////////////////
    auto block_builder = std::make_shared<mw::BlockBuilder>(152, cached_view);

    test::Tx builder_tx1 = test::Tx::CreatePegIn(150);
    PegInCoin builder_tx1_pegin{ 150, builder_tx1.GetKernels().front().GetCommitment() };
    bool tx1_status = block_builder->AddTransaction(
        builder_tx1.GetTransaction(),
        { builder_tx1_pegin }
    );
    BOOST_REQUIRE(tx1_status);

    mw::Block::Ptr built_block = block_builder->BuildBlock();
    BOOST_REQUIRE(built_block->GetKernels().front() == builder_tx1.GetKernels().front());
    BlockValidator().Validate(built_block, built_block->GetHash(), { builder_tx1.GetPegInCoin() }, {});
}

BOOST_AUTO_TEST_SUITE_END()