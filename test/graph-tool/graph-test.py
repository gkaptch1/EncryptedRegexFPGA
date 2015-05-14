#!/usr/local/bin/

from graph_tool.all import *
import random

g = Graph()

#---------------------------------------------------
def makeChildren(apex, nbr_range):
	if nbr_range != 0:
		num_nbrs = random.randint(0, nbr_range)
		for i in xrange(0, num_nbrs):
			v = g.add_vertex()
			g.add_edge(apex, v)
			makeChildren(v, nbr_range - 1)
#---------------------------------------------------

assert(g.is_directed())

# Add root node
root = g.add_vertex()

# Create child nodes recursively
makeChildren(root, 6)

# Draw the graph with some vertex appearance options
graph_draw(g, vertex_size=25, vertex_text=g.vertex_index, vertex_font_size=14,
		output_size=(1000, 1000), output="two-nodes.png")