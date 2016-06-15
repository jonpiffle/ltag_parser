import re, nltk, sys, heapq
import graphviz as gv
import itertools
import operator
import copy
import collections

from collections import deque
from functools import reduce

from xtag_verbnet.grammar import Grammar

XTAG_GRAMMAR = Grammar.load()

def flatten(lst_of_lsts):
    return [a for lst in lst_of_lsts for a in lst]

def prod(iterable):
    return reduce(operator.mul, iterable, 1)

class KHeapFactory(object):
    def __init__(self, k, key=lambda x: x):
        self.k = k 
        self.key = key

    def new_heap(self, initial=None):
        if len(initial) < k:
            return initial
        return KHeap(self.k, initial, self.key)

    def new_higher_order_heap(self, initial=None, f=sum):
        if len(initial) < k:
            return initial
        return KHeap(self.k, initial, key=lambda lst: f([self.key(x) for x in lst]))

class KHeap(collections.MutableSequence):
    def __init__(self, k=1, initial=None, key=lambda x: x):
        self.count = 0
        self.key = key
        self.k = k
        self._data = []
        if initial is not None: 
            for item in initial:
                self.push(item)

    def push(self, item):
        self.count += 1
        if len(self._data) >= k:
            heapq.heappushpop(self._data, (self.key(item), self.count, item))
        else:
            heapq.heappush(self._data, (self.key(item), self.count, item))
 
    def pop(self):
        return heapq.heappop(self._data)[-1]

    def __getitem__(self, index):
        return self._data[index][-1]

    def __setitem__(self, index, value):
        raise NotImplementedError("Use push/pop to set values")

    def __delitem__(self, index):
        raise NotImplementedError("Use push/pop to set values")

    def insert(self, index, value):
        raise NotImplementedError("Use push/pop to set values")

    def __len__(self):
        return len(self._data)

class DerivationForestParser(object):
    @classmethod
    def fromstring(cls, node_id, node_str_dict, node_dict):
        if node_id in node_dict:
            return node_dict[node_id]

        node_string = node_str_dict[node_id]
        split = node_string.split()
        if len(split) == 3:
            label_string, tree_string, dtr_string = split
        else:
            label_string, tree_string = split
            dtr_string = ""

        # parse node information
        label = label_string.replace(":", "")
        match = re.match("(.*?)\[(.*?)\]\<(.*?)\>\<(.*?)\>\<(.*?)\>\<(.*?)\>\[(.*?)\]", tree_string)
        treename, word, pos1, word_pos, node_type, pos2, feature_string = [match.group(i) for i in range(1, 8)]
        pos1, position = pos1.split("/")

        # parse children
        dtr_or_list = []
        dtr_or_label_list = cls.dtr_or_list_fromstring(dtr_string)
        for label_list in dtr_or_label_list:
            child_list = [DerivationForestParser.fromstring(c_id, node_str_dict, node_dict) for c_id in label_list]
            dtr_or_list.append(child_list)

        if len(dtr_or_list) == 0:
            node = AndNode(node_id, treename, word, pos1, position, word_pos, node_type, [])
        elif len(dtr_or_list) == 1:
            node = AndNode(node_id, treename, word, pos1, position, word_pos, node_type, dtr_or_list[0])
        else:
            and_nodes = []
            for and_list in dtr_or_list:
                and_node = AndNode(node_id + ",".join([c.node_id for c in and_list]), treename, word, pos1, position, word_pos, node_type, and_list)
                and_nodes.append(and_node)
            node = OrNode(node_id, and_nodes)

        node_dict[node.node_id] = node
        return node

    @classmethod
    def dtr_or_list_fromstring(cls, dtr_string):
        if dtr_string == "":
            return []
        dtr_string = dtr_string[1:-1] # drop first and last bracket
        dtr_or_list = [cls.labels_fromstring(labels_string) for labels_string in dtr_string.split("][")]
        return dtr_or_list

    @classmethod
    def labels_fromstring(cls, labels_string):
        labels = [l for l in labels_string.split(",") if l != "(nil)"]
        return labels

    @classmethod
    def fromfile(cls, filename):
        with open(filename, 'r') as f:
            lines = f.readlines()
        lines = [l.strip() for l in lines]
        lines = lines[1:-1] # ignore begin/end lines

        start_children = []
        node_str_dict = {}
        for line in lines:
            match = re.search("start\: (.*?) \[\#S_TOPFEATS\]", line)
            if match is not None:
                label = match.group(1)
                start_children.append(label)
            else:
                node_id = re.match(r"(.*?):", line).group(1)
                node_str_dict[node_id] = line

        children = [DerivationForestParser.fromstring(c_id, node_str_dict, {}) for c_id in start_children]
        if len(children) > 1:
            children = [OrNode('startor', children)]
        start_node = AndNode('start', '', '', '', '', 'initroot', '', children)
        return start_node

