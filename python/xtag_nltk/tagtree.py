from nltk.tree import *
from nltk.featstruct import FeatStruct
from nltk.draw.tree import draw_trees
from nltk import sem
from collections import deque, defaultdict
import copy, re

def global_featname(node_name, top):
    try:
        node_name[-1]
    except UnicodeDecodeError:
        node_name = node_name[:-3]
    if top:
        return '.'.join([node_name, 't'])
    else:
        return '.'.join([node_name, 'b'])

class TAGTree(Tree):
    """
    A TAG Tree represents a lexicalized Tree Adjoining Grammar (TAG)
    formalism. The TAGTree is inherited from Tree Object, A Tree 
    represents a hierarchical grouping of leaves and subtrees. And 
    it contains a top feature structure and a bottom feature structure
    for every node. Some nodes are specified as head nodes, foot nodes,
    or substitution nodes.

    The TAG Tree can be lexicalized, which means we can attach a word
    to the substitution node and unify the feature structures in the 
    tree with the feature structures of the attachd node.
    """    

    def __init__(self, node_with_fs, children, attr=None):
        self.tree_name = None # Gets set in load()
        self.semantics = None # Gets set in self.add_semantics()
        self.computed_semantics = None # Gets set in self.compute_semantics()
        self.rename_dict = {} # Needed for persistent bindings while caching 
        self.semantic_meaning = None # Used by STRUCT NLG System
        self.deriv_depth = None # Used by STRUCT's deriv -> parse system
        self.anchor = None # Technically can be more than one, but not in STRUCT currently
        self._lex_leaves = None # Some caching useful for heuristic
        self._preorder_traversal = None # Some caching useful for heuristic

        ### Hack to handle Determiners without features ###
        self.must_adjoin = False
        self.can_adjoin = True
        ### End Hack ###

        self.attr = attr
        self._lex = False
        self.comment = None
        self.start_feat = False
        if isinstance(node_with_fs, str):
            self._label = node_with_fs
            self.top_fs = FeatStruct()
            self.bot_fs = FeatStruct()
        else:
            self._label, self.top_fs, self.bot_fs = node_with_fs
        self.orig_label = self._label # Necessary for reconstructing parse trees from deriv trees
        Tree.__init__(self, self._label, children)

    def fast_copy(self):
        new_tree = TAGTree((self.label(), self.top_fs, self.bot_fs), [c.fast_copy() for c in self], attr=self.attr)
        new_tree.tree_name = self.tree_name
        new_tree.semantics = self.semantics
        new_tree.computed_semantics = self.computed_semantics
        new_tree.rename_dict = self.rename_dict.copy()
        new_tree.semantic_meaning = self.semantic_meaning
        new_tree.deriv_depth = self.deriv_depth
        new_tree.anchor = self.anchor
        new_tree._lex = self._lex
        new_tree.comment = self.comment
        new_tree.start_feat = self.start_feat
        new_tree.orig_label = self.orig_label
        ### Hack to handle Determiners without features ###
        new_tree.must_adjoin = self.must_adjoin
        new_tree.can_adjoin = self.can_adjoin
        ### End Hack ###
        return new_tree

    def set_comment(self, comment):
        comment = comment.replace("\\\"","\"")
        if len(comment) == 0:
            self.comment = "No Comments"
        else:
            self.comment = comment

    def draw(self):
        """
        Display the TAGTree on canvas
        """
        draw_trees(self)

    def copy(self, deep=False):
        """
        Return a new copy of ``self``. 

        :param deep: If true, create a deep copy; if false, create
            a shallow copy.
        """
        if deep:
            return copy.deepcopy(self)
        else:
            clone = copy.copy(self)
            if clone._lex:
                clone._lex_fs = copy.deepcopy(self._lex_fs)
        return clone

    def __get_node(self, name_or_attr):
        results = []
        nodes = [self]
        while len(nodes) > 0:
            last = nodes.pop()
            if last.label() == name_or_attr or last.attr == name_or_attr:
                results.append(last)
            else:
                nodes += [c for c in last if isinstance(c, TAGTree)]
        return results

    def get_head(self):
        """
        Get the head node of the TAG Tree
        """
        return self.__get_node('head')

    def get_head_name(self):
        """
        Get the label of the head node after replacing underscores
        """
        heads = self.get_head()
        assert len(heads) == 1
        return heads[0].get_node_name().replace('_','')


    def set_children(self, children):
        """
        Set the children of the current TAGTree
        """
        if isinstance(children, str):
            raise TypeError("%s() argument 2 should be a list, not a "
                            "string" % type(self).__name__)
        self.append(children)

    def init_lex(self, morph, fs1, fs2):
        """
        Prepare and initialization before lexicalization

        :param morph, the morphology to operate lexicalization
        :param fs1, the node feature structure to unify
        :param fs2, the tree feature structure to unify
        """
        self._morph = morph
        self._lex_fs = []
        for nf in fs1+fs2:
            for key in nf.keys():
                tf = FeatStruct()
                if key[-2:] not in ['.t', '.b']:
                    tf[global_featname(morph[0][1], False)] = FeatStruct()
                    tf[global_featname(morph[0][1], False)][key] = nf[key]
                else:
                    tf[key] = nf[key]
                self._lex_fs.append(tf)
        self._lex = True

    def lexicalize(self):
        """
        Do lexicalization operation, attach the lex
        to the substitution node, unify the feature
        structure defined in grammar files
        """
        morph_dict = {pos: word for word, pos in self._morph}
        for head in self.get_head():
            head_name = head.get_node_name().replace('_','')
            anchor = morph_dict[head_name]
            lex = TAGTree(anchor, [], 'lex')
            head.set_children(lex)
            self.anchor = anchor

            ### Hack for Determiners ###
            if self.tree_name == "\x02NXN" and head_name == "N" and anchor[0].islower() and anchor not in PRONOUNS:
                self.must_adjoin = True
                #print(self.tree_name, anchor, self.must_adjoin)
            ### End Hack for Determiners ###

        all_fs = self.combine_lex_fs_all_fs(self._lex_fs, self.get_all_fs())
        self.set_all_fs(all_fs)

    def missing_determiners(self):
        missing = int(self.must_adjoin)
        return missing + sum([c.missing_determiners() for c in self])

    def combine_lex_fs_all_fs(self, fs, all_fs):
        """
        Updates the feature structure after lexicalization
        """
        for f in fs:
            for node in f:
                for attr in f[node]:
                    if node in all_fs:
                        all_fs[node][attr] = f[node][attr]
                    else:
                        all_fs[node] = FeatStruct()
                        all_fs[attr] = f[node][attr]

            for node in f:
                stack = [[f[node], key] for key in f[node]]

                while len(stack) > 0:
                    path = stack.pop()
                    feat = path[0]
                    for e in range(1, len(path)):
                        feat = feat[path[e]]
                    first_key = list(feat.keys())[0]
                    if first_key[:5] == '__or_':
                        mapping = feat[first_key].split(':')
                        if len(mapping) == 2:
                            m_node = mapping[0]
                            m_attr = mapping[1][1:-1]

                            if len(m_attr.split()) > 1:
                                m_attr = tuple(m_attr.split())

                            if not m_attr in all_fs[m_node]:
                                all_fs[m_node][m_attr]['__or_'] = ''
                            m_key = list(all_fs[m_node][m_attr].keys())[0]
                            c_feat = all_fs[node]
                            for e in range(1, len(path)-1):
                                c_feat = c_feat[path[e]]
                            c_feat[path[-1]] = all_fs[m_node][m_attr]
                        else:
                            continue
                    else:
                        feat_list = []
                        for key in feat:
                            items = copy.copy(path)
                            items.append(key)
                            feat_list.append(items)
                        stack += feat_list
        return all_fs


    def unify(self, fs):
        """
        Unify feature structure with TAG tree:
        :param: unified feature structure
        :type: FeatStruct
        """
        all_fs = self.get_all_fs()
        temp = all_fs.unify(f)
        if temp:
            all_fs = temp
        else:
            raise NameError("Unify return None")
        self.set_all_fs(all_fs)


    def get_node_name(self):
        """
        Get node name of root node
        """
        return self.label()

    def get_all_fs(self):
        """
        Get the overall feature structure of the TAGTree
        """
        stack = [self]
        all_feat = FeatStruct()
        while len(stack) > 0:
            last = stack.pop()
            label = last.label()
                
            all_feat[label+'.'+'t'] = last.top_fs
            all_feat[label+'.'+'b'] = last.bot_fs

            stack += [c for c in last if isinstance(c, TAGTree)]

        return all_feat

    def set_all_fs(self, all_fs):
        """
        Update the feature structure of the TAGTree
        """
        if not all_fs:
            return

        nodes = [self]
        while len(nodes) > 0:
            last = nodes.pop()
            label = last._label

            if label+'.'+'t' not in all_fs and label+'.'+'b' not in all_fs:
                raise NameError('should contain feature structures')

            last.top_fs = all_fs[label+'.'+'t']
            last.bot_fs = all_fs[label+'.'+'b']

            nodes += [c for c in last if isinstance(c, TAGTree)]

    def leaves(self):
        """
        :return: leaves of this tree
        :rtype: list
        """
        leaves = []
        if len(self) == 0:
            leaves.append(self)
        for child in self:
            leaves.extend(child.leaves())
        return leaves

    def lex_leaves(self):
        if self._lex_leaves is None:
            self._lex_leaves = [l.label() for l in self.leaves() if l.attr == 'lex']
        return self._lex_leaves

    def preorder_traversal(self):
        if self._preorder_traversal is None:
            self._preorder_traversal = [self.label()]
            for c in self:
                self._preorder_traversal += c.preorder_traversal()
        return self._preorder_traversal

    def lex_leaves_with_node(self, node_label):
        leaves = []
        if (len(self) == 0 and self.attr == 'lex') or self.label() == node_label:
            leaves.append(self)
        for child in self:
            leaves.extend(child.lex_leaves_with_node(node_label))
        return leaves

    def cancel_substitution(self):
        """
        Cancel substitution node of this tree
        """
        if self.attr == 'subst':
            self.attr = None
        else:
            raise TypeError("No substitution node for this tree")

    def get_substitution_node(self):
        """
        :return: substitution subtree
        :rtype: TAGTree
        """
        sub = []
        for leaf in self.leaves():
            if leaf.attr == 'subst':
                sub.append(leaf)

        return sub

    def prefix_search(self, tree_name):
        """
        Search node with prefix in the tree
        :return: subtree with tree_name as prefix
        :rtype: TAGTree
        """
        trees = []
        label = self._label
        name = label.split('_')
        prefix = name[0]
        if prefix == tree_name:
            trees.append(self)
        for child in self:
            trees.extend(child.prefix_search(tree_name))
        return trees

    def delete_all_child(self):
        """
        Remove all child of this tree
        """
        result = []
        while(len(self)) > 0:
            self.pop()

    def cancel_adjunction(self):
        """
        Cancel adjunction node of this tree
        """
        if self.attr == 'foot':
            self.attr = None
        else:
            raise TypeError("No adjunction node")

    def search(self, tree_name):
        """
        Search node with name as tree_name in the tree
        :return: subtree with tree_name as root name
        :rtype: TAGTree
        """
        label = self._label
        if label == tree_name:
            return self
        else:
            for child in self:
                tree = child.search(tree_name)
                if tree != None:
                    return tree
        return None

    def get_child_node(self):
        return [child for child in self]

    def append_new_child(self, tree):
        self.append(tree)

    def get_top_feature(self):
        return self.top_fs

    def get_bottom_feature(self):
        return self.bot_fs

    def set_bottom_feature(self, fs):
        self.bot_fs = fs

    def set_top_feature(self, fs):
        self.top_fs = fs

    def get_foot_node(self):
        for leaf in self.leaves():
            if leaf.attr == 'foot':
                return leaf
        return None 

    def is_auxiliary(self):
        return '\x03' in self.tree_name

    def correct_name(self):
        """
        Get a copied tree with no name collided node
        """
        names = defaultdict(int)
        for sub in self.subtrees():
            if not isinstance(sub, TAGTree) or sub.attr == 'lex':
                continue

            label = sub.label()
            name = label.split("_")[0]

            label = name + '_' + str(names[name] + 1)
            sub._label = label

            names[name] += 1

        return self

    def check_name(self, names):
        """
        Check this tree for name collided nodes 
        :return: name collided number
        :rtype: int
        """
        for child in self:
            label = child._label
            name = label.split("_")[0]
            if name in names:
                label = name + '_' + str(names[name])
                child._label = label
            names[name] += 1
            child.check_name(names)
        return names

    def clean_semantic_label(self, label):
        clean_label = label.replace("%", "percent")
        clean_label = clean_label.replace("...", "ellipsis")
        clean_label = clean_label.replace("*", "asterisk")
        clean_label = clean_label.replace("&", "ampersand")
        clean_label = clean_label.replace("!", "exclamation")
        clean_label = clean_label.replace("'", "singlequote")
        clean_label = clean_label.replace("\"", "doublequote")
        clean_label = re.sub('\W+', '', clean_label)

        if len(clean_label) == 1:
            clean_label += "singlechar"

        if re.match('[a-z|A-Z]\d+', clean_label) is not None:
            clean_label += "singlechar"

        if clean_label in ["all", "exist", "exists", "and", "or"]:
            clean_label += "cleaned"

        return clean_label


    def add_semantics(self, semmap):
        """
        Takes a semantic map of the form {'label': DrtExpression}
        Stores the associated DrtExpression as node.semantics for each node in the tree
        If the node is lexicalized, need to pass the lexicalization as a param so that
            we can dynamically generate relations like "chase(x, y)" from the node "chase"
        """
        if self.attr == 'lex':
            clean_label = self.clean_semantic_label(self.label())
            #print(clean_label, self.label())
            self.semantics = semmap['lex'](clean_label)
        elif self.label() in semmap:
            self.semantics = semmap[self.label()]
        elif len(self) == 0:
            #self.semantics = sem.drt.DrtExpression.fromstring("DRS([],[])")
            self.semantics = sem.drt.DrtExpression.fromstring("\\P.P")
        elif len(self) == 1:
            self.semantics = sem.drt.DrtExpression.fromstring("\\P.P")
        elif len(self) == 2:
            self.semantics = sem.drt.DrtExpression.fromstring("\\P Q.P(Q)")
        else:
            print("WARNING: ", self.label(), " has no semantics set")
            self.semantics = None

        for c in self:
            c.add_semantics(semmap)

    def reset_semantics(self):
        if len(self) == 0:
            return self.computed_semantics is None
        else:
            if any(c.reset_semantics() for c in self):
                self.computed_semantics = None
                return True
            return False

    def compute_semantics(self, cache=False):
        # Turned off semantic caching because it interfered with the persistent variable bindings
        if cache and self.computed_semantics is not None:
            return self.computed_semantics

        self.computed_semantics = self.semantics
        for c in self:
            c.compute_semantics()
            self.computed_semantics = app(self.computed_semantics, c.computed_semantics)

            added = set()
            for k, v in sem.logic.rename_dict.items():
                added.add(v)
                if v in c.rename_dict:
                    v = c.rename_dict[v]
                self.rename_dict[k] = v

            for k,v in c.rename_dict.items():
                if k not in added:
                    self.rename_dict[k] = v

            #print('rename', self.rename_dict)
            sem.logic.rename_dict = {}

        return self.computed_semantics

