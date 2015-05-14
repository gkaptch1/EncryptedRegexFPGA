#!/usr/local/bin/

# 
# Creating a graph from a dot file, including all type and value information.
# ---------------------------------------------------------------------------
# Iterates over all lines in the graph twice, first to look for vertices and
# then to make connections.
#
# There's definitely a better way to do this, I just can't figure it out right
# now. The problem is that the vertices appear in depth first order and for  
# each vertex the edges/ connections appear in breadth first order
#

from graph_tool.all import *

# Create a new graph object
g = Graph()

# Creating empty maps for vertex and graph properties
vprop_node = g.new_vertex_property("string")	# Dot file node name
vprop_label = g.new_vertex_property("string")	# Clang AST labels
vprop_type = g.new_vertex_property("string")	# Type if variable/ function
vprop_name = g.new_vertex_property("string")	# Name if variable/ function
vprop_value = g.new_vertex_property("string")	# Value if operator

gprop_name = g.new_graph_property("string")		# Dot file - digraph name


def get_lvalue(tkn):
	idx = tkn.find("=")
	if(idx == -1):
		return ""
	else:
		return tkn[:idx].strip(";][}{\",") # Strip the rest

def get_rvalue(tkn):
	tkn = tkn.strip(";][}{\",") # Strip away brackets
	idx = tkn.find("=")
	if(idx == -1):
		return ""
	else:
		return tkn[idx+1:].strip(";][}{\",") # Strip the rest

def store_properties(v, prop, val):
	if(prop == "label"):
		vprop_label[v] = val
	elif(prop == "name"):
		vprop_name[v] = val
	elif(prop == "value"):
		vprop_value[v] = val
	elif(prop == "type"):
		vprop_type[v] = val


# To make the vertex reading more robust, consider doing the following:
#   1) Strip all white-space
#   2) Replace all commas and square brackets with spaces
#   3) Strip semicolon
#   4) Split by white-space
#   5) Store property values
#
#   0)Node0x7feb52000fd8 [shape=record , label="{CompoundStmt}"];
#   1)Node0x7feb52000fd8[shape=record,label="{CompoundStmt}"];
#   2)Node0x7feb52000fd8 shape=record label="{CompoundStmt}" ;
#   3)Node0x7feb52000fd8 shape=record label="{CompoundStmt}"
#   4)shape=record		-	label="{CompoundStmt}"
#   5)shape : record	-	label : CompoundStmt

# Open the dot file, iterate over lines, save vertices
with open("../opencl/matrixmult.dot") as f:
	for line in f:
		tkns = line.split()
		if(len(tkns) < 2):
			print "blank line" # TODO do nothing in if: block
		# Name the graph
		elif(tkns[0] == 'digraph'):
			gprop_name[g] = tkns[1]
		# If we come across a new node, add it and its properties
		elif(tkns[1] != "->"):
			v = g.add_vertex()
			vprop_node[v] = tkns[0]
			print vprop_node[v]
			for tkn in tkns[1:]:
				prop = get_lvalue(tkn)
				val  = get_rvalue(tkn)
				if(prop):
					store_properties(v, prop, val)

# Open the dot file, iterate over lines, create edges
with open("../opencl/matrixmult.dot") as f:
	for line in f:
		tkns = line.split()
		if(len(tkns) == 3 and tkns[1] == "->"):
			print tkns[0] + "     " + tkns[2]
			v1 = find_vertex(g, vprop_node, tkns[0])[0]
			print v1
			v2 = find_vertex(g, vprop_node, tkns[2].strip(';'))[0]
			print v2
			g.add_edge(v1, v2)

# Print the graph's vertices


# Draw the graph with some vertex appearance options
graph_draw(g, vertex_size=15, vertex_text=vprop_node, vertex_font_size=14,
		output_size=(1000, 1000), output="dot-file.png")













