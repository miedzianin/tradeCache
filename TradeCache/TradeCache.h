/* Copyright(c) 2021 Piotr Kudlacik */
#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <tuple>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/composite_key.hpp>


using namespace std;
using boost::multi_index_container;
using namespace boost::multi_index;
using namespace boost::multi_index::detail;


struct trade
{
	string orderId{};
	string securityId{};
	string side{};
	int quantity{};
	string userId{};
	string companyName{};

	friend bool operator<(const trade& left, const trade& right);
	friend std::ostream& operator<<(std::ostream& str, const trade& data);
};

typedef multi_index_container<
	trade,
	indexed_by<
	ordered_unique<identity<trade> >,
	ordered_unique<member<trade, string, &trade::orderId> >,
	ordered_non_unique<
	composite_key<
		trade,
		member<trade, string, &trade::securityId>,
		member<trade, int, &trade::quantity>
		>
	>,
	ordered_non_unique<
	composite_key<
		trade,
		member<trade, string, &trade::securityId>,
		member<trade, string, &trade::side>,
		member<trade, int, &trade::quantity>
		>,
		composite_key_compare<
		less<string>, 
		less<string>,
		greater<int>
		>
	>,
	ordered_non_unique<member<trade, string, &trade::userId> >
	>
> trade_set;

using trade_set_iterator = bidir_node_iterator<ordered_index_node<null_augment_policy, ordered_index_node<null_augment_policy, index_node_base<trade, std::allocator<trade>>>>>;

class tradeCache {
	trade_set cache;
	string getOppositeSide(const string& side);
	vector<trade_set_iterator> collectMatchedOrders(const trade_set_iterator& begin, const trade_set_iterator& end, int orderQuant, const string& company);
public:
	void add(const trade& order);
	size_t cancelSingleOrder(const string& orderId);
	size_t cancelAllOrdersByUserId(const string& userId);
	bool cancelAllOrdersAtQuantity(const string& securityId, int quantity);
	bool cancelAllOrdersAboveQuantity(const string& securityId, int quantity);
	size_t countOrders(const string& securityId);
	int matchOrders(const string& securityId, const string& side);
	void print(string&& comment);
};