import nltk, json, re
import shlex, subprocess
from nltk.corpus.util import LazyCorpusLoader
from nltk.corpus.reader import CategorizedBracketParseCorpusReader
from nltk.tree import Tree, ParentedTree, ImmutableTree
from parseval import parseval
import matplotlib.pyplot as plt
from multiprocessing import Pool

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

def tagged_to_sent(tagged):
    return [word for pos, word in tagged]

def sent_to_str(sent):
    return " ".join(sent)

def tagged_sent_to_str(tagged_sent):
    return " ".join([word + '_' + pos for pos, word in tagged_sent])

def remove_none_from_tagged(tagged):
    return [(pos, word) for pos, word in tagged if pos != "-NONE-"]

def create_tmp_tagged_file(tagged_str):
    filename = 'test/tmp.tagged'
    with open('../' + filename, 'w') as f:
        f.write(tagged_str + '\n')
    return filename

def parser_output_to_parse_deriv_trees(output):
    lines = output.strip().split("\n")
    deriv_tree_lines = lines[::2]
    parse_tree_lines = lines[1::2]

    parse_trees = [Tree.fromstring(line.replace('\x06', 'epsilon_')) for line in parse_tree_lines if line != '']
    deriv_trees = [Tree.fromstring(line) for line in deriv_tree_lines if line != '']
    return parse_trees, deriv_trees

def run_parser(sent, tagged_filename, max_derivations=15000):
    cmd = "cd ..; \
    echo '%s' | \
    bin/syn_get.bin data/english/english.grammar lib/xtag.prefs | \
    bin/tagger_filter %s | \
    bin/nbest_parser.bin data/english/english.grammar lib/xtag.prefs" % (sent, tagged_filename)
    count_cmd = cmd + " | bin/count_derivations"
    print_cmd = cmd + " | bin/print_deriv -b" 
    p = subprocess.Popen(count_cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True, universal_newlines=True)
    output, err = p.communicate()

    count_str = re.search('count=(\d+)', output)
    if count_str is None:
        print('no count_str found')
        return [], []

    num_derivations = int(count_str.group(1))
    print('derivations:', num_derivations)

    if num_derivations < max_derivations:
        p = subprocess.Popen(print_cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True, universal_newlines=True)
        output, err = p.communicate()
        return parser_output_to_parse_deriv_trees(output)
    else:
        return [], []

def get_subtree_set(tree):
    #subtree_set = set([tuple([l for l in i.leaves()]) for i in tree.subtrees(filter=lambda s: s.height() >= 3)])
    subtree_set = set([tuple([l for l in i.leaves() if 'epsilon_' not in l]) for i in tree.subtrees(filter=lambda s: s.height() >= 3)])
    return subtree_set

def distance(tree1, tree2):
    subtree_set1 = get_subtree_set(tree1)
    subtree_set2 = get_subtree_set(tree2)

    precision = 1 - (len(subtree_set1 - subtree_set2) / float(len(subtree_set1)))
    recall = 1 - (len(subtree_set2 - subtree_set1) / float(len(subtree_set2)))

    if precision + recall == 0:
        return 0

    fscore = 2 * precision * recall / (precision + recall)
    return fscore

def get_best_parse(fileid, i, parse, tagged, max_len=30):
    if len(tagged) > max_len:
        return

    print(i, len(tagged), len(parse))

    filename = 'parse_trees/%s_%d.txt' % (fileid.split('/')[-1].split('.')[0], i)
    tagged = flip_word_pos(tagged)
    tagged = remove_none_from_tagged(tagged)
    tagged = merge_tagged_nnps(tagged)
    sent = tagged_to_sent(tagged)

    try:
        gold_tree = merge_tree_nnps(parse)
    except IndexError:
        return

    tagged_filename = create_tmp_tagged_file(tagged_sent_to_str(tagged))
    parse_trees, deriv_trees = run_parser(sent_to_str(sent), tagged_filename)
    trees = zip(parse_trees, deriv_trees)

    best_parse, best_deriv, best_val = None, None, 0
    for test_parse_tree, test_deriv_tree in trees:
        val = distance(test_parse_tree, gold_tree)
        if val > best_val:
            best_parse, best_deriv, best_val = test_parse_tree, test_deriv_tree, val

    tree_dict = {"parse": str(best_parse), "deriv": str(best_deriv)}
    if best_parse is not None:
        with open(filename, 'w') as f:
            f.write(json.dumps(tree_dict))

def parse_wsj_file(fileid, parsed, tagged):
    corpus = zip(parsed, tagged)
    for i, parse, tagged in enumerate(corpus):
        get_best_parse(fileid, i, parse, tagged)

def parse_wsj(processes=8):
    ptb = LazyCorpusLoader( # Penn Treebank v3: WSJ portions
        'ptb', CategorizedBracketParseCorpusReader, r'wsj/\d\d/wsj_\d\d\d\d.mrg',
        cat_file='allcats.txt', tagset='wsj')

    fileids = ptb.fileids()[:10]
    params = []
    for f in fileids:
        corpus = zip(ptb.parsed_sents(f), ptb.tagged_sents(f))
        for i, (parsed, tagged) in enumerate(corpus):
            params.append((f, i, parsed, tagged))

    p = Pool(processes)
    p.starmap(get_best_parse, params)

if __name__ == '__main__':
    parse_wsj(2)
