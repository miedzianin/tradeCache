/* Copyright (c) 2021 Piotr Kudlacik */
#include "pch.h"
#include "CppUnitTest.h"
#include "../TradeCache/TradeCache.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace tradeCacheTest
{

	TEST_CLASS(tradeCacheTest)
	{
		vector<trade> orders{
			{"OrdId1", "US9128473801", "Buy",  1000, "User1", "CompanyA"},
			{"OrdId2", "US5422358DA3", "Sell", 3000, "User2", "CompanyB"},
			{"OrdId3", "US9128473801", "Sell", 500, "User3", "CompanyA"},
			{"OrdId4", "US5422358DA3", "Buy", 600, "User4", "CompanyC"},
			{"OrdId5", "US5422358DA3", "Buy", 100, "User5", "CompanyB"},
			{"OrdId6", "US19635GY645", "Buy", 1000, "User6", "CompanyD"},
			{"OrdId7", "US5422358DA3", "Buy", 2000, "User7", "CompanyE"},
			{"OrdId8", "US5422358DA3", "Sell", 5000, "User8", "CompanyE"}
		};

		vector<trade> nonUniqueUserId{
			{ "OrdId1", "US9128473801", "Buy", 100, "User1", "CompanyA" },
			{ "OrdId2", "US9128473802", "Buy", 1000, "User2", "CompanyA" },
			{ "OrdId3", "US9128473801", "Buy", 5000, "User1", "CompanyA" }
		};

		vector<trade> duplicatedSecurityIdAndQuantity{
			{"OrdId1", "US9128473801", "Sell", 500, "User3", "CompanyA"},
			{"OrdId2", "US5422358DA3","Buy", 100, "User1", "CompanyA"},
			{"OrdId3", "US5422358DA3", "Sell", 100, "User2", "CompanyB"},
			{"OrdId4", "US5422358DA3", "Sell", 700, "User3", "CompanyA"},
			{"OrdId5", "US5422358DA3", "Buy", 600, "User4", "CompanyC"}
		};

		vector<trade> sameQuantity{
			{"OrdId2", "US5422358DA3","Buy", 100, "User1", "CompanyA"},
			{"OrdId3", "US5422358DA3", "Sell", 1000, "User2", "CompanyB"},
			{"OrdId4", "US5422358DA3", "Sell", 100, "User3", "CompanyA"},
			{"OrdId5", "US5422358DA3", "Buy", 1000, "User4", "CompanyC"}
		};

		vector<trade> firstHigher{
			{"OrdId2", "US5422358DA3","Buy", 600, "User1", "CompanyA"},
			{"OrdId4", "US5422358DA3", "Sell", 700, "User2", "CompanyB"},
			{"OrdId3", "US5422358DA3", "Sell", 100, "User3", "CompanyC"},
			{"OrdId5", "US5422358DA3", "Sell", 800, "User3", "CompanyD"}
		};

		tradeCache cacheUnderTest;

	public:

		TEST_METHOD(ShouldAddOneOrder)
		{
			cacheUnderTest.add(orders[0]);
			Assert::AreEqual(size_t(1), cacheUnderTest.cancelSingleOrder("OrdId1"));
		}
		TEST_METHOD(ShouldCountOrdersBySecurityId)
		{
			for (int i = 0; i < 3; i++) {
				cacheUnderTest.add(orders[i]);
			}
			Assert::AreEqual(size_t(2), cacheUnderTest.countOrders("US9128473801"));
		}

		TEST_METHOD(ShouldCancelSingleOrderByOrderId)
		{
			for (auto& order : nonUniqueUserId) {
				cacheUnderTest.add(order);
			}
			Assert::AreEqual(size_t(1), cacheUnderTest.cancelSingleOrder("OrdId2"));
			Assert::AreEqual(size_t(2), cacheUnderTest.countOrders("US9128473801"));
		}

		TEST_METHOD(ShouldNotCancelAnyOtherOrderBesideGivenOrderId)
		{
			for (auto& order : nonUniqueUserId) {
				cacheUnderTest.add(order);
			}
			cacheUnderTest.cancelSingleOrder("OrdId2");
			Assert::AreEqual(size_t(2), cacheUnderTest.countOrders("US9128473801"));
		}
		TEST_METHOD(ShouldCancelAllOrdersByUserId)
		{
			for (auto& order : nonUniqueUserId) {
				cacheUnderTest.add(order);
			}
			Assert::AreEqual(size_t(2), cacheUnderTest.cancelAllOrdersByUserId("User1"));
		}
		TEST_METHOD(ShouldNotCancelAnyOtherOrderBesideGivenUserId)
		{
			for (auto& order : nonUniqueUserId) {
				cacheUnderTest.add(order);
			}
			cacheUnderTest.cancelAllOrdersByUserId("User1");
			Assert::AreEqual(size_t(1), cacheUnderTest.countOrders("US9128473802"));
		}
		TEST_METHOD(ShouldCancelAllOrdersAtQuantity)
		{
			for (auto& order : duplicatedSecurityIdAndQuantity) {
				cacheUnderTest.add(order);
			}
			Assert::IsTrue(cacheUnderTest.cancelAllOrdersAtQuantity("US5422358DA3", 100));
			Assert::AreEqual(size_t(0), cacheUnderTest.cancelSingleOrder("OrdId2"));
			Assert::AreEqual(size_t(0), cacheUnderTest.cancelSingleOrder("OrdId3"));
		}

		TEST_METHOD(ShouldCancelAllOrdersAboveQuantity)
		{
			for (auto& order : duplicatedSecurityIdAndQuantity) {
				cacheUnderTest.add(order);
			}
			Assert::IsTrue(cacheUnderTest.cancelAllOrdersAboveQuantity("US5422358DA3", 100));
			Assert::AreEqual(size_t(0), cacheUnderTest.cancelSingleOrder("OrdId4"));
			Assert::AreEqual(size_t(0), cacheUnderTest.cancelSingleOrder("OrdId5"));
		}

		TEST_METHOD(ShouldMatchSellWithBuyOrdersForSecurityId)
		{
			for (auto& order : orders) {
				cacheUnderTest.add(order);
			}
			Assert::AreEqual(2700, cacheUnderTest.matchOrders("US5422358DA3", "Sell"));
		}
		
		TEST_METHOD(ShouldMatchBuyWithSellOrdersForSecurityId)
		{
			for (auto& order : orders) {
				cacheUnderTest.add(order);
			}
			Assert::AreEqual(0, cacheUnderTest.matchOrders("US5422358DA3", "Buy"));
		}

		TEST_METHOD(ShouldMatchMultipleBuyWithSellOrdersForSecurityIdWithSameQuantity)
		{
			for (auto& order : sameQuantity) {
				cacheUnderTest.add(order);
			}
			Assert::AreEqual(1000, cacheUnderTest.matchOrders("US5422358DA3", "Buy"));
		}

		TEST_METHOD(ShouldMatchMultipleSellWithBuyOrdersForSecurityIdWithSameQuantity)
		{
			for (auto& order : sameQuantity) {
				cacheUnderTest.add(order);
			}
			Assert::AreEqual(1000, cacheUnderTest.matchOrders("US5422358DA3", "Sell"));
		}
		TEST_METHOD(ShouldRevertExtractionAfterCountingMatchedOrders)
		{
			for (auto& order : orders) {
				cacheUnderTest.add(order);
			}
			cacheUnderTest.matchOrders("US5422358DA3", "Sell");
			Assert::AreEqual(2700, cacheUnderTest.matchOrders("US5422358DA3", "Sell"));
		}

		TEST_METHOD(ShouldMatchMultipleBuyWithSellOrdersForSecurityIdOne)
		{
			for (auto& order : firstHigher) {
				cacheUnderTest.add(order);
			}
			Assert::AreEqual(100, cacheUnderTest.matchOrders("US5422358DA3", "Buy"));
		}
	};
}
