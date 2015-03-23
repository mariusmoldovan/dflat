/*{{{
Copyright 2012-2015, Bernhard Bliem
WWW: <http://dbai.tuwien.ac.at/research/project/dflat/>.

This file is part of D-FLAT.

D-FLAT is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

D-FLAT is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with D-FLAT.  If not, see <http://www.gnu.org/licenses/>.
*/
//}}}
#include <algorithm>

#include "Solver.h"
#include "../../Decomposition.h"
#include "../../Application.h"
#include "../../Printer.h"

namespace {

bool isJoinable(const ItemTreeNode& left, const ItemTreeNode& right)
{
	return left.getItems() == right.getItems() &&
		(left.getType() == ItemTreeNode::Type::UNDEFINED || right.getType() == ItemTreeNode::Type::UNDEFINED || left.getType() == right.getType());
}

ItemTreePtr join(unsigned int leftNodeIndex, const ItemTree* left, unsigned int rightNodeIndex, const ItemTree* right, bool setLeavesToAccept)
{
	assert(left);
	assert(right);
	assert(left->getNode());
	assert(right->getNode());
	if(!isJoinable(*left->getNode(), *right->getNode()))
		return ItemTreePtr();

	// Join left and right
	ItemTreeNode::Items items = left->getNode()->getItems();
	ItemTreeNode::Items auxItems;
	// Unify auxiliary items
	const auto& auxL = left->getNode()->getAuxItems();
	const auto& auxR = right->getNode()->getAuxItems();
	std::set_union(auxL.begin(), auxL.end(), auxR.begin(), auxR.end(), std::inserter(auxItems, auxItems.end()));

	ItemTreeNode::ExtensionPointers extensionPointers = {{left->getNode(), right->getNode()}};
	const bool leaves = left->getChildren().empty() && right->getChildren().empty();
	ItemTreeNode::Type type = left->getNode()->getType();
	if(type == ItemTreeNode::Type::UNDEFINED) {
		type = right->getNode()->getType();
		if(setLeavesToAccept && leaves && type == ItemTreeNode::Type::UNDEFINED)
			type = ItemTreeNode::Type::ACCEPT;
	}
	ItemTreePtr result(new ItemTree(ItemTree::Node(new ItemTreeNode(std::move(items), std::move(auxItems), std::move(extensionPointers), type))));

	// Set (initial) cost of this node
	if(leaves) {
		result->getNode()->setCost(left->getNode()->getCost() - left->getNode()->getCurrentCost() + right->getNode()->getCost());
		assert(left->getNode()->getCurrentCost() == right->getNode()->getCurrentCost());
		result->getNode()->setCurrentCost(left->getNode()->getCurrentCost());
	} else {
		assert(left->getNode()->getCurrentCost() == right->getNode()->getCurrentCost() && left->getNode()->getCurrentCost() == 0);
		switch(type) {
			case ItemTreeNode::Type::OR:
				// Set cost to "infinity"
				result->getNode()->setCost(std::numeric_limits<decltype(result->getNode()->getCost())>::max());
				break;

			case ItemTreeNode::Type::AND:
				// Set cost to minus "infinity"
				result->getNode()->setCost(std::numeric_limits<decltype(result->getNode()->getCost())>::min());
				break;

			case ItemTreeNode::Type::UNDEFINED:
				break;

			default:
				assert(false);
				break;
		}
	}

	// Join children recursively
	auto lit = left->getChildren().begin();
	auto rit = right->getChildren().begin();
	while(lit != left->getChildren().end() && rit != right->getChildren().end()) {
		ItemTreePtr childResult = join(leftNodeIndex, lit->get(), rightNodeIndex, rit->get(), setLeavesToAccept);
		if(childResult) {
			// lit and rit match
			// Remember position of rit. We will later advance rit until is doesn't match with lit anymore.
			auto mark = rit;
join_lit_with_all_matches:
			// Join lit will all partners starting at rit
			do {
				// Update cost
				switch(type) {
					case ItemTreeNode::Type::OR:
						result->getNode()->setCost(std::min(result->getNode()->getCost(), childResult->getNode()->getCost()));
						break;

					case ItemTreeNode::Type::AND:
						result->getNode()->setCost(std::max(result->getNode()->getCost(), childResult->getNode()->getCost()));
						break;

					case ItemTreeNode::Type::UNDEFINED:
						break;

					default:
						assert(false);
						break;
				}

				result->addChildAndMerge(std::move(childResult));
				++rit;
				if(rit == right->getChildren().end())
					break;
				childResult = join(leftNodeIndex, lit->get(), rightNodeIndex, rit->get(), setLeavesToAccept);
			} while(childResult);

			// lit and rit don't match anymore (or rit is past the end)
			// Advance lit. If it joins with mark, reset rit to mark.
			++lit;
			if(lit != left->getChildren().end()) {
				childResult = join(leftNodeIndex, lit->get(), rightNodeIndex, mark->get(), setLeavesToAccept);
				if(childResult) {
					rit = mark;
					goto join_lit_with_all_matches;
				}
			}
		}
		else {
			// lit and rit don't match
			// Advance iterator pointing to smaller value
			if((*lit)->getNode()->getItems() < (*rit)->getNode()->getItems())
				++lit;
			else
				++rit;
		}
	}

	// In leaves, make sure two branches can only be joined if they have the same length
	assert(result);
	if(result->getChildren().empty() && (!left->getChildren().empty() || !right->getChildren().empty()))
		result.reset();

	return result;
}

} // anonymous namespace

namespace solver { namespace default_join {

Solver::Solver(const Decomposition& decomposition, const Application& app, bool setLeavesToAccept)
	: ::Solver(decomposition, app)
	, setLeavesToAccept(setLeavesToAccept)
{
}

ItemTreePtr Solver::compute()
{
	assert(decomposition.getChildren().size() > 1);
	ItemTreePtr result;

	// Compute item trees of child nodes
	// When at least two have been computed, join them with the result so far
	// TODO Use a balanced join tree (with smaller "tables" further down)
	auto it = decomposition.getChildren().begin();
	const ItemTree* left = (*it)->getSolver().getResult();
	unsigned int leftChildIndex = (*it)->getNode().getGlobalId();
	for(++it; it != decomposition.getChildren().end(); ++it) {
		if(!left)
			return ItemTreePtr();
		const ItemTree* right = (*it)->getSolver().getResult();
		if(!right)
			return ItemTreePtr();
		result = join(leftChildIndex, left, (*it)->getNode().getGlobalId(), right, setLeavesToAccept);
		left = result.get();
		leftChildIndex = (*it)->getNode().getGlobalId();
	}

	if(result && result->finalize(app, decomposition.isRoot(), app.isPruningDisabled() == false || decomposition.isRoot()) == false)
		result.reset();
	app.getPrinter().solverInvocationResult(decomposition, result.get());
	return result;
}

}} // namespace solver::default_join
