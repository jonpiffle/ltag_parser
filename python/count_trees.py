import glob, json, nltk, re
from collections import defaultdict, deque
from itertools import tee

def treebank_name_to_tree_name(tree_name):
    return tree_name.replace('\x02', 'alpha').replace('\x03', 'beta')

def tree_name_to_treebank_name(tree_name):
    if tree_name is None:
        return None
    return tree_name.replace('alpha', '\x02').replace('beta', '\x03')

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
        global_counts[tree_name_to_treebank_name(tree_name)] += 1
        context_sensitive_counts[(tree_name_to_treebank_name(p_tree_name), loc, tree_name_to_treebank_name(tree_name))] += 1

        # Store totals for use in normalization
        global_counts["__TOTAL__"] += 1
        context_sensitive_counts[(tree_name_to_treebank_name(p_tree_name), loc, "__TOTAL__")] += 1

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
    '''
    annotated = set([
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
        "alphanx1V-PRO", # 522,
        "alphas0N1", # 489,
        "alphaN1s0", # 478,
        "alphaPu", # 378,
        "alphanx2Vnx1", # 303,
        "alphas0Ax1", # 295
        "betaPUs", # 2628 
        "betaARBs", # 2015 
        "betaCONJs", # 1966 
        #"alphanx0Pnx1-PRO", # 1483 
    ])
    '''

    annotated = set([
        "alphaNXN", # 31171,
        "betaDnx", # 9319,
        "betaNn", # 8469,
        #"betasPU", # 7642,
        #"betaVvx", # 5844,
        "betaAn", # 4869,
        #"betanxPnx", # 2981,
        #"betavxPnx", # 2642,
        "alphanx0Vnx1", # 2577,
        "alphanx1V", # 1715,
        #"betasPUs", # 1538,
        #"betanxPUnx", # 1431,
        #"betaARBvx", # 1233,
        #"betanx0Vs1", # 1225,
        #"alphanx0BEnx1", # 931,
        #"alphanx0Pnx1", # 879,
        #"betaVs", # 837,
        #"betanxGnx", # 803,
        "alphanx0Vnx1-PRO", # 794,
        #"betanxPUs", # 666,
        #"alphanx0Ax1", # 621,
        #"alphanx0N1", # 619,
        #"betaCOMPs", # 607,
        "alphanx0V", # 345,
        #"betanx1CONJnx2", # 656,
        #"betavxPs", # 614,
        #"betas1CONJs2", # 584,
       # "betaN1nx1V", # 576,
        "alphanx1V-PRO", # 522,
        #"alphas0N1", # 489,
        #"alphaN1s0", # 478,
        #"alphaPu", # 378,
        "alphanx2Vnx1", # 303,
        #"alphas0Ax1", # 295
        #"betaPUs", # 2628 
        #"betaARBs", # 2015 
        #"betaCONJs", # 1966 
        "alphanx0Pnx1-PRO", # 1483 
    ])
    
    tree_set = [
        "alphaNXN", # 78483 
        "betaDnx", # 31668 
        "betaNn", # 21141 
        "betaVvx", # 15070 
        "betaAn", # 13792 
        "betanxPnx", # 8997 
        "betavxPnx", # 8917 
        "betasPUs", # 5689 
        "alphanx0Vnx1", # 4493 
        "alphanx1V", # 4420 
        "alphanx1V-PRO", # 4170 
        "alphaN1s0", # 3662 
        "betanx0Vs1", # 3615 
        "alphas0N1", # 3495 
        "betaARBvx", # 3256 
        "betanxPUnx", # 3140 
        "alphanx0Vnx1-PRO", # 3082 
        "betaVs", # 2668 
        "betaPUs", # 2628 
        "betanx1CONJnx2", # 2568 
        "betanxGnx", # 2489 
        "betavxPs", # 2299 
        "betas1CONJs2", # 2169 
        "betaARBs", # 2015 
        "betaCONJs", # 1966 
        "alphanx0Pnx1", # 1875 
        "betanxPUs", # 1821 
        "alphanx0BEnx1", # 1813 
        "betaCOMPs", # 1786 
        "alphanx0N1-PRO", # 1734 
        "betaPnxs", # 1627 
        "betavxARB", # 1558 
        "alphanx0Pnx1-PRO", # 1483 
        "alphanx0N1", # 1380 
        "betasPU", # 1371 
        "betanx0Vs1-PRO", # 1318 
        "betaPss", # 1228 
        "betaN1nx1V", # 1217 
        "alphanx0Ax1", # 1171 
        "alphanx0Ax1-PRO", # 1166 
        "alphas0Ax1", # 1041 
        "alphanx0V", # 987 
        "alphaD", # 941 
        "alphaXGnx0Vs1", # 937 
        "alphaPu", # 889 
        "alphanx0V-PRO", # 848 
        "betaN0nx0Vnx1", # 720 
        "alphaPXPnx", # 687 
        "betaARBa", # 687 
        "betan1CONJn2", # 678 
        "alphaN", # 678 
        "alphaDnx0V", # 630 
        "alphanx2Vnx1", # 609 
        "betasPUnx", # 607 
        "betaXnx0Vs1", # 568 
        "betavxN", # 528 
        "betaN0nx0N1", # 502 
        "alphaNXNs", # 498 
        "betaNPnx1nx0Pnx1", # 496 
        "betaN0nx0Pnx1", # 482 
        "betaNEGvx", # 461 
        "alphanx2Vnx1-PRO", # 405 
        "alphaNXnxG", # 402 
        "alphaXW0nx0Vs1", # 384 
        "alphaDnx0Vs1", # 340 
        "betaVergativen", # 325 
        "alphapW1nx0Pnx1", # 313 
        "betaN1nx0Pnx1", # 301 
        "betaXNc0nx0Vs1", # 295 
        "betanxN", # 293 
        "betaN0nx0Ax1", # 290 
        "betaN0nx0V", # 286 
        "alphaP", # 260 
        "betaCnxPnx", # 239 
        "alphaW1nx1V", # 234 
        "betaARBnx", # 233 
        "betaENc1nx1V", # 232 
        "betanx1Vs2-PRO", # 205 
        "betanx1Vs2", # 203 
        "betaspuPs", # 203 
        "betaNpxs0N1", # 193 
        "betaN1nx0Vnx1", # 188 
        "betaNs", # 186 
        "betaa1CONJa2", # 180 
        "alphaA", # 176 
        "betaARBpx", # 175 
        "alphaW0nx0Vnx1", # 172 
        "betaXInx0Vs1", # 163 
        "betaARBd", # 154 
        "betaXNcnx0Vs1", # 150 
        "betaNpxnx1V", # 141 
        "alphas0Vnx1", # 136 
        "alphanx0Vplnx1", # 135 
        "betaN0nx0Vs1", # 134 
        "betanxARB", # 131 
        "alphanx0Vnx2nx1", # 130 
        "alphanx1Vpl", # 117 
        "betaNvx", # 113 
        "alphaEW1nx1V", # 112 
        "betaARBarb", # 109 
        "alphanx0Vplnx1-PRO", # 106 
        "betaspuPnx", # 105 
        "alphanx1Vpl-PRO", # 104 
        "betaCARBarb", # 101 
        "betaspuARB", # 100 
        "alphaW1nx0Vnx1", # 99 
        "betaspunxV", # 99 
        "betanx0Vnx1s2", # 94 
        "betaXnx0Vs1-PRO", # 89 
        "betanxPs", # 86 
        "betaCARBa", # 84 
        "betaspuVnx", # 80 
        "alphaAXA", # 80 
        "alphaW0nx0Vs1", # 78 
        "betaVvx-adj", # 76 
        "alphaDnxG", # 73 
        "alphaW0s0N1", # 73 
        "alphaW0nx0Pnx1", # 72 
        "alphaW0nx0V", # 70 
        "alphas0Vs1", # 68 
        "alphanx0Px1-PRO", # 67 
        "betaNpxnx0Vnx1", # 66 
        "betanx0Vnx1s2-PRO", # 64 
        "alphanx0Px1", # 64 
        "betanxVpus", # 63 
        "betavPU", # 60 
        "alphanx0VPnx1", # 56 
        "betapx1CONJpx2", # 56 
        "alphanx1Vpnx2-PRO", # 53 
        "betaaxPnx", # 52 
        "betapuARBpuvx", # 52 
        "betanxARBs", # 52 
        "alphaW1nx0Pnx1", # 52 
        "betaNpxs0Ax1", # 52 
        "alphanx0Vnx2nx1-PRO", # 51 
        "betanxP", # 51 
        "alphanx0Vnx1pnx2", # 48 
        "betavxnxARB", # 48 
        "alphanx0Vax1", # 46 
        "alphanx1Vpnx2", # 45 
        "alphaEnx1V", # 45 
        "alphaW1nx0N1", # 44 
        "betadD", # 43 
        "betaNEGa", # 42 
        "betaNpxnx0Pnx1", # 41 
        "alphaAd", # 40 
        "betaNpxnx0V", # 39 
        "alphanx0VPnx1-PRO", # 37 
        "alphaEnx1V-PRO", # 37 
        "alphaW0nx0N1", # 37 
        "betaN1nx2Vnx1", # 36 
        "betaNpxnx0Ax1", # 35 
        "betaENcnx1V", # 35 
        "alphaW0nx0Ax1", # 35 
        "betaN2nx2Vnx1", # 34 
        "alphanx0Vpl-PRO", # 31 
        "alphanx0N1s1-PRO", # 31 
        "alphaW1nx0Vs1", # 30 
        "alphanx1VPnx2", # 30 
        "alphanx0N1s1", # 29 
        "alphaAV", # 29 
        "alphaW0s0Ax1", # 27 
        "alphanx0Vpl", # 27 
        "alphanx1VP-PRO", # 26 
        "alphaW1nx2Vnx1", # 26 
        "betanxPUa", # 25 
        "betaN0nx0Px1", # 25 
        "betaNpxnx0N1", # 24 
        "alphaW2nx0Vnx2nx1", # 24 
        "betapuPpuvx", # 23 
        "betaN1nx1Vpl", # 22 
        "betavxP", # 22 
        "alphanx0Vnx1pl", # 22 
        "betaN0nx0Vplnx1", # 22 
        "betaN2nx0Vnx2nx1", # 21 
        "alphanx0Vnx1pl-PRO", # 21 
        "betaarb1CONJarb2", # 20 
        "alphanx1VPnx2-PRO", # 20 
        "alphanx1VP", # 20 
        "alphanx0Vax1-PRO", # 20 
        "alphaPW1nx0Px1", # 19 
        "alphaDEnx1V", # 18 
        "betanxnxARB", # 18 
        "betapunxVpuvx", # 18 
        "betaXN0nx0Vs1", # 18 
        "betaARBPss", # 17 
        "alphanx0lVN1", # 17 
        "alphaW2nx2Vnx1", # 16 
        "alphanx0Vnx1pnx2-PRO", # 16 
        "alphaW1nx0Vnx2nx1", # 15 
        "betaNpxnx0Vs1", # 14 
        "betaARBarbs", # 14 
        "betap1CONJp2", # 14 
        "betaN1nx1Vpnx2", # 14 
        "alphanx0Vnx1Pnx2", # 13 
        "betapunxVnx1pus", # 12 
        "betad1CONJd2", # 12 
        "alphapW1ItVpnx1s2", # 12 
        "betaax1CONJax2", # 11 
        "betavxARBPnx", # 11 
        "betaN1nx1Vs2", # 11 
        "betasnxARB", # 10 
        "alphanx0Vpnx1", # 10 
        "betaEN1nx1V", # 10 
        "betaspunxVnx1", # 10 
        "alphanx0Vpnx1-PRO", # 9 
        "betanARB", # 9 
        "betaN0nx0Vnx2nx1", # 9 
        "betaN0nx0Vpl", # 9 
        "betaaARB", # 9 
        "alphaW0nx0Vplnx1", # 8 
        "betaN0nx0Vax1", # 8 
        "alphaDnx0VPnx1", # 7 
        "alphanx0Vnx1Pnx2-PRO", # 7 
        "betaN1nx0Vnx2nx1", # 7 
        "betaARBPnxs", # 7 
        "alphapW2nx0Vnx1pnx2", # 7 
        "alphaW1ItVnx1s2", # 7 
        "alphanx1Vp-PRO", # 7 
        "alphaAXAs", # 6 
        "alphaW0nx0Px1", # 6 
        "alphaDnx0Vpl", # 6 
        "betaN1nx0Vplnx1", # 6 
        "betavpunxVpu", # 6 
        "alphaPXP", # 5 
        "betaN0nx0VPnx1", # 5 
        "betavpuVnxpu", # 5 
        "alphanx1Vp", # 5 
        "betaN0nx0Vnx1s2", # 5 
        "alphaW0nx0Vnx2nx1", # 5 
        "betaDAax", # 4 
        "alphaW1nx0Vnx1s2", # 4 
        "betaXNpxnx0Vs1", # 4 
        "alphaW1nx1Vs2", # 4 
        "alphaW1nx0Vplnx1", # 4 
        "alphanx0lVN1-PRO", # 4 
        "betaNPvx", # 4 
        "alphapW2nx1Vpnx2", # 4 
        "betaNpxnx1Vpl", # 4 
        "alphaW1ItVad1s2", # 4 
        "betavxDN", # 4 
        "alphaW1nx1Vpl", # 3 
        "alphaW0nx0VPnx1", # 3 
        "betaN0nx0Vnx1pl", # 3 
        "betavxARBPs", # 3 
        "betaARBpx1CONJpx2", # 3 
        "betavxDA", # 3 
        "betanx1CONJARBnx2", # 3 
        "alphaW1nx1VPnx2", # 3 
        "betapuVnxpuvx", # 3 
        "betaN1nx0Vnx1s2", # 3 
        "betasARB", # 3 
        "alphaW0nx0Vnx1s2", # 3 
        "betaN1nx1VP", # 3 
        "betapPU", # 3 
        "alphaW0s0Vs1", # 2 
        "betaN0nx0Vnx1pnx2", # 2 
        "betaVvx-arb", # 2 
        "betaNpxnx0VPnx1", # 2 
        "betaPNss", # 2 
        "betaNpxs0Vnx1", # 2 
        "betaN1nx0VPnx1", # 2 
        "betaNpxnx0Vpl", # 2 
        "betaNpx2nx1Vpnx2", # 2 
        "betaDNax", # 2 
        "betaN1nx1Vp", # 2 
        "betaNpxnx2Vnx1", # 2 
        "alphaW0nx0Vpl", # 2 
        "betaNpx2nx0Vnx1pnx2", # 2 
        "betaN1nx1VPnx2", # 2 
        "alphaW0s0Vnx1", # 2 
        "alphaW1nx1VP", # 2 
        "alphaREGnx1VPnx2", # 2 
        "alphaW2nx0Vnx1s2", # 2 
        "alphaREInx1VA2", # 2 
        "betaN2nx1Vpnx2", # 2 
        "betaNpxItVad1s2", # 2 
        "betaNpxnx0Px1", # 1 
        "betaNpxnx0Vnx1pl", # 1 
        "alphaRnx1VA2", # 1 
        "betaN0nx0Vnx1Pnx2", # 1 
        "betaNpxnx0Vplnx1", # 1 
        "betaNpxnx1Vpnx2", # 1 
        "alphaRW0nx0Vnx1Pnx2", # 1 
        "alphaW0nx0Vax1", # 1 
        "betapunxVnx1puvx", # 1 
        "alphaW0nx0Vnx1Pnx2", # 1 
        "betanxARBPnx", # 1 
        "betaNpxnx1Vs2", # 1 
        "alphaW1InvItVnx1s2", # 1 
        "alphaW0nx0N1s1", # 1 
        "betaN0nx0N1s1", # 1 
        "alphaW0nx0Vnx1pnx2", # 1 
        "alphaW0nx0Vnx1pl", # 1 
        "alphanx0APnx1-PRO", # 1 
        "betaspuARBPs", # 1 
        "betaN2nx0Vnx1pnx2", # 1 
        "alphanx0lVnx2N1", # 1 
        "betaNpxnx1VP", # 1 
        "betaNpxnx0Vnx2nx1", # 1 
        "betaENpxnx1V", # 1 
        "betaNPnxs", # 1 
        "betaN1nx0Vnx1Pnx2", # 1 
        "betaNpxItVnx1s2", # 1 
        "alphaDnx0Vpnx1", # 1 
        "alphaW1nx1Vpnx2", # 1 
        "betaNpx2nx1VPnx2", # 1 
        "betaARBnx1CONJnx2", # 1 
        "alphapW1nx0Vpnx1", # 1 
        "betavxPNs", # 1 
        "betaDApx", # 1 
        "betaNpx1nx0VPnx1", # 1 
        "betaN0nx0lVN1", # 1 
        "betaNpxs0NPnx1", # 1 
        "alphaW1nx0VPnx1", # 1 
    ]

    unannotated = [t for t in tree_set if t not in annotated]

    total_count = 0
    for treefile in glob.glob(treedir + '/*.txt'):
        tree_dict = json.load(open(treefile, 'r'))

        if 'deriv' not in tree_dict or tree_dict['deriv'] == 'None':
            continue
        total_count += 1

        deriv_tree_str = tree_dict['deriv']
        deriv_tree = nltk.Tree.fromstring(deriv_tree_str)
        if have_tree_semantics(deriv_tree, annotated):
            count += 1
    #print(unannotated)
    print("number of trees: ", total_count)
    print('Number of trees with semantic parses: ', count)

