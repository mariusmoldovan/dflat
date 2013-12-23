/*{{{
Copyright 2012-2013, Bernhard Bliem
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
// Originally from gringo. A few modifications were made.
// Copyright (c) 2010, Roland Kaminski <kaminski@cs.uni-potsdam.de>
//
// This file is part of gringo.
//
// gringo is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// gringo is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with gringo.  If not, see <http://www.gnu.org/licenses/>.
//}}}
//#include <clasp/program_builder.h>
//#include <clasp/shared_context.h>
//#include <gringo/storage.h>
//#include <gringo/domain.h>

#include "GringoOutputProcessor.h"

namespace solver { namespace asp {

GringoOutputProcessor::GringoOutputProcessor(Clasp::Asp::LogicProgram& out)
	: prg_(out)
{
	false_ = prg_.newAtom();
	prg_.setCompute(false_, false);
#ifdef DEBUG_OUTPUT
	std::cerr << "pb.setCompute(" << false_ << ",false);\n";
#endif
}

void GringoOutputProcessor::addBody(const LitVec& body) {
    for (auto x : body) {
        prg_.addToBody((Clasp::Var)std::abs(x), x > 0);
#ifdef DEBUG_OUTPUT
        std::cerr << ".addToBody(" << std::abs(x) << "," << (x > 0) << ")";
#endif
    }
}
void GringoOutputProcessor::addBody(const LitWeightVec& body) {
    for (auto x : body) {
        prg_.addToBody((Clasp::Var)std::abs(x.first), x.first > 0, x.second);
#ifdef DEBUG_OUTPUT
        std::cerr << ".addToBody(" << std::abs(x.first) << "," << (x.first > 0) << "," << x.second << ")";
#endif
    }
}
void GringoOutputProcessor::printBasicRule(unsigned head, LitVec const &body) {
#ifdef DEBUG_OUTPUT
    std::cerr << "pb.startRule().addHead(" << head << ")";
#endif
    prg_.startRule().addHead(head);
    addBody(body);
    prg_.endRule();
#ifdef DEBUG_OUTPUT
    std::cerr << ".endRule();\n";
#endif
}

void GringoOutputProcessor::printChoiceRule(AtomVec const &atoms, LitVec const &body) {
#ifdef DEBUG_OUTPUT
    std::cerr << "pb.startRule(Clasp::Asp::CHOICERULE)";
    for (auto x : atoms) { std::cerr << ".addHead(" << x << ")"; }
#endif
    prg_.startRule(Clasp::Asp::CHOICERULE);
    for (auto x : atoms) { prg_.addHead(x); }
    addBody(body);
    prg_.endRule();
#ifdef DEBUG_OUTPUT
    std::cerr << ".endRule();\n";
#endif
}

void GringoOutputProcessor::printCardinalityRule(unsigned head, unsigned lower, LitVec const &body) {
#ifdef DEBUG_OUTPUT
    std::cerr << "pb.startRule(Clasp::Asp::CONSTRAINTRULE, " << lower << ").addHead(" << head << ")";
#endif
    prg_.startRule(Clasp::Asp::CONSTRAINTRULE, lower).addHead(head);
    addBody(body);
    prg_.endRule();
#ifdef DEBUG_OUTPUT
    std::cerr << ".endRule();\n";
#endif
}

void GringoOutputProcessor::printWeightRule(unsigned head, unsigned lower, LitWeightVec const &body) {
#ifdef DEBUG_OUTPUT
    std::cerr << "pb.startRule(Clasp::Asp::WEIGHTRULE, " << lower << ").addHead(" << head << ")";
#endif
    prg_.startRule(Clasp::Asp::WEIGHTRULE, lower).addHead(head);
    addBody(body);
    prg_.endRule();
#ifdef DEBUG_OUTPUT
    std::cerr << ".endRule();\n";
#endif
}

void GringoOutputProcessor::printMinimize(LitWeightVec const &body) {
#ifdef DEBUG_OUTPUT
    std::cerr << "pb.startRule(Clasp::Asp::OPTIMIZERULE)";
#endif
    prg_.startRule(Clasp::Asp::OPTIMIZERULE);
    addBody(body);
    prg_.endRule();
#ifdef DEBUG_OUTPUT
    std::cerr << ".endRule();\n";
#endif
}

void GringoOutputProcessor::printDisjunctiveRule(AtomVec const &atoms, LitVec const &body) {
#ifdef DEBUG_OUTPUT
    std::cerr << "pb.startRule(Clasp::Asp::DISJUNCTIVERULE)";
    for (auto x : atoms) { std::cerr << ".addHead(" << x << ")"; }
#endif
    prg_.startRule(Clasp::Asp::DISJUNCTIVERULE);
    for (auto x : atoms) { prg_.addHead(x); }
    addBody(body);
    prg_.endRule();
#ifdef DEBUG_OUTPUT
    std::cerr << ".endRule();\n";
#endif
}

void GringoOutputProcessor::printSymbol(unsigned atomUid, Gringo::Value v) {
	if (v.type() == Gringo::Value::ID || v.type() == Gringo::Value::STRING) {
		prg_.setAtomName(atomUid, (*v.string()).c_str());
	}
	else {
		str_.str("");
		v.print(str_);
		prg_.setAtomName(atomUid, str_.str().c_str());
	}

#ifdef DEBUG_OUTPUT
    std::cerr << "pb.setAtomName(" << atomUid << ",\"" << v << "\");\n";
#endif
}

void GringoOutputProcessor::printExternal(unsigned atomUid) {
    prg_.freeze(atomUid);
#ifdef DEBUG_OUTPUT
    std::cerr << "pb.freeze(" << atomUid << ");\n";
#endif
}

bool &GringoOutputProcessor::disposeMinimize() {
    return disposeMinimize_;
}

/*
GringoOutputProcessor::GringoOutputProcessor(const ChildItemTrees& childItemTrees)
	: LparseConverter(0, false)
	, childItemTrees(childItemTrees)
	, b_(0)
	, lastUnnamed_(0)
{
}

void GringoOutputProcessor::initialize()
{
	LparseConverter::initialize();
	b_->setCompute(false_, false);
	lastUnnamed_ = atomUnnamed_.size();
	atomUnnamed_.clear();
}

std::vector<std::string> GringoOutputProcessor::getArguments(ValVec::const_iterator start, uint32_t arity) const
{
	std::vector<std::string> arguments;
	arguments.reserve(arity);
	const ValVec::const_iterator end = start + arity;
	while(start != end)
	{
		std::stringstream ss;
		start->print(s_, ss);
		arguments.emplace_back(ss.str());
		++start;
	}
	return arguments;
}

void GringoOutputProcessor::printBasicRule(int head, const AtomVec &pos, const AtomVec &neg)
{
	b_->startRule();
	b_->addHead(head);
	foreach(AtomVec::value_type atom, neg) { b_->addToBody(atom, false); }
	foreach(AtomVec::value_type atom, pos) { b_->addToBody(atom, true); }
	b_->endRule();
}

void GringoOutputProcessor::printConstraintRule(int head, int bound, const AtomVec &pos, const AtomVec &neg)
{
	b_->startRule(Clasp::CONSTRAINTRULE, bound);
	b_->addHead(head);
	foreach(AtomVec::value_type atom, neg) { b_->addToBody(atom, false); }
	foreach(AtomVec::value_type atom, pos) { b_->addToBody(atom, true); }
	b_->endRule();
}

void GringoOutputProcessor::printChoiceRule(const AtomVec &head, const AtomVec &pos, const AtomVec &neg)
{
	b_->startRule(Clasp::CHOICERULE);
	foreach(AtomVec::value_type atom, head) { b_->addHead(atom); }
	foreach(AtomVec::value_type atom, neg) { b_->addToBody(atom, false); }
	foreach(AtomVec::value_type atom, pos) { b_->addToBody(atom, true); }
	b_->endRule();
}

void GringoOutputProcessor::printWeightRule(int head, int bound, const AtomVec &pos, const AtomVec &neg, const WeightVec &wPos, const WeightVec &wNeg)
{
	b_->startRule(Clasp::WEIGHTRULE, bound);
	b_->addHead(head);
	WeightVec::const_iterator itW = wNeg.begin();
	for(AtomVec::const_iterator it = neg.begin(); it != neg.end(); it++, itW++)
		b_->addToBody(*it, false, *itW);
	itW = wPos.begin();
	for(AtomVec::const_iterator it = pos.begin(); it != pos.end(); it++, itW++)
		b_->addToBody(*it, true, *itW);
	b_->endRule();
}

void GringoOutputProcessor::printMinimizeRule(const AtomVec &pos, const AtomVec &neg, const WeightVec &wPos, const WeightVec &wNeg)
{
	b_->startRule(Clasp::OPTIMIZERULE);
	WeightVec::const_iterator itW = wNeg.begin();
	for(AtomVec::const_iterator it = neg.begin(); it != neg.end(); it++, itW++)
		b_->addToBody(*it, false, *itW);
	itW = wPos.begin();
	for(AtomVec::const_iterator it = pos.begin(); it != pos.end(); it++, itW++)
		b_->addToBody(*it, true, *itW);
	b_->endRule();
}

void GringoOutputProcessor::printDisjunctiveRule(const AtomVec &head, const AtomVec &pos, const AtomVec &neg)
{
	(void)head;
	(void)pos;
	(void)neg;
	throw std::runtime_error("Error: clasp cannot handle disjunctive rules use option --shift!");
}

void GringoOutputProcessor::printComputeRule(int models, const AtomVec &pos, const AtomVec &neg)
{
	(void)models;
	foreach(AtomVec::value_type atom, neg) { b_->setCompute(atom, false); }
	foreach(AtomVec::value_type atom, pos) { b_->setCompute(atom, true); }
}

void GringoOutputProcessor::printSymbolTableEntry(const AtomRef &atom, uint32_t arity, const std::string &name)
{
	// Set human-readable atom names
	std::stringstream ss;
	ss << name;
	if(arity > 0)
	{
		ValVec::const_iterator k = vals_.begin() + atom.second;
		const ValVec::const_iterator end = k + arity;
		char separator = '(';
		do
		{
			ss << separator;
			separator = ',';
			k->print(s_, ss);
		} while(++k != end);
		ss << ')';
	}
	b_->setAtomName(atom.first, ss.str().c_str());
	atomUnnamed_[atom.first - lastUnnamed_] = false;

	storeAtom(name, vals_.begin() + atom.second, arity, atom.first);
}

void GringoOutputProcessor::printExternalTableEntry(const AtomRef &atom, uint32_t arity, const std::string &name)
{
	(void)atom;
	(void)arity;
	(void)name;
}

uint32_t GringoOutputProcessor::symbol()
{
	uint32_t atom = b_->newAtom();
	atomUnnamed_.resize(atom + 1 - lastUnnamed_, true);
	return atom;
}

void GringoOutputProcessor::doFinalize()
{
	printSymbolTable();
	for(uint32_t i = 0; i < atomUnnamed_.size(); i++) { if(atomUnnamed_[i]) { b_->setAtomName(i + lastUnnamed_, 0); } }
	lastUnnamed_+= atomUnnamed_.size();
	atomUnnamed_.clear();
}

const LparseConverter::SymbolMap &GringoOutputProcessor::symbolMap(uint32_t domId) const
{
	return symTab_[domId];
}

ValRng GringoOutputProcessor::vals(Domain *dom, uint32_t offset) const
{
	return ValRng(vals_.begin() + offset, vals_.begin() + offset + dom->arity());
}
*/

}} // namespace solver::asp
