/*{{{
Copyright 2012-2016, Bernhard Bliem
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
#include "GringoOutputProcessor.h"

namespace asp_utils {

void GringoOutputProcessor::initProgram(bool) { }

void GringoOutputProcessor::endStep() { }

void GringoOutputProcessor::beginStep() { }

void GringoOutputProcessor::rule(Potassco::Head_t ht, const Potassco::AtomSpan& head, const Potassco::LitSpan& body) {
    if (auto p = prg()) { p->addRule(ht, head, body); }
}

void GringoOutputProcessor::rule(Potassco::Head_t ht, const Potassco::AtomSpan& head, Potassco::Weight_t bound, const Potassco::WeightLitSpan& body) {
    if (auto p = prg()) { p->addRule(ht, head, bound, body); }
}

void GringoOutputProcessor::minimize(Potassco::Weight_t prio, const Potassco::WeightLitSpan& lits) {
    if (auto p = prg()) { p->addMinimize(prio, lits); }
}

void GringoOutputProcessor::project(const Potassco::AtomSpan& atoms) {
    if (auto p = prg()) { p->addProject(atoms); }
}

void GringoOutputProcessor::output(Gringo::Symbol sym, Potassco::Atom_t atom) {
    std::ostringstream out;
    out << sym;
    if (atom != 0) {
        Potassco::Lit_t lit = atom;
        if (auto p = prg()) { p->addOutput(Potassco::toSpan(out.str().c_str()), Potassco::LitSpan{&lit, 1}); }
    }
    else {
        if (auto p = prg()) { p->addOutput(Potassco::toSpan(out.str().c_str()), Potassco::LitSpan{nullptr, 0}); }
    }
    storeAtom(atom, sym);
}

void GringoOutputProcessor::output(Gringo::Symbol sym, Potassco::LitSpan const& condition) {
    std::ostringstream out;
    out << sym;
    if (auto p = prg()) { p->addOutput(Potassco::toSpan(out.str().c_str()), condition); }
}

void GringoOutputProcessor::output(Gringo::Symbol sym, int value, Potassco::LitSpan const& condition) {
    std::ostringstream out;
    out << sym << "=" << value;
    if (auto p = prg()) { p->addOutput(Potassco::toSpan(out.str().c_str()), condition); }
}

void GringoOutputProcessor::acycEdge(int s, int t, const Potassco::LitSpan& condition) {
    if (auto p = prg()) { p->addAcycEdge(s, t, condition); }
}

void GringoOutputProcessor::heuristic(Potassco::Atom_t a, Potassco::Heuristic_t t, int bias, unsigned prio, const Potassco::LitSpan& condition) {
    if (auto p = prg()) { p->addDomHeuristic(a, t, bias, prio, condition); }
}

void GringoOutputProcessor::assume(const Potassco::LitSpan& lits) {
    if (auto p = prg()) { p->addAssumption(lits); }
}

void GringoOutputProcessor::external(Potassco::Atom_t a, Potassco::Value_t v) {
    if (auto p = prg()) {
        switch (v) {
            case Potassco::Value_t::False:   { p->freeze(a, Clasp::value_false); break; }
            case Potassco::Value_t::True:    { p->freeze(a, Clasp::value_true); break; }
            case Potassco::Value_t::Free:    { p->freeze(a, Clasp::value_free); break; }
            case Potassco::Value_t::Release: { p->unfreeze(a); break; }
        }
    }
}

void GringoOutputProcessor::theoryTerm(Potassco::Id_t, int) { }

void GringoOutputProcessor::theoryTerm(Potassco::Id_t, const Potassco::StringSpan&) { }

void GringoOutputProcessor::theoryTerm(Potassco::Id_t, int, const Potassco::IdSpan&) { }

void GringoOutputProcessor::theoryElement(Potassco::Id_t e, const Potassco::IdSpan&, const Potassco::LitSpan& cond) {
    if (auto p = prg()) {
        Potassco::TheoryElement const &elem = p->theoryData().getElement(e);
        if (elem.condition() == Potassco::TheoryData::COND_DEFERRED) { p->theoryData().setCondition(e, p->newCondition(cond)); }
    }
}

void GringoOutputProcessor::theoryAtom(Potassco::Id_t, Potassco::Id_t, const Potassco::IdSpan&) { }

void GringoOutputProcessor::theoryAtom(Potassco::Id_t, Potassco::Id_t, const Potassco::IdSpan&, Potassco::Id_t, Potassco::Id_t){ }

Clasp::Asp::LogicProgram *GringoOutputProcessor::prg() {
    return ctl_.update() ? static_cast<Clasp::Asp::LogicProgram*>(ctl_.clasp_->program()) : nullptr;
}

GringoOutputProcessor::~GringoOutputProcessor() noexcept = default;

} // namespace asp_utils