def window(iterable, size, left_nulls=False):
    """
    Iterates over an iterable size elements at a time
    [1, 2, 3, 4, 5], 3 ->
        [1, 2, 3],
        [2, 3, 4],
        [3, 4, 5]
    """

    # Pad left with None's so that the the first iteration is [None, ..., None, iterable[0]]
    if left_nulls:
        iterable = [None] * (size - 1) + iterable

    iters = tee(iterable, size)
    for i in range(1, size):
        for each in iters[i:]:
            next(each, None)
    return zip(*iters)

def get_lookahead_required(treedir='parse_trees'):
    adds_sem = set([
        "alphaNXN", # 31171,
        "betaDnx", # 9319,
        "betaNn", # 8469,
        #"betasPU", # 7642,
        "betaVvx", # 5844,
        "betaAn", # 4869,
        "betanxPnx", # 2981,
        "betavxPnx", # 2642,
        "alphanx0Vnx1", # 2577,
        "alphanx1V", # 1715,
        #"betasPUs", # 1538,
        #"betanxPUnx", # 1431,
        #"betaARBvx", # 1233,
        "betanx0Vs1", # 1225,
        "alphanx0BEnx1", # 931,
        "alphanx0Pnx1", # 879,
        "betaVs", # 837,
        "betanxGnx", # 803,
        "alphanx0Vnx1-PRO", # 794,
        #"betanxPUs", # 666,
        "alphanx0Ax1", # 621,
        "alphanx0N1", # 619,
        #"betaCOMPs", # 607,
        "alphanx0V", # 345,
        #"betanx1CONJnx2", # 656,
        "betavxPs", # 614,
        #"betas1CONJs2", # 584,
        "betaN1nx1V", # 576,
        "alphanx1V-PRO", # 522,
        "alphas0N1", # 489,
        "alphaN1s0", # 478,
        #"alphaPu", # 378,
        "alphanx2Vnx1", # 303,
        "alphas0Ax1", # 295
        #"betaPUs", # 2628 
        #"betaARBs", # 2015 
        #"betaCONJs", # 1966 
        "alphanx0Pnx1-PRO", # 1483 
        "alphanx0N1", # 1380 
        #"betasPU", # 1371 
        "betanx0Vs1-PRO", # 1318 
        #"betaPss", # 1228 
        "betaN1nx1V", # 1217 
        "alphanx0Ax1", # 1171 
        "alphanx0Ax1-PRO", # 1166 
        "alphas0Ax1", # 1041 
        "alphanx0V", # 987 
        "alphaD", # 941 
        "alphaXGnx0Vs1", # 937 
        #"alphaPu", # 889 
        "alphanx0V-PRO", # 848 
        "betaN0nx0Vnx1", # 720 
        "alphaPXPnx", # 687 
        #"betaARBa", # 687 
        #"betan1CONJn2", # 678 
        "alphaN", # 678 
        "alphaDnx0V", # 630 
        "alphanx2Vnx1", # 609 
        #"betasPUnx", # 607 
        "betaXnx0Vs1", # 568 
        "betavxN", # 528 
        "betaN0nx0N1", # 502 
        "alphaNXNs", # 498 
        "betaNPnx1nx0Pnx1", # 496 
        "betaN0nx0Pnx1", # 482 
        "betaNEGvx", # 461 
        "alphanx2Vnx1-PRO", # 405 
        "alphaNXnxG", # 402 
        "alphaXW0nx0Vs1", # 384 
        "alphaDnx0Vs1", # 340 
        #"betaVergativen", # 325 
        "alphapW1nx0Pnx1", # 313 
        "betaN1nx0Pnx1", # 301 
        "betaXNc0nx0Vs1", # 295 
        "betanxN", # 293 
        "betaN0nx0Ax1", # 290 
        "betaN0nx0V", # 286 
        "alphaP", # 260 
        "betaCnxPnx", # 239 
        "alphaW1nx1V", # 234 
        "betaARBnx", # 233 
        "betaENc1nx1V", # 232 
        "betanx1Vs2-PRO", # 205 
        "betanx1Vs2", # 203 
        #"betaspuPs", # 203 
        "betaNpxs0N1", # 193 
        "betaN1nx0Vnx1", # 188 
        "betaNs", # 186 
        #"betaa1CONJa2", # 180 
        "alphaA", # 176 
        "betaARBpx", # 175 
        "alphaW0nx0Vnx1", # 172 
        "betaXInx0Vs1", # 163 
        "betaARBd", # 154 
        #"betaXNcnx0Vs1", # 150 
        "betaNpxnx1V", # 141 
        "alphas0Vnx1", # 136 
        "alphanx0Vplnx1", # 135 
        "betaN0nx0Vs1", # 134 
        "betanxARB", # 131 
        "alphanx0Vnx2nx1", # 130 
        "alphanx1Vpl", # 117 
        "betaNvx", # 113 
        "alphaEW1nx1V", # 112 
        "betaARBarb", # 109 
        "alphanx0Vplnx1-PRO", # 106 
        #"betaspuPnx", # 105 
        "alphanx1Vpl-PRO", # 104 
        #"betaCARBarb", # 101 
        #"betaspuARB", # 100 
        "alphaW1nx0Vnx1", # 99 
        #"betaspunxV", # 99 
        "betanx0Vnx1s2", # 94 
        "betaXnx0Vs1-PRO", # 89 
        "betanxPs", # 86 
        "betaCARBa", # 84 
        "betaspuVnx", # 80 
        "alphaAXA", # 80 
        "alphaW0nx0Vs1", # 78 
        "betaVvx-adj", # 76 
        "alphaDnxG", # 73 
        "alphaW0s0N1", # 73 
        "alphaW0nx0Pnx1", # 72 
        "alphaW0nx0V", # 70 
        "alphas0Vs1", # 68 
        "alphanx0Px1-PRO", # 67 
        "betaNpxnx0Vnx1", # 66 
        "betanx0Vnx1s2-PRO", # 64 
        "alphanx0Px1", # 64 
        "betanxVpus", # 63 
        #"betavPU", # 60 
        "alphanx0VPnx1", # 56 
        "betapx1CONJpx2", # 56 
        "alphanx1Vpnx2-PRO", # 53 
        #"betaaxPnx", # 52 
        #"betapuARBpuvx", # 52 
        "betanxARBs", # 52 
        "alphaW1nx0Pnx1", # 52 
        "betaNpxs0Ax1", # 52 
        "alphanx0Vnx2nx1-PRO", # 51 
        "betanxP", # 51 
        "alphanx0Vnx1pnx2", # 48 
        "betavxnxARB", # 48 
        "alphanx0Vax1", # 46 
        "alphanx1Vpnx2", # 45 
        "alphaEnx1V", # 45 
        "alphaW1nx0N1", # 44 
        "betadD", # 43 
        "betaNEGa", # 42 
        "betaNpxnx0Pnx1", # 41 
        "alphaAd", # 40 
        "betaNpxnx0V", # 39 
        "alphanx0VPnx1-PRO", # 37 
        "alphaEnx1V-PRO", # 37 
        "alphaW0nx0N1", # 37 
        "betaN1nx2Vnx1", # 36 
        "betaNpxnx0Ax1", # 35 
        #"betaENcnx1V", # 35 
        "alphaW0nx0Ax1", # 35 
        "betaN2nx2Vnx1", # 34 
        "alphanx0Vpl-PRO", # 31 
        "alphanx0N1s1-PRO", # 31 
        "alphaW1nx0Vs1", # 30 
        "alphanx1VPnx2", # 30 
        "alphanx0N1s1", # 29 
        "alphaAV", # 29 
        "alphaW0s0Ax1", # 27 
        "alphanx0Vpl", # 27 
        "alphanx1VP-PRO", # 26 
        "alphaW1nx2Vnx1", # 26 
        #"betanxPUa", # 25 
        "betaN0nx0Px1", # 25 
        "betaNpxnx0N1", # 24 
        "alphaW2nx0Vnx2nx1", # 24 
        #"betapuPpuvx", # 23 
        "betaN1nx1Vpl", # 22 
        "betavxP", # 22 
        "alphanx0Vnx1pl", # 22 
        "betaN0nx0Vplnx1", # 22 
        "betaN2nx0Vnx2nx1", # 21 
        "alphanx0Vnx1pl-PRO", # 21 
        #"betaarb1CONJarb2", # 20 
        "alphanx1VPnx2-PRO", # 20 
        "alphanx1VP", # 20 
        "alphanx0Vax1-PRO", # 20 
        "alphaPW1nx0Px1", # 19 
        "alphaDEnx1V", # 18 
        "betanxnxARB", # 18 
        #"betapunxVpuvx", # 18 
        "betaXN0nx0Vs1", # 18 
        #"betaARBPss", # 17 
        "alphanx0lVN1", # 17 
        "alphaW2nx2Vnx1", # 16 
        "alphanx0Vnx1pnx2-PRO", # 16 
        "alphaW1nx0Vnx2nx1", # 15 
        "betaNpxnx0Vs1", # 14 
        "betaARBarbs", # 14 
        #"betap1CONJp2", # 14 
        "betaN1nx1Vpnx2", # 14 
        "alphanx0Vnx1Pnx2", # 13 
        "betapunxVnx1pus", # 12 
        #"betad1CONJd2", # 12 
        "alphapW1ItVpnx1s2", # 12 
        #"betaax1CONJax2", # 11 
        "betavxARBPnx", # 11 
        "betaN1nx1Vs2", # 11 
        "betasnxARB", # 10 
        "alphanx0Vpnx1", # 10 
        "betaEN1nx1V", # 10 
        #"betaspunxVnx1", # 10 
        "alphanx0Vpnx1-PRO", # 9 
        "betanARB", # 9 
        "betaN0nx0Vnx2nx1", # 9 
        "betaN0nx0Vpl", # 9 
        "betaaARB", # 9 
        "alphaW0nx0Vplnx1", # 8 
        "betaN0nx0Vax1", # 8 
        "alphaDnx0VPnx1", # 7 
        "alphanx0Vnx1Pnx2-PRO", # 7 
        "betaN1nx0Vnx2nx1", # 7 
        "betaARBPnxs", # 7 
        "alphapW2nx0Vnx1pnx2", # 7 
        "alphaW1ItVnx1s2", # 7 
        "alphanx1Vp-PRO", # 7 
        "alphaAXAs", # 6 
        "alphaW0nx0Px1", # 6 
        "alphaDnx0Vpl", # 6 
        "betaN1nx0Vplnx1", # 6 
        "betavpunxVpu", # 6 
        "alphaPXP", # 5 
        "betaN0nx0VPnx1", # 5 
        "betavpuVnxpu", # 5 
        "alphanx1Vp", # 5 
        "betaN0nx0Vnx1s2", # 5 
        "alphaW0nx0Vnx2nx1", # 5 
        "betaDAax", # 4 
        "alphaW1nx0Vnx1s2", # 4 
        "betaXNpxnx0Vs1", # 4 
        "alphaW1nx1Vs2", # 4 
        "alphaW1nx0Vplnx1", # 4 
        "alphanx0lVN1-PRO", # 4 
        "betaNPvx", # 4 
        "alphapW2nx1Vpnx2", # 4 
        "betaNpxnx1Vpl", # 4 
        "alphaW1ItVad1s2", # 4 
        "betavxDN", # 4 
        "alphaW1nx1Vpl", # 3 
        "alphaW0nx0VPnx1", # 3 
        "betaN0nx0Vnx1pl", # 3 
        "betavxARBPs", # 3 
        "betaARBpx1CONJpx2", # 3 
        "betavxDA", # 3 
        #"betanx1CONJARBnx2", # 3 
        "alphaW1nx1VPnx2", # 3 
        #"betapuVnxpuvx", # 3 
        "betaN1nx0Vnx1s2", # 3 
        "betasARB", # 3 
        "alphaW0nx0Vnx1s2", # 3 
        "betaN1nx1VP", # 3 
        #"betapPU", # 3 
        "alphaW0s0Vs1", # 2 
        "betaN0nx0Vnx1pnx2", # 2 
        "betaVvx-arb", # 2 
        "betaNpxnx0VPnx1", # 2 
        "betaPNss", # 2 
        "betaNpxs0Vnx1", # 2 
        "betaN1nx0VPnx1", # 2 
        "betaNpxnx0Vpl", # 2 
        "betaNpx2nx1Vpnx2", # 2 
        "betaDNax", # 2 
        "betaN1nx1Vp", # 2 
        "betaNpxnx2Vnx1", # 2 
        "alphaW0nx0Vpl", # 2 
        "betaNpx2nx0Vnx1pnx2", # 2 
        "betaN1nx1VPnx2", # 2 
        "alphaW0s0Vnx1", # 2 
        "alphaW1nx1VP", # 2 
        "alphaREGnx1VPnx2", # 2 
        "alphaW2nx0Vnx1s2", # 2 
        "alphaREInx1VA2", # 2 
        "betaN2nx1Vpnx2", # 2 
        "betaNpxItVad1s2", # 2 
        "betaNpxnx0Px1", # 1 
        "betaNpxnx0Vnx1pl", # 1 
        "alphaRnx1VA2", # 1 
        "betaN0nx0Vnx1Pnx2", # 1 
        "betaNpxnx0Vplnx1", # 1 
        "betaNpxnx1Vpnx2", # 1 
        "alphaRW0nx0Vnx1Pnx2", # 1 
        "alphaW0nx0Vax1", # 1 
        "betapunxVnx1puvx", # 1 
        "alphaW0nx0Vnx1Pnx2", # 1 
        #"betanxARBPnx", # 1 
        "betaNpxnx1Vs2", # 1 
        "alphaW1InvItVnx1s2", # 1 
        "alphaW0nx0N1s1", # 1 
        "betaN0nx0N1s1", # 1 
        "alphaW0nx0Vnx1pnx2", # 1 
        "alphaW0nx0Vnx1pl", # 1 
        "alphanx0APnx1-PRO", # 1 
        #"betaspuARBPs", # 1 
        "betaN2nx0Vnx1pnx2", # 1 
        "alphanx0lVnx2N1", # 1 
        "betaNpxnx1VP", # 1 
        "betaNpxnx0Vnx2nx1", # 1 
        "betaENpxnx1V", # 1 
        "betaNPnxs", # 1 
        "betaN1nx0Vnx1Pnx2", # 1 
        "betaNpxItVnx1s2", # 1 
        "alphaDnx0Vpnx1", # 1 
        "alphaW1nx1Vpnx2", # 1 
        "betaNpx2nx1VPnx2", # 1 
        #"betaARBnx1CONJnx2", # 1 
        "alphapW1nx0Vpnx1", # 1 
        #"betavxPNs", # 1 
        #"betaDApx", # 1 
        "betaNpx1nx0VPnx1", # 1 
        "betaN0nx0lVN1", # 1 
        "betaNpxs0NPnx1", # 1 
        "alphaW1nx0VPnx1", # 1 
    ])

    complexities = []
    for treefile in glob.glob(treedir + '/*.txt'):
        tree_dict = json.load(open(treefile, 'r'))

        if 'deriv' not in tree_dict or tree_dict['deriv'] == 'None':
            continue

        deriv_tree_str = tree_dict['deriv']
        deriv_tree = nltk.Tree.fromstring(deriv_tree_str)
        parse_tree_str = tree_dict['parse']
        parse_tree = nltk.Tree.fromstring(parse_tree_str)

        complexity = deriv_to_no_sem_depth(adds_sem, deriv_tree)
        complexities.append(complexity)
    import matplotlib.pyplot as plt
    plt.hist(complexities, 8, normed=1, facecolor='w', edgecolor='k', alpha=0.75)
    plt.xlabel('Depth', fontsize=32)
    plt.show()
    with open('complexities.txt', 'w') as f:
        f.write(str(complexities))
    #plt.savefig('xtag/exp_results/depth_required_hist.pdf', bbox_inches='tight')

