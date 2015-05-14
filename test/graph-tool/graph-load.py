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

# TODO use internal property maps?
# Creating empty maps for vertex and graph properties
vprop_node = g.new_vertex_property("string")	# Dot file node name
vprop_label = g.new_vertex_property("string")	# Clang AST labels
vprop_type = g.new_vertex_property("string")	# Type if variable/ function
vprop_name = g.new_vertex_property("string")	# Name if variable/ function
vprop_value = g.new_vertex_property("string")	# Value if operator

gprop_name = g.new_graph_property("string")		# Dot file - digraph name

# Some helper functions ----------------------------------------------------
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
# End helper functions -----------------------------------------------------

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

# TODO - get file from arguments
# Open the dot file, iterate over lines, save vertices
with open("../opencl/matrixmult.dot") as f:
	for line in f:
		tkns = line.split()
		if(len(tkns) < 2):
			pass
		# Name the graph
		elif(tkns[0] == 'digraph'):
			gprop_name[g] = tkns[1]
		# If we come across a new node, add it and its properties
		elif(tkns[1] != "->"):
			v = g.add_vertex()
			vprop_node[v] = tkns[0]
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
			v1 = find_vertex(g, vprop_node, tkns[0])[0]
			v2 = find_vertex(g, vprop_node, tkns[2].strip(';'))[0]
			g.add_edge(v1, v2)

# Draw the original graph with some vertex appearance options
graph_draw(g, vertex_size=15, vertex_text=vprop_node, vertex_font_size=14,
		output_size=(1000, 1000), output="dot-file.png")

#-----------------------------------------------------------------------------

# Replace function calls with variables using subgraph_isomorphism()
sub = Graph()
subprop_label = sub.new_vertex_property("string")

# The function call expression
v0 = sub.add_vertex()
subprop_label[v0] = "CallExpr"
# And its children...
# ... the name of the function
v1 = sub.add_vertex()
subprop_label[v1] = "DeclRefExpr"
sub.add_edge(v0, v1)
# ... and the argument (?)
v2 = sub.add_vertex()
subprop_label[v2] = "IntegerLiteral"
sub.add_edge(v0,v2)

# Get property map of isomorphism
iso = subgraph_isomorphism(sub, g, vertex_label=[subprop_label, vprop_label])

# Set different vertex properties for the isomorphism only
# TODO - replace this with code that turns function calls into variables
for i in range(len(iso)):
	g.set_vertex_filter(None)
	g.set_edge_filter(None)
	vmask, emask = mark_subgraph(g, sub, iso[i])
	g.set_vertex_filter(vmask)
	g.set_edge_filter(emask)
g.set_vertex_filter(None)
g.set_edge_filter(None)

# Draw the subgraph with some vertex appearance options
graph_draw(sub, vertex_size=15, vertex_text=subprop_label, vertex_font_size=14,
		output_size=(1000, 1000), output="subgraph-isomorphism.png")

# Draw the graph with the subgraph embedded
graph_draw(g, vertex_size=15, vertex_text=vprop_node, vertex_font_size=14,
		vertex_fill_color=vmask, edge_color=emask,
		output_size=(1000, 1000), output="iso-embed.png")
