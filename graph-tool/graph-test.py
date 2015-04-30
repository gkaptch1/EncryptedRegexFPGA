#!/usr/local/bin/

from graph_tool.all import *

g = Graph()
assert(g.is_directed())

v1 = g.add_vertex()
v2 = g.add_vertex()
v3 = g.add_vertex()

e1 = g.add_edge(v1,v2)
e2 = g.add_edge(v1,v3)

graph_draw(g, vertex_text=g.vertex_index, vertex_font_size=18,
		output_size=(200, 200), output="two-nodes.png")