def deriv_to_no_sem_depth(adds_sem, deriv, running_depth=0):
    label = deriv.label() if isinstance(deriv, nltk.Tree) else deriv
    tree_name, word, loc = label_to_tree_word_loc(label)
    if tree_name not in adds_sem:
        new_running = running_depth + 1
    else:
        new_running = 0
    return max([new_running] + [deriv_to_no_sem_depth(adds_sem, c, new_running) for c in deriv if isinstance(deriv, nltk.Tree)])

def count_words(treedir='parse_trees', output_file='word_corr.txt'):
    counts = defaultdict(lambda: defaultdict(lambda: defaultdict(int)))

    for treefile in glob.glob(treedir + '/*.txt'):
        tree_dict = json.load(open(treefile, 'r'))

        if 'deriv' not in tree_dict or tree_dict['deriv'] == 'None':
            continue

        parse_tree_str = tree_dict['parse']
        parse_tree = nltk.Tree.fromstring(parse_tree_str)
        sent = [w for w in parse_tree.leaves() if 'epsilon' not in w and 'PRO' not in w]
        for word1, word2, word3, word4, word5 in window(sent, 5):
            counts[-2][word3][word1] += 1
            counts[-1][word3][word2] += 1
            counts[1][word3][word4] += 1
            counts[2][word3][word5] += 1
    counts = dict(counts)

    for index, outer_dict in counts.items():
        for word, inner_dict in outer_dict.items():
            counts[index][word] = dict(inner_dict)
        counts[index] = dict(outer_dict)

    with open(output_file, 'w') as f:
        f.write(str(counts))

def count_errors(treedir='parse_trees'):
    types = []
    for treefile in glob.glob(treedir + '/*.txt'):
        tree_dict = json.load(open(treefile, 'r'))
        if 'deriv' in tree_dict:
            continue
        types.append(tree_dict['type'])
    from collections import Counter
    c = Counter(types)
    print(c)

if __name__ == '__main__':
    #count_words()
    #count_corpus()
    #investigate_counts()
    count_semantic_parses()
    #parse_deriv_semantics()
    #count_successful_parses()
    #get_lookahead_required()
    #count_errors()
