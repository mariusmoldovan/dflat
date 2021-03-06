/*{{{
Copyright 2012-2016, Bernhard Bliem, Marius Moldovan
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
#include "GringoOutputProcessor.h"

namespace {

std::vector<std::string> getArguments(Gringo::Symbol sym)
{
	std::vector<std::string> result;
	result.reserve(sym.args().size());
	for(const auto& arg : sym.args()) {
		std::ostringstream ss;
		arg.print(ss);
		result.push_back(ss.str());
	}
	return result;
}

} // anonymous namespace

namespace solver { namespace clasp { namespace trees {

GringoOutputProcessor::GringoOutputProcessor(Clasp::Asp::LogicProgram& out, const ChildItemTrees& childItemTrees)
	: asp_utils::GringoOutputProcessor(out)
	, childItemTrees(childItemTrees)
{
}

const GringoOutputProcessor::ItemAtomInfos& GringoOutputProcessor::getItemAtomInfos() const
{
	return itemAtomInfos;
}

const GringoOutputProcessor::AuxItemAtomInfos& GringoOutputProcessor::getAuxItemAtomInfos() const
{
	return auxItemAtomInfos;
}

const GringoOutputProcessor::ExtendAtomInfos& GringoOutputProcessor::getExtendAtomInfos() const
{
	return extendAtomInfos;
}

const GringoOutputProcessor::CurrentCostAtomInfos& GringoOutputProcessor::getCurrentCostAtomInfos() const
{
	return currentCostAtomInfos;
}

const GringoOutputProcessor::CostAtomInfos& GringoOutputProcessor::getCostAtomInfos() const
{
	return costAtomInfos;
}

const GringoOutputProcessor::LengthAtomInfos& GringoOutputProcessor::getLengthAtomInfos() const
{
	return lengthAtomInfos;
}

const GringoOutputProcessor::OrAtomInfos& GringoOutputProcessor::getOrAtomInfos() const
{
	return orAtomInfos;
}

const GringoOutputProcessor::AndAtomInfos& GringoOutputProcessor::getAndAtomInfos() const
{
	return andAtomInfos;
}

const GringoOutputProcessor::CounterRemAtomInfos& GringoOutputProcessor::getCounterRemAtomInfos() const
{
	return counterRemAtomInfos;
}

const GringoOutputProcessor::AllCounterIncAtomInfos& GringoOutputProcessor::getAllCounterIncAtomInfos() const
{
	return allCounterIncAtomInfos;
}

const GringoOutputProcessor::AllCurrentCounterIncAtomInfos& GringoOutputProcessor::getAllCurrentCounterIncAtomInfos() const
{
	return allCurrentCounterIncAtomInfos;
}

const GringoOutputProcessor::AllCounterAtomInfos& GringoOutputProcessor::getAllCounterAtomInfos() const
{
	return allCounterAtomInfos;
}

const GringoOutputProcessor::AllCurrentCounterAtomInfos& GringoOutputProcessor::getAllCurrentCounterAtomInfos() const
{
	return allCurrentCounterAtomInfos;
}

const Clasp::Var* GringoOutputProcessor::getAcceptAtomKey() const
{
	return acceptAtomKey.get();
}

const Clasp::Var* GringoOutputProcessor::getRejectAtomKey() const
{
	return rejectAtomKey.get();
}

void GringoOutputProcessor::storeAtom(uint32_t atom, Gringo::Symbol sym)
{
	// Store the atom together with its symbol table key and extracted arguments
	const std::string predicate = *sym.name();
	if(predicate == "item") {
		ASP_CHECK(sym.args().size() == 2, "'item' predicate does not have arity 2");
		std::vector<std::string> arguments = getArguments(sym);
		itemAtomInfos.emplace_back(ItemAtomInfo{ItemAtomArguments{static_cast<unsigned int>(std::stoi(arguments[0])), std::move(arguments[1])}, atom});
	} else if(predicate == "auxItem") {
		ASP_CHECK(sym.args().size() == 2, "'auxItem' predicate does not have arity 2");
		std::vector<std::string> arguments = getArguments(sym);
		auxItemAtomInfos.emplace_back(AuxItemAtomInfo{AuxItemAtomArguments{static_cast<unsigned int>(std::stoi(arguments[0])), std::move(arguments[1])}, atom});
	} else if(predicate == "extend") {
		ASP_CHECK(sym.args().size() == 2, "'extend' predicate does not have arity 2");
		const std::vector<std::string> arguments = getArguments(sym);
		const unsigned int level = std::stol(arguments[0]);
		const std::string extended = std::move(arguments[1]);
		// (Decomposition) child node number is before the first '_' (and after the leading 'n')
		// '_' then separates item tree child indices
		size_t underscorePos = extended.find('_');
		const unsigned int decompositionChildId = std::stoi(std::string(extended, 1, underscorePos-1));
		ASP_CHECK(childItemTrees.find(decompositionChildId) != childItemTrees.end(), "Extension pointer refers to nonexistent decomposition child");

		const ItemTree* current = childItemTrees.at(decompositionChildId).get();
		while(underscorePos != std::string::npos) {
			assert(current);
			const size_t lastUnderscorePos = underscorePos;
			underscorePos = extended.find('_', underscorePos+1);
			unsigned int childNumber = std::stoi(std::string(extended, lastUnderscorePos+1, underscorePos));
			ASP_CHECK(childNumber < current->getChildren().size(), "Extension pointer refers to nonexistent item tree node");

			ItemTree::Children::const_iterator childIt = current->getChildren().begin();
			std::advance(childIt, childNumber);
			current = childIt->get();
		}
		// XXX Instead of the previous loop which runs through all levels, it could be beneficial to assign a globally unique ID to each item tree node and then use a lookup-table. (The globally unique ID could be either an integer, as is already the case in the Decomposition class, but it could also be a string like the one we are already using.)

		extendAtomInfos.emplace_back(ExtendAtomInfo{{level, decompositionChildId, current->getNode()}, atom});
	} else if(predicate == "currentCost") {
		ASP_CHECK(sym.args().size() == 1, "'currentCost' predicate does not have arity 1");
		currentCostAtomInfos.emplace_back(CurrentCostAtomInfo{{std::stol(getArguments(sym).front())}, atom});
//		std::ostringstream argument;
//		sym.args().front().print(argument);
//		allCurrentCounterAtomInfos["cost"].emplace_back(CurrentCounterAtomInfo{CurrentCounterAtomArguments{"cost", std::stol(argument.str())}, atom});
	} else if(predicate == "cost") {
		ASP_CHECK(sym.args().size() == 1, "'cost' predicate does not have arity 1");
		costAtomInfos.emplace_back(CostAtomInfo{{std::stol(getArguments(sym).front())}, atom});
//		std::ostringstream argument;
//		sym.args().front().print(argument);
//		allCounterAtomInfos["cost"].emplace_back(CounterAtomInfo{CounterAtomArguments{"cost", std::stol(argument.str())}, atom});
	} else if(predicate == "length") {
		ASP_CHECK(sym.args().size() == 1, "'length' predicate does not have arity 1");
		lengthAtomInfos.emplace_back(LengthAtomInfo{{static_cast<unsigned int>(std::stol(getArguments(sym).front()))}, atom});
	} else if(predicate == "or") {
		ASP_CHECK(sym.args().size() == 1, "'or' predicate does not have arity 1");
		orAtomInfos.emplace_back(OrAtomInfo{{static_cast<unsigned int>(std::stol(getArguments(sym).front()))}, atom});
	} else if(predicate == "and") {
		ASP_CHECK(sym.args().size() == 1, "'and' predicate does not have arity 1");
		andAtomInfos.emplace_back(AndAtomInfo{{static_cast<unsigned int>(std::stol(getArguments(sym).front()))}, atom});
	} else if(predicate == "accept") {
		ASP_CHECK(sym.type() == Gringo::Value::ID, "'accept' predicate does not have arity 0");
		assert(!acceptAtomKey);
		acceptAtomKey.reset(new Clasp::Var(atom));
	} else if(predicate == "reject") {
		ASP_CHECK(sym.type() == Gringo::Value::ID, "'reject' predicate does not have arity 0");
		assert(!rejectAtomKey);
		rejectAtomKey.reset(new Clasp::Var(atom));
	} else if(predicate == "counterInc") {
		ASP_CHECK(sym.args().size() == 2, "'counterInc' predicate does not have arity 2");
		std::ostringstream counter;
		sym.args().front().print(counter);
		std::ostringstream value;
		sym.args()[1].print(value);
		allCounterIncAtomInfos[counter.str()].emplace_back(CounterIncAtomInfo{CounterIncAtomArguments{counter.str(), std::stol(value.str())}, atom});
	} else if(predicate == "counter") {
		ASP_CHECK(sym.args().size() == 2, "'counter' predicate does not have arity 2");
		std::ostringstream counter;
		sym.args().front().print(counter);
		std::ostringstream value;
		sym.args()[1].print(value);
		allCounterAtomInfos[counter.str()].emplace_back(CounterAtomInfo{CounterAtomArguments{counter.str(), std::stol(value.str())}, atom});
	} else if(predicate == "currentCounter") {
		ASP_CHECK(sym.args().size() == 2, "'currentCounter' predicate does not have arity 2");
		std::ostringstream currentCounter;
		sym.args().front().print(currentCounter);
		std::ostringstream value;
		sym.args()[1].print(value);
		allCurrentCounterAtomInfos[currentCounter.str()].emplace_back(CurrentCounterAtomInfo{CurrentCounterAtomArguments{currentCounter.str(), std::stol(value.str())}, atom});
	} else if(predicate == "currentCounterInc") {
		ASP_CHECK(sym.args().size() == 2, "'currentCounterInc' predicate does not have arity 2");
		std::ostringstream currentCounter;
		sym.args().front().print(currentCounter);
		std::ostringstream value;
		sym.args()[1].print(value);
		allCurrentCounterIncAtomInfos[currentCounter.str()].emplace_back(CurrentCounterIncAtomInfo{CurrentCounterIncAtomArguments{currentCounter.str(), std::stol(value.str())}, atom});
	} else if(predicate == "counterRem") {
		ASP_CHECK(sym.args().size() == 1, "'counterRem' predicate does not have arity 1");
		std::ostringstream argument;
		sym.args().front().print(argument);
		counterRemAtomInfos.emplace_back(CounterRemAtomInfo{{argument.str()}, atom});
	}
}

}}} // namespace solver::clasp::trees
