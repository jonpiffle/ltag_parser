# Natural Language Toolkit: Tree-Adjoining Grammar
#
# Copyright (C) 2001-2013 NLTK Project
# Author: WANG Ziqi, Haotian Zhang <{zwa47,haotianz}@sfu.ca>
#
# URL: <http://www.nltk.org/>
# For license information, see LICENSE.TXT
#

from nltk.parse.projectivedependencyparser import *
from nltk.classify.naivebayes import *

####################################
# Tree Compatibility ###############
####################################

def get_name_prefix(name):
    """
    Return the name prefix, if there is no prefix then just return itself.
    e.g. S_r --> S; NP_1 --> NP; VP --> VP
    """
    index = name.find('_')
    if index != -1:
        name = name[:index]
    return name

def check_name_equality(name_1,name_2):
    """
    Check whether the two nodes' name are the same. Notice that the name of
    a node may contain a '_XXX' suffix, so we need to get rid of this.
    """
    name_1 = get_name_prefix(name_1)
    name_2 = get_name_prefix(name_2)
    if name_1 == name_2:
        return True
    else:
        return False

def check_substitution(tree_1,tree_2):
    """
    Returns a list of substitution nodes in tree_1 in which tree_2 could be
    substituted
    """
    substitution_nodes = tree_1.get_substitution_node()
    return [sn for sn in substitution_nodes if check_name_equality(sn.label(), tree_2.label())]

def perform_substitution(tree_1, tree_2, sub_node, feature_enabled=False):
    """
    Returns tree_1 after substituting tree_2 at sub_node (which is in tree_1)
    """
    assert check_name_equality(tree_2.label(), sub_node.label()), ValueError("tree_2 and sub_node label do not match")

    #new_tree_1 = tree_1.copy(deep=True)
    #new_tree_2 = tree_2.copy(deep=True)
    new_tree_1 = tree_1
    new_tree_2 = tree_2.fast_copy()
    new_sub_node = new_tree_1.search(sub_node.label())

    if feature_enabled:
        new_top_feature = new_sub_node.get_top_feature().unify(new_tree_2.get_top_feature())
        if new_top_feature == None:
            raise ValueError("unify_failed")

        new_sub_node.set_top_feature(new_top_feature)
        new_sub_node.set_bottom_feature(new_tree_2.get_bottom_feature())

    new_sub_node.semantics = new_tree_2.semantics

    # Append all child nodes of tree_2 to the substitution node
    for c in new_tree_2:
        new_sub_node.append_new_child(c)
      
    # This node cannot be designated as a substitution node, so we need to cancel that
    new_sub_node.cancel_substitution()
    new_tree_1 = new_tree_1.correct_name()

    # Reset some caching
    new_tree_1.reset_semantics()
    new_tree_1._lex_leaves = None
    new_tree_1._preorder_traversal = None

    ### Determiner Hack ###
    new_sub_node.must_adjoin = new_tree_2.must_adjoin
    new_sub_node.can_adjoin = new_tree_2.can_adjoin
    ### End Determiner Hack ###

    return new_tree_1

def perform_deriv_based_substitution(tree_1, tree_2, deriv_depth, orig_label, feature_enabled=False):
    """
    Returns the tree_2 substituted into tree_1 with the sub_node specified by
    the original node label and its depth in the derivation tree
    """
    sub_nodes = tree_1.get_substitution_node()
    sub_nodes = [s for s in sub_nodes if s.deriv_depth == deriv_depth and s.orig_label == orig_label]
    assert len(sub_nodes) == 1
    sub_node = sub_nodes[0]
    return perform_substitution(tree_1, tree_2, sub_node, feature_enabled)

def perform_all_substitutions(tree_1, tree_2, feature_enabled=False):
    """Returns a list of all possible substitutions of tree_2 into tree_1"""
    sub_nodes = tree_1.get_substitution_node()
    sub_nodes = [sn for sn in sub_nodes if check_name_equality(sn.label(), tree_2.label())]
    results = []
    for sub_node in sub_nodes:
        new_tree = perform_substitution(tree_1, tree_2, sub_node, feature_enabled)
        results.append(new_tree)
    return results

