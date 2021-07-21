/* Copyright(c) 2021 Piotr Kudlacik */
#include "TradeCache.h"
#include <limits>
#include <vector>
#include <iterator>


std::ostream& operator<<(std::ostream& str, const trade& data)
{
	return str << data.orderId << " : " << data.securityId << " : " << data.side << " : " << data.quantity << " : " << data.userId << " : " << data.companyName;
}


bool operator<(const trade& left, const trade& right) {
	return std::tie(left.orderId, left.securityId, left.quantity, left.side, left.userId, left.companyName)
		< std::tie(right.orderId, right.securityId, right.quantity, right.side, right.userId, right.companyName);
}

string tradeCache::getOppositeSide(const string& side) {
	if (side.compare("Buy") == 0) {
		return "Sell";
	}
	else if (side.compare("Sell") == 0) {
		return "Buy";
	}
	return{};
}

void tradeCache::add(const trade& order) {
	cache.insert(order);
}
size_t tradeCache::cancelSingleOrder(const string& orderId) {
	auto& usrId = cache.get<1>();
	return usrId.erase(orderId);
}

size_t tradeCache::cancelAllOrdersByUserId(const string& userId) {
	auto& usrId = cache.get<4>();
	return usrId.erase(userId);
}

bool tradeCache::cancelAllOrdersAtQuantity(const string& securityId, int quantity) {
	auto& secId = cache.get<2>();
	auto range = secId.equal_range(make_tuple(securityId, quantity));
	if (range.first != secId.end()) {
		secId.erase(range.first, range.second);
		return true;
	}
	return false;
}

bool tradeCache::cancelAllOrdersAboveQuantity(const string& securityId, int quantity) {
	auto& secId = cache.get<2>();
	auto begin = secId.upper_bound(make_tuple(securityId, quantity));
	auto end = secId.upper_bound(make_tuple(securityId, numeric_limits<int>::max()));
	if (begin != secId.end()) {
		secId.erase(begin, end);
		return true;
	}
	return false;
}

vector<trade_set_iterator> tradeCache::collectMatchedOrders(const trade_set_iterator& begin, const trade_set_iterator& end, int orderQuant, const string& company)
{
	int currQuant{ 0 };
	vector<trade_set_iterator> toExtract;
	for (auto it = begin; it != end; it++) {
		if (company == it->companyName) {
			continue;
		}
		currQuant += it->quantity;
		if (currQuant > orderQuant) {
			return toExtract;
		}
		cout << "---matched to:  " << *it << endl;
		toExtract.push_back(it);
	}
	return toExtract;
}

int tradeCache::matchOrders(const string& securityId, const string& side) {
	string otherSide = getOppositeSide(side);
	if (otherSide.empty()) {
		return 0;
	}
	auto& orders = cache.get<3>();
	auto range = orders.equal_range(make_tuple(securityId, side));
	vector<trade_set::node_type> extracted;
	int totalQuantityMatched{ 0 };
	for (auto item = range.first; item != range.second && item != orders.end(); ++item) {
		cout << *item << endl;
		int orderQuant = item->quantity;
		auto begin = orders.lower_bound(make_tuple(securityId, otherSide, orderQuant));
		auto end = orders.upper_bound(make_tuple(securityId, otherSide, 0));
		int currQuant{ 0 };
		vector<trade_set_iterator> toExtract;
		auto it = begin;
		while(it != end){
			if (item->companyName == it->companyName) {
				++it;
				continue;
			}
			currQuant += it->quantity;
			if (currQuant > orderQuant) {
				break;
			}
			cout << "---matched to:  " << *it << endl;
			totalQuantityMatched += it->quantity;
			extracted.push_back(orders.extract(it++));
		}
	}
	std::for_each(extracted.begin(), extracted.end(), [&orders](auto& in) {orders.insert(move(in)); });
	return totalQuantityMatched;
}

