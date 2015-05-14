#!/usr/local/bin/

# Creating a graph from a dot file, including all type and value information.
# Because dot files are constructed depth first, this module uses depth-first 
# search logic to minimize the information that it needs to store

from graph_tool.all import *

# Create a new graph object
g = Graph()

# Creating empty maps for vertex and graph properties
vprop_name = g.new_vertex_property("string")
vprop_type = g.new_vertex_property("string")

gprop_name = g.new_graph_property("string")

# Open the dot file, iterate over lines, save vertices
with open("../f.dot") as f:
	for line in f:
		#print line
		tkns = line.split()
		if(len(tkns) < 2):
			print "blank line" # TODO do nothing in if: block
		# Name the graph
		elif(tkns[0] == 'digraph'):
			gprop_name[g] = tkns[1]
		# If we come across a new node, add it and its properties
		elif(tkns[1] != "->"):
			v = g.add_vertex()
			vprop_name[v] = tkns[0][15:]
			vprop_type[v] = tkns[1][15:]

# Open the dot file, iterate over lines, create edges
with open("../f.dot") as f:
	for line in f:
		tkns = line.split()
		if(len(tkns) == 3 and tkns[1] == "->"):
			v1 = find_vertex(g, vprop_name, tkns[0][15:])[0]
			v2 = find_vertex(g, vprop_name, tkns[2].strip(';')[15:])[0]
			g.add_edge(v1, v2)

# Draw the graph with some vertex appearance options
graph_draw(g, vertex_size=15, vertex_text=vprop_name, vertex_font_size=14,
		output_size=(1000, 1000), output="dot-file.png")













