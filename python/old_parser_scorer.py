import nltk, json, re, os
import shlex, subprocess
from nltk.corpus.util import LazyCorpusLoader
from nltk.corpus.reader import CategorizedBracketParseCorpusReader
from nltk.tree import Tree, ParentedTree, ImmutableTree
import matplotlib.pyplot as plt
from multiprocessing import Pool

from parser import WSJEntry
from parser_constants import Constants


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

def get_subtree_set(tree):
    #subtree_set = set([tuple([l for l in i.leaves()]) for i in tree.subtrees(filter=lambda s: s.height() >= 3)])
    subtree_set = set([tuple([l for l in i.leaves() if 'epsilon_' not in l and 'PRO' not in l]) for i in tree.subtrees()])
    subtree_set = set([s for s in subtree_set if len(s) >= 2])
    return subtree_set

def distance(tree1, tree2):
    subtree_set1 = get_subtree_set(tree1)
    subtree_set2 = get_subtree_set(tree2)
    return len(subtree_set1 & subtree_set2)

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

def score(file_id, sent_num, parse_tree, max_derivations=30000):
    entry = WSJEntry(file_id, sent_num, parse_tree)
    entry = entry.preprocess()

    print(entry)

    status_dict = json.load(open(entry.parser_status_filename(), 'r'))
    status = status_dict["action"]

    if status != 'SUCCESS':
        return

    if os.path.exists(entry.parser_parse_filename()):
        return 

    error = {'type': None, 'message': None}
    parse_trees, deriv_trees = handle_parser_output(entry.parser_output_filename(), max_derivations=max_derivations, error=error)

    if error['type'] is not None:
        tree_dict = error
        print(error)
    else:
        best_parse, best_deriv = find_closest_tree(entry.parse_tree, parse_trees, deriv_trees)
        tree_dict = {"parse": str(best_parse), "deriv": str(best_deriv)}

    with open(entry.parser_parse_filename(), 'w') as f:
        f.write(json.dumps(tree_dict))

def main(processes=1):
    ptb = Constants().ptb
    fileids = list(ptb.fileids())

    params = []
    for fileid in fileids[:10]:
        for sent_num, parse_tree in enumerate(ptb.parsed_sents(fileid)):
            params.append((fileid, sent_num, parse_tree))

    if processes > 1:
        p = Pool(processes)
        p.starmap(score, sorted(params, key=lambda x: (x[0], x[1])))
    else:
        for param in params:
            score(*param)

if __name__ == '__main__':
    main(3)
