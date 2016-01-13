import glob, json, nltk, re
from collections import defaultdict, deque

def treebank_name_to_tree_name(tree_name):
    return tree_name.replace('\x02', 'alpha').replace('\x03', 'beta')

def label_to_tree_word_loc(label):
    tree_name = re.search('(.*)\[', label).group(1)
    word = re.search('\[(.*?)\]', label).group(1)
    loc_match = re.search('\<(.*?)\>', label)
    loc = loc_match.group(1) if loc_match is not None else None
    return tree_name, word, loc

def process_deriv_tree(deriv_tree, global_counts, context_sensitive_counts):
    queue = deque([(None, deriv_tree)])
    while len(queue) > 0:
        parent, d_tree = queue.popleft()

        if isinstance(d_tree, nltk.Tree):
            tree_label = d_tree.label()
            queue += [(d_tree, c) for c in d_tree]
        elif isinstance(d_tree, str):
            tree_label = d_tree
        else:
            raise ValueError

        if parent is None:
            p_tree_name = None
        else:
            p_tree_name, p_word, p_loc = label_to_tree_word_loc(parent.label())

        tree_name, word, loc = label_to_tree_word_loc(tree_label)
        global_counts[tree_name] += 1
        context_sensitive_counts[(p_tree_name, loc, tree_name)] += 1

def count_corpus(treedir='parse_trees', count_file='tree_counts.txt'):
    # just stores counts of each tree of form {'treename': count}
    global_counts = defaultdict(int)
    # stores context sensitive counts of each tree of form {('parent_treename', 'action location', 'treename'): count}
    context_sensitive_counts = defaultdict(int)

    parses = 0
    for treefile in glob.glob(treedir + '/*.txt'):
        tree_dict = json.load(open(treefile, 'r'))

        if 'deriv' not in tree_dict or tree_dict['deriv'] == 'None':
            continue

        parses += 1

        deriv_tree_str = tree_dict['deriv']
        deriv_tree = nltk.Tree.fromstring(deriv_tree_str)
        process_deriv_tree(deriv_tree, global_counts, context_sensitive_counts)

    counts_dict = {
        'global_counts': dict(global_counts), 
        'context_sensitive_counts': dict(context_sensitive_counts)
    }

    with open(count_file, 'w') as f:
        f.write(str(counts_dict))

    print("Total parses: %d, Total_trees: %d" % (parses, sum(global_counts.values())))

def investigate_counts(count_file='tree_counts.txt'):
    with open(count_file, 'r') as f:
        counts_dict = eval(f.read())

    global_counts = counts_dict['global_counts']
    for tree, count in sorted(global_counts.items(), key=lambda x: x[1], reverse=True):
        print(treebank_name_to_tree_name(tree), count)

def have_tree_semantics(deriv_tree, tree_set):
    queue = deque([deriv_tree])
    while len(queue) > 0:
        d_tree = queue.popleft()

        if isinstance(d_tree, nltk.Tree):
            tree_label = d_tree.label()
            queue += [c for c in d_tree]
        elif isinstance(d_tree, str):
            tree_label = d_tree
        else:
            raise ValueError

        tree_name, word, loc = label_to_tree_word_loc(tree_label)
        if tree_name not in tree_set:
            return False
    return True

def count_successful_parses(treedir='parse_trees'):
    result_counts = defaultdict(int)

    for treefile in glob.glob(treedir + '/*.txt'):
        tree_dict = json.load(open(treefile, 'r'))

        if 'type' not in tree_dict:
            tree_dict['type'] = 'success'

        result_counts[tree_dict['type']] += 1 

    print(result_counts)

def parse_deriv_semantics(treedir='parse_trees'):
    tree_set = set([
        "alphaNXN", # 18954
        "betaDnx", # 5663
        "betaNn", # 5156
        "betasPU", # 4607
        "betaVvx", # 3500
        "betaAn", # 2937
        "betanxPnx", # 1797
        "alphanx0Vnx1", # 1577
        "alphanx1V", # 1018
        "alphanx0V", # 205
        "betavxPnx", # 1636
        "betasPUs", # 929
        "betanxPUnx", # 876
        "betaARBvx", # 766
        "betanx0Vs1", # 754
        "alphanx0BEnx1", # 570
        "alphanx0Pnx1", # 535
        "betanxGnx", # 500
        "betaVs", # 496
        "alphanx0Vnx1-PRO", # 470
        "betaCOMPs", # 388
        "betanxPUs", # 384
        #"betanx1CONJnx2", # 384
        "alphanx0N1", # 375
    ])

    count = 0
    for treefile in glob.glob(treedir + '/*.txt'):
        tree_dict = json.load(open(treefile, 'r'))

        if 'deriv' not in tree_dict:
            continue

        deriv_tree_str = tree_dict['deriv']
        deriv_tree = nltk.Tree.fromstring(deriv_tree_str)
        parse_tree_str = tree_dict['parse']
        parse_tree = nltk.Tree.fromstring(parse_tree_str)

        if have_tree_semantics(deriv_tree, tree_set):
            count+=1
            if count > 1:
                deriv_tree.draw()
                print(str(deriv_tree))
                parse_tree.draw()
                exit()

