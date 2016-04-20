import re, nltk
import graphviz as gv
import itertools
import operator

from collections import deque
from functools import reduce

def prod(iterable):
    return reduce(operator.mul, iterable, 1)

class DerivationForestParser(object):
    total_count = 0
    in_dict_count = 0

    @classmethod
    def fromstring(cls, node_id, node_str_dict, node_dict, parent_pos=""):
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
            child_list = [DerivationForestParser.fromstring(c_id, node_str_dict, node_dict, parent_pos=pos1) for c_id in label_list]
            dtr_or_list.append(child_list)

        if len(dtr_or_list) == 0:
            node = AndNode(node_id, treename, word, pos1, position, word_pos, node_type, parent_pos, [])
        elif len(dtr_or_list) == 1:
            node = AndNode(node_id, treename, word, pos1, position, word_pos, node_type, parent_pos, dtr_or_list[0])
        else:
            and_nodes = []
            for and_list in dtr_or_list:
                and_node = AndNode(node_id, treename, word, pos1, position, word_pos, node_type, parent_pos, and_list)
                and_nodes.append(and_node)
            node = OrNode(and_nodes)

        ### Need to fix this at some point to enable full structure sharing.  ###
        ### The problem is that we're copying the and node many times, so each has the same id ###
        #node_dict[node.node_id] = node
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

        children = [DerivationForestParser.fromstring(c_id, node_str_dict, {}, "") for c_id in start_children]
        if len(children) > 1:
            children = [OrNode(children)]
        start_node = AndNode('start', '', '', '', '', '', 'initroot', '', children)
        return start_node

class Node(object):
    def find(self, label):
        found = []
        queue = deque([self])
        while len(queue) > 0:
            node = queue.popleft()
            if str(node) == label:
                found.append(node)
            queue += [c for c in node.children]
        return found

    def __repr__(self):
        return str(self)

class OrNode(Node):
    def __init__(self, children):
        self.children = children
        self.node_type = 'or'
        self.position = 'top'
        self.treename = 'ortree'
        self.word = ''
        self.node_id = ",".join(sorted([c.node_id for c in children]))

    def deriv_trees(self):
        return [d for c in self.children for d in c.deriv_trees()]

    def count_derivations(self):
        return sum([c.count_derivations() for c in self.children])

    def two_or_children(self):
        return any([c.two_or_children() for c in self.children])

    def collapse_nodes(self, parent):
        has_root_child = False
        children = [c.collapse_nodes(self) for c in self.children]
        if len(children) > 0:
            children, child_has_root_list = zip(*[(clist, c_has_root_child) for clist, c_has_root_child in children])
            children = [c for clist in children for c in clist]
            self.children = children
            has_root_child = any(child_has_root_list)
        self.children = children
        return [self], has_root_child

    def __str__(self):
        return "OR"

class AndNode(Node):
    def __init__(self, node_id, treename, word, pos1, position, word_pos, node_type, parent_pos, children):
        self.node_id = node_id
        self.treename = treename
        self.word = word
        self.pos1 = pos1
        self.position = position
        self.word_pos = word_pos
        self.node_type = node_type
        self.parent_pos = parent_pos
        self.children = children

    def draw(self):
        root = nltk.Tree(str(self.node_id), [])
        queue = deque([(c, root) for c in self.children])
        while len(queue):
            node, parent = queue.popleft()
            tree = nltk.Tree(str(node), [])
            parent.append(tree)
            if len(node.children) > 0:
                queue += [(c, tree) for c in node.children]
        root.draw()

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

    def count_derivations(self):
        if self.is_leaf():
            return 1
        return prod([c.count_derivations() for c in self.children])

    def deriv_trees(self):
        location = self.parent_pos #if "alpha" in self.treename else self.pos1
        if self.is_leaf():
            return [DerivationNode(self.treename, self.word, location, node_type=self.node_type)]

        derivs = []
        child_derivs = [c.deriv_trees() for c in self.children]
        product = [list(d) for d in itertools.product(*child_derivs)]
        for c_deriv in product:
            deriv = DerivationNode(self.treename, self.word, location, children=c_deriv, node_type=self.node_type)
            derivs.append(deriv)

        return derivs

    def collapse_nodes(self, parent=None):
        children = [c.collapse_nodes(self) for c in self.children]
        if len(children) > 0:
            children, child_has_root_list = zip(*[(clist, c_has_root_child) for clist, c_has_root_child in children])
            children = [c for clist in children for c in clist]
            self.children = children
            has_root_child = self.node_type in ['initroot', 'auxroot'] or any(child_has_root_list)
        else:
            has_root_child = self.node_type in ['initroot', 'auxroot']

        if parent is None:
            return self

        if not has_root_child:
            return [], False
        elif self.treename == parent.treename and self.node_type not in ['initroot', 'auxroot'] and parent.position == 'top' and self.position == 'bot':
            return children, has_root_child
        elif parent.word == 'nil' and self.node_type == 'initroot':
            parent.replace(self)
            return children, has_root_child

        # This is annoying case where there are multiple possible adjunctions happening on a bottom node
        # This causes the top node, followed by the OR node, followed by the bottom options
        # When this is the case, we just want to replace the top node with the OR node
        # but there is not a nice way to do that on the OR node, so must do here
        elif len(children) == 1 and isinstance(children[0], OrNode) and children[0].children[0].treename == self.treename and children[0].children[0].node_type not in ['initroot', 'auxroot'] and self.position == 'top' and children[0].children[0].position == 'bot':
            for gc in children[0].children:
                gc.parent_pos = self.parent_pos
            
            return children, has_root_child

        elif parent.treename == self.treename and self.node_type in ['internal', 'auxfoot']:
            return children, has_root_child
        else:
            return [self], has_root_child

    def two_or_children(self):
        if self.is_leaf():
            return False
        return (len(self.children) == 2 and all([isinstance(c, OrNode) for c in self.children])) or any([c.two_or_children() for c in self.children])

    def replace(self, other):
        self.node_id = other.node_id
        self.treename = other.treename
        self.word = other.word
        self.pos1 = other.pos1
        self.position = other.position
        self.word_pos = other.word_pos
        self.node_type = other.node_type
        self.parent_pos = other.parent_pos
        self.children = other.children
        return self

    def __repr__(self):
        return str(self)

    def __str__(self):
        return "%s{%s}<%s/%s><%s>" % (self.treename, self.word, self.pos1, self.position, self.node_type)

class DerivationNode(nltk.Tree):
    def __init__(self, treename, word, location, children=None, node_type=None):
        self.treename = treename
        self.word = word
        self.location = location
        self.node_type = node_type
        if children is None:
            children = []
        nltk.Tree.__init__(self, str(self), children)

    def __str__(self):
        return "%s[%s]<%s>%s" % (self.treename, self.word, self.location, self.node_type)

    def __repr__(self):
        return str(self)

if __name__ == '__main__':
    filename = '../test/treebank.output'
    #filename = '../test/chased.output'
    #filename = '../test/chased_simple.output'
    s = DerivationForestParser.fromfile(filename)
    print(s.count_derivations())
    s = s.collapse_nodes()
    #n = s.find("betaN0nx0Ax1{old}<NP_r/top><auxroot>")
    #import code; code.interact(local=locals())
    print(s.count_derivations())
    print(len(s.deriv_trees()))
    #s.draw()
    s.deriv_trees()[0].draw()
    #s = s.collapse_nodes()
    #d[0].draw()
