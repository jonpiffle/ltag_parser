#!/usr/bin/env python

"""
Calculates parsing evaluation metrics: precision, recall, labeled precision and
labeled recall.
"""

import copy
from nltk.tree import Tree

def precision(gold, parse, ignore_labels=True):
    """Return the proportion of brackets in the suggested parse tree that are
    in the gold standard. Parameters gold and parse are NLTK Tree objects."""
    suc,total = precision_half(gold, parse, ignore_labels)
    return float(suc)/total

def precision_half(gold,parse,ignore_labels=True):
    parsebrackets = list_brackets(parse)
    goldbrackets = list_brackets(gold)

    parsebrackets_u = list_brackets(parse, ignore_labels=ignore_labels)
    goldbrackets_u = list_brackets(gold, ignore_labels=ignore_labels)

    if ignore_labels:
        candidate = parsebrackets_u
        gold = goldbrackets_u
    else:
        candidate = parsebrackets
        gold = goldbrackets

    total = len(candidate)
    successes = 0
    for bracket in candidate:
        if bracket in gold:
            successes += 1
    return (successes,total)

def recall(gold, parse, ignore_labels=True):
    """Return the proportion of brackets in the gold standard that are in the
    suggested parse tree."""
    suc,total = recall_half(gold, parse, ignore_labels)
    return float(suc)/total

def recall_half(gold, parse, ignore_labels=True):
    parsebrackets = list_brackets(parse)
    goldbrackets = list_brackets(gold)

    parsebrackets_u = list_brackets(parse, ignore_labels=ignore_labels)
    goldbrackets_u = list_brackets(gold, ignore_labels=ignore_labels)

    if ignore_labels:
        candidate = parsebrackets_u
        gold = goldbrackets_u
    else:
        candidate = parsebrackets
        gold = goldbrackets

    total = len(gold)
    successes = 0
    for bracket in gold:
        if bracket in candidate:
            successes += 1
    return (successes,total)

def parseval(gold_tree,parse_tree):
    pcs_suc,pcs_tol = precision_half(gold_tree,parse_tree)
    rcl_suc,rcl_tol = recall_half(gold_tree,parse_tree)
    pcs = float(pcs_suc)/(pcs_tol)
    rcl = float(rcl_suc)/(rcl_tol)
    if pcs + rcl == 0:
        f_score = 0
    else:
        f_score = 2*pcs*rcl/(pcs+rcl)
    return f_score
        
def labeled_precision(gold, parse):
    return precision(gold, parse, ignore_labels=False)

def labeled_recall(gold, parse):
    return recall(gold, parse, ignore_labels=False)

def words_to_indexes(tree):
    """Return a new tree based on the original tree, such that the leaf values
    are replaced by their indexs."""

    out = copy.deepcopy(tree)
    leaves = out.leaves()
    for index in range(0, len(leaves)):
        path = out.leaf_treeposition(index)
        out[path] = index + 1
    return out

def firstleaf(tr):
    return tr.leaves()[0]

def lastleaf(tr):
    return tr.leaves()[-1]

def list_brackets(tree, ignore_labels=False):
    tree = words_to_indexes(tree)

    def not_pos_tag(tr):
        return tr.height() > 2

    def label(tr):
        if ignore_labels:
            return "ignore"
        else:
            return tr.label()

    out = []
    subtrees = tree.subtrees(filter=not_pos_tag)
    return [(firstleaf(sub), lastleaf(sub), label(sub)) for sub in subtrees]

def test():
    gold_tree = Tree.fromstring("(S (NP (NP (NNP Pierre) (NNP Vinken)) (, ,) (ADJP (NP (CD 61) (NNS years)) (JJ old)) (, ,)) (VP (MD will) (VP (VB join) (NP (DT the) (NN board)) (PP (IN as) (NP (DT a) (JJ nonexecutive) (NN director))) (NP (NNP Nov.) (CD 29)))) )")
    gold_tree.draw()
    with open("treebank_parsetrees.txt", "r") as f:
        parse_trees = [Tree.fromstring(line) for line in f.readlines()]
    best, best_val = None, 0
    for test_tree in parse_trees:
        val = parseval(gold_tree, test_tree)
        if val > best_val:
            best, best_val = test_tree, val
            print(val)
            print(test_tree)
    best.draw()

def main():
    test_file = "parseval_test.txt"
    gold_file = "parseval_gold.txt"
    test_tree = Tree.fromstring(open(test_file, 'r').read())
    gold_tree = Tree.fromstring(open(gold_file, 'r').read())
    print(parseval(gold_tree, test_tree))

if __name__ == "__main__": 
    test()