class Node(object):
    def draw(self):
        root = nltk.Tree(str(self), [])
        queue = deque([(c, root) for c in self.children])
        while len(queue):
            node, parent = queue.popleft()
            tree = nltk.Tree(str(node), [])
            parent.append(tree)
            if len(node.children) > 0:
                queue += [(c, tree) for c in node.children]
        root.draw()

    def find(self, label):
        found = []
        queue = deque([self])
        while len(queue) > 0:
            node = queue.popleft()
            if str(node) == label:
                found.append(node)
            queue += [c for c in node.children]
        return found

    def find_by_treename(self, label):
        found = []
        queue = deque([self])
        while len(queue) > 0:
            node = queue.popleft()
            if node.treename == label:
                found.append(node)
            queue += [c for c in node.children]
        return found

    def find_by_type(self, node_type):
        found = []
        queue = deque([self])
        while len(queue) > 0:
            node = queue.popleft()
            if node.node_type == node_type:
                found.append(node)
            queue += [c for c in node.children]
        return found

    def get_parents(self, child):
        found = []
        queue = deque([self])
        while len(queue) > 0:
            node = queue.popleft()
            if child in node.children:
                found.append(node)
            queue += [c for c in node.children]
        return found

    def unique_nodes(self):
        found = set()
        queue = deque([self])
        while len(queue) > 0:
            node = queue.popleft()
            found.add(node)
            queue += set(node.children) - found
        return found

    def __repr__(self):
        return str(self)

class OrNode(Node):
    def __init__(self, node_id, children):
        self.children = children
        self.node_type = 'or'
        self.position = 'top'
        self.treename = 'ortree'
        self.word = ''
        self.node_id = node_id

    def deriv_trees(self, heap_fact=None, parent=None):
        child_derivs = [c.deriv_trees(heap_fact=heap_fact, parent=parent) for c in self.children]
        child_derivs = flatten(child_derivs)
        child_derivs = [c for c in heap_fact.new_higher_order_heap(child_derivs)]
        return child_derivs

    def count_derivations(self):
        return sum([c.count_derivations() for c in self.children])

    def __str__(self):
        return "OR"

class AndNode(Node):
    def __init__(self, node_id, treename, word, pos1, position, word_pos, node_type, children):
        self.node_id = node_id
        self.treename = treename
        self.word = word
        self.pos1 = pos1
        self.position = position
        self.word_pos = word_pos
        self.node_type = node_type
        self.children = children
        self.derivs = None

    def is_leaf(self):
        return len(self.children) == 0

    def top(self):
        return "top" in self.pos1

    def bot(self):
        return "bot" in self.pos1

    def pos(self):
        return self.pos1.split("/")[0]

    def is_pair(self, o):
        return self.treename == o.treename and self.word == o.word and self.pos1 == o.pos1

    def is_start(self):
        return self.node_id == 'start'

    def count_derivations(self):
        if self.is_leaf():
            return 1
        return prod([c.count_derivations() for c in self.children])

    def copy_deriv_trees_with_label(self, derivs, label):
        new_deriv_list = []
        for deriv_list in derivs:
            new_derivs = [] 
            for deriv in deriv_list:
                new_deriv = deriv.copy()
                new_deriv.location = label
                new_derivs.append(new_deriv)
            new_deriv_list.append(new_derivs)
        return new_deriv_list

    def deriv_trees(self, heap_fact=None, parent=None):
        if self.derivs is not None:
            return self.copy_deriv_trees_with_label(self.derivs, parent.pos1)

        if self.is_start():
            derivs = self.children[0].deriv_trees(heap_fact=heap_fact, parent=self)
            derivs = [d[0] for d in derivs]
            return derivs

        child_derivs = [c.deriv_trees(heap_fact, self) for c in self.children]
        product = [list(d) for d in itertools.product(*child_derivs)]
        product = [flatten(p) for p in product]

        if self.node_type not in ['initroot', 'auxroot']:
            return [d for d in heap_fact.new_higher_order_heap(product)] # if no heap, just return product

        if len(product) == 0:
            derivs = [DerivationNode(self.treename, self.word, parent.pos1, node_type=self.node_type)]
        else:
            derivs = [DerivationNode(self.treename, self.word, parent.pos1, children=c_deriv, node_type=self.node_type) for c_deriv in product]

        derivs = [[d] for d in heap_fact.new_heap(derivs)]
        self.derivs = derivs
        return derivs

    def __repr__(self):
        return str(self)

    def __str__(self):
        return "%s{%s}<%s/%s><%s>" % (self.treename, self.word, self.pos1, self.position, self.node_type)

