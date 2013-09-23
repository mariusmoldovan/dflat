/*
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

#pragma once

#include "Application.h"
#include "Decomposition.h"
#include "Module.h"

class Hypergraph;

class Decomposer : public Module
{
public:
	Decomposer(Application& app, const std::string& optionName, const std::string& optionDescription, bool newDefault = false);

	virtual Decomposition decompose(const Hypergraph& instance) const = 0;

	virtual void select();
};