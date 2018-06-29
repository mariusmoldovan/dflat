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

// Taken from gringo (GPL), made some modifications.
#pragma once
//}}}
#include <gringo/backend.hh>
#include <clasp/literal.h>
#include <clasp/logic_program.h>
#include <unordered_map>
#include <clingo/clingocontrol.hh>
#include <gringo/symbol.hh>

#include "../ItemTree.h"

namespace asp_utils {

class GringoOutputProcessor : public Gringo::Backend
{
public:
	GringoOutputProcessor(Gringo::ClingoControl& ctl) : ctl_(ctl) { }
	GringoOutputProcessor(const GringoOutputProcessor&) = delete;
	GringoOutputProcessor& operator=(const GringoOutputProcessor&) = delete;
	void initProgram(bool incremental) override;
	void beginStep() override;
	void rule(Potassco::Head_t ht, const Potassco::AtomSpan& head, const Potassco::LitSpan& body) override;
	void rule(Potassco::Head_t ht, const Potassco::AtomSpan& head, Potassco::Weight_t bound, const Potassco::WeightLitSpan& body) override;
	void minimize(Potassco::Weight_t prio, const Potassco::WeightLitSpan& lits) override;
	void project(const Potassco::AtomSpan& atoms) override;
	void output(Gringo::Symbol sym, Potassco::Atom_t atom) override;
	void output(Gringo::Symbol sym, Potassco::LitSpan const& condition) override;
	void output(Gringo::Symbol sym, int value, Potassco::LitSpan const& condition) override;
	void external(Potassco::Atom_t a, Potassco::Value_t v) override;
	void assume(const Potassco::LitSpan& lits) override;
	void heuristic(Potassco::Atom_t a, Potassco::Heuristic_t t, int bias, unsigned prio, const Potassco::LitSpan& condition) override;
	void acycEdge(int s, int t, const Potassco::LitSpan& condition) override;
	void theoryTerm(Potassco::Id_t termId, int number) override;
	void theoryTerm(Potassco::Id_t termId, const Potassco::StringSpan& name) override;
	void theoryTerm(Potassco::Id_t termId, int cId, const Potassco::IdSpan& args) override;
	void theoryElement(Potassco::Id_t elementId, const Potassco::IdSpan& terms, const Potassco::LitSpan& cond) override;
	void theoryAtom(Potassco::Id_t atomOrZero, Potassco::Id_t termId, const Potassco::IdSpan& elements) override;
	void theoryAtom(Potassco::Id_t atomOrZero, Potassco::Id_t termId, const Potassco::IdSpan& elements, Potassco::Id_t op, Potassco::Id_t rhs) override;
	void endStep() override;
	~GringoOutputProcessor() noexcept override;

private:
	Clasp::Asp::LogicProgram *prg();
	Gringo::ClingoControl& ctl_;

protected:
	virtual void storeAtom(uint32_t atom, Gringo::Symbol sym) = 0;
};

} // namespace asp_utils
