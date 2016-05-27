import nltk
from nltk.tree import Tree, ParentedTree, ImmutableTree

def flip_word_pos(lst):
    return [(pos, word) for word, pos in lst]

def merge_tree_nnps(tree):
    """
    Takes a parse tree and merges any consecutive leaf nodes that come from NNPs
    For example if there is a segment of:
        (NP
            (JJ old)
            (NNP Pierre)
            (NNP Vinken)
        )
    Returns:
        (NP
            (JJ old)
            (NNP PierreVinken)
        )
    """

    # require a parented tree to get a subtrees tree position
    p = ParentedTree.convert(tree)

    # iterates subtrees of height 3. This is where NP's leading to NNP's leading to lexicalizations will be
    for s in p.subtrees(filter=lambda s: s.height() == 3):
        # merge NNP's in the list representation of this trees children: [(POS, word), ...] 
        new_noun_phrase = merge_tagged_nnps([(c.label(), c[0]) for c in s])
        child_str = " ".join("(%s %s)" % (pos, word) for pos, word in new_noun_phrase)
        # create new subtree with merged NNP's
        new_s = ParentedTree.fromstring("(%s %s)" % (s.label(), child_str))

        # replace old subtree with new subtree
        p[s.treeposition()] = new_s
    return Tree.convert(p)

def lowercase_tree_first_word(tree):
    node = tree
    while isinstance(node[0], nltk.Tree):
        node = node[0]

    if node.label() != "NNP":
        node[0] = node[0][0].lower() + node[0][1:]

    return tree

def merge_tagged_nnps(tagged):
    new_sentence = []
    current_nnp = []

    for pos, word in tagged:
        if pos == "NNP":
            current_nnp.append(word)
        elif len(current_nnp) > 0:
            new_nnp = ("NNP", "".join(current_nnp))
            new_sentence.append(new_nnp)
            current_nnp = []
            new_sentence.append((pos, word))
        else:
            new_sentence.append((pos, word))

    if len(current_nnp) > 0:
        new_nnp = ("NNP", "".join(current_nnp))
        new_sentence.append(new_nnp)
    return new_sentence

def remove_punc_from_tree(tree):
    if tree[-1].label() == '.':
        del tree[-1]
    return tree

def remove_punc_from_tagged(tagged):
    if len(tagged) == 0:
        return tagged

    pos, word = tagged[-1]
    if pos == ".":
        tagged = tagged[:-1]
    return tagged

def tagged_to_sent(tagged):
    return [word for pos, word in tagged]

def sent_to_str(sent):
    return " ".join(sent)

def tagged_sent_to_str(tagged_sent):
    return " ".join([word + '_' + pos for pos, word in tagged_sent])

def remove_none_from_tagged(tagged):
    return [(pos, word) for pos, word in tagged if pos not in ( "-NONE-", '``', '""', "''")]

def lowercase_tagged_first_word(tagged):
    if len(tagged) == 0:
        return tagged

    pos, word = tagged[0]
    if pos != "NNP":
        word = word[0].lower() + word[1:]
    tagged[0] = (pos, word)
    return tagged

def parse_to_tagged(parse):
    """Returns a list of [(pos, word) ... ] from a parse tree"""
    return [(s.label(), s[0]) for s in parse.subtrees(lambda s: s.height() == 2)]