def add_rename_dict(obj):
    if not hasattr(obj, 'rename_dict'):
        obj.rename_dict = {}

def app(drt1, drt2):
    #add_rename_dict(drt1)
    #add_rename_dict(drt2)
    result = sem.drt.DrtApplicationExpression(drt1, drt2).simplify()
    #add_rename_dict(result)
    #print('result', result.rename_dict)
    return result

class TAGTreeSet(dict):
    """
    Basic data classes for representing a set of TAG Trees, and for
    iterating the set and performing basic operations on those 
    TAG Tree Set.  The set is a mapping from tree names to TAG trees.

    TAG Tree Sets are typically used to store the Tree information
    about TAG Tree Set. It can be merged with other TAG Tree Set.
    """
    def __init__(self, trees=None):
        dict.__init__(self)
        self.depth = 0
        self.start_fs = None
        if trees:
            self.update(trees)

    def __setitem__(self, tree_name, tree):
        dict.__setitem__(self, tree_name, tree)

    def __getitem__(self, tree_name):
        return self.get(tree_name, 0)

    def __str__(self):
        items = ['%r: %r' % (s, self[s]) for s in list(self.keys())[:10]]
        if len(self) > 10:
            items.append('...')
        return '<TAGTreeSet: %s>' % ', '.join(items)

    def __repr__(self):
        return '<TAGTreeSet with %d trees>' % (len(self))

    def __add__(self, other):
        clone = self.copy()
        clone.update(other)
        return clone

    def __le__(self, other):
        if not isinstance(other, TAGTreeSet): return False
        return set(self).issubset(other) and all(self[key] <= other[key] for key in self)
    def __lt__(self, other):
        if not isinstance(other, TAGTreeSet): return False
        return self <= other and self != other
    def __ge__(self, other):
        if not isinstance(other, TAGTreeSet): return False
        return other <= self
    def __gt__(self, other):
        if not isinstance(other, TAGTreeSet): return False
        return other < self

    def get_tag_trees(self):
        trees = []
        q = deque(self.values())

        while len(q) > 0:
            t = q.popleft()
            if isinstance(t, TAGTreeSet):
                q.extend(t.values())
            elif isinstance(t, TAGTree):
                trees.append(t)
            else:
                continue
        return trees

    def get_tag_tree_dict(self):
        trees = {}
        q = deque(self.items())

        while len(q) > 0:
            tree_name, tree = q.popleft()
            if isinstance(tree, TAGTreeSet):
                q.extend(tree.items())
            elif isinstance(tree, TAGTree):
                trees[tree_name] = tree
            else:
                continue

        return trees

    def family_to_tree_names(self, family):
        family = family + '.trees'
        q = deque(self.items())

        while len(q) > 0:
            (k, t) = q.popleft()

            if k == family:
                return list(t.keys())
            elif isinstance(t, TAGTreeSet):
                q.extend(t.items())
            else:
                continue

        return None

    def tree_name_to_family(self, tree_name):
        q = deque([self.items()])

        while len(q) > 0:
            (k, t) = q.popleft()
            if not isinstance(t, TAGTreeSet):
                continue
            elif tree_name in t.keys():
                return k.replace('.trees', '')
            else:
                q.extend([t.items()])
         
        return None

    def find_tree(self, tree_name):
        q = deque(self.items())

        while len(q) > 0:
            (k, t) = q.popleft()

            if k == tree_name:
                return t
            elif isinstance(t, TAGTreeSet):
                q.extend(t.items())
            else:
                continue

        return None

    def tree_names(self):
        """
        Get the name of trees
        """
        return self.keys()

    def update(self, trees):
        if not isinstance(trees, type(self)):
            raise TypeError("%s: Expected a TAGTreeSet value"
                             % type(self).__name__)
        for j in trees:
            if j in self:
                raise TypeError("%s: Overlapped Set " 
                                 % type(self).__name__)
            else:
                self[j] = trees[j]

    def tree_count(self):
        """
        Get the total number of trees
        """
        total = 0 
        for tree in self:
            if isinstance(self[tree], TAGTreeSet):
                total += self[tree].tree_count()
            else:
                total += 1
        return total

    def tree_family_count(self):
        """
        Get the total number of tree families
        """
        total = 0 
        for tree in self:
            if isinstance(self[tree], TAGTreeSet) and tree[-6:] == '.trees':
                total += 1
            elif isinstance(self[tree], TAGTreeSet):
                total += self[tree].tree_family_count()
        return total

    def copy(self, deep=True):
        """
        Return a new copy of ``self``. 

        :param deep: If true, create a deep copy; if false, create
            a shallow copy.
        """
        if deep:
            return copy.deepcopy(self)
        else:
            clone = type(self)()
            for tree in self:
                if not isinstance(self[tree], TAGTreeSet):
                    clone[tree] = self[tree].copy(False)
                else:
                    clone += copy.copy(tree)
            return clone    

    def set_start_fs(self, fs):
        """
        Set start feature of all TAG trees.
        :param fs: start feature
        :type: FeatStruct
        """
        self.start_fs = fs 

PRONOUNS = set([
    "all",
    "another",
    "any",
    "anybody",
    "anyone",
    "anything",
    "both",
    "each",
    "each other",
    "either",
    "everybody",
    "everyone",
    "everything",
    "few",
    "he",
    "her",
    "hers",
    "herself",
    "him",
    "himself",
    "his",
    "i",
    "it",
    "its",
    "itself",
    "little",
    "many",
    "me",
    "mine",
    "more",
    "most",
    "much",
    "my",
    "myself",
    "neither",
    "no one",
    "nobody",
    "none",
    "nothing",
    "one",
    "one another",
    "other",
    "others",
    "our",
    "ours",
    "ourselves",
    "several",
    "she",
    "some",
    "somebody",
    "someone",
    "something",
    "that",
    "their",
    "theirs",
    "them",
    "themselves",
    "these",
    "they",
    "this",
    "those",
    "us",
    "we",
    "what",
    "whatever",
    "which",
    "whichever",
    "who",
    "whoever",
    "whom",
    "whomever",
    "whose",
    "you",
    "your",
    "yours",
    "yourself",
    "yourselves"
])