size_t tradeCache::countOrders(const string& securityId) {
	const auto& secId = cache.get<2>();
	return secId.count(make_tuple(securityId));
}


void tradeCache::print(string&& comment = "") {
	cout << endl;
	cout << comment << endl;
	cout << "orders: " << endl;
	for (auto& item : cache) {
		cout << item << endl;
	}
	cout << "orders end" << endl;
}

int main()
{
	vector<trade> orders{
		{"OrdId1", "US9128473801", "Buy",  1000, "User1", "CompanyA"},
		{"OrdId2", "US5422358DA3", "Sell", 2000, "User2", "CompanyB"},
		{"OrdId3", "US9128473801", "Sell", 500, "User3", "CompanyA"},
		{"OrdId4", "US5422358DA3", "Buy", 600, "User4", "CompanyC"},
		{"OrdId5", "US5422358DA3", "Buy", 100, "User5", "CompanyB"},
		{"OrdId6", "US19635GY645", "Buy", 1000, "User6", "CompanyD"},
		{"OrdId7", "US5422358DA3", "Buy", 2000, "User7", "CompanyE"},
		{"OrdId8", "US5422358DA3", "Sell", 5000, "User8", "CompanyE"}
	};

	vector<trade> duplicatedSecurityIdAndQuantity{
		{"OrdId1", "US9128473801", "Sell", 500, "User3", "CompanyA"},
		{"OrdId2", "US5422358DA3", "Buy", 100, "User1", "CompanyA"},
		{"OrdId3", "US5422358DA3", "Sell", 100, "User2", "CompanyB"},
		{"OrdId4", "US9128473801", "Sell", 700, "User3", "CompanyA"},
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


	tradeCache cache2;
	for (auto& i : orders) {
		cache2.add(i);
	}
	cache2.print("orders - initial state:");
	cache2.cancelAllOrdersAboveQuantity("US5422358DA3", 600);
	cache2.print("AFTER cancelAllOrdersAboveQuantity US5422358DA3 600");
	cache2.cancelAllOrdersAboveQuantity("US5422358DA3", 99);
	cache2.print("AFTER cancelAllOrdersAboveQuantity US5422358DA3 99");

	tradeCache cache;
	for (auto& i : duplicatedSecurityIdAndQuantity) {
		cache.add(i);
	}
	cache.print("duplicatedSecurityIdAndQuantity - initial state:");
	cache.cancelAllOrdersAtQuantity("US5422358DA3", 100);
	cache.print("AFTER cancelAllOrdersAtQuantity US5422358DA3 100");
	cache.cancelAllOrdersAboveQuantity("US9128473801", 100);
	cache.print("AFTER cancelAllOrdersAboveQuantity US9128473801, 100");
	cout << endl;

	cout << "Pring matching" << endl;
	tradeCache cache3;
	for (auto& i : orders) {
		cache3.add(i);
	}
	cout << endl;
	cout << "Total Matched: " << cache3.matchOrders("US5422358DA3", "Sell") << endl;
	cout << endl;
	cout << "Total Matched: " << cache3.matchOrders("US5422358DA3", "Sell") << endl;
	cout << endl;
	cout << "Total Matched: " << cache3.matchOrders("US5422358DA3", "Buy") << endl;
	cout << endl;
	cout << "Print sameQuantity matching" << endl;
	tradeCache cache4;
	for (auto& i : sameQuantity) {
		cache4.add(i);
	}
	cout << "Total Matched: " << cache4.matchOrders("US5422358DA3", "Buy") << endl;
	cout << endl;
	cout << "Total Matched: " << cache4.matchOrders("US5422358DA3", "Sell") << endl;
	cout << endl;

	tradeCache cache5;
	for (auto& i : firstHigher) {
		cache5.add(i);
	}
	cout << "Total Matched: " << cache5.matchOrders("US5422358DA3", "Buy") << endl;
	cout << endl;
}

