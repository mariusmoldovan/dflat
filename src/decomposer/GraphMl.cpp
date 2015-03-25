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
#include <sstream>
#include <stack>

#include "GraphMl.h"
#include "../Decomposition.h"
#include "../Application.h"
#include "../Printer.h"
#include "../tinyxml2.h"

namespace {
	void error(const char* msg)
	{
		std::ostringstream str;
		str << "Error reading XML document (" << msg << ')';
		throw std::runtime_error(str.str());
	}

	Hypergraph::Vertices bagOfNode(const tinyxml2::XMLElement* node)
	{
		assert(node);
		const tinyxml2::XMLElement* data = node->FirstChildElement("data");
		if(!data)
			error("Node without data element");

		Hypergraph::Vertices bag;
		if(data->GetText()) {
			std::stringstream ss(data->GetText());
			std::string item;
			while(std::getline(ss, item, ',')) {
				if(item.at(0) == ' ')
					item.erase(0,1);
				bag.insert({std::move(item)});
			}
		}
		return bag;
	}
}

namespace decomposer {

const std::string GraphMl::OPTION_SECTION = "GraphML decomposition reader";

GraphMl::GraphMl(Application& app, bool newDefault)
	: Decomposer(app, "graphml", "Use decomposition specified in GraphML file", newDefault)
	, optFile("graphml-in", "file", "Read decomposition in GraphML format from <file>")
{
	optFile.addCondition(selected);
	app.getOptionHandler().addOption(optFile, OPTION_SECTION);
}

void GraphMl::select()
{
	::Decomposer::select();
	if(!optFile.isUsed())
		throw std::runtime_error("GraphML decomposition reader requires a file to be specified");
}

DecompositionPtr GraphMl::decompose(const Hypergraph& instance) const
{
	using namespace tinyxml2;
	XMLDocument doc;
	if(doc.LoadFile(optFile.getValue().c_str()))
		error(doc.ErrorName());

	XMLElement* graphml = doc.RootElement();
	if(!graphml)
		error("No root element");

	XMLElement* graph = graphml->FirstChildElement("graph");
	if(!graph)
		error("No graph element");

	// XXX We assume the GraphML file has nodes and edges ordered as in a post-order traversal
	std::stack<DecompositionPtr> nodes;
	XMLElement* rootNode = graph->FirstChildElement("node");

	nodes.emplace(new Decomposition(bagOfNode(rootNode), app.getSolverFactory()));

	const XMLElement* element = rootNode;
	while((element = element->NextSiblingElement()) != nullptr) {
		if(strcmp(element->Name(), "node") == 0)
			nodes.emplace(new Decomposition(bagOfNode(element), app.getSolverFactory()));
		else if(strcmp(element->Name(), "edge") == 0) {
			if(nodes.size() < 2)
				error("Not enough nodes declared before edges");
			DecompositionPtr to = nodes.top();
			nodes.pop();
			const DecompositionPtr& from = nodes.top();
			from->addChild(std::move(to));
		}
		else
			error("Unexpected element");
	}
	if(nodes.size() != 1)
		error("Not all non-root nodes connected to parents");
	return nodes.top();
}

} // namespace decomposer
