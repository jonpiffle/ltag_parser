import nltk, json, re, os
import shlex, subprocess
from nltk.corpus.util import LazyCorpusLoader
from nltk.corpus.reader import CategorizedBracketParseCorpusReader
from nltk.tree import Tree, ParentedTree, ImmutableTree
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

def create_tmp_tagged_file(fileid, i, tagged_str):
    filename = '../test/tagged/%s_%d.tagged' % (fileid.split('/')[-1].split('.')[0], i)
    with open(filename, 'w') as f:
        f.write(tagged_str + '\n')
    return filename

def parser_output_to_parse_deriv_trees(output):
    lines = output.strip().split("\n")
    deriv_tree_lines = lines[::2]
    parse_tree_lines = lines[1::2]

    parse_trees = [Tree.fromstring(line.replace('\x06', 'epsilon_')) for line in parse_tree_lines if line != '']
    deriv_trees = [Tree.fromstring(line) for line in deriv_tree_lines if line != '']
    return parse_trees, deriv_trees

def all_trees_from_parser_output(parser_output_filename):
    p1_cmd = shlex.split("cat %s" % parser_output_filename)
    p1 = subprocess.Popen(p1_cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    p2_cmd = shlex.split("../bin/print_deriv -b")
    p2 = subprocess.Popen(p2_cmd, stdin=p1.stdout, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
    output, err = p2.communicate()

    return parser_output_to_parse_deriv_trees(output)

def sample_trees_from_parser_output(parser_output_filename, sample_num=5000):
    parse_trees, deriv_trees = [], []

    while len(parse_trees) < sample_num:
        p1_cmd = shlex.split("cat %s" % parser_output_filename)
        p1 = subprocess.Popen(p1_cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

        p2_cmd = "../bin/truncate_graph"
        p2 = subprocess.Popen(p2_cmd, stdin=p1.stdout, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

        p3_cmd = shlex.split("../bin/print_deriv -b")
        p3 = subprocess.Popen(p3_cmd, stdin=p2.stdout, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
        output, err = p3.communicate()

        sampled_parse_trees, sampled_deriv_trees = parser_output_to_parse_deriv_trees(output)
        parse_trees += sampled_parse_trees
        deriv_trees += sampled_deriv_trees

    return parse_trees, deriv_trees

def handle_parser_output(parser_output_filename, max_derivations, error):
    # Count derivations returned by parser
    p1_cmd = shlex.split("cat %s" % parser_output_filename)
    p1 = subprocess.Popen(p1_cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    p2_cmd = "../bin/count_derivations"
    p2 = subprocess.Popen(p2_cmd, stdin=p1.stdout, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
    output, err = p2.communicate()

    count_str = re.search('count=(\d+)', output)
    if count_str is None:
        error["type"] = "ParseFailed"
        error["message"] = "output: %s error: %s" % (output, err)
        return [], []

    num_derivations = int(count_str.group(1))
    print('derivations:', num_derivations)

    if num_derivations == 0:
        error["type"] = "NoParseFound"
        error["message"] = "Parser ran without error, but no parses found"
        return [], []

    try:
        if num_derivations > max_derivations:
            return sample_trees_from_parser_output(parser_output_filename)
        else:
            return all_trees_from_parser_output(parser_output_filename)
    except Exception as e:
        error["type"] = "ReadingTreeOutputFailed"
        error["message"] = str(e)
        return [], []

def run_parser(sent, tagged_filename, parser_output_filename, timeout=360, max_derivations=30000, error=None):
    # Run parser and get intermediate output
    if not os.path.exists(parser_output_filename):
        # Send sentence on stdin
        p1_cmd = shlex.split("echo \"%s\"" % sent)
        p1 = subprocess.Popen(p1_cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

        # Get the syntax entries
        p2_cmd = shlex.split("../bin/syn_get.bin ../data/english/english.grammar ../lib/xtag.prefs")
        p2 = subprocess.Popen(p2_cmd, stdin=p1.stdout, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

        # Filter the syntax entries by part-of-speech
        p3_cmd = shlex.split("../bin/tagger_filter %s" % tagged_filename)
        p3 = subprocess.Popen(p3_cmd, stdin=p2.stdout, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

        # Run the parser
        p4_cmd = shlex.split("../bin/nbest_parser.bin ../data/english/english.grammar ../lib/xtag.prefs")
        p4 = subprocess.Popen(p4_cmd, stdin=p3.stdout, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
        try:
            parser_output, err = p4.communicate(timeout=timeout)
        except subprocess.TimeoutExpired:
            p4.kill()
            error["type"] = "ParserTimeout"
            error["message"] = "Parser took longer than %d seconds" % timeout
            return [], []

        with open(parser_output_filename, 'w') as f:
            f.write(parser_output)

    return handle_parser_output(parser_output_filename, max_derivations, error)

def get_subtree_set(tree):
    #subtree_set = set([tuple([l for l in i.leaves()]) for i in tree.subtrees(filter=lambda s: s.height() >= 3)])
    subtree_set = set([tuple([l for l in i.leaves() if 'epsilon_' not in l and 'PRO' not in l]) for i in tree.subtrees()])
    subtree_set = set([s for s in subtree_set if len(s) >= 2])
    return subtree_set

def fscore_distance(tree1, tree2):
    subtree_set1 = get_subtree_set(tree1)
    subtree_set2 = get_subtree_set(tree2)

    print(len(subtree_set1), len(subtree_set2), subtree_set1)

    precision = 1 - (len(subtree_set1 - subtree_set2) / float(len(subtree_set1)))
    recall = 1 - (len(subtree_set2 - subtree_set1) / float(len(subtree_set2)))

    if precision + recall == 0:
        return 0

    fscore = 2 * precision * recall / (precision + recall)
    return fscore

def distance(tree1, tree2):
    subtree_set1 = get_subtree_set(tree1)
    subtree_set2 = get_subtree_set(tree2)
    return len(subtree_set1 & subtree_set2)

def get_best_parse(fileid, i, parse, tagged):
    print(fileid, i, len(tagged), len(parse))

    filename = 'parse_trees/%s_%d.txt' % (fileid.split('/')[-1].split('.')[0], i)
    parser_output_filename = 'parser_output/%s_%d.txt' % (fileid.split('/')[-1].split('.')[0], i)
    timeout = 300
    
    if os.path.exists(filename):
        tree_dict =  json.load(open(filename, 'r'))
        if 'error' in tree_dict and tree_dict['error'] == "ParserTimeout" and str(timeout) not in tree_dict['message']:
            pass
        else:
            print('parse already exists')
            return 

    tagged = flip_word_pos(tagged)
    tagged = remove_none_from_tagged(tagged)
    tagged = merge_tagged_nnps(tagged)
    tagged = lowercase_tagged_first_word(tagged)
    tagged = remove_punc_from_tagged(tagged)
    sent = tagged_to_sent(tagged)

    try:
        gold_tree = merge_tree_nnps(parse)
        gold_tree = remove_punc_from_tree(gold_tree)
        gold_tree = lowercase_tree_first_word(gold_tree)
    except IndexError:
        print("Index error in merge_tree_nnps")
        return

    tagged_filename = create_tmp_tagged_file(fileid, i, tagged_sent_to_str(tagged))

    error = {'type': None, 'message': None}
    parse_trees, deriv_trees = run_parser(sent_to_str(sent), tagged_filename, parser_output_filename, max_derivations=30000, timeout=timeout, error=error)

    if error['type'] is not None:
        tree_dict = error
        print(error)
    else:
        best_parse, best_deriv = find_closest_tree(gold_tree, parse_trees, deriv_trees)
        tree_dict = {"parse": str(best_parse), "deriv": str(best_deriv)}

    with open(filename, 'w') as f:
        f.write(json.dumps(tree_dict))

def find_closest_tree(gold_tree, parse_trees, deriv_trees):
    trees = zip(parse_trees, deriv_trees)
    best_parse, best_deriv, best_val = None, None, 0
    subtree_set1 = get_subtree_set(gold_tree)

    for test_parse_tree, test_deriv_tree in trees:
        subtree_set2 = get_subtree_set(test_parse_tree)
        similarity = len(subtree_set1 & subtree_set2)
        if similarity > best_val:
            best_parse, best_deriv, best_val = test_parse_tree, test_deriv_tree, similarity

    return best_parse, best_deriv

def parse_wsj_file(fileid, parsed, tagged):
    corpus = zip(parsed, tagged)
    for i, parse, tagged in enumerate(corpus):
        get_best_parse(fileid, i, parse, tagged)

def parse_wsj(processes=8):
    ptb = LazyCorpusLoader( # Penn Treebank v3: WSJ portions
        'ptb', CategorizedBracketParseCorpusReader, r'wsj/\d\d/wsj_\d\d\d\d.mrg',
        cat_file='allcats.txt', tagset='wsj')

    fileids = ptb.fileids()
    params = []
    for f in fileids:
        corpus = zip(ptb.parsed_sents(f), ptb.tagged_sents(f))
        for i, (parsed, tagged) in enumerate(corpus):
            params.append((f, i, parsed, tagged))

    p = Pool(processes)
    p.starmap(get_best_parse, sorted(params, key=lambda x: (x[0], x[1])))

if __name__ == '__main__':
    #parse_wsj(2)