def check_adjunction(tree_1,tree_2):
    """
    Returns a list of adj_nodes in tree_1 for which tree_2 could be adjoined
    """
    foot_2 = tree_2.get_foot_node()
    assert foot_2 is not None, ValueError("tree_2 is not an adjoin tree")
    foot_name = foot_2.label()
    foot_name_prefix = get_name_prefix(foot_name)

    adj_nodes = tree_1.prefix_search(foot_name_prefix)

    ### Determiner Hack ###
    adj_nodes = [a for a in adj_nodes if a.can_adjoin]
    ### End Determiner Hack ###

    return adj_nodes

def perform_adjunction(tree_1, tree_2, adj_node, feature_enabled=False):
    """
    Returns tree_1 after adjoining tree_2 onto adj_node (part of tree_1)
    """
    #new_tree_1 = deepcopy(tree_1)
    #new_tree_2 = deepcopy(tree_2)
    new_tree_1 = tree_1
    new_tree_2 = tree_2.fast_copy()
    foot_node = tree_2.get_foot_node()

    # Locate foot and adj nodes in copied trees
    new_foot = new_tree_2.search(foot_node.label())
    adj_node = new_tree_1.search(adj_node.label())

    # Append all child node to the foot node
    for c in adj_node:
        new_foot.append_new_child(c)

    # Delete all nodes on the node being adjoint
    adj_node.delete_all_child()

    # Then attach tree_2 in the whole to the adj_node
    for c in new_tree_2:
        adj_node.append_new_child(c)
        
    if feature_enabled:
        # Next we will change the feature structure
        # The detail is recorded in the technical report
        t = adj_node.get_top_feature()
        b = adj_node.get_bottom_feature()
        tr = new_tree_2.get_top_feature()
        br = new_tree_2.get_bottom_feature()
        tf = new_foot.get_top_feature()
        bf = new_foot.get_bottom_feature()
        adj_node.set_top_feature(t.unify(tr))
        adj_node.set_bottom_feature(br)
        new_foot.set_bottom_feature(b.unify(bf))

    # ADDED SEMANTIC FEATURE UNIFICATION
    adj_node_semantics = adj_node.semantics
    tree_2_semantics = new_tree_2.semantics
    adj_node.semantics = tree_2_semantics
    new_foot.semantics = adj_node_semantics

    # Don't forget to let the foot node become a normal node
    new_foot.cancel_adjunction()

    new_tree_1 = new_tree_1.correct_name()

    # Reset some caching
    new_tree_1.reset_semantics()
    new_tree_1._lex_leaves = None
    new_tree_1._preorder_traversal = None

    ### Determiner Hack ###
    if new_tree_2.tree_name == "\x03Dnx" and adj_node.must_adjoin:
        adj_node.must_adjoin = False
        new_tree_1.can_adjoin = False
    ### End Determiner Hack ###

    return new_tree_1

def perform_deriv_based_adjunction(tree_1, tree_2, deriv_depth, orig_label, feature_enabled=False):
    """
    Returns tree_2 adjoined onto tree_1 with the adj_node specified by the
    original node label and its depth in the derivation tree
    """
    foot_node = tree_2.get_foot_node() 
    foot_name_prefix = get_name_prefix(foot_node.label())
    adj_nodes = tree_1.prefix_search(foot_name_prefix)
    adj_nodes = [a for a in adj_nodes if a.deriv_depth == deriv_depth and a.orig_label == orig_label]
    assert len(adj_nodes) == 1
    adj_node = adj_nodes[0]
    return perform_adjunction(tree_1, tree_2, adj_node, feature_enabled)

def perform_all_adjunctions(tree_1, tree_2, feature_enabled=False):
    """Returns a list of all possible adjunctions of tree_2 onto tree_1"""
    foot_node = tree_2.get_foot_node() 
    foot_name_prefix = get_name_prefix(foot_node.label())
    adj_nodes = tree_1.prefix_search(foot_name_prefix)
    results = []
    for adj_node in adj_nodes:
        new_tree = perform_adjunction(tree_1, tree_2, adj_node, feature_enabled)
        results.append(new_tree)
    return results