def count_semantic_parses(treedir='parse_trees'):
    count = 0
    tree_set = set([
        "alphaNXN", # 31171,
        "betaDnx", # 9319,
        "betaNn", # 8469,
        "betasPU", # 7642,
        "betaVvx", # 5844,
        "betaAn", # 4869,
        "betanxPnx", # 2981,
        "betavxPnx", # 2642,
        "alphanx0Vnx1", # 2577,
        "alphanx1V", # 1715,
        "betasPUs", # 1538,
        "betanxPUnx", # 1431,
        "betaARBvx", # 1233,
        "betanx0Vs1", # 1225,
        "alphanx0BEnx1", # 931,
        "alphanx0Pnx1", # 879,
        "betaVs", # 837,
        "betanxGnx", # 803,
        "alphanx0Vnx1-PRO", # 794,
        "betanxPUs", # 666,
        "alphanx0Ax1", # 621,
        "alphanx0N1", # 619,
        "betaCOMPs", # 607,
        "alphanx0V", # 345,
        "betanx1CONJnx2", # 656,
        "betavxPs", # 614,
        "betas1CONJs2", # 584,
        "betaN1nx1V", # 576,
        #"betavxARB", # 564,
        "alphanx1V-PRO", # 522,
        "alphas0N1", # 489,
        "alphaN1s0", # 478,
        #"betaN0nx0Vnx1", # 416,
        #"betaPUs", # 389,
        "alphaPu", # 378,
        #"betan1CONJn2", # 355,
        #"alphaN", # 355,
        #"betaN0nx0Pnx1", # 327,
        #"betaARBs", # 322,
        "alphanx2Vnx1", # 303,
        "alphas0Ax1", # 295,
        #"betaARBa", # 275,
        #"betaN0nx0N1", # 270,
        #"betanx0Vs1-PRO", # 263,
        #"alphaD", # 239,
        #"betaPnxs", # 233,
        #"alphaXGnx0Vs1", # 226,
        #"betaCONJs", # 221,
        #"betaNPnx1nx0Pnx1", # 215,
        #"betaN1nx0Pnx1", # 207,
        #"betaNEGvx", # 194,
        #"alphanx0N1-PRO", # 192,
        #"alphanx0Pnx1-PRO", # 183,
        #"alphaDnx0V", # 182,
        #"alphaNXnxG", # 181,
        #"alphanx0Ax1-PRO", # 174,
        #"betasPUnx", # 172,
        #"alphaPXPnx", # 156,
        #"betaN0nx0Ax1", # 155,
        #"alphaNXNs", # 154,
        #"alphanx0V-PRO", # 147,
        #"alphaXW0nx0Vs1", # 131,
        #"betaPss", # 130,
        #"betaN0nx0V", # 127,
        #"betanxN", # 117,
        #"betaN0nx0Vs1", # 108,
        #"alphaP", # 102,
        #"betaVergativen", # 100,
        #"alphanx0Vplnx1", # 95,
        #"alphanx0Vnx2nx1", # 91,
        #"betaN1nx0Vnx1", # 88,
        #"betaCnxPnx", # 82,
        #"betaXnx0Vs1", # 82,
        #"betavxN", # 81,
        #"betaENc1nx1V", # 80,
        #"betaARBnx", # 80,
        #"betaXInx0Vs1", # 80,
        #"betanx1Vs2", # 78,
        #"alphaDnx0Vs1", # 78,
        #"betaXNc0nx0Vs1", # 76,
        #"alphaW0nx0Vnx1", # 75,
        #"alphanx2Vnx1-PRO", # 70,
        #"betaspuPs", # 69,
        #"betaa1CONJa2", # 68,
        #"alphaA", # 68,
        #"alphapW1nx0Pnx1", # 68,
        #"betanx0Vnx1s2", # 66,
        #"betanxARB", # 63,
        #"alphaEW1nx1V", # 57,
        #"betaspuPnx", # 56,
        #"betaARBpx", # 56,
        #"alphanx0VPnx1", # 51,
        #"betaXNcnx0Vs1", # 50,
        #"alphaW1nx1V", # 49,
        #"betaARBd", # 44,
        #"betanx1Vs2-PRO", # 44,
        #"betaspuARB", # 43,
        #"alphanx1Vpl", # 41,
        #"alphaAXA", # 41,
        #"alphas0Vnx1", # 40,
        #"betaARBarb", # 38,
        #"betaCARBarb", # 34,
        #"alphanx0Px1", # 33,
        #"alphanx1Vpnx2", # 32,
        #"alphanx0Vax1", # 32,
        #"betaNs", # 31,
        #"betaNvx", # 31,
        #"betaNpxnx1V", # 31,
        #"betapuARBpuvx", # 30,
        #"alphaDnxG", # 27,
        #"alphanx0Vplnx1-PRO", # 27,
        #"betaCARBa", # 26,
        #"alphaW1nx0Vnx1", # 25,
        #"betapunxVpuvx", # 25,
        #"betanxVpus", # 24,
        #"betanxPs", # 24,
        #"betaVvx-adj", # 23,
        #"alphanx0Vnx2nx1-PRO", # 22,
        #"betaNpxnx0Vnx1", # 22,
        #"alphaW0nx0Pnx1", # 22,
        #"alphanx0VPnx1-PRO", # 21,
        #"betanxARBs", # 21,
        #"betaNpxs0N1", # 20,
        #"alphanx0Vnx1pnx2", # 20,
        #"alphanx0Vnx1Pnx2", # 20,
        #"betaN1nx1Vpl", # 20,
        #"alphanx1VPnx2", # 19,
        #"betaN1nx2Vnx1", # 19,
        #"betapx1CONJpx2", # 19,
        #"alphanx0Vnx1pl", # 18,
        #"betanxP", # 18,
        #"betavPU", # 18,
        #"betaspuVnx", # 17,
        #"alphaW0nx0V", # 16,
        #"alphaW1nx0Pnx1", # 16,
        #"betaaxPnx", # 16,
        #"betaN0nx0Px1", # 15,
        #"betaNEGa", # 15,
        #"betaN2nx0Vnx2nx1", # 15,
        #"alphanx1Vpl-PRO", # 15,
        #"alphaW0s0N1", # 14,
        #"betaN2nx2Vnx1", # 14,
        #"alphanx0N1s1", # 13,
        #"alphaAd", # 13,
        #"betadD", # 12,
        #"betaN0nx0Vplnx1", # 12,
        #"betaNpxnx0Pnx1", # 12,
        #"betaN0nx0Vnx2nx1", # 11,
        #"alphaW1nx2Vnx1", # 11,
        #"betapuPpuvx", # 11,
        #"alphanx0Vpl", # 11,
        #"alphaW0nx0Ax1", # 11,
        #"alphas0Vs1", # 11,
        #"betaNpxnx0N1", # 11,
        #"betaXnx0Vs1-PRO", # 11,
        #"betaENcnx1V", # 10,
        #"alphaW1nx0Vs1", # 10,
        #"alphaW0nx0Vs1", # 10,
        #"alphanx0Vpnx1", # 9,
        #"betanxPUa", # 9,
        #"alphaAV", # 9,
        #"alphanx0Vpl-PRO", # 8,
        #"betaNpxnx0Ax1", # 8,
        #"betaN0nx0VPnx1", # 8,
        #"betanx0Vnx1s2-PRO", # 8,
        #"betaNpxs0Ax1", # 8,
        #"betapuVnxpuvx", # 7,
        #"betaN1nx1Vs2", # 7,
        #"alphanx0lVN1", # 6,
        #"alphaEnx1V", # 6,
        #"betaNpxnx2Vnx1", # 6,
        #"alphanx1VP", # 6,
        #"betaN0nx0Vpl", # 6,
        #"alphaW1nx0N1", # 6,
        #"alphanx1Vpnx2-PRO", # 6,
        #"betaNpxnx0Vs1", # 6,
        #"betaARBarbs", # 6,
        #"betaspunxV", # 6,
        #"betaaARB", # 6,
        #"betaNpxnx0V", # 6,
        #"alphaW0nx0Vnx2nx1", # 6,
        #"alphanx0Px1-PRO", # 6,
        #"betaN1nx1Vpnx2", # 6,
        #"betaN0nx0Vnx1s2", # 6,
        #"betaN0nx0Vnx1pnx2", # 5,
        #"alphaW0nx0Vplnx1", # 5,
        #"alphaW0s0Ax1", # 5,
        #"betaarb1CONJarb2", # 5,
        #"alphaPW1nx0Px1", # 5,
        #"alphanx0Vax1-PRO", # 5,
        #"betap1CONJp2", # 5,
        #"betavxP", # 5,
        #"betaEN1nx1V", # 5,
        #"alphanx1VPnx2-PRO", # 4,
        #"alphaW2nx0Vnx2nx1", # 4,
        #"betaXN0nx0Vs1", # 4,
        #"betanxnxARB", # 4,
        #"alphanx0Vnx1pnx2-PRO", # 4,
        #"betavxnxARB", # 4,
        #"betavxDN", # 4,
        #"betaN1nx0Vplnx1", # 4,
        #"betavxDA", # 3,
        #"alphaW0nx0N1", # 3,
        #"alphanx0Vnx1pl-PRO", # 3,
        #"betaax1CONJax2", # 3,
        #"betad1CONJd2", # 3,
        #"betavpunxVpu", # 3,
        #"betaN0nx0Vpnx1", # 3,
        #"alphaW1nx0Vnx2nx1", # 3,
        #"betavxARBPnx", # 3,
        #"alphaEnx1V-PRO", # 3,
        #"alphanx1Vp-PRO", # 3,
        #"betaN0nx0Vnx1pl", # 3,
        #"alphanx0Vpnx1-PRO", # 2,
        #"betanARB", # 2,
        #"betavpuVnxpu", # 2,
        #"betapunxVnx1puvx", # 2,
        #"betaARBPnxs", # 2,
        #"alphaAXAs", # 2,
        #"alphaW2nx0Vnx1s2", # 2,
        #"betaN1nx1VPnx2", # 2,
        #"betaN0nx0Vax1", # 2,
        #"alphaW0s0Vnx1", # 2,
        #"alphaDnx0Vpl", # 2,
        #"betaN0nx0N1s1", # 2,
        #"betaN1nx0Vnx1pnx2", # 2,
        #"betaN1nx1VP", # 2,
        #"betapunxVnx1pus", # 2,
        #"betaARBpx1CONJpx2", # 2,
        #"alphanx1VP-PRO", # 2,
        #"betaN1nx0Vnx1s2", # 2,
        #"betaN2nx0Vnx1pnx2", # 2,
        #"betaNpxItVad1s2", # 1,
        #"betaNpxnx1VP", # 1,
        #"alphaW1ItVnx1s2", # 1,
        #"betaRNc1nx1VPnx2", # 1,
        #"betanx1CONJARBnx2", # 1,
        #"betaN1nx0Vnx2nx1", # 1,
        #"betaNpxnx0Vplnx1", # 1,
        #"betavxARBPs", # 1,
        #"alphapW2nx0Vnx1pnx2", # 1,
        #"alphanx1Vp", # 1,
        #"betapPU", # 1,
        #"betaNPvx", # 1,
        #"betaspuARBPs", # 1,
        #"betaRNc0nx0Vnx1Pnx2", # 1,
        #"betaNpxnx0Vpl", # 1,
        #"alphanx0lVnx2N1", # 1,
        #"alphaW2nx2Vnx1", # 1,
        #"betaNPnx", # 1,
        #"alphaDnx0VPnx1", # 1,
        #"alphaW1ItVad1s2", # 1,
        #"alphaREW1nx1VPnx2", # 1,
        #"alphaPXP", # 1,
        #"betaN2nx1Vpnx2", # 1,
        #"betaN0nx0lVN1", # 1,
        #"betaN2nx0Vnx1Pnx2", # 1,
        #"betaN1nx0VPnx1", # 1,
        #"betasnxARB", # 1,
        #"alphaW0nx0Vpl", # 1,
        #"alphaW0nx0Px1", # 1,
        #"alphas0V", # 1,
        #"betaNpxnx0Vax1", # 1,
        #"alphaRW1nx1VA2", # 1,
        #"betanxARBPnx", # 1,
        #"betaDNpx", # 1,
        #"alphanx0N1s1-PRO", # 1,
        #"betaDAax", # 1,
        #"betaNpxnx1Vpl", # 1,
        #"alphaRW1nx0Vnx1Pnx2", # 1,
        #"alphaW1nx0Vplnx1", # 1,
        #"alphanx0Vnx1Pnx2-PRO", # 1,
        #"alphaW1nx1Vpl", # 1,
        #"alphaRnx1VPnx2", # 1,
        #"betaRNc2nx1VPnx2", # 1,
        #"betaN0nx0Vnx1Pnx2", # 1,
        #"alphaW1nx0Vnx1Pnx2", # 1,
    ])

    for treefile in glob.glob(treedir + '/*.txt'):
        tree_dict = json.load(open(treefile, 'r'))

        if 'deriv' not in tree_dict:
            continue

        deriv_tree_str = tree_dict['deriv']
        deriv_tree = nltk.Tree.fromstring(deriv_tree_str)
        if have_tree_semantics(deriv_tree, tree_set):
            count += 1
    print('Number of trees with semantic parses: ', count)

if __name__ == '__main__':
    #count_corpus()
    #investigate_counts()
    count_semantic_parses()
    #parse_deriv_semantics()
    count_successful_parses()
