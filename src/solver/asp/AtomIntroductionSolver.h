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

#pragma once
//}}}
#include <unordered_set>
#include <unordered_map>

#include "SolverBase.h"
#include "../../Hypergraph.h"

namespace solver { namespace asp {

class AtomIntroductionSolver : public SolverBase
{
public:
	AtomIntroductionSolver(const Decomposition& decomposition, const Application& app, const Hypergraph::Vertex& introducedAtom);

protected:
	virtual ItemTreePtr compute() override;

private:
	const Hypergraph::Vertex introducedAtom;
};

}} // namespace solver::asp