class DerivationNode(nltk.Tree):
    def __init__(self, treename, word, location, children=None, node_type=None, _parse_tree=None, _deriv_depth=None):
        self.treename = treename
        self.word = word
        self.location = location
        self.node_type = node_type
        self._parse_tree = _parse_tree
        self._deriv_depth = _deriv_depth

        if children is None:
            children = []
        nltk.Tree.__init__(self, str(self), children)

    def find(self, label):
        return [s for s in self.subtrees() if s.label() == label][0]

    def parse_tree(self, depth=0):
        if self._parse_tree is not None:
            tree = self._parse_tree.copy() 
            for s in tree.subtrees():
                s.deriv_depth += depth
            return tree

        tree = XTAG_GRAMMAR.get(self.treename)
        tree.lexicalize(self.word)

        self._deriv_depth = depth
        for s in tree.subtrees():
            s.deriv_depth = depth

        for c in self:
            c_parse = c.parse_tree(depth+1)

            if 'alpha' in c.treename:
                sub_nodes = [s for s in tree.subst_nodes() if s.original_label() == c.location and s.deriv_depth == depth]
                assert len(sub_nodes) == 1
                sub_node = sub_nodes[0]
                tree.substitute(c_parse, sub_node.label())
            elif 'beta' in c.treename:
                adj_nodes = [s for s in tree.subtrees() if s.original_label() == c.location and s.deriv_depth == depth]
                assert len(adj_nodes) == 1
                adj_node = adj_nodes[0]
                tree.adjoin(c_parse, adj_node.label())

        self._parse_tree = tree
        return tree

    def copy(self):
        return DerivationNode(
            self.treename, 
            self.word,
            self.location,
            children=[c.copy() for c in self],
            node_type=self.node_type,
            _parse_tree=self._parse_tree,
            _deriv_depth=self._deriv_depth
        )

    def __str__(self):
        return "%s[%s]<%s>%s" % (self.treename, self.word, self.location, self.node_type)

    def __repr__(self):
        return str(self)

class SpanSet(object):
    def __init__(self):
        self.spans = set()
        self.weights = {}

    def add(self, span, weight):
        if span in self.weights:
            self.weights[span] = max(self.weights[span], weight)
        else:
            self.weights[span] = weight
        self.spans.add(span)

    def total_weight(self):
        return sum(self.weights.values()) + 1

    def difference_weight(self, other_spanset):
        spandiff = self.spans - other_spanset.spans
        return sum([self.weights[s] for s in spandiff])

class Scorer(object):

    @classmethod
    def subtrees_with_depth(cls, tree):
        subtrees = []
        cls._subtrees_with_depth(tree, 0, subtrees)
        return subtrees

    @classmethod
    def _subtrees_with_depth(cls, tree, depth, subtrees):
        if not isinstance(tree, nltk.Tree):
            return

        subtrees.append((depth, tree))
        for c in tree:
            cls._subtrees_with_depth(c, depth + 1, subtrees)

    @classmethod
    def get_span_set(cls, tree):
        spanset = SpanSet()
        height = tree.height()
        for depth, subtree in cls.subtrees_with_depth(tree):
            span = subtree.leaves()
            span = [l for l in span if 'epsilon' not in l and 'PRO' not in l]
            if len(span) >= 2:
                span = tuple(span)
                s_height = subtree.height()
                spanset.add(span, (height - depth) / height)
        return spanset

    @classmethod
    def fscore_distance(cls, tree1, tree2):
        spanset1 = cls.get_span_set(tree1)
        spanset2 = cls.get_span_set(tree2)

        precision = 1 - spanset1.difference_weight(spanset2) / spanset1.total_weight()
        recall = 1 - spanset2.difference_weight(spanset1) / spanset2.total_weight()

        if precision + recall == 0:
            return 0

        fscore = 2 * precision * recall / (precision + recall)
        return fscore

if __name__ == '__main__':
    filename = 'revised_parser_output/0001_0.txt'
    #filename = '../test/chased.output'
    #filename = '../test/chased_simple.output'
    s = DerivationForestParser.fromfile(filename)
    #orig_deriv = s.count_derivations()
    #s = s.collapse_nodes()
    #assert s.count_derivations() == orig_deriv
    print(len(s.unique_nodes()))

    #alphap_nodes = set(s.find_by_treename("alphaP"))
    #print(len(alphap_nodes))

    print(s.count_derivations())
    #s.draw()

    #gold = nltk.Tree.fromstring("(S (NP (NP (DT the) (NN dog)) (SBAR (WHNP (WP who)) (S (VP (VBD chased) (NP (DT the) (NN cat)))))) (VP (VBD ran)))")
    gold = nltk.Tree.fromstring("(S (NP (NP (NNP Pierre) (NNP Vinken)) (, ,) (ADJP (NP (CD 61) (NNS years)) (JJ old)) (, ,)) (VP (MD will) (VP (VB join) (NP (DT the) (NN board)) (PP (IN as) (NP (DT a) (JJ nonexecutive) (NN director))) (NP (NNP Nov.) (CD 29)))))")
    score_funct = lambda deriv_tree: Scorer.fscore_distance(gold, deriv_tree.parse_tree())
    k = 1
    heap_fact = KHeapFactory(k, score_funct)
    deriv_trees = s.deriv_trees(heap_fact)
    #deriv_trees = s.deriv_trees(None)
    #import code; code.interact(local=locals())

    print(len(deriv_trees))

    parse_trees = []
    for i, d in enumerate(deriv_trees):
        print(i)
        p = d.parse_tree()
        parse_trees.append(p)
        #d.draw()
        #p.draw()
    print('done getting parse trees')

    #d = max(deriv_trees, key=lambda deriv_tree: Scorer.fscore_distance(gold, deriv_tree.parse_tree()))
    #d.draw()
    #d.parse_tree().draw()
